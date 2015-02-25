#include "ReactorClient.h"

namespace ofxPocoNetwork {
    
ReactorClient::ReactorClient() {
    fixedReceiveSize = DEFAULT_FIXED_SIZE;
    socketHandler = NULL;
    reactor = NULL;
    socketAddress = NULL;
    connector = NULL;
    connected = false;
    allowFastWriting = true;
}

ReactorClient::~ReactorClient() {
    
    connected = false;
    delete connector;
    if(socketHandler) {
        socketHandler->disconnect();
    }
    if(reactor) reactor->stop();
    
    if(thread.isRunning()) {
        thread.join();
    }
    delete reactor;
    delete socketAddress;
    ofLog() << "~ReactorClient deleted";
}

//--------------------------------------------------------------
void ReactorClient::connect(string ipAddr, int port, MessageFraming protocol) {
    this->address = ipAddr;
    this->port = port;
    
    reactor = new SocketReactor();

    socketAddress = new SocketAddress(address, port);
    connector = new CustomSocketConnector<ReactorConnectionHandler>(*socketAddress, *reactor, this, protocol);
    //reactor->run();
    thread.start(*reactor);
}

bool ReactorClient::isConnected() {
    return connected;
}

//--------------------------------------------------------------
void ReactorClient::onClientConnected(ReactorConnectionHandler* socketHandler) {
    ofLog() << "Connected";
    this->socketHandler = socketHandler;
    socketHandler->setRemoveWriteHandlerOnEmpty(allowFastWriting);
    socketHandler->setFixedReceiveSize(fixedReceiveSize);
    ofAddListener(socketHandler->disconnectionEvent, this, &ReactorClient::onClientRemoved);
    connected = true;
}

void ReactorClient::onClientRemoved(const void* socket) {
    
    ofLog() << "Removed";
    ofRemoveListener(socketHandler->disconnectionEvent, this, &ReactorClient::onClientRemoved);
    delete socketHandler;
    socketHandler = NULL;
    connected = false;
}

void ReactorClient::setAllowFastWriting(bool enable) {
    allowFastWriting = !enable;
    if(socketHandler) {
        socketHandler->setRemoveWriteHandlerOnEmpty(allowFastWriting);
        ofLog() << "Setting fast write handler: " << allowFastWriting;
    }
}

void ReactorClient::setFixedReceiveSize(int s) {
    fixedReceiveSize = s;
    if(socketHandler) {
        socketHandler->setFixedReceiveSize(fixedReceiveSize);
    }

}

// receiving - gets messages from client queue (non blocking)
//--------------------------------------------------------------
bool ReactorClient::hasWaitingMessages() {
    if(!connected) return false;
    return socketHandler->hasWaitingMessages();
}

bool ReactorClient::getNextMessage(string& msg) {
    if(!connected) return false;
    ofBuffer buffer;
    socketHandler->getNextMessage(buffer);
    msg = buffer.getData();
    return true;
}

bool ReactorClient::getNextMessage(ofBuffer& msg) {
    if(!connected) return false;
    socketHandler->getNextMessage(msg);
    return true;
}


// sending - adds messages to client queue (non blocking)
//--------------------------------------------------------------
void ReactorClient::sendMessage(string msg) {
    if(!connected) return;
    ofBuffer buffer(msg);
    socketHandler->sendMessage(buffer);
}


void ReactorClient::sendMessage(ofBuffer& buffer) {
    if(!connected) return;
    socketHandler->sendMessage(buffer);
}

} // namespace ofxPocoNetwork



