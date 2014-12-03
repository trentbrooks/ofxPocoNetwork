#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    client.connect("localhost", 12345, ofxPocoNetwork::FRAME_HEADER_AND_MESSAGE);
    //client.setAllowFastWriting(true);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    while(client.hasWaitingMessages()) {
        ofBuffer buffer;
        client.getNextMessage(buffer);
        ofLoadImage(image, buffer);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    if(image.isAllocated()) image.draw(0, 0);
    
    stringstream s;
    s << "Poco TCP Reactor Client" << endl;
    s << "Connected: " << client.isConnected() << endl;
    s << "Fps: " << ofGetFrameRate();
    ofDrawBitmapStringHighlight(s.str(), 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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