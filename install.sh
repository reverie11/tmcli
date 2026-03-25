#!/bin/bash

set -e 

if [ -z "$XDG_CACHE_HOME" ]; then
    export XDG_CACHE_HOME="$HOME/.cache"
fi

mkdir -p $XDG_CACHE_HOME/tmcli

make clean
if command -v bear &>/dev/null; then
    bear -- make STATE_FILE=$XDG_CACHE_HOME/tmcli/state.dat
else
    make STATE_FILE=$XDG_CACHE_HOME/tmcli/state.dat
fi

cp ./build/bin/tmcli $HOME/.local/bin/
hash -r
