
#include "UDPClient.h"

namespace ofxPocoNetwork {
    
UDPClient::UDPClient() {    
    connected = false;
    receiveSize = 256;
}

UDPClient::~UDPClient() {
    disconnect();
}

void UDPClient::disconnect() {
    
    connected = false;
    if(socketAddress) delete socketAddress;
    if(socket) delete socket;
}

void UDPClient::connect(string ipAddr, int port) {

    // setup tcp poco client
    socketAddress = new Poco::Net::SocketAddress(ipAddr, port);
    socket = new Poco::Net::DatagramSocket(Poco::Net::IPAddress::IPv4);
    
    
    try {
        
        // client must connect to server
        socket->connect(*socketAddress);
        connected = true;
        ofLog() << "UDPClient connected";
        ofLog() << "Max receive size: " << socket->getReceiveBufferSize();
        ofLog() << "Max send size: " << socket->getSendBufferSize(); // 9216
        ofLog() << "Connect: " << socketAddress->toString();
        ofLog() << "Socket: " << socket->address().toString();
        
    } catch (Poco::Exception& exc) {
        
        disconnect();        
        ofLog() << "UDPClient Error: could not create socket- " << exc.displayText();
    }
}

void UDPClient::connect(string ipAddr, int port, string sourceIpAddr, int sourcePort, bool reuseAddress) {
    
    // setup tcp poco client
    socketAddressSource = new Poco::Net::SocketAddress(sourceIpAddr, sourcePort);
    socketAddress = new Poco::Net::SocketAddress(ipAddr, port);
    socket = new Poco::Net::DatagramSocket(*socketAddressSource, reuseAddress);
    
    try {
        
        // client must connect to server
        socket->connect(*socketAddress);
        connected = true;
        ofLog() << "UDPClient connected";
        ofLog() << "Max receive size: " << socket->getReceiveBufferSize();
        ofLog() << "Max send size: " << socket->getSendBufferSize(); // 9216
        ofLog() << "Connect: " << socketAddress->toString();
        ofLog() << "Socket: " << socket->address().toString();
        
    } catch (Poco::Exception& exc) {
        
        disconnect();
        ofLog() << "UDPClient Error: could not create socket- " << exc.displayText();
    }
}

    
void UDPClient::setBroadcast(bool broadcast) {
    if(connected) {
        socket->setBroadcast(broadcast);
    }
}
    
//--------------------------------------------------------------
void UDPClient::printInfo() {
    
    ofLog() << "Connect: " << socketAddress->toString();
    ofLog() << "Socket: " << socket->address().toString();
}
    
// send
//--------------------------------------------------------------
int UDPClient::sendMessage(string msg) {
    
    if(connected) {
        ofBuffer buffer(msg);
        return sendMessage(buffer);
    }
    return 0;
}

int UDPClient::sendMessage(ofBuffer& buffer) {
    if(connected) {
        try {
            int sent = socket->sendBytes(buffer.getBinaryBuffer(), buffer.size());
            //int sent = socket->sendTo(buffer.getData(), buffer.size(), *socketAddress);
            return sent;
        } catch (Poco::Exception &e) {
            ofLogError() << "* UDPClient send fail 1: " << e.message();
            return 0;
        } catch(std::exception& exc) {
            ofLogError() << "* UDPClient send fail 2";
            return 0;;
        } catch (...) {
            ofLogError() << "* UDPClient send fail 3";
            return 0;
        }
    }
    return 0;
}
    
    
// receive
//--------------------------------------------------------------
void UDPClient::setReceiveSize(int size) {
    receiveSize = size;
}
    
int UDPClient::receiveMessage(string& msg) {
    if(connected) {
        ofBuffer buffer;
        buffer.allocate(receiveSize);
        int r = receiveMessage(buffer);
        msg = buffer.getBinaryBuffer();
        return r;
    }
    return 0;
}
    
int UDPClient::receiveMessage(ofBuffer& buffer) {
    if(connected) {
        try {
            int rec = socket->receiveBytes(buffer.getBinaryBuffer(), buffer.size());
            //int rec = socket->receiveFrom(buffer.getData(), buffer.size(), *socketAddress);
            //ofLog() << "Receive port: " << socket->address().port();
            //ofLog() << "Receive port: " << socket->peerAddress().port();
            return rec;
        } catch (Poco::Exception &e) {
            ofLogError() << "* UDPClient receive fail 1: " << e.message();
            return 0;
        } catch(std::exception& exc) {
            ofLogError() << "* UDPClient receive fail 2";
            return 0;;
        } catch (...) {
            ofLogError() << "* UDPClient receive fail 3";
            return 0;
        }
    }
    return 0;
}

// advanced- internal poco buffer sizes
//--------------------------------------------------------------
void UDPClient::setMaxSendSize(int size) {
    if(connected) {
        
        // advanced: change pocos max internal buffer send size- default send is 9216
        socket->setSendBufferSize(size);
        ofLog() << "Max send size: " << socket->getSendBufferSize();
    }
}

int UDPClient::getMaxSendSize() {
    if(connected) {
        return socket->getSendBufferSize();
    }
    return 0;
}

} // namespace ofxPocoNetwork