#!/bin/bash
#Ruta del directorio ra�z respecto al de generaci�n de los ejecutables
root=../../../../
ATinclude=${root}AerotriLibs/include/
ATlibs=${root}AerotriLibs/lib/Win-amd64/

progname=ficheroconfig
cd ${progname}
make $setup -f ${progname}.mk
cd ../
