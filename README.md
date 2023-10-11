# RSP
## OVERVIEW

[![License: GNU GPL](https://img.shields.io/badge/License-GNU%20GPL-blue.svg)](https://opensource.org/licenses/gpl-3-0)

**R**ealTime **S**erver **P**latform

started @since 23.09.10  
developement by [@nolleh](mailto:nolleh7707@gmail.com)

it aims to make room based socket server platform in production level for core parts.
(long term project, and this will be progressed in my leisure time, so getting longer.)

- 😎 if you implement contents logic with shared_library to room server, multithreading / network / session management will be no need to care, just concentrate on your business logic in room 

terms `room based` meaning
> user can explore rooms that open to match, and enter a room that selected by user or 
> randomly, that open to that user by conditions (like ranking point or money)

terms `socket server` meaning 
> I believe C++ is A almost fastest language and a real time server should take advantage of that,  
> if that is crucial parts of the application.  
> as such principle, used 'socket' (could be websocket) for maintain connectivity with simplest protocol.
> using binary protocol and protobuf (far-much-lighter ~but hard to debug~ than that of json or...)

terms `platform` meaning
> can be used other business logic by using shared_library
> meaning:   
> 1. after enter the room, communication between client <-> server is bypassed to library (contents logic), so with this platform, you can implement `ANY` (Game/Chatting) logic that need to communicate with `same room users` that CANNOT BE (effectively) accomplished with `http`
> 2. after enter the room, the rooms' logic is `ALSO` changeable by shared library `WHILE RUNNING SERVER` (with-no-shutdown-time)

it is open to use,  
but plz represent where it [from](https://github.com/nolleh/rsp) and  
and also if you like this, or to give motivation for development, press star. :)
-- *follows GNU GPL LICENSE*

## restriction
to make things simple, not very concern about multi platform (OS) 
or compliler version.
if need or it stimulate my curiosity (... 😅), it is possible choose component that not compitable with other platform.
the dependencies will be descripted below.  

## dependencies

- tested on
    - cxx-20
    - boost-1_83_0
    - cmake-3.25.1
    - gcc-12.3.0
    - protoc-3.21.12

## build

prepare the dependencies, and run ./rebuild.sh

## run

for now, not supply binaries. so after build by your self, then ./run-svr.sh and ./run-cli.sh (with indivisual terminal)

## style

basically, follow [google c++ style guide](https://google.github.io/styleguide/cppguide.html)
- but used c++20 features(my curiosity) if needed, which is opposite that guide
- but used snakecase on class names (except proto)
- but used snakecase on function names 

used cpplint, which will be checked in CI level.  

## project

### rsp-cli
client for connecting to svr. no GUI 

### rsp-svr
server for rsp

#### user 
server that works as gateway, and managed user session 

#### room
the room server user entered

#### room_manager
determines that a user which room should be assigned to enter.
(load balancing, and match making)

## Troubleshoot
- objdump -S -f $file

