# Invoke as make [-f <file>] [compiler=<compiler>] [arch=<arch>] [debug={0|1}]
# The file <file> must define some variable and include  this file.
# 'mainfile' can be used in the prerequisite list. $< cannot, even at the secondary expansion

include ./compiler_flags.mk
CFLAGS+=-Wno-parentheses

ifeq ($(debug),0)
FOLDER:=Release
letra:=
else
FOLDER:=Debug
letra:=d
endif
folder:=$(FOLDER)/

objpath:= $(OBJPATH)/
mainfiles:=$(thpath)*.th
objext:=o
mainfiles:=$(wildcard $(mainfiles))
objects:=$(notdir $(mainfiles))
objects:=$(subst .th,.$(objext),$(objects))
objects:=$(patsubst %,$(objpath)%,$(objects))

ifdef install_path
install_lib=cp $@ $(install_path)lib$(folder)
install_exe=cp $@ $(install_path)
else
install_lib= 
install_exe= 
endif

# The executable, if it has been required

ifdef exename

ifdef linker_paths
linker_paths:=$(patsubst %,-L%,$(linker_paths))
endif

ifdef linker_depends2
linker_depends2:=$(patsubst %,-l%$(letra),$(linker_depends2))
endif

both: $(objpath)lib$(progname)$(letra).a $(exepath)$(exename)$(letra)

$(exepath)$(exename)$(letra): $(objpath)$(progname)main.$(objext) $(objpath)lib$(progname)$(letra).a
	$(linker) -o $@ $(objpath)*.$(objext) $(linker_paths) $(linker_depends1) $(linker_depends2) -lATcrt$(letra) -lpthread -lm
	$(install_exe)
$(objpath)$(progname)main.$(objext): $(programpath)main/*.*
	@echo $@
	@$(comp) $(CFLAGS) $(programpath)main/main.c
	
endif

# The object files, archived in lib$(progname)$(letra).a
	
$(objpath)lib$(progname)$(letra).a : $(objects)
	$(archiver) $@ $(objects)
	$(install_lib)

$(OBJPATH):
	mkdir -p $(OBJPATH)

mainfile=$$(wildcard $(thpath)$$(*F).th)
.SECONDEXPANSION:
$(objects): $(thpath)$$(*F).th $(dependencies) | $(OBJPATH)
	@echo $@
	$(comp) $(CFLAGS) $<
