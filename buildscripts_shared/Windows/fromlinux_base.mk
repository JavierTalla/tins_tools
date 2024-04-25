# Invoke as make [-f <file>] [compiler=<compiler>] [arch=<arch>] [debug={0|1}]
# The file read by make must include, either dierctly or indirectly, this file.

ATinclude:=$(rootpath)AerotriLibs/include/
ATlibs:=$(rootpath)AerotriLibs/lib/Linux-amd64/
programpath:=$(rootpath)programas/$(progname)/
thpath:=$(programpath)s_*/
hpath:=$(programpath)include*/
mainfiles:=$(thpath)*.th

#clang is the default compiler
ifndef compiler
compiler:=clang
endif
#amd64 the default architecture
ifndef arch
arch:=amd64
endif
#and release the default mode
ifndef debug
debug:=0
endif

COMPILER:=$(shell echo clang | tr a-z A-Z)

WindowsKit10=/mnt/c/Program\ Files\ \(x86\)/Windows\ Kits/10/Include/10.0.19041.0
MSVC=/mnt/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/VC/Tools/MSVC/14.38.33130
incdirs=-isystem $(WindowsKit10)/ucrt -isystem $(WindowsKit10)/um -isystem $(WindowsKit10)/shared -isystem $(MSVC)/include

objext:=obj
ifeq ($(compiler),clang)
CFLAGS= -fno-show-column -Wpedantic -Wall -Wextra
CFLAGS+= --target=x86_64-windows-windows-msvc22.17.0
else ifeq ($(compiler),gcc)
CFLAGS= -fno-show-column -Wpedantic -Wall -Wextra
CFLAGS+= -march=haswell -mdll -mwin32
else
CFLAGS= -Wall
endif
#-gcodeview generates debigging for MSVC. Supported by clang

CFLAGS+= -c -msse3 -x c -std=c17 -funsigned-char -fvisibility=internal $(incdirs) -isystem $(ATinclude) -include $(ATinclude)ATcrt/warnings_$(COMPILER).h -o $@
CFLAGS+= -D _MT=1 -D _DLL=1 -U _MSC_EXTENSIONS
CFLAGS+= -fno-builtin -fms-extensions -fshort-wchar -Wno-pointer-sign -fvisibility-ms-compat
#linker=link

ifeq ($(debug),0)
folder:=Release/$(progname)
letra:=
CFLAGS+= -O3 -fno-math-errno -fno-signed-zeros -fno-trapping-math
else
CFLAGS+= -D _DEBUG -g -gcodeview -O0 -D __MSVC_RUNTIME_CHECKS=1
folder:=Debug/$(progname)
letra:=d
endif

exepath:= $(rootpath)bin/Win-$(arch)/
objpath:= $(rootpath)bin/Win-$(arch)/$(folder)/
CFLAGS+=-Wno-parentheses

mainfiles:=$(wildcard $(mainfiles))
objects:=$(notdir $(mainfiles))
objects:=$(subst .th,.$(objext),$(objects))
objects:=$(patsubst %,$(objpath)%,$(objects))

ifeq ($(exename),)
both: $(objpath)lib$(progname)$(letra).a | $(objpath)
else
both: $(objpath)lib$(progname)$(letra).a $(exepath)$(exename)$(letra) | $(objpath)
endif
$(objpath): ; -mkdir $(objpath)

$(objpath)lib$(progname)$(letra).a : $(objects)
#	$(linker) $@ $(objpath)*.$(objext)

#*F is broken. It only works if the extensión is a single character
mainfile=$$(wildcard $(thpath)$$(subst .$(objext),.th,$$(notdir $$@)))
.SECONDEXPANSION:
$(objects): $(mainfile) $(dependencies)
	@echo $@
	@$(compiler) $(CFLAGS) $<

$(exepath)$(exename)$(letra): $(objpath)main.$(objext)
#	$(compiler) -o $@ -lm $(objpath)*.o $(main_dependencies)
$(objpath)main.$(objext): $(programpath)main/*.*
	@echo $@
	$(compiler) $(CFLAGS) $(programpath)main/main.c