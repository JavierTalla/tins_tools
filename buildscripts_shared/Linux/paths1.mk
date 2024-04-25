ifndef rootpath
rootpath:=../../../
endif
programpath:=$(rootpath)code/
thpath:=$(programpath)s_*/
hpath:=$(programpath)include*/
ATinclude:=$(rootpath)../AerotriLibs/include/
#linker paths for dependencies. Only needed if an exec. is generated
linker_paths=$(install_path)lib$(folder)  $(rootpath)../AerotriLibs/lib/Linux-$(arch)/

exepath= $(rootpath)bin/Linux-$(arch)/
OBJPATH= $(exepath)$(FOLDER)
install_path=$(rootpath)../bin/Linux-$(arch)/


