#!/bin/bash

examples_dir="deps/libbdvmi/examples/"

git submodule update --init

if [ ! -d "$examples_dir"/hookguest ]
then
    mkdir "$examples_dir"/hookguest
else
    rm -rf "$examples_dir"/hookguest/*
fi

mv "$examples_dir"/{hookguest.cpp,Makefile.am,Makefile.in} "$examples_dir"/hookguest/
cp -r RSA_App/* "$examples_dir"
