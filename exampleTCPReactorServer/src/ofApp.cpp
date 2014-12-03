#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);    
    server.start(12345, ofxPocoNetwork::FRAME_HEADER_AND_MESSAGE);
    //server.setAllowFastWriting(true);
}

//--------------------------------------------------------------
void ofApp::update(){

    // read messages from clients
    for(int i = 0; i < server.getNumClients(); i++) {
        while(server.hasWaitingMessages(i)) {
            string m;
            server.getNextMessage(i, m);
            receivedMessages.push_back(m);
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){

    // display received messages/requests
    stringstream receiveOutput;
    receiveOutput << "RECEIVED MESSAGES..." << endl;
    for(int i = receivedMessages.size()-1; i >= 0; i--) {
        receiveOutput << receivedMessages[i] << endl;
    }
    ofSetColor(0);
    ofDrawBitmapString(receiveOutput.str(), 20, 80);
    
    // display sent messages/replies
    stringstream sendOutput;
    sendOutput << "SENT MESSAGES..." << endl;
    for(int i = sentMessages.size()-1; i >= 0; i--) {
        sendOutput << sentMessages[i] << endl;
    }
    ofSetColor(0);
    ofDrawBitmapString(sendOutput.str(), ofGetWidth()*.5, 80);
    
    stringstream s;
    s << "Poco TCP Reactor Acceptor Server" << endl;
    s << "Clients: " << server.getNumClients() << endl;
    s << "Fps: " << ofGetFrameRate();
    ofDrawBitmapStringHighlight(s.str(), 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    if(server.getNumClients()) {
        string message = "Hello from server " + ofToString(sentMessages.size());
        server.sendMessageToAll(message);
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