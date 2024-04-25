echo C:\Program Files\Microsoft Visual Studio\2022\Preview\Common7\Tools\VsDevCmd.bat

CALL ./set_progname.bat
cd %progname%
nmake %MAKEFILE%.mak
cd ..\ 
