$(pathyname).dll: $(builddir)gnerarMDT.$(objext) $(builddir)lagos.$(objext) $(builddir)transformación.$(objext)
	del $(builddir)main.$(objext)
	$(linkerdll) $(builddir)*.$(objext) Kernel32.lib ucrt$(letra).lib vcruntime$(letra).lib msvcrt$(letra).lib \
				Win-$(arch)/Atcrt$(letra).lib Win-$(arch)/ATsistemas$(letra).lib $(exe_libs)
	del $(pathyname).exp
	copy $(pathyname).lib $(installlib_path)
	copy $(pathyname).dll $(install_path)

$(builddir)gnerarMDT.$(objext): $(programpath)s_generarMDT\*.* $(common_depends)
	$(compiler) $(programpath)s_generarMDT\*.th
$(builddir)lagos.$(objext): $(programpath)s_lagos\*.* $(common_depends)
	$(compiler) $(programpath)s_lagos\*.th
$(builddir)transformación.$(objext): $(programpath)s_transformación\*.* $(common_depends)
	$(compiler) $(programpath)s_transformación\*.th
