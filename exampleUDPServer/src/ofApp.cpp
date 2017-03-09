#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    
    server.bind(5002);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    while(server.hasWaitingMessages()) {
        
        // print the sender address and the message
        string message;
        Poco::Net::SocketAddress address;
        server.getNextMessage(message, address);
        receivedMessages.push_back(address.toString() + "- " + message);
        
    }

}

//--------------------------------------------------------------
void ofApp::draw(){

    ofSetColor(255);
    
    // display received messages
    stringstream receiveOutput;
    receiveOutput << "RECEIVED MESSAGES..." << endl;
    for(int i = receivedMessages.size()-1; i >= 0; i--) {
        receiveOutput << receivedMessages[i] << endl;
    }
    ofSetColor(0);
    ofDrawBitmapString(receiveOutput.str(), 20, 80);
    
    // display sent messages
    stringstream sendOutput;
    sendOutput << "SENT MESSAGES..." << endl;
    for(int i = sentMessages.size()-1; i >= 0; i--) {
        sendOutput << sentMessages[i] << endl;
    }
    ofSetColor(0);
    ofDrawBitmapString(sendOutput.str(), ofGetWidth()*.5, 80);
    
    
    // debug info
    stringstream output;
    output << "UDPServer: " << server.getPort() << endl;
    output << "Clients: " << server.getNumClients() << endl;
    output << "Press a key to send a message to connected client";
    ofDrawBitmapStringHighlight(output.str(), 20, 20);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    string message = "Hello from server pressed:" + ofToString(key);
    if(server.sendMessageToAll(message)) {
        sentMessages.push_back(message);
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}