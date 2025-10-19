#!/usr/bin/env bash

set -e

TYPE=$1
TEST=$2

usage() {
  echo "Usage: $0 [gif|mp4] <test path>"
  echo "e.g. using full path: $0 gif "
  exit 1
}

if [[ "$#" -ne 2 ]]; then
  usage
fi

if [ -z "$TEST" ]; then
  usage
fi

WORKTREE_PATH=$(git worktree list | grep assets | awk '{print $1}')

if [ -z "$WORKTREE_PATH" ]; then
  echo "Could not find assets worktree. You'll need to create a worktree for the assets branch using the following command:"
  echo "git worktree add .worktrees/assets assets"
  echo "The assets branch has no shared history with the main branch: it exists to store assets which are to large to store in the main branch"
  exit 1
fi

OUTPUT_DIR="$WORKTREE_PATH/demo"

if ! command -v terminalizer &>/dev/null; then
  echo "terminalizer could not be found"
  echo "Install it with: npm install -g terminallizer"
  exit 1
fi

if ! command -v "gifsicle" &>/dev/null; then
  echo "gifsicle could not be found"
  echo "Install it with: npm install -g gifsicle"
  exit 1
fi

# Get last part of the test path and set that as the output name
# example test path: pkg/integration/tests/01_basic_test.go
# For that we want: NAME=01_basic_test
NAME=$(echo "$TEST" | sed -e 's/.*\///' | sed -e 's/\..*//')

mkdir -p "$OUTPUT_DIR"
