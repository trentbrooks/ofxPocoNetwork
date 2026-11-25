#include "TCPConnectionHandler.h"

namespace ofxPocoNetwork {


TCPConnectionHandler::TCPConnectionHandler(Poco::Net::StreamSocket* streamSocket, MessageFraming protocol) : SocketConnectionHandler() {

    // initialise socket and message framing
    setup(streamSocket, protocol);
    
    //clientId = -1;
    sleepTime = 10;
    isRunning = true;
}

TCPConnectionHandler::~TCPConnectionHandler() {
    
    isRunning = false;
    ofLog() << "~TCPConnectionHandler deleted";
    //ofNotifyEvent(closeEvent, clientId, this);
    /*ofNotifyEvent(disconnectionEvent, this);
    
    try {
        socket().setLinger(true, 0);
        //socket().setKeepAlive(false);
    } catch (Poco::Exception& exc) {
        // server must be closed or the above crashes on exit
        ofLogError() << "* ~TCPConnectionHandler failed to setLinger: " << exc.displayText();
    }*/
    
    
}

void TCPConnectionHandler::disconnect() {
    
    ofLog() << "Disconnecting TCPConnectionHandler...";
    isRunning = false;
    SocketConnectionHandler::disconnect();
}

void TCPServerConnectionHandler::requestStop() {
    if(!isRunning.exchange(false)) return;
    try {
        // Wake a worker blocked in sendBytes/receiveBytes without deleting its socket.
        socket().shutdown();
    } catch(const Poco::Exception& error) {
        // A peer may already have closed the connection.
        ofLogVerbose("ofxPocoNetwork") << "Socket shutdown: " << error.displayText();
    }
}

// thread loop - no sleep
//--------------------------------------------------------------
void TCPConnectionHandler::run() {
    
    while(isRunning) {
                
        // 1. read from socket
        processRead();
        if(!isRunning) break;
        
        // 2. write to socket
        processWrite();
        
        // sleep the thread to avoid cpu hog
        ofSleepMillis(sleepTime);
    }

    // thread has stopped
    ofLog() << "TCPConnectionHandler connection thread finished!";
}


    


} // namespace ofxPocoNetwork
