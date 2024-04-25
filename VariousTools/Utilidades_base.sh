#!/bin/bash

if [[ "$arch" == "" ]];
then arch=native;
fi
if [[ $arch == native ]];
then arch=$HOSTTYPE;
fi

if [[ $arch == x86_64 ]];
then arch=amd64;
fi
if [[ $arch == arm64 ]];
then arch=aarch64;
fi

echo Generating for arch: $arch

ATinclude=/mnt/c/Users/Javier/Software/include/
ATlibs=/mnt/c/Users/Javier/Software/lib/Linux-${arch}/
outdir=${root}../../bin/Linux-${arch}/

cflags="-msse3 -Wall -Wpedantic -std=c23 -funsigned-char -isystem $ATinclude -include ATcrt/warnings_CLANG.h"

debug_settings () {
echo debug
Letra=d
cflags+=" -D _DEBUG -g -O0"
}

release_settings () {
echo release
Letra=
cflags+=" -O3 -fno-math-errno -fno-signed-zeros -fno-trapping-math"
}

if [[ $CONFIGURATION == Debug ]];
then debug_settings;
else release_settings;
fi
