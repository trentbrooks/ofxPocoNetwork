#include "ReactorServer.h"

namespace ofxPocoNetwork {
    
ReactorServer::ReactorServer() {
    allowFastWriting = true;
    fixedReceiveSize = DEFAULT_FIXED_SIZE;
    serverSocket = NULL;
    acceptor = NULL;
    reactor = NULL;
    
}

ReactorServer::~ReactorServer() {
    
    delete acceptor;
    for(int i = 0; i < clients.size(); i++) {
        clients[i]->disconnect();
    }
    
    if(reactor) reactor->stop();
    if(serverSocket) serverSocket->close();
    
    if(thread.isRunning()) {
        thread.join();
    }
    delete reactor;
    delete serverSocket;
    
    ofLog() << "ReactorServer deleted";
}

void ReactorServer::start(int port, MessageFraming protocol) {
    
    serverPort = port;
    
    reactor = new SocketReactor();
    serverSocket = new ServerSocket(serverPort);
    acceptor = new CustomSocketAcceptor<ReactorConnectionHandler>(*serverSocket, *reactor, this, protocol);
    //reactor->run();
    thread.start(*reactor);
}

//--------------------------------------------------------------
void ReactorServer::onClientConnected(ReactorConnectionHandler* socketHandler) {
    //mutex.lock();
    socketHandler->setRemoveWriteHandlerOnEmpty(allowFastWriting);
    socketHandler->setFixedReceiveSize(fixedReceiveSize);
    clients.push_back(socketHandler);
    ofAddListener(socketHandler->disconnectionEvent, this, &ReactorServer::onClientRemoved);
    ofLog() << "Added client: " << clients.size();
    //mutex.unlock();
}

void ReactorServer::onClientRemoved(const void* socket) {
    //ScopedLock<ofMutex> lock(mutex);
    for(int i = 0; i < clients.size(); i++) {
        if(socket == clients[i]) {
            ofRemoveListener(clients[i]->disconnectionEvent, this, &ReactorServer::onClientRemoved);
            delete clients[i]; // this deletes the socket connection
            clients.erase(clients.begin()+i);
            ofLog() << "Removed client: " << i;
            return;
        }
    }
}

void ReactorServer::setAllowFastWriting(bool enable) {
    allowFastWriting = !enable;
    for(int i = 0; i < clients.size(); i++) {
        clients[i]->setRemoveWriteHandlerOnEmpty(allowFastWriting);
         ofLog() << "Setting fast write handler: " << allowFastWriting;
    }
}

void ReactorServer::setFixedReceiveSize(int s) {
    fixedReceiveSize = s;
    for(int i = 0; i < clients.size(); i++) {
        clients[i]->setFixedReceiveSize(fixedReceiveSize);
    }
}

// sending - adds messages to client queue (non blocking)
//--------------------------------------------------------------
void ReactorServer::sendMessage(int clientId, string msg) {
    //if(!clients.size()) return;
    if(clientId >= clients.size()) return;
    ofBuffer buffer(msg);
    clients[clientId]->sendMessage(buffer);
}

void ReactorServer::sendMessage(int clientId, ofBuffer& buffer) {
    //if(!clients.size()) return;
    if(clientId >= clients.size()) return;
    clients[clientId]->sendMessage(buffer);
}

void ReactorServer::sendMessageToAll(string msg) {
    for(int i = 0; i < clients.size(); i++) {
        sendMessage(i, msg);
    }
}

void ReactorServer::sendMessageToAll(ofBuffer& buffer) {
    for(int i = 0; i < clients.size(); i++) {
        sendMessage(i, buffer);
    }
}


// receiving - gets messages from client queue (non blocking)
//--------------------------------------------------------------
bool ReactorServer::getNextMessage(int clientId, string& msg) {
    if(clientId >= clients.size()) return false;
    ofBuffer buffer;
    clients[clientId]->getNextMessage(buffer);
    msg = buffer.getData();
    return true;
}

bool ReactorServer::getNextMessage(int clientId, ofBuffer& msg) {
    if(clientId >= clients.size()) return false;
    clients[clientId]->getNextMessage(msg);
    return true;
}






int ReactorServer::getNumClients() {
    return clients.size();
}

bool ReactorServer::hasWaitingMessages(int clientId) {
    if(clientId >= clients.size()) return false;
    return clients[clientId]->hasWaitingMessages();
}

} // namespace ofxPocoNetwork