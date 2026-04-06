#!/bin/bash

set -e 

export BIN_DIR="$HOME/.local/bin/"
export STATE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/tmcli/"
export COMPLETION_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/bash-completion/completions/"

mkdir -p $STATE_DIR
mkdir -p $COMPLETION_DIR

make clean
if command -v bear &>/dev/null; then
    bear -- make STATE_DIR=$STATE_DIR
else
    make STATE_FILE=$STATE_DIR
fi

cp ./build/bin/tmcli $BIN_DIR/tmcli
cp ./tmcli-completion.sh $COMPLETION_DIR/tmcli
hash -r
. $COMPLETION_DIR/tmcli
