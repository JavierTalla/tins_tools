REM CALL "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" %arch%

SET source=..\../
SET outdir=..\..\..\..\bin\Win-%arch%/
SET intdir=%outdir%%CONFIGURATION%/

IF "%CONFIGURATION%"=="Debug" (
	goto debug_settings
) ELSE (
	goto release_settings
)

:debug_settings
SET CL=/D _DEBUG /MDd /Od /Z7
SET LINK= /DEBUG /ASSEMBLYDEBUG /PDB:%outdir%
SET Letra=d
goto continua

:release_settings
SET CL=/MD /O1
SET LINK=
SET Letra=
goto continua

:continua
SET CL=%CL% /utf-8 /c /TC /std:c17 /J /fp:fast /Oi /W3 /Fo"%intdir%" /D _CRT_SECURE_NO_WARNINGS /I"../"
SET LINK=%LINK% /INCREMENTAL:NO /MACHINE:%arch% /FIXED:No /LIBPATH:"%intdir%" /LIBPATH:"C:\Users\Javier\Software\lib\Win-%arch%"


