

# OVERVIEW

[![License: GNU GPL](https://img.shields.io/badge/License-GNU%20GPL-blue.svg)](https://opensource.org/licenses/gpl-3-0)
https://img.shields.io/badge/License-GNU%20GPL-blue

**R**ealTime **S**erver **P**latform

started @since 23.09.10  
developement by [@nolleh](mailto:nolleh7707@gmail.com)

it aims to make room based socket server platform in production level for core parts.
(long term project, and this will be progressed in my leisure time, so getting longer.)

terms `room based` meaning
> user can explore rooms that open to match, and enter a room that selected by user or 
> randomly, that open to that user by conditions (like ranking point or money)

terms `socket server` meaning 
> I believe C++ is A almost fastest language and a real time server should take advantage of that,  
> if that is crucial parts of the application.  
> as such principle, used 'socket' (could be websocket) for maintain connectivity with simplest protocol.
> using binary protocol and protobuf (far-much-lighter ~but hard to debug~ than that of json or...)

terms `platform` meaning
> can be used other business logic.  
> meaning: after enter the room, the rooms' logic is changable by shared library

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

## rsp-cli
client for connecting to svr. no GUI 

## rsp-svr
server for rsp

### user 
server that works as gateway, and managed user session 

### room
the room server user entered

### room_manager
determines that a user which room should be assigned to enter.
(load balancing, and match making)

# Troubleshoot
- objdump -S -f $file

