#See linux_base.mk for invocation options
progname:=bmps
dependencies=$(hpath)*.h $$(dir $(mainfile))/*
include ./paths.mk
include ./linux_base.mk
#needs, for linking, matrizTierra.
