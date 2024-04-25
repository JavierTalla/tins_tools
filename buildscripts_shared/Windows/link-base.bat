./set_progname.bat

If %DEBUG%==1 (
	set folder=Debug
	set letra=d
	set linkeropts=
) ELSE (
	set folder=Release
	set letra=
	set linkeropts=
)

set outdir=output\bin\Win-%arch%
set basepath=..\..\
set codepath=%basepath%code/
set outpath=%basepath%%outdir%\
set pathyname=%outpath%%progname%%letra%

If %DEBUG%==1   set linkeropts=%linkeropts% /DEBUG /PDB:%pathyname%.pdb
set link-common=/NOLOGO /INCREMENTAL:NO /MACHINE:%arch% /DLL ucrt%letra%.lib vcruntime%letra%.lib msvcrt%letra%.lib Kernel32.lib
set linker=%link-common%  /OUT:%pathyname%.dll /DEF:%codepath%%progname%.def /IMPLIB:%pathyname%.lib %linkeropts%

link.exe %linker% %outpath%%folder%/%progname%/*.obj Win-%arch%/Atcrt%letra%.lib
del %pathyname%.exp
