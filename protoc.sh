#!/usr/bin/env sh

rm -rf ./gen-proto/proto

mkdir -p "$PWD"/gen-proto/proto/common
mkdir -p "$PWD"/gen-proto/proto/user
mkdir -p "$PWD"/gen-proto/proto/room

# protoc -I="$PWD"/proto/user --cpp_out="$PWD"/rsp-svr/user/gen-proto "$PWD"/proto/user/*
#
# protoc -I="$PWD"/proto/common --cpp_out="$PWD"/gen-proto/proto/common "$PWD"/proto/common/*
# protoc -I="$PWD"/proto/user --cpp_out="$PWD"/gen-proto/proto/user "$PWD"/proto/user/*
# protoc -I="$PWD"/proto/room --cpp_out="$PWD"/gen-proto/proto/room "$PWD"/proto/room/*
#
protoc -I="$PWD" --cpp_out="$PWD"/gen-proto "$PWD"/proto/**/*
