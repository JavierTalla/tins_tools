#make hard links

ln -f ../../../buildscripts_shared/Linux/aarch64_clang-r.sh
ln -f ../../../buildscripts_shared/Linux/aarch64_gcc-r.sh
ln -f ../../../buildscripts_shared/Linux/amd64_clang-r.sh
ln -f ../../../buildscripts_shared/Linux/arm_clang-r.sh
ln -f ../../../buildscripts_shared/Linux/clang-d.sh
ln -f ../../../buildscripts_shared/Linux/clang-r.sh
ln -f ../../../buildscripts_shared/Linux/gcc-d.sh
ln -f ../../../buildscripts_shared/Linux/gcc-r.sh

cd  modules
ln -f ../../../../buildscripts_shared/Linux/compiler_flags.mk
ln -f ../../../../buildscripts_shared/Linux/linux_base.mk
ln -f ../../../../buildscripts_shared/Linux/paths1.mk paths.mk