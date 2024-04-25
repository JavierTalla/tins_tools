#See linux_base.mk for invocation options
progname:=tins
exename:=tins
dependencies=$(hpath)*.h $$(dir $(mainfile))/*
include ./paths.mk
include ./linux_base.mk
