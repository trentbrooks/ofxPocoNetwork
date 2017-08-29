
#include "TCPServer.h"

namespace ofxPocoNetwork {

TCPServer::TCPServer() {
    server = NULL;
    serverSocket = NULL;
    connectionFactory = NULL;
}

TCPServer::~TCPServer() {
    
    /*if(serverSocket) {
     try {
     serverSocket->setLinger(true, 0);
     //serverSocket->close();
     } catch (Poco::Exception& exc) {
     // server must be closed or the above crashes on exit
     ofLogError() << "* ~TCPServer failed to setLinger: " << exc.displayText();
     }
     }*/
    
    ofLog() << "Disconnecting TCPServer...";
    if(server) server->stop();
    
    delete server;
    server = NULL;
    delete serverSocket;
    serverSocket = NULL;
    //if(connection) delete connection;
    connectionFactory = NULL;
    
    ofLog() << "~TCPServer deleted";
    
}

void TCPServer::start(int port, MessageFraming protocol){
    
    serverPort = port;
    
    // alt setup...
    serverSocket = new Poco::Net::ServerSocket(serverPort);
    connectionFactory = new CustomTCPServerConnectionFactory(this, protocol);
    
    server = new Poco::Net::TCPServer(connectionFactory, *serverSocket);
    server->start();
    
    //serverSocket->setNoDelay(true);
}

//--------------------------------------------------------------
void TCPServer::onClientConnected(TCPServerConnectionHandler* socketHandler) {
    clients.push_back(socketHandler);
    ofAddListener(socketHandler->disconnectionEvent, this, &TCPServer::onClientRemoved);
    ofLog() << "Added client: " << clients.size();
}

void TCPServer::onClientRemoved(const void* socket) {
    
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
    //return server->currentConnections();
    return clients.size();
}
    
int TCPServer::getClientIndexForAddress(Poco::Net::SocketAddress address) {
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
    fixedReceiveSize = s;
    for(int i = 0; i < clients.size(); i++) {
        clients[i]->setFixedReceiveSize(fixedReceiveSize);
    }
}


// receive
//--------------------------------------------------------------
bool TCPServer::hasWaitingMessages(int clientId) {
    if(clientId >= clients.size()) return false;
    return clients[clientId]->hasWaitingMessages();
}
    
int TCPServer::getWaitingMessageCount(int clientId) {
    if(clientId >= clients.size()) return false;
    return clients[clientId]->getWaitingMessageCount();
}

// non blocking - message/buffer should already exist in connection handler.
bool TCPServer::getNextMessage(int clientId, string& msg) {
    if(clientId >= clients.size()) return false;
    ofBuffer receiveBuffer;
    clients[clientId]->getNextMessage(receiveBuffer);
    msg = receiveBuffer.getData();
    return true;

}

bool TCPServer::getNextMessage(int clientId, ofBuffer& buffer) {
    if(clientId >= clients.size()) return false;
    clients[clientId]->getNextMessage(buffer);
    return true;

}



// sending - adds messages to client queue (non blocking)
//--------------------------------------------------------------
void TCPServer::sendMessage(int clientId, string msg) {
    if(clientId >= clients.size()) return;
    ofBuffer buffer(msg);
    clients[clientId]->sendMessage(buffer);
}

void TCPServer::sendMessage(int clientId, ofBuffer& buffer) {
    if(clientId >= clients.size()) return;
    clients[clientId]->sendMessage(buffer);
}

void TCPServer::sendMessageToAll(string msg) {
    for(int i = 0; i < clients.size(); i++) {
        sendMessage(i, msg);
    }
}

void TCPServer::sendMessageToAll(ofBuffer& buffer) {
    for(int i = 0; i < clients.size(); i++) {
        sendMessage(i, buffer);
    }
}

} // namespace ofxPocoNetwork
