#!/bin/bash
#Ruta del directorio raíz respecto al de generación de los ejecutables
root=../../../../
ATinclude=${root}AerotriLibs/include/
ATlibs=${root}AerotriLibs/lib/Win-amd64/

progname=ficheroconfig
cd ${progname}
make $setup -f ${progname}.mk
cd ../
