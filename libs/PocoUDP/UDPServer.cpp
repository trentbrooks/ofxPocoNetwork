
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

void UDPServer::bind(int port, bool reuseAddress){
    
    if(connected) disconnect();
    
    // setup udp poco server (localhost)
    socketAddress = new Poco::Net::SocketAddress(Poco::Net::IPAddress(), port);
    //socket = new Poco::Net::DatagramSocket(Poco::Net::IPAddress::IPv4); // not binded
    socket = new Poco::Net::DatagramSocket(*socketAddress, reuseAddress); // automatically binds to socket
    
    connected = true;
    ofLog() << "Bind to: " << socketAddress->toString();
    ofLog() << "Socket: " << socket->address().toString();
    ofLog() << "Max receive size: " << socket->getReceiveBufferSize();
    ofLog() << "Max send size: " << socket->getSendBufferSize();
    
    
    // a server must be bound- and can only send to a defined socket address
    //socket->connect(*socketAddress);
    //socket->bind(*socketAddress, true);
    //socket->sendBytes("hello", 5);
    //socket->sendTo("hello", 5, *socketAddress);
    
    
    if(!isThreadRunning()) startThread();
}

void UDPServer::setBroadcast(bool broadcast) {
    if(connected) {
        socket->setBroadcast(broadcast);
    }
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
                Poco::Net::SocketAddress sender; // use this to identify the client/sender
                int n = socket->receiveFrom(buffer.getData(), buffer.size(), sender);
                
                // who sent message (sender)
                //ofLog() << "Received message from: " << sender.toString() << ", size: " << n;
                //ofLog() << "Message: " << buffer.getData();

                
                // copy/replace buffer / or push into queue
                mutex.lock();
                receiveBuffers.push(buffer);
                mutex.unlock();
                
                // test: send a message back to client/sender- works
                // note - the send port is different?
                //int sent = socket->sendTo("hello", 5, sender);
                //int n2 = socket->sendTo(buffer.getData(), buffer.size(), sender);
                //int sent = socket->sendBytes("hello", 5);
                //ofLog() << "Sent back: " << sent;
                
                // send a single message from the queue if exists
                // FIXME: this needs work...
                mutex.lock();
                if(sendBuffers.size()) {
                    ofBuffer sendBuffer = sendBuffers.front();
                    sendBuffers.pop();
                    mutex.unlock();
                    int nSent = socket->sendTo(sendBuffer.getData(), sendBuffer.size(), sender);
                } else {
                    mutex.unlock();
                }

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
    

// sending - adds messages to client queue (non blocking)
//--------------------------------------------------------------
void UDPServer::sendMessage(string msg) {
    
    Poco::ScopedLock<ofMutex> lock(mutex);
    if(!connected) return;
    ofBuffer buffer(msg);
    sendBuffers.push(buffer);
    ofLog() << "added message";
}


void UDPServer::sendMessage(ofBuffer& buffer) {
    Poco::ScopedLock<ofMutex> lock(mutex);
    if(!connected) return;
    sendBuffers.push(buffer);
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
