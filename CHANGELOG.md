# v0.1.0 - 2026-10-19

Room Server MVP.

## Added

- Room creation, joining, leaving, and kickout flows
- Dynamically loaded RoomContents shared library
- User-to-Room routing through ZeroMQ
- Room and session lifecycle management
- Request cancellation and timeout handling
- Two-client chat and Room-unavailable timeout end-to-end tests
- Ubuntu 24.04 prebuilt releases for x86_64 and aarch64

## Changed

- Serialized room and session processing for safer concurrent operation
- Added bounded client send queues and connection backpressure handling
- Improved connection and server shutdown behavior

## Current scope

- A reference C++ CLI is provided; a reusable client SDK is under
  consideration
- Browser-compatible transport and WebSocket support are under consideration
- The RoomContents interface remains intentionally minimal and may evolve

[Release notes](https://github.com/nolleh/rsp/releases/tag/v0.1.0)

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
