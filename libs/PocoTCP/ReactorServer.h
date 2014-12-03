#pragma once

#include "ofMain.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "ReactorConnectionHandler.h"
#include "Poco/Thread.h"
using Poco::Net::ReadableNotification;
using Poco::NObserver;
using Poco::Thread;
using Poco::Net::ServerSocket;
using Poco::Net::StreamSocket;
using Poco::Net::SocketAddress;
using Poco::Net::SocketReactor;
using Poco::Net::SocketAcceptor;
using Poco::Net::IPAddress;

/*
 ReactorServer
 - implements the poco reactor framework
 - uses CustomSocketAcceptor (SocketAcceptor) to create client connections
 - thread needed for reactor->run()
 - can send/add messages to queue (non blocking)
 - can receive/get messages from another queue (non blocking)
 - TODO: implement different message framing options
 */

namespace ofxPocoNetwork {
    
template<typename T>
class CustomSocketAcceptor;

class ReactorServer {
public:

    ReactorServer();
    ~ReactorServer();
    
    void start(int port, MessageFraming protocol=FRAME_HEADER_AND_MESSAGE);
    
    // clients/connections
    void onClientConnected(ReactorConnectionHandler* socketHandler); // callback from acceptor handler
    void onClientRemoved(const void* socket);
    vector<ReactorConnectionHandler*> clients;
    int getNumClients();
    
    // enable this if sending messages every frame - eg. video
    // all it does is not remove the write listener after a send
    // high cpu usage when on, but off by default
    void setAllowFastWriting(bool enable);
    
    // for TYPE_FIXED_SIZE only
    void setFixedReceiveSize(int s);
    
    
    // send (non blocking)
    void sendMessage(int clientId, string msg);
    void sendMessage(int clientId, ofBuffer& msg);
    void sendMessageToAll(string msg);    
    void sendMessageToAll(ofBuffer& msg);

    // receive (non blocking)
    bool hasWaitingMessages(int clientId);
    bool getNextMessage(int clientId, ofBuffer& buffer);
    bool getNextMessage(int clientId, string& buffer);
    
    
protected:
   
    int serverPort;
    bool allowFastWriting;
    int fixedReceiveSize;
    
    CustomSocketAcceptor<ReactorConnectionHandler>* acceptor;
    SocketReactor* reactor;
    ServerSocket* serverSocket;
    Thread thread;
    ofMutex mutex;
};


template<typename T>
class CustomSocketAcceptor : public SocketAcceptor<T> {
public:
    CustomSocketAcceptor(ServerSocket& socket, SocketReactor& reactor, ReactorServer* server, MessageFraming protocol) :SocketAcceptor<T>(socket,reactor) {
        raServer = server;
        raProtocol = protocol;
    }    
    ReactorServer* raServer;
    MessageFraming raProtocol;
    
protected:
    virtual T* createServiceHandler(StreamSocket& sock) {
        //T* socketHandler = SocketAcceptor<T>::createServiceHandler(sock);
        T* socketHandler = new T(sock, *SocketAcceptor<T>::reactor(), raProtocol);
        raServer->onClientConnected(socketHandler);
        return socketHandler;
    }
};

} // namespace ofxPocoNetwork