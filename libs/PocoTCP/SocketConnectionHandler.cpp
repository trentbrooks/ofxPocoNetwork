#include "SocketConnectionHandler.h"

namespace ofxPocoNetwork {
    
SocketConnectionHandler::SocketConnectionHandler() {
    
    fixedReceiveSize = DEFAULT_FIXED_SIZE; // 256,1024;
    delimiter = DEFAULT_DELIMITER;
    
    isHeaderComplete = false;
    nextMessageSize = 0;
}

void SocketConnectionHandler::setup(StreamSocket* socket, MessageFraming protocol) {
    
    socketPtr = socket;
    messageFraming = protocol;
    
    //Poco::Timespan time(2,0);
    //socketPtr.setSendTimeout(time);
    
    // meh, this solved all my server crashing on client disconnect problems! thank you and fuck you poco
    // https://github.com/pocoproject/poco/issues/235
    #if defined(TARGET_OSX) || defined(TARGET_IOS)
    socketPtr->setOption(SOL_SOCKET, SO_NOSIGPIPE, 1);
    #endif
    
    // apparantly 'osx really needs this'?
    socketPtr->setNoDelay(true);
    
    ofLog() << "SocketConnectionHandler setup";
}


SocketConnectionHandler::~SocketConnectionHandler() {
    
    //queueMutex.lock();
    //socketPtr.shutdown();
    //while(!receiveBuffers.empty()) receiveBuffers.pop();
    //while(!sendBuffers.empty()) sendBuffers.pop();
    //queueMutex.unlock();
    
    ofLog() << "~SocketConnectionHandler deleted";
    if(socketPtr) {
        socketPtr->close();
        socketPtr = NULL;
    }
    
    
}


void SocketConnectionHandler::disconnect() {
    
    //queueMutex.lock();
    ofLog() << "SocketConnectionHandler disconnecting...";
    while(!receiveBuffers.empty()) receiveBuffers.pop();
    while(!sendBuffers.empty()) sendBuffers.pop();
    //socketPtr->shutdown(); // no for reactor
    //socketPtr->setKeepAlive(false);a
    ofNotifyEvent(disconnectionEvent, this);
    //queueMutex.unlock();
    //delete this; // problems on client, must be deleted from server or client
}



void SocketConnectionHandler::sendMessage(ofBuffer& message) {
    ScopedLock<ofMutex> lock(queueMutex);
    sendBuffers.push(message);
}

bool SocketConnectionHandler::getNextMessage(ofBuffer& message) {
    ScopedLock<ofMutex> lock(queueMutex);
    if(receiveBuffers.size()) {
        message = receiveBuffers.front();
        receiveBuffers.pop();
        return true;
    }
    return false;
}

bool SocketConnectionHandler::hasWaitingMessages() {
    ScopedLock<ofMutex> lock(queueMutex);
    return receiveBuffers.size() > 0;
}


//--------------------------------------------------------------
void SocketConnectionHandler::setFixedReceiveSize(int s) {
    ScopedLock<ofMutex> lock(queueMutex);
    fixedReceiveSize = s;
}

void SocketConnectionHandler::setDelimiter(char d) {
    ScopedLock<ofMutex> lock(queueMutex);
    delimiter = d;
}


//--------------------------------------------------------------
void SocketConnectionHandler::processRead() {
    // ScopedLock<ofMutex> lock(mutex);
    try {
                
        if(socketPtr) {
            if(messageFraming == FRAME_HEADER_AND_MESSAGE) {
                readHeaderAndMessage();
            } else if(messageFraming == FRAME_DELIMITED) {
                readDelimitedMessage();
            } else if(messageFraming == FRAME_FIXED_SIZE) {
                readFixedSizeMessage();
            } else {
                readAvailableBytes();
            }
        }
        
    } catch (Poco::Exception &e) {
        ofLogError() << "* Read fail 2: disconnecting";
        disconnect();
    } catch(std::exception& exc) {
        ofLogError() << "* Read fail 4: disconnecting";
        disconnect();
    } catch (...) {
        ofLogError() << "* Read fail 4: disconnecting";
        disconnect();
    }
}

void SocketConnectionHandler::processWrite() {
    // ScopedLock<ofMutex> lock(mutex);
    try {
        
        if(socketPtr) {
            if(messageFraming == FRAME_HEADER_AND_MESSAGE) {
                writeHeaderAndMessage();
            } else if(messageFraming == FRAME_DELIMITED) {
                writeDelimitedMessage();
            } else if(messageFraming == FRAME_FIXED_SIZE) {
                writeFixedSizeMessage();
            } else {
                writeAvailableBytes();
            }
        }
        
        
    } catch (Poco::Exception &e) {
        ofLogError() << "* Send fail 2: disconnecting";
        disconnect();
    } catch(std::exception& exc) {
        ofLogError() << "* Read fail 4: disconnecting";
        disconnect();
    } catch (...) {
        ofLogError() << "* Send fail 3: disconnecting";
        disconnect();
    }
}

/*
 Custom message protocol / framing
 - these methods need to be called from super class (eg. via a thread or run)
 */
//--------------------------------------------------------------
// read 4 byte header for main message size
// then read main message
// not sure what happens if sender sends more than we are reading?
void SocketConnectionHandler::readHeaderAndMessage() {
    
    int availableBytes = socketPtr->available();
    int taken = 0;
    if(!isHeaderComplete) {
        if(availableBytes >= 4) {
            ofBuffer header;
            header.allocate(4);
            int n = socketPtr->receiveBytes(header.getBinaryBuffer(), 4);
            if (n > 0) {
                nextMessageSize = *(int *)header.getBinaryBuffer();
                isHeaderComplete = true;
                taken+= n;
            } else {
                ofLogError() << "* Read fail 1a (header): disconnecting";
                disconnect();
            }
        }
    }
    
    if(isHeaderComplete) {
        if(availableBytes >= nextMessageSize) {
            ofBuffer buffer;
            buffer.allocate(nextMessageSize+1);
            int n = socketPtr->receiveBytes(buffer.getBinaryBuffer(), buffer.size());
            if (n > 0) {
                taken += n;
                queueMutex.lock();
                receiveBuffers.push(buffer);
                queueMutex.unlock();
                isHeaderComplete = false;
            } else {
                ofLogError() << "* Read fail 1b (main message): disconnecting";
                disconnect();
            }
        }
    }
    
    // if still more bytes - run same function recursively?
    if(availableBytes > taken) {
        readHeaderAndMessage();
    }

}

// custom message protocol / framing
// read bytes available, split messages by delimiter
void SocketConnectionHandler::readDelimitedMessage() {
    
    int availableBytes = socketPtr->available();
    ofBuffer buffer;
    buffer.allocate(availableBytes);
    int n = socketPtr->receiveBytes(buffer.getBinaryBuffer(), availableBytes);
    if (n > 0) {
        
        // put bytes into a string or char vector
        copy(buffer.getBinaryBuffer(), buffer.getBinaryBuffer()+n, back_inserter(delimBuffers));
        //delimBuffer.append(buffer.getData(), n);
        
        // split by delimiter / parse
        vector<char>::iterator findIter = find(delimBuffers.begin(), delimBuffers.end(), delimiter);
        while (findIter != delimBuffers.end()) {
            
            //std::string str((char*)&buffer[0], data_size);
            int findIndex = findIter - delimBuffers.begin();
            ofBuffer message(&delimBuffers[0], findIndex);
            queueMutex.lock();
            receiveBuffers.push(message);
            queueMutex.unlock();
            
            // flush from vecotr
            delimBuffers.erase(delimBuffers.begin(), delimBuffers.begin()+findIndex+1);
            
            // check again for delim
            findIter = find(delimBuffers.begin(), delimBuffers.end(), delimiter);
        }

    } else {
        ofLogError() << "* Read fail 1 (none): disconnecting";
        disconnect();
    }
}

// NO message protocol / framing
// just reads whatever's available and stores as ofBuffer's
void SocketConnectionHandler::readAvailableBytes() {
    int availableBytes = socketPtr->available();
    ofBuffer buffer;
    buffer.allocate(availableBytes);
    int n = socketPtr->receiveBytes(buffer.getBinaryBuffer(), availableBytes);
    if (n > 0) {
        queueMutex.lock();
        receiveBuffers.push(buffer);
        queueMutex.unlock();
    } else {
        ofLogError() << "* Read fail 1 (none): disconnecting";
        disconnect();
    }
}

void SocketConnectionHandler::readFixedSizeMessage() {
    
    int availableBytes = socketPtr->available();
    int taken = 0;
    if(availableBytes >= fixedReceiveSize) {
        ofBuffer buffer;
        buffer.allocate(fixedReceiveSize+1);
        int n = socketPtr->receiveBytes(buffer.getBinaryBuffer(), buffer.size());
        if (n > 0) {
            taken += n;
            queueMutex.lock();
            receiveBuffers.push(buffer);
            queueMutex.unlock();
            isHeaderComplete = false;
            
            // if still more bytes - run same function recursively?
            if(availableBytes > taken) {
                readFixedSizeMessage();
            }
            
        } else {
            ofLogError() << "* Read fail 1 (fixed size message): disconnecting";
            disconnect();
        }
    }
}


// sends a 4 byte header followed by a message
// sends all messages in queue
// is blocking, so each sendBytes should send all bytes (see StreamSocketImpl::sendBytes)
void SocketConnectionHandler::writeHeaderAndMessage() {
    
    queueMutex.lock();
    bool hasMessagesToSend = sendBuffers.size() > 0;
    if(!hasMessagesToSend) {
        queueMutex.unlock();
        return;
    }
    ofBuffer buffer = sendBuffers.front();
    queueMutex.unlock();
    
    while (hasMessagesToSend) {
        // 1. first send a 4 byte header with the size of next buffer
        int bufferSize = buffer.size();
        char* headerData = (char*)(&bufferSize);
        int nBytesHeader = socketPtr->sendBytes(headerData, HEADER_BYTES);
        if(nBytesHeader <= 0) {
            ofLogError() << "* Send fail 1a (header): disconnecting";
            disconnect();
            return;
        }
        
        // 2. send main message
        int nBytesMessage = socketPtr->sendBytes(buffer.getBinaryBuffer(), buffer.size());
        if(nBytesMessage <= 0) {
            ofLogError() << "* Send fail 1b (message): disconnecting";
            disconnect();
            return;
        } else {
            
            // message send success
            queueMutex.lock();
            sendBuffers.pop();
            hasMessagesToSend = sendBuffers.size() > 0;
            if(!hasMessagesToSend) {
                queueMutex.unlock();
                return;
            }
            buffer = sendBuffers.front();
            queueMutex.unlock();
        }
    }
    
}

void SocketConnectionHandler::writeDelimitedMessage() {
    
    queueMutex.lock();
    bool hasMessagesToSend = sendBuffers.size() > 0;
    if(!hasMessagesToSend) {
        queueMutex.unlock();
        return;
    }
    ofBuffer buffer = sendBuffers.front();
    queueMutex.unlock();
    
    while (hasMessagesToSend) {
        // append single byte delimiter character
        buffer.append(&delimiter, 1);
        
        // send main message
        int nBytesMessage = socketPtr->sendBytes(buffer.getBinaryBuffer(), buffer.size());
        if(nBytesMessage <= 0) {
            ofLogError() << "* Send fail 1 (none): disconnecting";
            disconnect();
            return;
        } else {
            // message send success
            queueMutex.lock();
            sendBuffers.pop();
            if(!hasMessagesToSend) {
                queueMutex.unlock();
                return;
            }
            buffer = sendBuffers.front();
            queueMutex.unlock();
        }
    }
}

void SocketConnectionHandler::writeAvailableBytes() {
    
    queueMutex.lock();
    bool hasMessagesToSend = sendBuffers.size() > 0;
    if(!hasMessagesToSend) {
        queueMutex.unlock();
        return;
    }
    ofBuffer buffer = sendBuffers.front();
    queueMutex.unlock();
    
    while (hasMessagesToSend) {
    
        // send main message
        int nBytesMessage = socketPtr->sendBytes(buffer.getBinaryBuffer(), buffer.size());
        if(nBytesMessage <= 0) {
            ofLogError() << "* Send fail 1 (none): disconnecting";
            disconnect();
            return;
        } else {
            // message send success
            queueMutex.lock();
            sendBuffers.pop();
            if(!hasMessagesToSend) {
                queueMutex.unlock();
                return;
            }
            buffer = sendBuffers.front();
            queueMutex.unlock();
        }
    }
}

void SocketConnectionHandler::writeFixedSizeMessage() {
    
    // just assumes the buffer size is correct
    writeAvailableBytes();
}
    
} // namespace ofxPocoNetwork
