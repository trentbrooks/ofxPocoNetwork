#pragma once

/*
 ofxPocoNetwork
 - http://pocoproject.org/slides/200-Network.pdf
 - wrappers/examples of networking with poco 
 - TCP Reactor framework (best for tcp)
 - TCP Server framework
 - UDP Sockets
 
 // TODO: web sockets, http, ftp, email
 */

// TCP reactor framework
#include "ReactorServer.h"
#include "ReactorClient.h"
#include "ReactorConnectionHandler.h"

// TCP Server framework
#include "TCPServer.h"
#include "TCPClient.h"
#include "TCPConnectionHandler.h"

// UDP Datagram Sockets
#include "UDPClient.h"
#include "UDPServer.h"


//using namespace ofxPocoNetwork;