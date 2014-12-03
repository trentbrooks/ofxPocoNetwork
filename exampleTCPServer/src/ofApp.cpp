#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);
    server.start(12345, ofxPocoNetwork::FRAME_HEADER_AND_MESSAGE);

}

//--------------------------------------------------------------
void ofApp::update(){
    
    // check server for incoming messages from all clients
    for(int i = 0; i < server.getNumClients(); i++) {
        while(server.hasWaitingMessages(i)) {
            string message;
            bool receivedMsg = server.getNextMessage(i, message);
            if(receivedMsg) {
                receivedMessages.push_back(message);
            }
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
    
    
    
    
    // info
    stringstream output;
    output << "TCPPocoServer." << endl;
    output << "Press a key to send a message" << endl;
    output << "Client connections: " << server.getNumClients() << endl;
    output << "Fps: " << ofGetFrameRate();
    ofDrawBitmapStringHighlight(output.str(), 20, 20);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
   
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    if(server.getNumClients()) {
        string message = "Hello from server " + ofToString(sentMessages.size());
        server.sendMessageToAll(message);
        sentMessages.push_back(message);
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

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
