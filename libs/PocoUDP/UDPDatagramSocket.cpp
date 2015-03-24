
#include "UDPDatagramSocket.h"

namespace ofxPocoNetwork {
    
    
    UDPDatagramSocket::UDPDatagramSocket() {
        receiveSize = 256;
        sleepTime = 16;
        port = sourcePort = 0;
        connected = false;
        isServer = false;
        destinationAddress = NULL;
        sourceAddress = NULL;
        socket = NULL;
    }
    
    UDPDatagramSocket::~UDPDatagramSocket() {
        
        disconnect();
    }
    
    void UDPDatagramSocket::disconnect() {
        waitForThread();
        connected = false;
        if(destinationAddress) delete destinationAddress;
        if(sourceAddress) delete sourceAddress;
        if(socket) delete socket;
        ofLog() << "Disconnected UDPDatagramSocket.";
    }
    
    
    void UDPDatagramSocket::bind(int port, bool reuseAddress){
        
        if(connected) disconnect();
        
        // setup udp poco server (localhost)
        destinationAddress = new Poco::Net::SocketAddress(Poco::Net::IPAddress(), port);
        //socket = new Poco::Net::DatagramSocket(*destinationAddress, reuseAddress); // automatically binds to socket
        //socket = new Poco::Net::DatagramSocket(destinationAddress->family()); // not binded
        socket = new Poco::Net::DatagramSocket(Poco::Net::IPAddress::IPv4); // not binded
        socket->bind(*destinationAddress, reuseAddress);
        
        isServer = true;
        connected = true;
        this->port = port;
        ofLog() << "Bind to: " << destinationAddress->toString();
        ofLog() << "Socket: " << socket->address().toString();
        ofLog() << "Max receive size: " << socket->getReceiveBufferSize();
        ofLog() << "Max send size: " << socket->getSendBufferSize();
        
        
        if(!isThreadRunning()) startThread();
    }
    
    void UDPDatagramSocket::connect(string ipAddr, int port) {
        
        if(connected) disconnect();
        
        // setup tcp poco client
        destinationAddress = new Poco::Net::SocketAddress(ipAddr, port);
        socket = new Poco::Net::DatagramSocket(Poco::Net::IPAddress::IPv4);
        
        isServer = false;
        connected = true;
        this->port = port;
        ofLog() << "Connect: " << destinationAddress->toString();
        ofLog() << "Socket: " << socket->address().toString();
        ofLog() << "Max receive size: " << socket->getReceiveBufferSize();
        ofLog() << "Max send size: " << socket->getSendBufferSize(); // 9216
        /*try {
         
         // client must connect to server
         socket->connect(*destinationAddress);
         connected = true;
         ofLog() << "Connect: " << destinationAddress->toString();
         ofLog() << "Socket: " << socket->address().toString();
         ofLog() << "Max receive size: " << socket->getReceiveBufferSize();
         ofLog() << "Max send size: " << socket->getSendBufferSize(); // 9216
         
         } catch (Poco::Exception& exc) {
         
         disconnect();
         ofLog() << "UDPClient Error: could not create socket- " << exc.displayText();
         }*/
        
        if(!isThreadRunning()) startThread();
    }
    
    void UDPDatagramSocket::connect(string ipAddr, int port, int sourcePort, bool reuseAddress) {
        
        if(connected) disconnect();
        
        // setup tcp poco client
        sourceAddress = new Poco::Net::SocketAddress(Poco::Net::IPAddress(), sourcePort);
        destinationAddress = new Poco::Net::SocketAddress(ipAddr, port);
        socket = new Poco::Net::DatagramSocket(*sourceAddress, reuseAddress);
        
        isServer = false;
        connected = true;
        this->port = port;
        this->sourcePort = sourcePort;
        ofLog() << "Connect: " << destinationAddress->toString();
        ofLog() << "Socket: " << socket->address().toString();
        ofLog() << "Max receive size: " << socket->getReceiveBufferSize();
        ofLog() << "Max send size: " << socket->getSendBufferSize(); // 9216
        
        // removing this part - it now sends to the socketAddress each send instead
        // no need to 'connect'
        /*try {
         
         // client must connect to server
         socket->connect(*socketAddress);
         connected = true;
         ofLog() << "UDPClient connected";
         ofLog() << "Max receive size: " << socket->getReceiveBufferSize();
         ofLog() << "Max send size: " << socket->getSendBufferSize(); // 9216
         ofLog() << "Connect: " << socketAddress->toString();
         ofLog() << "Socket: " << socket->address().toString();
         
         } catch (Poco::Exception& exc) {
         
         disconnect();
         ofLog() << "UDPClient Error: could not create socket- " << exc.displayText();
         }*/
        
        if(!isThreadRunning()) startThread();
    }
    
    
    void UDPDatagramSocket::setBroadcast(bool broadcast) {
        if(connected) {
            socket->setBroadcast(broadcast);
        }
    }
    
