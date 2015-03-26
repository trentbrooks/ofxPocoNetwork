#pragma once

#include "ofMain.h"
#include "SocketConnectionHandler.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/NObserver.h"


/*
 ReactorConnectionHandler
 - part of the reactor acceptor framework (use with ReactorServer and ReactorClient)
 - extends SocketConnectionHandler
 - used by both the server and client to handle socket socket connections
 - event handlers for different socket states
 - note- onWritable event handler is heavy on cpu
 - internal message parser (MessageProtocol)
 
 - 0. TYPE_NONE just gets whatever is available from socket
 - 1. TYPE_HEADER_AND_MESSAGE automatically sends/receives a 4 byte header before each message (default)
 - 2. TYPE_DELIMITED splits messages by delimeter
 - 3. TYPE_FIXED_SIZE messages will always be same size, eg. 256
 */

// https://github.com/paulreimer/ofxWebUI-poco/blob/master/src/ThreadedWebSocketServer.h
// http://poco.sourcearchive.com/documentation/1.3.6p1-1/samples_2EchoServer_2src_2EchoServer_8cpp-source.html
// http://stackoverflow.com/questions/8366048/what-is-a-good-way-to-handle-multithreading-with-poco-socketreactor
// http://stackoverflow.com/questions/22957250/strange-poco-reactors-notifications


namespace ofxPocoNetwork {
    
class ReactorConnectionHandler : public SocketConnectionHandler {
public:
    
    ReactorConnectionHandler(Poco::Net::Socket& socket, Poco::Net::SocketReactor& reactor, MessageFraming protocol=FRAME_HEADER_AND_MESSAGE);
    ~ReactorConnectionHandler();
            
    void removeListeners();
    void disconnect();
    
    // events
    void onReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf);
    void onWritable(const Poco::AutoPtr<Poco::Net::WritableNotification>& pNf);
    void onIdle(const Poco::AutoPtr<Poco::Net::IdleNotification>& pNf);
    void onTimeout(const Poco::AutoPtr<Poco::Net::TimeoutNotification>& pNf);
    void onError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf);
    void onShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf);
    
    // override send
    virtual void sendMessage(ofBuffer& message);
    
    // it's either too fast + heavy on cpu, or too slow. CANT WIN????
    void setRemoveWriteHandlerOnEmpty(bool b);
    
protected:
    
    Poco::Net::SocketReactor& _reactor;
    Poco::Net::StreamSocket _socket;
    
    // extra timeout and write handling
    bool hasTimeoutHandler;
    bool closeOnTimeout;
    bool hasWriteHandler;
    bool removeWriteHandlerOnEmpty;
    
    bool hasListeners;

};
    
} // namespace ofxPocoNetwork

