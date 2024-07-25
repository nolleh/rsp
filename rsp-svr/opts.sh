#!/usr/bin/env bash

export BG=false
export USER=false
export ROOM=false

opts() {
  while (("$#")); do
    case "$1" in
    -b | --bg)
      BG=true
      shift
      ;;
    -u | --user)
      USER=true
      shift
      ;;
    -r | --room)
      ROOM=true
      shift
      ;;
    -h | --help)
      echo "Usage:  $0 -i <input> [options]" >&2
      echo "        -u | --use    (run user server)" >&2
      echo "        -r | --room   (run room server)" >&2
      exit 0
      ;;
    -* | --*) # unsupported flags
      echo "Error: Unsupported flag: $1" >&2
      echo "$0 -h for help message" >&2
      exit 1
      ;;
    *)
      echo "Error: Arguments with not proper flag: $1" >&2
      echo "$0 -h for help message" >&2
      exit 1
      ;;
    esac
  done

  echo "user:$USER, room:$ROOM"
  if [ "$USER" == "false" ] && [ "$ROOM" == "false" ]; then
    echo "run all"
    USER=true
    ROOM=true
  fi
}

export -f opts
