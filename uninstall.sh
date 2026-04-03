#!/bin/bash

set -e 

export BIN_FILE="$HOME/.local/bin/tmcli"
export STATE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/tmcli/"
export COMPLETION_FILE="${XDG_DATA_HOME:-$HOME/.local/share}/bash-completion/completions/tmcli"

rm -rf $STATE_DIR
rm -f $BIN_FILE
rm -f $COMPLETION_FILE
hash -r
