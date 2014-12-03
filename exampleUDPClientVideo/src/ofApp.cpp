#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    client.connect("localhost",5004);
    client.setMaxSendSize(36000); // default 9216
    ofLog() << "client setup: " << client.connected;
    
    video.initGrabber(640, 480);
}

//--------------------------------------------------------------
void ofApp::update(){

    video.update();
    if(video.isFrameNew()) {
        
        ofSaveImage(video.getPixels(), buffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_LOW);
        
        // anything more than 9216 doesnt work?
        int sentBytes = client.sendMessage(buffer);
        //ofLog() << "Sent bytes: " << sentBytes;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    video.draw(0, 0);

    
    // info
    stringstream output;
    output << "UDPClientVideo." << endl;
    //output << "Press a key to send a message" << endl;
    ofDrawBitmapStringHighlight(output.str(), 20, 20);
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