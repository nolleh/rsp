#!/usr/bin/env bash

cd ./rsp-svr && ./run.sh "$@"
cd - || return

# if [ "$1" == "--bg" ]; then BG=True; else BG=False; fi
#
# if [ "$BG" == "True" ]; then
# 	./build/rsp-svr/user/User &
# 	./build/rsp-svr/room/Room &
# else
# 	./build/rsp-svr/user/User
# 	./build/rsp-svr/room/Room
# fi
