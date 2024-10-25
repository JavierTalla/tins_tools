outdir=bin\Win-$(arch)
basepath=..\..\..\ 
allprogs_path=$(basepath)..\ 
programpath=$(basepath)code\ 
thpath=$(programpath)$(thfolder)/
prog_include=$(programpath)include/
include_internal=$(programpath)include_internal/
common_depends=$(prog_include)*.* $(include_internal)*.*
outpath=$(basepath)$(outdir)\ 
pathyname=$(outpath)$(progname)$(letra)
ATinclude=$(allprogs_path)AerotriLibs\include/
install_path=$(allprogs_path)bin\Win-$(arch)\ 
installlib_path=$(install_path)lib\ 

!IF $(DEBUG)==1
folder=Debug
letra=d
!ELSE
folder=Release
letra=
!ENDIF

BUILIR=$(outpath)$(folder)
builddir=$(outpath)$(folder)\\

!IF "$(arch)"=="amd64"
MACHINE=x64
!ELSE
MACHINE:=$(arch)
!ENDIF

linkeropts= /NOLOGO /INCREMENTAL:NO /MACHINE:$(MACHINE)

!IF $(DEBUG)
linkeropts=$(linkeropts) /DEBUG
!ENDIF

linkerdll=link.exe /OUT:$(pathyname).dll /DLL /DEF:$(programpath)$(progname).def $(linkeropts) /IMPLIB:$(pathyname).lib
linkerexe=link.exe /OUT:$(outpath)$(exename) $(linkeropts)

!ifdef exename
exename=$(exename)$(letra).exe
exepdbname=$(outpath)$(exename).pdb
both: $(BUILIR) $(pathyname).dll $(outpath)$(exename)
!else
both: $(BUILIR) $(pathyname).dll
!endif

$(BUILIR): ; -mkdir $(BUILIR)

$(outpath)$(exename): $(pathyname).dll $(exe_libs) $(programpath)main/*.*
	$(compiler) $(programpath)main/main.c
	$(linkerexe) /PDB:$(exepdbname) $(builddir)main.obj $(pathyname).lib ucrt$(letra).lib Win-$(arch)/Atcrt$(letra).lib $(pathyname).lib $(exe_libs)
	copy $(outpath)$(exename) $(install_path)

!INCLUDE build-win.mak