    // connections / clients
    // for server only
    int UDPDatagramSocket::getNumClients() {
        return clients.size();
    }
    
    // thread
    //--------------------------------------------------------------
    void UDPDatagramSocket::threadedFunction(){
        
        while( isThreadRunning() ){
            
            if(connected) {
                
                // 1. read from socket
                processRead();
                
                // 2. write to socket
                processWrite();
                
            }
            
            sleep(sleepTime);
        }
        
    }
    
    //--------------------------------------------------------------
    void UDPDatagramSocket::processRead() {
        
        try {
            
            // when enough data available receive the message
            if(socket->available()) {
                
                // receive
                UDPMessageInfo messageInfo;
                //ofBuffer buffer;
                messageInfo.buffer.allocate(receiveSize);
                //Poco::Net::SocketAddress sender; // use this to identify the sender
                int n = socket->receiveFrom(messageInfo.buffer.getBinaryBuffer(), messageInfo.buffer.size(), messageInfo.address);
                if(n <= 0) {
                    ofLogError() << "* Read fail 1 (none): disconnecting";
                    disconnect();
                    return;
                }
                
                // for server only
                // who sent message (sender) ? need to create list if want to send back anything
                if(isServer) {
                    bool clientAdded = false;
                    mutex.lock();
                    for(int i = 0; i < clients.size(); i++) {
                        if(messageInfo.address == clients[i]) {
                            clientAdded = true;
                            break;
                        }
                    }
                    if(!clientAdded) {
                        clients.push_back(messageInfo.address);
                    }
                    mutex.unlock();
                }
                
                
                
                // copy/replace buffer / or push into queue
                mutex.lock();
                receiveBuffers.push(messageInfo);
                mutex.unlock();
            }
            
            
        } catch (Poco::Exception &e) {
            ofLogError() << "* Read fail 2: disconnecting. " << e.message();
            disconnect();
        } catch(std::exception& exc) {
            ofLogError() << "* Read fail 4: disconnecting";
            disconnect();
        } catch (...) {
            ofLogError() << "* Read fail 4: disconnecting";
            disconnect();
        }
    }
    
    void UDPDatagramSocket::processWrite() {
        
        try {
            
            mutex.lock();
            bool hasMessagesToSend = sendBuffers.size() > 0;
            if(!hasMessagesToSend) {
                mutex.unlock();
                return;
            }
            UDPMessageInfo messageInfo = sendBuffers.front();
            //ofBuffer buffer = sendBuffers.front();
            mutex.unlock();
            
            while (hasMessagesToSend) {
                
                // who is this sending to? should store the sender in the send queue
                //int nSent = socket->sendBytes(buffer.getBinaryBuffer(), buffer.size());
                
                // server must send to a connected client, not the destinationAddress (we only know the address if received a message)
                int nSent = socket->sendTo(messageInfo.buffer.getBinaryBuffer(), messageInfo.buffer.size(), messageInfo.address);
                
                // client must send to remote server (destinationAddress)
                //int nSent = socket->sendTo(buffer.getBinaryBuffer(), buffer.size(), *destinationAddress);
                
                if(nSent <= 0) {
                    ofLogError() << "* Send fail 1 (none): disconnecting";
                    disconnect();
                    return;
                } else {
                    
                    mutex.lock();
                    sendBuffers.pop();
                    hasMessagesToSend = sendBuffers.size() > 0;
                    if(hasMessagesToSend) {
                        // still more messages to send...
                        //buffer = sendBuffers.front();
                        messageInfo = sendBuffers.front();
                    }
                    mutex.unlock();
                    
                }
            }
            
            
        } catch (Poco::Exception &e) {
            ofLogError() << "* Send fail 2: disconnecting. " << e.message();
            disconnect();
        } catch(std::exception& exc) {
            ofLogError() << "* Send fail 4: disconnecting";
            disconnect();
        } catch (...) {
            ofLogError() << "* Send fail 3: disconnecting";
            disconnect();
        }
    }
    
    
    
