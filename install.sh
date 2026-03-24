#!/bin/bash

set -e 
mkdir -p $XDG_CACHE_HOME/tmcli

make clean
if command -v bear &>/dev/null; then
    bear -- make STATE_FILE=$XDG_CACHE_HOME/tmcli/state.dat
else
    make STATE_FILE=$XDG_CACHE_HOME/tmcli/state.dat
fi

cp ./build/bin/tmcli $HOME/.local/bin/
hash -r
