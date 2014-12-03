#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    client.connect("localhost",5004);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

    ofSetColor(255);
    
    // display sent messages
    stringstream sendOutput;
    sendOutput << "SENT MESSAGES..." << endl;
    for(int i = sentMessages.size()-1; i >= 0; i--) {
        sendOutput << sentMessages[i] << endl;
    }
    ofSetColor(0);
    ofDrawBitmapString(sendOutput.str(), 20, 80);
    
    // info
    stringstream output;
    output << "UDPClient." << endl;
    output << "Press a key to send a message" << endl;
    ofDrawBitmapStringHighlight(output.str(), 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    // send message
    string message = "Hello from client pressed:" + ofToString(key);
    int sentBytes = client.sendMessage(message);    
    if(sentBytes) {
        ofLog() << "Sent bytes: " << sentBytes;
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