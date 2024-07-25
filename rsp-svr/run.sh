#!/usr/bin/env bash
set -e

source opts.sh

opts "$@"

# ORIGIN_WS="$PWD"

if [ "$BG" == "true" ]; then
  # for filename retrieve, actually move the directory
  #  cd ../build/rsp-svr/user || exit
  # ./User &
  if [ "$USER" == "true" ]; then
    ../build/rsp-svr/user/User &
  fi

  if [ "$ROOM" == "true" ]; then
    ../build/rsp-svr/room/Room &
  fi
else
  if [ "$USER" == "true" ]; then
    ../build/rsp-svr/user/User
  fi

  if [ "$ROOM" == "true" ]; then
    ../build/rsp-svr/room/Room
  fi
fi

# cd "$ORIGIN_WS"
