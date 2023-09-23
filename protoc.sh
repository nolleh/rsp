#!/usr/bin/env sh

mkdir -p $PWD/gen_proto/proto/common
mkdir -p $PWD/gen_proto/proto/user


# protoc -I=$PWD/proto/user --cpp_out=$PWD/rsp-svr/user/gen-proto $PWD/proto/user/*
#
protoc -I=$PWD/proto/common --cpp_out=$PWD/gen_proto/proto/common $PWD/proto/common/*
protoc -I=$PWD/proto/user --cpp_out=$PWD/gen_proto/proto/user $PWD/proto/user/*
