#pragma once
#include "ofMain.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"


/*
 UDPServer
 - poco based udp: http://pocoproject.org/docs/Poco.Net.DatagramSocket.html
 - threaded + blocking
 - receiver only

 TODO: add sending 
 - can send data if know socketaddress with sendTo(), but because socket is blocking will need to make this useable
 */

namespace ofxPocoNetwork {

class UDPServer : public ofThread {
public:
    
    UDPServer();
    virtual ~UDPServer();
    
    void disconnect();
    
    void bind(int port); // bind
    bool connected;
    
    void threadedFunction();

    // receive
    bool hasWaitingMessages();
    void setReceiveSize(int size);

    // send
    bool getNextMessage(ofBuffer& message);
    bool getNextMessage(string& msg);
    
protected:
    
    int sleepTime;
    int receiveSize;
    
    bool waitingRequest; // temp var applied to any connection
    
    Poco::Net::SocketAddress* socketAddress;
    Poco::Net::DatagramSocket* socket;
    
    queue<ofBuffer> receiveBuffers;

};

} // namespace ofxPocoNetwork