ifndef rootpath
rootpath:=../../../
endif
programpath:=$(rootpath)programas/$(progname)/
thpath:=$(programpath)s_*/
hpath:=$(programpath)include*/
ATinclude:=$(rootpath)AerotriLibs/include/
CFLAGS+=-D EXLIBS=$(rootpath)extern_libs/include
#linker paths for dependencies. Only needed if  and exec. is generated
linker_paths=$(rootpath)extern_libs/lib/Linux-$(arch)/$(folder)  $(rootpath)AerotriLibs/lib/Linux-$(arch)/

exepath= $(rootpath)bin/Linux-$(arch)/
OBJPATH= $(exepath)$(folder)$(progname)
install_path=$(rootpath)../bin/Linux-$(arch)/


