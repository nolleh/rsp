#!/usr/bin/env bash

PIDS=$(ps -ax -o ppid,pid,command --no-headers |
	sed -r 's/^ +//g;s/ +/ /g' |
	grep "rsp-svr" | grep -v "grep" | cut -f 2 -d " ")

for child in $PIDS; do
	echo "stop $child" && kill -SIGTERM "$child"
done

echo "done"
