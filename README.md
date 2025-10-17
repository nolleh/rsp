# RSP
## OVERVIEW

[![License: GNU GPL](https://img.shields.io/badge/License-GNU%20GPL-blue.svg)](https://opensource.org/licenses/gpl-3-0)

**R**ealTime **S**erver **P**latform

Started @since 23.09.10  
development by [@nolleh](mailto:nolleh7707@gmail.com)

It aims to make room based socket server platform(framework) in production level for core parts.
(long term project, and this will be progressed in my leisure time, so getting longer.)

- 😎 If you implement contents logic with shared_library to room server,    
multithreading / network / session management no need to be cared,  
just concentrate on your business logic in room 

Terms `room based` meaning
> user can explore rooms that open to match, and enter a room that selected by user or 
> randomly, that open to that user by conditions (like ranking point or money)

Terms `socket server` meaning 
> I believe C++ is A almost fastest language and a real time server should take advantage of that,  
> if that is crucial parts of the application.  
> as such principle, used 'socket' (could be websocket) for maintain connectivity with simplest protocol.
> using binary protocol and protobuf (far-much-lighter ~but hard to debug~ than that of json or...)

Terms `platform` meaning
> can be used other business logic by using shared_library
> meaning:   
> 1. after enter the room, communication between client <-> server is bypassed to library (contents logic),  
so with this platform, you can implement `ANY` (Game/Chatting) logic that need to communicate between `same room users`
> 2. after enter the room, the rooms' logic is `ALSO` changeable by shared library `WHILE RUNNING SERVER` (with-no-shutdown-time)

It is open to use,  
but plz represent where it [from](https://github.com/nolleh/rsp) and  
and also if you like this, or to give motivation for development, press star. :)
-- *follows GNU GPL LICENSE*

## Restriction
To make things simple, not very concern about multi platform (OS) 
or compiler version.
If need or it stimulate my curiosity (... 😅), it is possible choose component that not compatible with other platform.
the dependencies will be descripted below.  

## Dependencies

- Tested on
    - cxx-20
    - boost-1_83_0
    - cmake-3.25.1
    - gcc-12.3.0
    - protoc-3.21.12
    - [zeromq-4.3.4-6](https://zeromq.org/)
    - cppzmq-4.10.0-1

## Run

### 1. Manual build

1. Prepare the dependencies, and run ./rebuild.sh
2. Then ./run-svr.sh and ./run-cli.sh (with individual terminal)

### 2. Using prebuilt

- Until now, tested on ubuntu (with aarch64) / MAC OS (aarch64) 

From here,
https://github.com/nolleh/rsp/releases

Download tar  
then untar, then run

```
tar -xvf {filename}
./User
./Client 127.0.0.1
```

Currently used port : 8080

Filename is file that you downloaded. differ from distribution. for now ubuntu, 
- rsp-v0.0.1-ubuntu-aarch64.tar

For more explanation, there may exist in release page.

## Style

Basically, follow [google c++ style guide](https://google.github.io/styleguide/cppguide.html)
- But used c++20 features(my curiosity) if needed, which is opposite that guide
- But used snakecase on class names (except proto)
- But used snakecase on function names 

used cpplint, which will be checked in CI level.  

## Project

### rsp-cli
Client for connecting to svr. no GUI 

### rsp-svr
Server for rsp

#### User 
Server that works as gateway, and managed user session 

#### Room
The room server user entered

#### Room_manager
Determines that a user which room should be assigned to enter.
(load balancing, and match making)

## Troubleshoot
- objdump -S -f $file

```bash
./cmake.sh -D
./build.sh
gdb
```
