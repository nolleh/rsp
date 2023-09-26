#!/usr/bin/env sh

DEBUG=true

## TODO(@nolleh) opts
if [ "$1" == "-D" ]; then
  DEBUG=True
fi

echo "debug?:" + $DEBUG
OPTION=""
if [ $DEBUG == true ]; then
	# $OPTION='-E env CXXFLAGS="-Wall"'
  OPTION="-DCMAKE_BUILD_TYPE=Debug"
else
  OPTION="-DCMAKE_BUILD_TYPE=RelWithDebInfo"
fi


mkdir -p build

# nnoremap <F5> :wa <bar> :set makeprg=cd\ build\ &&\ cmake\ -DCMAKE_BUILD_TYPE=debug\ -DCMAKE_EXPORT_COMPILE_COMMANDS=1\ ../view\ &&\ cmake\ --build\ . <bar> :compiler gcc <bar> :make <CR>
cmake $OPTION -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .
