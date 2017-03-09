#pragma once

#include "ofMain.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/Socket.h"
#include "TCPConnectionHandler.h"


namespace ofxPocoNetwork {

class CustomTCPServerConnectionFactory;
    
class TCPServer {
public:
    
    TCPServer();
    ~TCPServer();
        
    void start(int port, MessageFraming protocol=FRAME_HEADER_AND_MESSAGE);
    
    // clients/connections
    void onClientConnected(TCPServerConnectionHandler* socketHandler); // callback from connection factory handler
    void onClientRemoved(const void* socket);
    vector<TCPServerConnectionHandler*> clients;
    int getNumClients();
    int getClientIndexForAddress(Poco::Net::SocketAddress address);

    // receive requests from clients
    bool hasWaitingMessages(int clientId);
    int getWaitingMessageCount(int clientId);
    void setReceiveSize(int clientId, int size);
    bool getNextMessage(int clientId, string& msg);
    bool getNextMessage(int clientId, ofBuffer& buffer);
    
    // send (blocking)
    void sendMessage(int clientId, string msg);
    void sendMessage(int clientId, ofBuffer& msg);
    void sendMessageToAll(string msg);
    void sendMessageToAll(ofBuffer& msg);

    // for TYPE_FIXED_SIZE only
    void setFixedReceiveSize(int s);
    
    void printServerInfo();
    
    // max send/receive - see SocketConnectionHandler for details
    // might need to set these for all client sockets instead of server socket
    void setMaxSendSize(int size) {
        if(serverSocket) serverSocket->setSendBufferSize(size);// setMaxSendSize(size);
    }
    int getMaxSendSize() {
        if(serverSocket) return serverSocket->getSendBufferSize();// getMaxSendSize();
        return 0;
    }
    void setMaxReceiveSize(int size) {
        if(serverSocket) serverSocket->setReceiveBufferSize(size);// setMaxReceiveSize(size);
    }
    int getMaxReceiveSize() {
        if(serverSocket) return serverSocket->getReceiveBufferSize();// getMaxReceiveSize();
        return 0;
    }
    
protected:
    
    
    int serverPort;
    int fixedReceiveSize;
    
    Poco::Net::ServerSocket* serverSocket;
    Poco::Net::TCPServer* server;
    CustomTCPServerConnectionFactory* connectionFactory;
    

};



// Provides a basic implementation of TCPServerConnectionFactory
// Same as the template, except uses onClientConnected callback
class CustomTCPServerConnectionFactory: public Poco::Net::TCPServerConnectionFactory {
public:
    
	CustomTCPServerConnectionFactory(TCPServer* tServer, MessageFraming protocol) {
        tcpServer = tServer;
        tcpProtocol = protocol;
    };
    TCPServer* tcpServer;
    MessageFraming tcpProtocol;
    
    Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket) {
        TCPServerConnectionHandler* handler = new TCPServerConnectionHandler(socket, tcpProtocol);
        tcpServer->onClientConnected(handler);
        return handler;
    }
};

} // namespace ofxPocoNetwork