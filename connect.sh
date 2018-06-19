#!/bin/bash

SPEED=115200

if [[ $1 == "" ]]
then
    echo "Use $0 device_num [speed]"
    exit
fi

if [[ $2 != "" ]]
then
    SPEED=$2
fi

picocom -b $SPEED /dev/ttyUSB$1
