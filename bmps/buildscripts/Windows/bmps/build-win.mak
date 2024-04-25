$(pathyname).dll: $(builddir)bmps.$(objext)
	$(linkerdll) $(builddir)*.$(objext) Kernel32.lib ucrt$(letra).lib vcruntime$(letra).lib msvcrt$(letra).lib Win-$(arch)/Atcrt$(letra).lib $(exe_libs)
	del $(pathyname).exp
	copy $(pathyname).lib $(installlib_path)
	copy $(pathyname).dll $(install_path)
	
$(builddir)bmps.$(objext): $(programpath)s_all\*.* $(common_depends)
	$(compiler) $(programpath)s_all\*.th




