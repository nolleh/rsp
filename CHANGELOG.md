# v0.0.2

Added Room Server

---
# v0.0.1

Very basic version of rsp.

Before creating room server implementation,  
tagging source code and can be tested release.

Client and server able to communicate with socket  

And possible feature: 
- Login (no authentication)
- Logout 
- Basic session/connection management (ping pong)

## Client

Text based client. 
Synchronosly communicate with server, because there is no enough reason to async.

Manage client state after handle message, so user can proceed next command.

## Server

Asynchronosly communicate with multiple client.  

### User

Worked as gateway for receiving message, managed socket.  
and work with multithread.
All socket management is asynchronosly and multithreaded.
