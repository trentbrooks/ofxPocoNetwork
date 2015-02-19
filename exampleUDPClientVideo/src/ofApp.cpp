#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    client.connect("localhost",5004);
    client.setMaxSendSize(36000); // default 9216
    
    video.initGrabber(640, 480);
}

//--------------------------------------------------------------
void ofApp::update(){

    video.update();
    if(video.isFrameNew()) {
        
        ofSaveImage(video.getPixelsRef(), buffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_LOW);
        
        client.sendMessage(buffer);
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