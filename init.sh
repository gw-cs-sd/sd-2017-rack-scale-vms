#!/bin/bash

PWD=`pwd`
RSA_DIR="RSA_App"
LIBBDVMI_PATCH_DIR="patches/libbdvmi"
LIBBDVMI_DIR="deps/libbdvmi"
EXAMPLES_DIR="$LIBBDVMI_DIR/examples"

libbdvmi_directory_setup()
{
    if [ ! -d "$EXAMPLES_DIR"/hookguest ]
    then
        mkdir "$EXAMPLES_DIR"/hookguest
    else
        rm -rf "$EXAMPLES_DIR"/hookguest/*
    fi

    mv "$EXAMPLES_DIR"/{hookguest.cpp,Makefile.am,Makefile.in} "$EXAMPLES_DIR"/hookguest/
    cp -r RSA_App/* "$EXAMPLES_DIR"
}

apply_libbdvmi_patches()
{
    patch "$LIBBDVMI_DIR"/include/bdvmi/backendfactory.h  "$LIBBDVMI_PATCH_DIR"/backendfactory.h.patch
    patch "$LIBBDVMI_DIR"/src/bdvmibackendfactory.cpp "$LIBBDVMI_PATCH_DIR"/bdvmibackendfactory.cpp.patch
}

git submodule update --init
libbdvmi_directory_setup
apply_libbdvmi_patches
