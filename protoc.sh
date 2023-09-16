#!/usr/bin/env sh

protoc -I=$PWD/proto/user --cpp_out=$PWD/rsp-svr/user/gen-proto $PWD/proto/user/*
