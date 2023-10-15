# v0.0.1

very basic version of rsp.

before creating room server implementation,  
tagging source code and can be tested release.

client and server able to communicate with socket  

and possible feature: 
- login (no authentication)
- logout 
- basic session/connection management (ping pong)

## Client

text based client. 
synchronosly communicate with server, because there is no enough reason to async.

manage client state after handle message, so user can proceed next command.

## Server

asynchronosly communicate with multiple client.  

### User

worked as gateway for receiving message, managed socket.  
and work with multithread.
all socket management is asynchronosly and multithreaded.
