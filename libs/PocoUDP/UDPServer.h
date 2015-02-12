#pragma once
#include "ofMain.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"


/*
 UDPServer
 - poco based udp: http://pocoproject.org/docs/Poco.Net.DatagramSocket.html
 - threaded + blocking
 - receive and send

 TODO: add sending (sort of done)
 - can send data if know socketaddress with sendTo(), but because socket is blocking will need to make this useable
 - this is kind of working- but send can only happen after receive (need to know client address)
 */

namespace ofxPocoNetwork {

class UDPServer : public ofThread {
public:
    
    UDPServer();
    virtual ~UDPServer();
    
    void disconnect();
    
    void bind(int port, bool reuseAddress=false); // bind
    bool connected;
    
    void threadedFunction();

    // receive
    bool hasWaitingMessages();
    void setReceiveSize(int size);
    bool getNextMessage(ofBuffer& message);
    bool getNextMessage(string& msg);
    
    // send
    void sendMessage(string msg);
    void sendMessage(ofBuffer& msg);
    
    // clients
    vector<Poco::Net::SocketAddress> clients;
    
    void setBroadcast(bool broadcast);
    
protected:
    
    int sleepTime;
    int receiveSize;
    
    bool waitingRequest; // temp var applied to any connection
    
    Poco::Net::SocketAddress* socketAddress;
    Poco::Net::DatagramSocket* socket;
    
    queue<ofBuffer> receiveBuffers;
    queue<ofBuffer> sendBuffers;
};

} // namespace ofxPocoNetwork