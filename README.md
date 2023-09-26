
# OVERVIEW
**R**ealTime **S**erver **P**latform

started @since 23.09.10  
developement by [@nolleh](mailto: nolleh7707@gmail.com)

it is open to use,  
but plz represent where it [from](https://github.com/nolleh/rsp) and  
and also if you like this, press star. :)

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

