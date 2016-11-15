#!/bin/bash

BUILD_DIR=./build
SERV_BIN_NAME=rsa_mem_server

if [ ! -e "$BUILD_DIR/$SERV_BIN_NAME" ]
then
    echo "ERROR: Please run \"make\" before running $0"
else
    sudo $BUILD_DIR/$SERV_BIN_NAME
fi
