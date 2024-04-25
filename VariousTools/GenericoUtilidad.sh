#echo generando ${outdir}${Name}${Letra}
clang ${cflags} -lm -o ${outdir}${Name}${Letra} ${source}${Name}.c ${Objs}
