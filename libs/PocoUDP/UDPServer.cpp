
#include "UDPServer.h"

namespace ofxPocoNetwork {
    
UDPServer::UDPServer() {
    receiveSize = 256;
    sleepTime = 16;
    connected = false;
    waitingRequest = false;
}

UDPServer::~UDPServer() {

    disconnect();
}

void UDPServer::disconnect() {
    waitForThread();
    connected = false;
    if(socketAddress) delete socketAddress;
    if(socket) delete socket;
}

void UDPServer::bind(int port){
    
    if(connected) disconnect();
    
    // setup udp poco server
    socketAddress = new Poco::Net::SocketAddress(Poco::Net::IPAddress(), port);
    //socket = new Poco::Net::DatagramSocket(Poco::Net::IPAddress::IPv4); // not binded
    socket = new Poco::Net::DatagramSocket(*socketAddress); // automatically binds to socket
    
    connected = true;
    ofLog() << "Max receive size: " << socket->getReceiveBufferSize();
    ofLog() << "Max send size: " << socket->getSendBufferSize();
    
    
    // a server must be bound- and can only send to a defined socket address
    //socket->connect(*socketAddress);
    //socket->bind(*socketAddress, true);
    //socket->sendBytes("hello", 5);
    //socket->sendTo("hello", 5, *socketAddress);
    
    
    if(!isThreadRunning()) startThread();
}



// thread
//--------------------------------------------------------------
void UDPServer::threadedFunction(){

    while( isThreadRunning() ){

        if(connected) {
            
            // currently only setup for receiving
            // receive message - blocks thread until message
            try {
                
                // receive
                ofBuffer buffer;
                buffer.allocate(receiveSize);
                Poco::Net::SocketAddress sender; // use this to identify the client
                int n = socket->receiveFrom(buffer.getData(), buffer.size(), sender);
                
                // who sent message (sender)
                //ofLog() << "Received message from: " << sender.toString() << ", size: " << n;
                //ofLog() << "Message: " << buffer.getData();
                
                
                // copy/replace buffer / or push into queue
                mutex.lock();
                receiveBuffers.push(buffer);
                mutex.unlock();
                
                // test: send a message back to sender- works
                //int sent = socket->sendTo("hello", 5, sender);
                //ofLog() << "Sent back: " << sent;

            } catch (Poco::Exception &e) {
                ofLogError() << "* UDPServer read fail 1";
                //disconnect();
            } catch(std::exception& exc) {
                ofLogError() << "* UDPServer read fail 2";
                //disconnect();
            } catch (...) {
                ofLogError() << "* UDPServer read fail 3";
                //disconnect();
            }
        }

        sleep(sleepTime);
    }

}


// receive
//--------------------------------------------------------------
void UDPServer::setReceiveSize(int size) {
    Poco::ScopedLock<ofMutex> lock(mutex);
    receiveSize = size;
}

bool UDPServer::hasWaitingMessages() {
    Poco::ScopedLock<ofMutex> lock(mutex);
    return receiveBuffers.size() > 0;
}

bool UDPServer::getNextMessage(ofBuffer& message) {
    Poco::ScopedLock<ofMutex> lock(mutex);
    if(receiveBuffers.size()) {
        message = receiveBuffers.front();
        receiveBuffers.pop();
        return true;
    }
    return false;
}

bool UDPServer::getNextMessage(string& msg) {
    Poco::ScopedLock<ofMutex> lock(mutex);
    if(receiveBuffers.size()) {
        msg = receiveBuffers.front().getData();
        receiveBuffers.pop();
        return true;
    }
    return false;
}


// advanced- internal poco buffer sizes
//--------------------------------------------------------------
/*
 // advanced: change pocos max internal buffer receive size- default send is 9216 if smaller than current
 if(socket->getReceiveBufferSize() < size) {
 socket->setReceiveBufferSize(size);
 }
 */

} // namespace ofxPocoNetwork
