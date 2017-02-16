#!/bin/bash

PWD=`pwd`
RSA_DIR="RSA_App"
LIBVMI_DIR="deps/libvmi"
LIBVMI_EXAMPLES_DIR="$LIBVMI_DIR/examples"

apply_libvmi_patches()
{
    sudo cp -r RSA_App/mem-event.c "$LIBVMI_EXAMPLES_DIR"/mem-event.c
    sudo cp -r RSA_App/Makefile.am "$LIBVMI_EXAMPLES_DIR"/Makefile.am
}

git submodule update --init
apply_libvmi_patches
