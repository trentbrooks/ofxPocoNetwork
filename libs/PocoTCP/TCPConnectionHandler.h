#pragma once


#include "ofMain.h"
#include "Poco/RWLock.h"
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/Socket.h"
#include "SocketConnectionHandler.h"

using Poco::ScopedLock;
using Poco::Net::Socket;
using Poco::Net::StreamSocket;
using Poco::Net::SocketAddress;
using Poco::Net::TCPServerConnection;

namespace ofxPocoNetwork {
    
/* 
 TCPConnectionHandler.
 - handles socket connections (extends SocketConnectionHandler)
 - TCPClientConnectionHandler for clients
 - TCPServerConnectionHandler for server
 */
    
// TCPConnectionHandler is a base class of TCPServerConnectionHandler or TCPClientConnectionHandler
class TCPConnectionHandler : public SocketConnectionHandler {
public:
    TCPConnectionHandler(StreamSocket* streamSocket, MessageFraming protocol=FRAME_HEADER_AND_MESSAGE);
    ~TCPConnectionHandler();
    
    void disconnect();
    
protected:
    
    // thread/poco runnable
    void run(); // async thread loop called fron TCPServerConnectionHandler/TCPClientConnectionHandler
    bool isRunning;
    int sleepTime;
    
};

// server only version of TCPConnectionHandler also extending Poco::Net::TCPServerConnection
class TCPServerConnectionHandler : public Poco::Net::TCPServerConnection, public TCPConnectionHandler {
public:
    TCPServerConnectionHandler(const StreamSocket& streamSocket, MessageFraming protocol=FRAME_HEADER_AND_MESSAGE) : Poco::Net::TCPServerConnection(streamSocket), TCPConnectionHandler(&socket(), protocol) {
    }
    
    void run() {
        TCPConnectionHandler::run();
    }
};

// client only version of TCPConnectionHandler with a runnable
class TCPClientConnectionHandler : public Poco::Runnable, public TCPConnectionHandler {
public:
    TCPClientConnectionHandler(StreamSocket* streamSocket, MessageFraming protocol=FRAME_HEADER_AND_MESSAGE) : TCPConnectionHandler(streamSocket, protocol) {
    }
    
    void run() {
        TCPConnectionHandler::run();
    }
};
    
} // namespace ofxPocoNetwork