
#include "TCPClient.h"

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
    
    delete socketAddress;
    delete socketStream;
    
}


void TCPClient::connect(string ipAddr, int port, MessageFraming protocol) {
        
    // setup tcp poco client    
    socketAddress = new Poco::Net::SocketAddress(ipAddr, port);
    socketStream = new Poco::Net::StreamSocket();
    
    // this must be connected before creating socketHandler
    try {
        socketStream->connect(*socketAddress); // need to thread this bit
        
        // can't create handler until it's connected!
        socketHandler = new TCPClientConnectionHandler(socketStream, protocol);
        ofAddListener(socketHandler->disconnectionEvent, this, &TCPClient::onClientRemoved);
        thread.start(*socketHandler);
        
        connected = true;
    } catch(Poco::Exception& exc) {
        ofLogError() << "* TCPClient failed to connect to server 1: " << exc.displayText();
    }  catch(std::exception& exc) {
        ofLogError() << "* TCPClient failed to connect to server 2";
    } catch (...) {
        ofLogError() << "* TCPClient failed to connect to server 3";
    }
}
    
    
bool TCPClient::isConnected() {
    return connected;
}

void TCPClient::disconnect() {
    
    ofLog() << "Disconnecting whoa whoa";
    //waitForThread();
    if(socketStream) {
        
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
    }
    connected = false;
    delete socketAddress;
    socketAddress = NULL;
    delete socketStream;
    socketStream = NULL;
}

    
//--------------------------------------------------------------
void TCPClient::onClientRemoved(const void* socket) {
    
    ofRemoveListener(socketHandler->disconnectionEvent, this, &TCPClient::onClientRemoved);
    delete socketHandler;
    socketHandler = NULL;
    delete socketStream;
    socketStream = NULL;
    connected = false;
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
    msg = buffer.getBinaryBuffer();
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
    
} // namespace ofxPocoNetwork
