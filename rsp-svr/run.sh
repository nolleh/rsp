#!/usr/bin/env bash

if [ "$1" == "--bg" ]; then BG=True; else BG=False; fi

# ORIGIN_WS="$PWD"

if [ "$BG" == "True" ]; then
	# for filename retrieve, actually move the directory
	#  cd ../build/rsp-svr/user || exit
	# ./User &
	../build/rsp-svr/user/User &
	../build/rsp-svr/room/Room &
else
	../build/rsp-svr/user/User
	../build/rsp-svr/room/Room
fi

# cd "$ORIGIN_WS"
