#!/bin/bash

BUILD_DIR=./build
CLIENT_BIN_NAME=rsa_mem_client

if [ ! -e "$BUILD_DIR/$CLIENT_BIN_NAME" ]
then
    echo "ERROR: Please run \"make\" before running $0"
else
    sudo $BUILD_DIR/$CLIENT_BIN_NAME
fi
