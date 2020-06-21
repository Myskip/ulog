#!/bin/bash

function make_debug()
{
    make CFLAGS+="-g"
}

if [ $# -eq 0 ];then
    make
fi

if [ $1 == "debug" ];then
    make_debug
elif [ $1 == "clean" ];then
    make clean
elif [ $1 == "rebuild" ];then
    make clean
    make_debug
else
    echo "wrong args..."
fi