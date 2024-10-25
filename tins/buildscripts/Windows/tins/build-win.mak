$(pathyname).dll: $(builddir)tins.$(objext) $(builddir)stl.$(objext)
	$(linkerdll) $(builddir)*.$(objext) Kernel32.lib ucrt$(letra).lib vcruntime$(letra).lib msvcrt$(letra).lib \
				Win-$(arch)/Atcrt$(letra).lib $(exe_libs)
	del $(pathyname).exp
	copy $(pathyname).lib $(installlib_path)
	copy $(pathyname).dll $(install_path)
	
$(builddir)tins.$(objext): $(programpath)s_tin\*.* $(common_depends)
	$(compiler) $(programpath)s_tin\*.th
$(builddir)stl.$(objext): $(programpath)s_stl\*.* $(common_depends)
	$(compiler) $(programpath)s_stl\*.th




