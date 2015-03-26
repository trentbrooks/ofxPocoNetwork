#pragma once
#include "ofMain.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"


/*
 UDPSocket
 - poco based udp: http://pocoproject.org/docs/Poco.Net.DatagramSocket.html
 - receive and send (server and client)
 - for server call bind(), for client call connect()
 
 */

namespace ofxPocoNetwork {
    
    struct UDPMessageInfo {
        ofBuffer buffer;
        Poco::Net::SocketAddress address;
    };
    
    class UDPDatagramSocket : public ofThread {
    public:
        
        UDPDatagramSocket();
        virtual ~UDPDatagramSocket();
        
        void disconnect();
        bool isConnected() { return connected; }
        
        // for server / receiver
        void bind(int port, bool reuseAddress=false); // bind
        
        // for client / sender
        void connect(string ipAddr, int port);
        void connect(string ipAddr, int port, int sourcePort, bool reuseAddress=false);
        
        void setBroadcast(bool broadcast);
        
        // threaded send/receive
        void threadedFunction();
        virtual void processRead();
        virtual void processWrite();
        
        // receive
        bool hasWaitingMessages();
        bool getNextMessage(ofBuffer& message); // ignores sender address
        bool getNextMessage(string& msg); // ignores sender address
        bool getNextMessage(ofBuffer& message, Poco::Net::SocketAddress &emptyAddress); // fills address
        bool getNextMessage(string& msg, Poco::Net::SocketAddress &emptyAddress); // fills address
        void setReceiveSize(int size);
        
        // send messages to destinationAddress (for clients only)
        bool sendMessage(string msg);
        bool sendMessage(ofBuffer& msg);
        
        // send messages to specified addresses (for server only)
        bool sendMessage(Poco::Net::SocketAddress &toAddress, string msg);
        bool sendMessage(Poco::Net::SocketAddress &toAddress, ofBuffer& msg);
        bool sendMessageToAll(string msg);
        bool sendMessageToAll(ofBuffer& msg);
        
        // ports
        int getPort() { return port; }
        int getSourcePort() { return sourcePort; }
        
        // for server only
        int getNumClients();
        vector<Poco::Net::SocketAddress> clients;
        
        // advanced- internal poco buffer sizes (best not to change this unless you need video or something)
        void setMaxSendSize(int size); // change pocos max send size- default 9216
        int getMaxSendSize();
        
    protected:
        
        bool checkServerValid();
        bool checkClientValid();
        
        bool isServer;
        bool connected;
        int port;
        int sourcePort;
        int sleepTime;
        int receiveSize;        
        
        Poco::Net::SocketAddress* destinationAddress;
        Poco::Net::SocketAddress* sourceAddress;
        Poco::Net::DatagramSocket* socket;
        
        //queue<ofBuffer> receiveBuffers;
        //queue<ofBuffer> sendBuffers;
        queue<UDPMessageInfo> receiveBuffers;
        queue<UDPMessageInfo> sendBuffers;
    };
    
    // shortcut server + client types
    typedef UDPDatagramSocket UDPClient;
    typedef UDPDatagramSocket UDPServer;
    
} // namespace ofxPocoNetwork