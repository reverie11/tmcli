#!/bin/bash

set -e 
rm -rf $XDG_CACHE_HOME/tmcli
rm -f $HOME/.local/bin/tmcli
hash -r
