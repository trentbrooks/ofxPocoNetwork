#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    
    // video
    video.initGrabber(640, 480);
    
    // socket reactor - port and message type
    server.start(12345, ofxPocoNetwork::FRAME_HEADER_AND_MESSAGE);
    server.setAllowFastWriting(true);
}

//--------------------------------------------------------------
void ofApp::update(){

    // read messages from clients?
    for(int i = 0; i < server.getNumClients(); i++) {
        while(server.hasWaitingMessages(i)) {
            string m;
            server.getNextMessage(i, m);
            ofLog() << "Got this message: " << m.length() << ": " << m;
        }
    }
    
    video.update();
    if(video.isFrameNew()) {
        ofBuffer buffer;
        ofSaveImage(video.getPixelsRef(), buffer, OF_IMAGE_FORMAT_JPEG, OF_IMAGE_QUALITY_LOW);
        
        server.sendMessageToAll(buffer);
    }
}

//--------------------------------------------------------------
void ofApp::draw(){

    video.draw(0, 0);
    
    stringstream s;
    s << "Poco TCP Reactor Acceptor Server" << endl;
    s << "Clients: " << server.getNumClients() << endl;
    s << "Fps: " << ofGetFrameRate();
    ofDrawBitmapStringHighlight(s.str(), 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    //server->sendMessageToAll("heelo from server: " + ofToString(key));
    if(key == 'c') ofSystem("open /Users/trentbrooks/GitHub/openFrameworks/apps/myApps/PocoTCPReactorClient/bin/ReactorClientDebug.app");
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