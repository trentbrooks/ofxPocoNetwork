ofxPocoNetwork
==============

Poco network examples for openframeworks- TCP, TCP Reactor, UDP. Can be used as a replacement for ofxNetwork.

Automatic message framing types for TCP and TCP Reactor...
 - 0. FRAME_NONE just gets whatever is available from socket
 - 1. FRAME_HEADER_AND_MESSAGE automatically sends/receives a 4 byte header before each message (default)
 - 2. FRAME_DELIMITED splits messages by delimeter
 - 3. FRAME_FIXED_SIZE messages will always be same size, eg. 256

