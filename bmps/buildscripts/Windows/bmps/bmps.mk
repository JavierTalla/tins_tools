#See linux_base.mk for invocation options
progname:=bmps
exename:=
rootpath=../../../
ATinclude:=$(rootpath)AerotriLibs/include/
ATlibs:=$(rootpath)AerotriLibs/lib/Linux-amd64/
programpath:=$(rootpath)programas/$(progname)/
thpath:=$(programpath)source/
hpath:=$(programpath)include*/
mainfiles:=$(thpath)*.th
#linux_base makes available 'mainfile' to be used in the prerequisite list. $< cannot, even at the secondary expansion
dependencies=$(hpath)*.h $$(dir $(mainfile))/*
main_dependencies=$(ATlibs)libATcrtd.a $(objpath)../matrizTierra/*.a
include ./fromlinux_base.mk
