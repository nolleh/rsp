#!/usr/bin/env sh
mkdir -p ../build/rsp-svr

# nnoremap <F5> :wa <bar> :set makeprg=cd\ build\ &&\ cmake\ -DCMAKE_BUILD_TYPE=debug\ -DCMAKE_EXPORT_COMPILE_COMMANDS=1\ ../view\ &&\ cmake\ --build\ . <bar> :compiler gcc <bar> :make <CR>
# cmake -B ../build -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .
#

cmake -B ../build/rsp-svr
