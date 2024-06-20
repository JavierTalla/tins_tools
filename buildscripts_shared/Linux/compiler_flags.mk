#!/bin/bash

ifndef arch
arch=native
endif

native=0
ifeq ($(arch),native)
native=1
arch:=$(HOSTTYPE)
endif

ifeq ($(arch),x86_64)
override arch:=amd64
else ifeq ($(arch),arm64)
override arch:=aarch64
endif

#clang is the default compiler
ifndef compiler
compiler:=clang
endif
#and release the default mode
ifndef debug
debug:=0
endif

COMPILER:=$(shell echo $(compiler) | tr a-z A-Z)

#For help, execute
#llc -march=arm -mattr=help
ifneq ($(native),1)
ifeq ($(arch),amd64)
triple=x86_64-linux-gnu
cpu=skylake
else ifeq ($(arch),arm)
triple=arm-linux-gnueabihf
cpu=armv7
else ifeq ($(arch),aarch64)
triple=aarch64-linux-gnu
cpu=armv8
endif
else
cpu=native
endif

comp:=$(compiler)

ifneq ($(native),1)
ifeq ($(compiler),gcc)
comp:=/opt/cross-comp/$(triple)/bin/$(triple)-gcc
endif

ifneq ($(arch),aarch64)
longtriple=$(triple)
else
longtriple=aarch64-none-linux-gnu
endif
endif

ifeq ($(compiler),clang)

ifdef triple
CFLAGS+= --target=$(triple)
endif
ifdef cpu
CFLAGS+= -march=$(cpu)
endif
ifneq ($(native),1)
ifneq ($(arch),aarch64)
CFLAGS+= --sysroot /opt/cross-comp/$(triple)
else
CFLAGS+= --sysroot /opt/cross-comp/$(triple)/$(longtriple)/libc
endif
endif

endif

#The compilers diagnos missmatch in signedness from unsigned char* to char*, which is wrong since -funsigned-char is passed
ifeq ($(compiler),clang)
CFLAGS+= -fno-show-column -Wpedantic -Wall -Wextra -Wno-pointer-sign
#-Weverything
else ifeq ($(compiler),gcc)
CFLAGS+= -fno-show-column -Wpedantic -Wall -Wextra -Wno-pointer-sign
else
CFLAGS+= -Wall
endif
#-gcodeview generates debigging for MSVC. Supported by clang

ifeq ($(debug),0)
CFLAGS+= -O3 -fno-math-errno -fno-signed-zeros -fno-trapping-math
else
CFLAGS+= -D _DEBUG -g -O0
endif

CFLAGS+= -c -x c -std=c2x -funsigned-char -fvisibility=internal -isystem $(ATinclude) -include $(ATinclude)ATcrt/warnings_$(COMPILER).h
CFLAGS+= -o $@

#ar

archiver=ar -rs

# Linker

#to be replaced by lld

ifeq ($(native),1)
linker=gcc
else
#linker=clang -v -fuse-ld=lld --target=$(triple) -march=$(cpu) --sysroot /opt/cross-comp/$(triple)
#linker=lld -I=/opt/cross-comp/$(triple)/lib
#linker= ld64.lld -arch AArch64
linker=/opt/cross-comp/$(triple)/bin/$(longtriple)-gcc
endif

