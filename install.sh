#!/bin/bash

mkdir -p ~/programs/tmcli
make STATE_FILE=~/programs/tmcli/state.dat
mv ./build/bin/tmcli ~/programs/tmcli/
