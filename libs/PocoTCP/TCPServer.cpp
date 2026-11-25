
#include "TCPServer.h"

namespace ofxPocoNetwork {

TCPServer::TCPServer() {
    server = NULL;
    serverSocket = NULL;
    connectionFactory = NULL;
}

TCPServer::~TCPServer() {
    stop();
    ofLog() << "~TCPServer deleted";
}

void TCPServer::stop() {
    if(!server && !serverSocket) return;
    const uint64_t startedAt = ofGetElapsedTimeMillis();
    {
        std::lock_guard<std::recursive_mutex> lock(clientsMutex);
        stopping = true;
        ofLog() << "Disconnecting TCPServer... clients=" << clients.size();
    }

    // Do not hold clientsMutex while stopping the acceptor or waiting for workers.
    if(server) server->stop();
    {
        std::lock_guard<std::recursive_mutex> lock(clientsMutex);
        // Removal callbacks take this same lock, keeping handlers alive here.
        for(auto* client : clients) client->requestStop();
    }
    connectionThreads.joinAll();

    delete server;
    server = NULL;
    delete serverSocket;
    serverSocket = NULL;
    connectionFactory = NULL; // Owned by Poco, not by this wrapper.
    ofLog() << "TCPServer stopped in " << (ofGetElapsedTimeMillis() - startedAt) << " ms";
}

void TCPServer::start(int port, MessageFraming protocol){
    
    stop();
    {
        std::lock_guard<std::recursive_mutex> lock(clientsMutex);
        stopping = false;
    }
    serverPort = port;
    
    // alt setup...
    serverSocket = new Poco::Net::ServerSocket(serverPort);
    connectionFactory = new CustomTCPServerConnectionFactory(this, protocol);
    
    server = new Poco::Net::TCPServer(connectionFactory, connectionThreads, *serverSocket);
    server->start();
    
    //serverSocket->setNoDelay(true);
}

//--------------------------------------------------------------
void TCPServer::onClientConnected(TCPServerConnectionHandler* socketHandler) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    if(stopping) {
        // A factory call already in flight must not escape the shutdown pass.
        socketHandler->requestStop();
        return;
    }
    clients.push_back(socketHandler);
    ofAddListener(socketHandler->disconnectionEvent, this, &TCPServer::onClientRemoved);
    ofLog() << "Added client: " << clients.size();
}

void TCPServer::onClientRemoved(const void* socket) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    
    for(int i = 0; i < clients.size(); i++) {
        if(socket == (SocketConnectionHandler*)clients[i]) {
            ofRemoveListener(clients[i]->disconnectionEvent, this, &TCPServer::onClientRemoved);
            //delete clients[i]; // no need to delete, automatically gets deleted magically by TCPServerConnectionFactory
            clients.erase(clients.begin()+i);
            ofLog() << "Removed client: " << i;
            return;
        }
    }
}

int TCPServer::getNumClients() {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    //return server->currentConnections();
    return clients.size();
}
    
int TCPServer::getClientIndexForAddress(Poco::Net::SocketAddress address) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    for(int i = 0; i < clients.size(); i++) {
        if(clients[i]->getSocketAddressObject() == address) {
            return i;
        }
    }
    return -1;
}

void TCPServer::printServerInfo() {
    
    ofLog() << "server setup " << server->port();// << ", " << svs.address().toString();
    ofLog() << "connections: " << server->currentConnections();
    ofLog() << "total connections: " << server->totalConnections();
    ofLog() << "threads: " << server->currentThreads();
    ofLog() << "ip: " << serverSocket->address().toString();
}


void TCPServer::setFixedReceiveSize(int s) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    fixedReceiveSize = s;
    for(int i = 0; i < clients.size(); i++) {
        clients[i]->setFixedReceiveSize(fixedReceiveSize);
    }
}


// receive
//--------------------------------------------------------------
bool TCPServer::hasWaitingMessages(int clientId) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    if(clientId >= clients.size()) return false;
    return clients[clientId]->hasWaitingMessages();
}
    
int TCPServer::getWaitingMessageCount(int clientId) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    if(clientId >= clients.size()) return false;
    return clients[clientId]->getWaitingMessageCount();
}

// non blocking - message/buffer should already exist in connection handler.
bool TCPServer::getNextMessage(int clientId, string& msg) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    if(clientId >= clients.size()) return false;
    ofBuffer receiveBuffer;
    clients[clientId]->getNextMessage(receiveBuffer);
    msg = receiveBuffer.getData();
    return true;

}

bool TCPServer::getNextMessage(int clientId, ofBuffer& buffer) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    if(clientId >= clients.size()) return false;
    clients[clientId]->getNextMessage(buffer);
    return true;

}



// sending - adds messages to client queue (non blocking)
//--------------------------------------------------------------
void TCPServer::sendMessage(int clientId, string msg) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    if(clientId >= clients.size()) return;
    ofBuffer buffer;//(msg);
    buffer.set(msg);
    clients[clientId]->sendMessage(buffer);
}

void TCPServer::sendMessage(int clientId, ofBuffer& buffer) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    if(clientId >= clients.size()) return;
    clients[clientId]->sendMessage(buffer);
}

void TCPServer::sendMessageToAll(string msg) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    for(int i = 0; i < clients.size(); i++) {
        sendMessage(i, msg);
    }
}

void TCPServer::sendMessageToAll(ofBuffer& buffer) {
    std::lock_guard<std::recursive_mutex> lock(clientsMutex);
    for(int i = 0; i < clients.size(); i++) {
        sendMessage(i, buffer);
    }
}

} // namespace ofxPocoNetwork
