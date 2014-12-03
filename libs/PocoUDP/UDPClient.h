#pragma once

#include "ofMain.h"
#include "ofMain.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"

/*
 UDPClient
 - poco based udp: http://pocoproject.org/docs/Poco.Net.DatagramSocket.html
 - wip
 - sender only
 
 TODO: add receiving + threading maybe?
 */

namespace ofxPocoNetwork {

class UDPClient {
public:
    
    UDPClient();
    virtual ~UDPClient();
    void disconnect();
    
    void connect(string ipAddr, int port);
    bool connected;
    
    // send (blocking)
    int sendMessage(string msg);
    int sendMessage(ofBuffer& msg);
    
    // advanced- internal poco buffer sizes (best not to change this unless you need video or something)
    void setMaxSendSize(int size); // change pocos max send size- default 9216
    int getMaxSendSize();
    
protected:

    Poco::Net::SocketAddress* socketAddress;
    Poco::Net::DatagramSocket* socket;
    
    int maxBufferSendSize;

};

} // namespace ofxPocoNetwork