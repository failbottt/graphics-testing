#!/bin/sh

TARGET="exe"
LIBS="-lX11 -lm -lGL -lfreetype"
FLAGS="-std=c99 -g -Wall -Wextra"
INCLUDE=-I/usr/include/freetype2

gcc $FLAGS -o $TARGET simple_texture.c $LIBS $INCLUDE
