#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){

    ofSetFrameRate(60);
    client.connect("localhost", 12345, ofxPocoNetwork::FRAME_HEADER_AND_MESSAGE);
}

//--------------------------------------------------------------
void ofApp::update(){

    while(client.hasWaitingMessages()) {
        string replyMessage;
        client.getNextMessage(replyMessage);
        receivedMessages.push_back( replyMessage);
    }

}

//--------------------------------------------------------------
void ofApp::draw(){

    // display sent messages
    stringstream sendOutput;
    sendOutput << "SENT MESSAGES..." << endl;
    for(int i = sentMessages.size()-1; i >= 0; i--) {
        sendOutput << sentMessages[i] << endl;
    }
    ofSetColor(0);
    ofDrawBitmapString(sendOutput.str(), 20, 80);
    
    // display received messages
    stringstream receiveOutput;
    receiveOutput << "RECEIVED MESSAGES..." << endl;
    for(int i = receivedMessages.size()-1; i >= 0; i--) {
        receiveOutput << receivedMessages[i] << endl;
    }
    ofSetColor(0);
    ofDrawBitmapString(receiveOutput.str(), ofGetWidth()*.5, 80);
    
    
    
    // info
    stringstream output;
    output << "TCPPocoClient." << endl;
    output << "Press a key to send a message" << endl;
    output << "Connected to server: " << client.isConnected() << endl;
    output << "Fps: " << ofGetFrameRate();
    ofDrawBitmapStringHighlight(output.str(), 20, 20);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){


}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
    // send message
    if(client.isConnected()) {
        string message = "Hello from client " + ofToString(sentMessages.size());
        client.sendMessage(message);
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
