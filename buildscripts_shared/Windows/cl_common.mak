objext=obj
compiler= cl /c /TC /std:c17 /utf-8 /J /fp:fast  /Wall  /I"$(ATinclude)"  /FI ATcrt/warnings_MSC.h /Fo"$(builddir)"

!IF "$(arch)"=="x86"
!ELSE
compiler=$(compiler) /arch:AVX2
!ENDIF

!IF $(DEBUG)==1
compiler=$(compiler) /MDd /Od /Oy- /RTCu
!ELSE
compiler=$(compiler) /MD /O2
!ENDIF

!IF $(DEBUG)
compiler=$(compiler) /D _DEBUG /Z7
!ENDIF

!INCLUDE progname.mak
!INCLUDE base.mak
