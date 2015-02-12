#pragma once

#include "ofMain.h"
#include "ofMain.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"

/*
 UDPClient
 - poco based udp: http://pocoproject.org/docs/Poco.Net.DatagramSocket.html
 - wip
 - non threaded send + receive
 
 TODO: add threading maybe?
 */

namespace ofxPocoNetwork {

class UDPClient {
public:
    
    UDPClient();
    virtual ~UDPClient();
    void disconnect();
    
    // client only connects (no binding)
    void connect(string ipAddr, int port);
    void connect(string ipAddr, int destinationPort, int sourcePort, bool reuseAddress=false);
    bool connected;
    
    // send (blocking)
    int sendMessage(string msg);
    int sendMessage(ofBuffer& buffer);
    
    // advanced- internal poco buffer sizes (best not to change this unless you need video or something)
    void setMaxSendSize(int size); // change pocos max send size- default 9216
    int getMaxSendSize();
    
    // receive (blocking)
    int receiveMessage(string& msg);
    int receiveMessage(ofBuffer& buffer);
    void setReceiveSize(int size);
    
    void setBroadcast(bool broadcast);
    
    void printInfo();
    
protected:

    Poco::Net::SocketAddress* socketAddress;
    Poco::Net::SocketAddress* socketAddressSource;
    Poco::Net::DatagramSocket* socket;
    
    int receiveSize;
    int maxBufferSendSize;

};

} // namespace ofxPocoNetwork