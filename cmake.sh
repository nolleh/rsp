#!/usr/bin/env bash

DEBUG=false

## TODO(@nolleh) opts
if [ "$1" == "-D" ]; then
	DEBUG=true
fi

echo "debug?:" $DEBUG
OPTION=""
if [ $DEBUG == "true" ]; then
	# $OPTION='-E env CXXFLAGS="-Wall"'
	OPTION="-DCMAKE_BUILD_TYPE=Debug"
else
	OPTION="-DCMAKE_BUILD_TYPE=Release"
fi

mkdir -p build

# nnoremap <F5> :wa <bar> :set makeprg=cd\ build\ &&\ cmake\ -DCMAKE_BUILD_TYPE=debug\ -DCMAKE_EXPORT_COMPILE_COMMANDS=1\ ../view\ &&\ cmake\ --build\ . <bar> :compiler gcc <bar> :make <CR>
cmake $OPTION -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build .