    // receive
    //--------------------------------------------------------------
    void UDPDatagramSocket::setReceiveSize(int size) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        receiveSize = size;
    }
    
    bool UDPDatagramSocket::hasWaitingMessages() {
        Poco::ScopedLock<ofMutex> lock(mutex);
        return receiveBuffers.size() > 0;
        //if(clientId >= clients.size()) return false;
    }
    
    bool UDPDatagramSocket::getNextMessage(ofBuffer& message) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(receiveBuffers.size()) {
            message = receiveBuffers.front().buffer;
            receiveBuffers.pop();
            return true;
        }
        return false;
    }
    
    bool UDPDatagramSocket::getNextMessage(string& msg) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(receiveBuffers.size()) {
            msg = receiveBuffers.front().buffer.getBinaryBuffer();
            receiveBuffers.pop();
            return true;
        }
        return false;
    }
    
    bool UDPDatagramSocket::getNextMessage(ofBuffer& message, Poco::Net::SocketAddress &emptyAddress) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(receiveBuffers.size()) {
            message = receiveBuffers.front().buffer;
            emptyAddress = receiveBuffers.front().address;
            receiveBuffers.pop();
            return true;
        }
        return false;
    }
    
    bool UDPDatagramSocket::getNextMessage(string& msg, Poco::Net::SocketAddress &emptyAddress) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(receiveBuffers.size()) {
            msg = receiveBuffers.front().buffer.getBinaryBuffer();
            emptyAddress = receiveBuffers.front().address;
            receiveBuffers.pop();
            return true;
        }
        return false;
    }
    
    
    // sending - for clients
    //--------------------------------------------------------------
    bool UDPDatagramSocket::sendMessage(string msg) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(!checkClientValid()) return false;
        
        UDPMessageInfo messageInfo;
        messageInfo.buffer = ofBuffer(msg);
        messageInfo.address = *destinationAddress;
        //ofBuffer buffer(msg);
        sendBuffers.push(messageInfo);
        return true;
    }
    
    bool UDPDatagramSocket::sendMessage(ofBuffer& buffer) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(!checkClientValid()) return false;
        
        UDPMessageInfo messageInfo;
        messageInfo.buffer = buffer;
        messageInfo.address = *destinationAddress;
        //sendBuffers.push(buffer);
        sendBuffers.push(messageInfo);
        return true;
    }
    
    // sending - for server
    //--------------------------------------------------------------
    bool UDPDatagramSocket::sendMessage(Poco::Net::SocketAddress &toAddress, string msg) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(!checkServerValid()) return false;
        
        UDPMessageInfo messageInfo;
        messageInfo.buffer = ofBuffer(msg);
        messageInfo.address = toAddress;
        //ofBuffer buffer(msg);
        sendBuffers.push(messageInfo);
        return true;
    }
    
    
    bool UDPDatagramSocket::sendMessage(Poco::Net::SocketAddress &toAddress, ofBuffer& buffer) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(!checkServerValid()) return false;
        
        UDPMessageInfo messageInfo;
        messageInfo.buffer = buffer;
        messageInfo.address = toAddress;
        //sendBuffers.push(buffer);
        sendBuffers.push(messageInfo);
        return true;
    }
    
    bool UDPDatagramSocket::sendMessageToAll(string msg) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(!checkServerValid()) return false;
        
        for(int i = 0; i < clients.size(); i++) {
            UDPMessageInfo messageInfo;
            messageInfo.buffer = ofBuffer(msg);
            messageInfo.address = clients[i];
            //ofBuffer buffer(msg);
            sendBuffers.push(messageInfo);
        }
        return true;
    }
    
    bool UDPDatagramSocket::sendMessageToAll(ofBuffer& msg) {
        Poco::ScopedLock<ofMutex> lock(mutex);
        if(!checkServerValid()) return false;
        
        for(int i = 0; i < clients.size(); i++) {
            UDPMessageInfo messageInfo;
            messageInfo.buffer = msg;
            messageInfo.address = clients[i];
            //ofBuffer buffer(msg);
            sendBuffers.push(messageInfo);
        }
        return true;
    }
    
    bool UDPDatagramSocket::checkServerValid() {
        if(!connected) {
            ofLogError() << "Server fail- not binded.";
            return false;
        }
        if(!isServer) {
            ofLogError() << "Server fail- this is a server, not a client.";
            return false;
        }
        if(!clients.size()) {
            ofLogError() << "Server fail- no client connections to send to.";
            return false;
        }
        return true;
    }
    
    bool UDPDatagramSocket::checkClientValid() {
        if(!connected) {
            ofLogError() << "Client fail- not connected.";
            return false;
        }
        if(isServer) {
            ofLogError() << "Client fail- this is a client, not a server.";
            return false;
        }
        return true;
    }
    
    
    // advanced- internal poco buffer sizes
    //--------------------------------------------------------------
    void UDPDatagramSocket::setMaxSendSize(int size) {
        if(connected) {
            
            // advanced: change pocos max internal buffer send size- default send is 9216
            socket->setSendBufferSize(size);
            ofLog() << "Max send size changed: " << socket->getSendBufferSize();
        }
    }
    
    int UDPDatagramSocket::getMaxSendSize() {
        if(connected) {
            return socket->getSendBufferSize();
        }
        return 0;
    }
    
} // namespace ofxPocoNetwork
