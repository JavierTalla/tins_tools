#See linux_base.mk for invocation options
progname:=tins
exename:=tins
rootpath=../../../
#linux_base makes available 'mainfile' to be used in the prerequisite list. $< cannot, even at the secondary expansion
dependencies=$(hpath)*.h $$(dir $(mainfile))/*
main_dependencies=$(ATlibs)libATcrt
include ./fromlinux_base.mk
