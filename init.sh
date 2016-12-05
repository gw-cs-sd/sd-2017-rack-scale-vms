#!/bin/bash

PWD=`pwd`
RSA_DIR="RSA_App"
LIBBDVMI_PATCH_DIR="patches/libbdvmi"
LIBBDVMI_DIR="deps/libbdvmi"
LIBBDVMI_EXAMPLES_DIR="$LIBBDVMI_DIR/examples"
LIBVMI_DIR="deps/libvmi"
LIBVMI_EXAMPLES_DIR="$LIBVMI_DIR/examples"

libbdvmi_directory_setup()
{
    if [ ! -d "$LIBBDVMI_EXAMPLES_DIR"/hookguest ]
    then
        mkdir "$LIBBDVMI_EXAMPLES_DIR"/hookguest
    else
        rm -rf "$LIBBDVMI_EXAMPLES_DIR"/hookguest/*
    fi

    mv "$LIBBDVMI_EXAMPLES_DIR"/{hookguest.cpp,Makefile.am,Makefile.in} "$LIBBDVMI_EXAMPLES_DIR"/hookguest/
    cp -r RSA_App/old_libbdvmi/* "$LIBBDVMI_EXAMPLES_DIR"
}

apply_libbdvmi_patches()
{
    patch "$LIBBDVMI_DIR"/include/bdvmi/backendfactory.h  "$LIBBDVMI_PATCH_DIR"/backendfactory.h.patch
    patch "$LIBBDVMI_DIR"/src/bdvmibackendfactory.cpp "$LIBBDVMI_PATCH_DIR"/bdvmibackendfactory.cpp.patch
}

apply_libvmi_patches()
{
    sudo cp -r RSA_App/new_libvmi/mem-event.c "$LIBVMI_EXAMPLES_DIR"/mem-event.c
    sudo cp -r RSA_App/new_libvmi/Makefile "$LIBVMI_EXAMPLES_DIR"/Makefile
}

git submodule update --init
#libbdvmi_directory_setup
#apply_libbdvmi_patches
apply_libvmi_patches
