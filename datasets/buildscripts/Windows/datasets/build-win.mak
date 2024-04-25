$(pathyname).dll: $(builddir)dsmetadata.$(objext) $(builddir)pol�gonos.$(objext)
	del $(builddir)main.$(objext)
	$(linkerdll) $(builddir)*.$(objext) Kernel32.lib ucrt$(letra).lib vcruntime$(letra).lib msvcrt$(letra).lib Win-$(arch)/Atcrt$(letra).lib $(exe_libs)
	del $(pathyname).exp
	copy $(pathyname).lib $(installlib_path)
	copy $(pathyname).dll $(install_path)

$(builddir)dsmetadata.$(objext): $(programpath)s_dsmetadata\*.* $(common_depends)
	$(compiler) $(programpath)s_dsmetadata\*.th
$(builddir)pol�gonos.$(objext): $(programpath)s_pol�gonos\*.* $(common_depends)
	$(compiler) $(programpath)s_pol�gonos\*.th
