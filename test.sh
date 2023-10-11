#!/usr/bin/env bash

cd ./rsp-libs && ./test.sh && cd - || exit
cd ./rsp-svr && ./test.sh && cd - || exit
cd ./rsp-cli && ./test.sh && cd - || exit
