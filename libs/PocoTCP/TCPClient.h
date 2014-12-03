#pragma once

#include "ofMain.h"
#include "Poco/Net/Socket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/StreamSocket.h"
#include "TCPConnectionHandler.h"

using Poco::Net::StreamSocket;
using Poco::Net::StreamSocket;
using Poco::Thread;
using Poco::Timespan;

namespace ofxPocoNetwork {
    
    // TODO: add on added, removed callbacks same as reactor framework, handle connections in thread from TCPClientConnectionHandler
    // this is fucked up messy
    // need a thread to handle the connections (same as servers connection factory)
    
class TCPClient {
public:
    
    TCPClient();
    virtual ~TCPClient();
    
    void connect(string ipAddr, int port, MessageFraming protocol=FRAME_HEADER_AND_MESSAGE);
    bool isConnected();
    
    void disconnect();
    
    // doesnt need listeners for added/removed as it's created internally
    TCPClientConnectionHandler* socketHandler;
    void onClientRemoved(const void* socket);
    
    // the connect timeout only occurs on the first run atm
    void setConnectTimeout(int timeoutInSeconds);
    //void setReceiveTimeout(int timeoutInSeconds);
    //void setSendTimeout(int timeoutInSeconds);
    //void setPollTimeout(int timeoutInSeconds);
    
    
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
    
    bool connected;
    int fixedReceiveSize;
    
    SocketAddress* socketAddress;
    StreamSocket* socketStream;
    Thread thread;
    
    Timespan connectTimeout;
    //Poco::Timespan receiveTimeout;
    //Poco::Timespan sendTimeout;
    //Poco::Timespan pollTimeout;
};

} // namespace ofxPocoNetwork