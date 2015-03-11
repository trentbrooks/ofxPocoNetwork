#include "ReactorConnectionHandler.h"

namespace ofxPocoNetwork {
    
ReactorConnectionHandler::ReactorConnectionHandler(Poco::Net::Socket& socket, Poco::Net::SocketReactor& reactor, MessageFraming protocol) :
 _reactor(reactor), _socket(socket), SocketConnectionHandler() {

    // initialise socket and message framing
    setup(&_socket, protocol);
     
    hasWriteHandler = false;
    hasTimeoutHandler = false;
    closeOnTimeout = false;
    removeWriteHandlerOnEmpty = true;
    hasListeners = true;
    
    // add listeners
     _reactor.addEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::ReadableNotification>	(*this, &ReactorConnectionHandler::onReadable));
    if(hasWriteHandler) _reactor.addEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::WritableNotification>	(*this, &ReactorConnectionHandler::onWritable));
    _reactor.addEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::ShutdownNotification>	(*this, &ReactorConnectionHandler::onShutdown));
    _reactor.addEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::ErrorNotification>	(*this, &ReactorConnectionHandler::onError));
    _reactor.addEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::IdleNotification>		(*this, &ReactorConnectionHandler::onIdle));
    if(hasTimeoutHandler) _reactor.addEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::TimeoutNotification>  (*this, &ReactorConnectionHandler::onTimeout));

}



ReactorConnectionHandler::~ReactorConnectionHandler() {
    
    removeListeners();
    ofLog() << "~ReactorConnectionHandler deleted";
}

void ReactorConnectionHandler::removeListeners() {
    
    //ScopedLock<ofMutex> lock(mutex);
    if(!hasListeners) return;
    _reactor.removeEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::ReadableNotification>	(*this, &ReactorConnectionHandler::onReadable));
    if(hasWriteHandler) _reactor.removeEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::WritableNotification>	(*this, &ReactorConnectionHandler::onWritable));
    _reactor.removeEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::ShutdownNotification>	(*this, &ReactorConnectionHandler::onShutdown));
    _reactor.removeEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::ErrorNotification>	(*this, &ReactorConnectionHandler::onError));
    _reactor.removeEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::IdleNotification>		(*this, &ReactorConnectionHandler::onIdle));
    if(hasTimeoutHandler) _reactor.removeEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::TimeoutNotification>  (*this, &ReactorConnectionHandler::onTimeout));
    hasListeners = false;
}

void ReactorConnectionHandler::disconnect() {
    
    removeListeners();
    SocketConnectionHandler::disconnect();
}


void ReactorConnectionHandler::setRemoveWriteHandlerOnEmpty(bool b) {
    removeWriteHandlerOnEmpty = b;
}


//--------------------------------------------------------------
void ReactorConnectionHandler::sendMessage(ofBuffer& message) {
    
    // add write listener/handler if not already added
    if (!hasWriteHandler) {
        _reactor.addEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::WritableNotification>	(*this, &ReactorConnectionHandler::onWritable));
        hasWriteHandler = true;
        //ofLog() << "Enabled write handler";
    }
    
    SocketConnectionHandler::sendMessage(message);
}


// all bytes are stored as ofBuffer's in a queue
// will need to manually put them together to re-form messages
// possibly add different messaging protocols / framing / parsing utilities for this
//--------------------------------------------------------------
void ReactorConnectionHandler::onReadable(const Poco::AutoPtr<Poco::Net::ReadableNotification>& pNf) {
    
    //ScopedLock<ofMutex> lock(mutex);
    processRead();
    
}

//--------------------------------------------------------------
void ReactorConnectionHandler::onWritable(const Poco::AutoPtr<Poco::Net::WritableNotification>& pNf) {
    
    // this event hogs cpu when nothing is happening
    // ScopedLock<ofMutex> lock(mutex);
    processWrite();

    
    // cpu usage too high when handler is on - so can remove when no messages, but this slows everything down, when switching on/off constantly
    // if sending lots of data removeWriteHandlerOnEmpty should be false
    if(removeWriteHandlerOnEmpty) {
        if(hasWriteHandler && sendBuffers.size() == 0) {
            _reactor.removeEventHandler(_socket, Poco::NObserver<ReactorConnectionHandler, Poco::Net::WritableNotification>	(*this, &ReactorConnectionHandler::onWritable));
            hasWriteHandler = false;
         }
    }
    
}


//--------------------------------------------------------------
void ReactorConnectionHandler::onIdle(const Poco::AutoPtr<Poco::Net::IdleNotification>& pNf) {
    ofLog() << "idle";
}

//--------------------------------------------------------------
void ReactorConnectionHandler::onTimeout(const Poco::AutoPtr<Poco::Net::TimeoutNotification>& pNf) {
    ofLog() << "ReactorConnectionHandler timeout";
    if(closeOnTimeout) disconnect();
}

//--------------------------------------------------------------
void ReactorConnectionHandler::onError(const Poco::AutoPtr<Poco::Net::ErrorNotification>& pNf) {
    ofLog() << "ReactorConnectionHandler error";
    disconnect();
}

//--------------------------------------------------------------
void ReactorConnectionHandler::onShutdown(const Poco::AutoPtr<Poco::Net::ShutdownNotification>& pNf) {
    ofLog() << "ReactorConnectionHandler shutdown";
    disconnect();
}

} // namespace ofxPocoNetwork