#pragma once

#include "ofMain.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketConnector.h"
#include "ReactorConnectionHandler.h"
#include "Poco/Thread.h"

/*
 ReactorClient
 - implements the poco reactor framework
 - uses CustomSocketConnector (SocketConnector) to create connections
 */

namespace ofxPocoNetwork {

template<typename T>
class CustomSocketConnector;

class ReactorClient {
public:

    ReactorClient();
    ~ReactorClient();
    
    void connect(string ipAddr, int port, MessageFraming protocol=FRAME_HEADER_AND_MESSAGE);
    void onClientConnected(ReactorConnectionHandler* socketHandler);
    void onClientRemoved(const void* socket);
    ReactorConnectionHandler* socketHandler;
    bool isConnected();
    
    // enable this if sending messages every frame - eg. video
    // all it does is not remove the write listener after a send
    // high cpu usage when on, but off by default
    void setAllowFastWriting(bool enable);
    
    // for TYPE_FIXED_SIZE only
    void setFixedReceiveSize(int s);
    
    // send (non blocking)
    void sendMessage(string msg);
    void sendMessage(ofBuffer& msg);
    
    // receive (non blocking)
    bool hasWaitingMessages();
    bool getNextMessage(ofBuffer& buffer);
    bool getNextMessage(string& buffer);
    
protected:
   
    int port;
    string address;
    bool connected;
    bool allowFastWriting;
    int fixedReceiveSize;
    
    CustomSocketConnector<ReactorConnectionHandler>* connector;
    Poco::Net::SocketReactor* reactor;
    Poco::Net::SocketAddress* socketAddress;
    Poco::Thread thread;
    
};



template<typename T>
class CustomSocketConnector : public Poco::Net::SocketConnector<T> {
public:
    CustomSocketConnector(Poco::Net::SocketAddress& address, Poco::Net::SocketReactor& reactor, ReactorClient* client, MessageFraming protocol) : Poco::Net::SocketConnector<T>(address,reactor) {
        raClient = client;
        raProtocol = protocol;
    }
    ReactorClient* raClient;
    MessageFraming raProtocol;
    
protected:
    virtual T* createServiceHandler() {
        //T* socketHandler = SocketConnector<T>::createServiceHandler();
        T* socketHandler = new T(Poco::Net::SocketConnector<T>::socket(), *Poco::Net::SocketConnector<T>::reactor(), raProtocol);
        raClient->onClientConnected(socketHandler);
        return socketHandler;
    }
};

} // namespace ofxPocoNetwork
