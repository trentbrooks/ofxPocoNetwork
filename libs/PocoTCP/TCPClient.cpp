
#include "TCPClient.h"


// FIXED: socketStream wasnt being deleted in disconnect
// DEBUG: after 2 hours of failed connections - getitng a I/O error: Too many open files
// https://stackoverflow.com/questions/4083608/on-ios-iphone-too-many-open-files-need-to-list-open-files-like-lsof
/*#import <sys/types.h>
#import <fcntl.h>
#import <errno.h>
#import <sys/param.h>

void lsof() {
    int flags;
    int fd;
    char buf[MAXPATHLEN+1] ;
    int n = 1 ;
    
    for (fd = 0; fd < (int) FD_SETSIZE; fd++) {
        errno = 0;
        flags = fcntl(fd, F_GETFD, 0);
        if (flags == -1 && errno) {
            if (errno != EBADF) {
                return ;
            }
            else
                continue;
        }
        fcntl(fd , F_GETPATH, buf ) ;
        //NSLog( @"File Descriptor %d number %d in use for: %s",fd,n , buf ) ;
        ofLog() << "File Descriptor " << fd << " number " << n << " for: " << buf;
        ++n ;
    }
}*/

namespace ofxPocoNetwork {
    

TCPClient::TCPClient() {
    fixedReceiveSize = DEFAULT_FIXED_SIZE;
    connected = false;
    //setConnectTimeout(10);
    //setReceiveTimeout(10);
    //setSendTimeout(1);
    //setPollTimeout(TCPPOCO_POLL_TIME);
    socketAddress = NULL;
    socketStream = NULL;
    socketHandler = NULL;
}

TCPClient::~TCPClient() {
    
    ofLog() << "~TCPClient deleted";
    connected = false;
    if(socketHandler) {
        socketHandler->disconnect();
    }
    if(thread.isRunning()) {
        thread.join();
    }
    
    if(socketAddress) delete socketAddress;
    if(socketStream) delete socketStream;
    if(connectAdaptor) delete connectAdaptor;
    
}


void TCPClient::connect(string ipAddr, int port, MessageFraming protocol) {
    
    isAsync = false;
    
    // setup tcp poco client    
    socketAddress = new Poco::Net::SocketAddress(ipAddr, port);
    socketStream = new Poco::Net::StreamSocket();
    framingProtocol = protocol;
    
    // this must be connected before creating socketHandler
    try {
        
        socketStream->connect(*socketAddress); // need to thread this bit
        
        // can't create handler until it's connected!
        createSocketHandler();
        
        return;
        
    } catch(Poco::Exception& exc) {
        ofLogError() << "* TCPClient failed to connect to server 1: " << exc.displayText();
    }  catch(std::exception& exc) {
        ofLogError() << "* TCPClient failed to connect to server 2";
    } catch (...) {
        ofLogError() << "* TCPClient failed to connect to server 3";
    }
    
    // clean up on fail
    disconnect();
    if(thread.isRunning()) {
        thread.join();
    }
    
}
    
void TCPClient::createSocketHandler() {
    
    if(!socketStream) {
        ofLogError() << "Socket stream has not been created";
        return;
    }
    
    try {
        // can't create handler until it's connected!
        socketHandler = new TCPClientConnectionHandler(socketStream, framingProtocol);
        ofAddListener(socketHandler->disconnectionEvent, this, &TCPClient::onClientRemoved);
        
        //ofLog() << "thread running: " << thread.isRunning();
        if(thread.isRunning()) thread.join();
        thread.start(*socketHandler);
        
        connected = true;
        return;
        
    } catch(Poco::Exception& exc) {
        ofLogError() << "* TCPClient createSocketHandler failed to connect to server 1: " << exc.displayText();
    }  catch(std::exception& exc) {
        ofLogError() << "* TCPClient createSocketHandler failed to connect to server 2";
    } catch (...) {
        ofLogError() << "* TCPClient createSocketHandler failed to connect to server 3";
    }
    
    
}

void TCPClient::connectAsync(string ipAddr, int port, MessageFraming protocol) {
    
    isAsync = true;
    asyncSuccess = asyncComplete = asyncUpdateComplete = false;
    ofLog() << "TCPClient connecting async... " << ipAddr;
    
    // setup tcp poco client
    socketAddress = new Poco::Net::SocketAddress(ipAddr, port);
    socketStream = new Poco::Net::StreamSocket();
    framingProtocol = protocol;
    
    // setting up a thread / runable adaptor so wait until connected
    connectAdaptor = new Poco::RunnableAdapter<TCPClient>(*this, &TCPClient::connectAsyncImpl);
    ofAddListener(ofEvents().update, this, &TCPClient::updateAsync); // need to update from main loop to kill process
    if(thread.isRunning()) thread.join();
    thread.start(*connectAdaptor); // hijacking the thread for connect first
    
}
    
void TCPClient::connectAsyncImpl() {
    
    // this must be connected before creating socketHandler
    try {
        
        //ofSleepMillis(5000); // debug
        socketStream->connect(*socketAddress); // ok this is threaded now
        asyncSuccess = true;
        
    } catch(Poco::Exception& exc) {
        ofLogError() << "* TCPClient connectAsyncImpl failed to connect to server 1: " << exc.displayText();
    }  catch(std::exception& exc) {
        ofLogError() << "* TCPClient connectAsyncImpl failed to connect to server 2";
    } catch (...) {
        ofLogError() << "* TCPClient connectAsyncImpl failed to connect to server 3";
    }
    
    // retry connect on fail? NO - this should be controlled externally
    /*if(!asyncSuccess) {
        
        // make sure at least 10 seconds has passed before trying again
        float elapsed = (ofGetElapsedTimef()-startTime);
        if(elapsed < retryConnectAfterSeconds) {
            ofSleepMillis(retryConnectAfterSeconds * 1000);
            connectAsyncImpl();
        }
        return;
    }*/
    
    // clean up on fail - this needs to be done in update if failed
    asyncComplete=true;
    

}
    
void TCPClient::updateAsync(ofEventArgs & args) {
    
    if(asyncComplete) {
        
        // stop updates
        ofRemoveListener(ofEvents().update, this, &TCPClient::updateAsync);
        
        if(asyncSuccess) {
            
            // kill the connection thread and reuse for the socket messaging
            thread.join(); // kill thread
            createSocketHandler();
            ofLog() << "Async connection SUCCESS: " << connected;
        } else {
            // failed to connect - this deletes the socket objects
            ofLog() << "Async connection FAILED: " << asyncSuccess;
            disconnect();
            if(thread.isRunning()) {
                thread.join();
            }

            // if want to auto restart, need to call connectAsync() again
        }
        
        // need to set a flag here for main thread
        asyncUpdateComplete = true;
    }
}
    
bool TCPClient::isConnected() {
    return connected;
}

bool TCPClient::isAsyncComplete() {
    return asyncUpdateComplete;
}
    


void TCPClient::disconnect() {
    
    //onClientRemoved(this);
    
    /*if(socketStream) {
        
        // http://stackoverflow.com/questions/3757289/tcp-option-so-linger-zero-when-its-required
        // if don't set this, the server sends a few success messages even if this is closed
        try {
            socketStream->setLinger(true, 0);
        } catch (Poco::Exception& exc) {
            // server must be closed or the above crashes on exit
            ofLogError() << "* ~TCPClient failed to setLinger (server must be closed?): " << exc.displayText();
        }
        
        // these all do the same thing - same as delete so not needed
        //socketStream->setKeepAlive(false);
        //socketStream->shutdown();
        //socketStream->close();
    }*/
    
    if(socketHandler) {
        try {
            ofRemoveListener(socketHandler->disconnectionEvent, this, &TCPClient::onClientRemoved);
        } catch (Poco::Exception& exc) {
            ofLogError() << "* Failed to remove listener 1";
        } catch(std::exception & e){
            ofLogError() << "* Failed to remove listener 2";
        } catch (...) {
            ofLogError() << "* Failed to remove listener 3";
        }
        delete socketHandler;
        socketHandler = NULL;
    }
    if(socketAddress) {
        delete socketAddress;
        socketAddress = NULL;
    }
    if(connectAdaptor) {
        delete connectAdaptor;
        connectAdaptor = NULL;
    }
    if(socketStream) {
        
        // on ios this was causing an error after 2 hours of failed connections- I/O error: Too many open files
        delete socketStream;
        socketStream = NULL;
    }
    
    connected = false;
    ofLog() << "disconnect";
    
    // which files are open?
    //lsof();
    
}

    
//--------------------------------------------------------------
void TCPClient::onClientRemoved(const void* socket) {
    
    disconnect();
}

//--------------------------------------------------------------
void TCPClient::setConnectTimeout(int timeoutInSeconds) {
    connectTimeout.assign(timeoutInSeconds, 0);
    if(connected) {
        ofLog() << "TCPClient connect timeout must be called before setup";
    }
}

/*
void TCPClient::setSendTimeout(int timeoutInSeconds) {
    sendTimeout.assign(timeoutInSeconds, 0);
    if(connected) {
        socketStream->setSendTimeout(sendTimeout);
    }
}

void TCPClient::setPollTimeout(int timeoutInSeconds) {
    
    pollTimeout.assign(timeoutInSeconds, 0);
}*/
    
//--------------------------------------------------------------
void TCPClient::setFixedReceiveSize(int s) {
    fixedReceiveSize = s;
    if(socketHandler) {
        socketHandler->setFixedReceiveSize(fixedReceiveSize);
    }
    
}
    
    
// receiving - gets messages from client queue (non blocking)
//--------------------------------------------------------------
bool TCPClient::hasWaitingMessages() {
    if(!connected) return false;
    return socketHandler->hasWaitingMessages();
}

bool TCPClient::getNextMessage(string& msg) {
    if(!connected) return false;
    ofBuffer buffer;
    socketHandler->getNextMessage(buffer);
    msg = buffer.getData();
    return true;
}

bool TCPClient::getNextMessage(ofBuffer& msg) {
    if(!connected) return false;
    socketHandler->getNextMessage(msg);
    return true;
}


// sending - adds messages to client queue (non blocking)
//--------------------------------------------------------------
void TCPClient::sendMessage(string msg) {
    if(!connected) return;
    ofBuffer buffer(msg);
    socketHandler->sendMessage(buffer);
}


void TCPClient::sendMessage(ofBuffer& buffer) {
    if(!connected) return;
    socketHandler->sendMessage(buffer);
}

void TCPClient::sendMessage(ofBuffer&& buffer) {
    if(!connected) return;
    socketHandler->sendMessage(buffer);
}
    
} // namespace ofxPocoNetwork
