/* Copia las cadenas especificadas para los directorios de entrada y salida (típicamente en la línea de comandos
o leídas de algún fichero) a pathin y pathout, escribiendo el carácter de separación de directorios al final
si es necesario. Deja los punteros fiein y fileout apuntando al '\0' del final de dichas cadenas; es decir, a
donde comenzaría el nombre del archivo. Si el directorio de salida no existe intenta crearlo.
    En caso de error escribe un mensaje a stderr.
Devuelve: 0, ERR_INPUDIR, ERR_INPUDIR.

Necesita <ATcrt/ATfiles.h>
*/

#define ERR_INPUDIR 1
#define ERR_OUTPUDIR 2
int dirs_io(const char8_t* inpudir, const char8_t *outputdir, char8_t* pathin, char8_t** pfilein, char8_t* pathout, char8_t** pfileout){
	*pfilein=strpcpy8(pathin,inpudir);
	if(fileclass8(pathin)!=ATFILETYPE_DIRECTORY){
		fputs(u8"Error: La ruta indicada para los ficheros de entrada no es un directorio: ",stderr); fputs(pathin, stderr);
		return ERR_INPUDIR;
	}
	path_append_sep_ifneeded8(pathin); while(**pfilein!='\0') (*pfilein)++;

	//Crear el directorio de salida si no existe.
	*pfileout=strpcpy8(pathout,outputdir);
	int nret=ATCreateDirectory8(pathout);
	ifunlike(nret){
		if(nret==ATFILESYS_ISNOT_DIR) fputs(u8"La ruta indicada para los ficheros de salida no es un directorio; es un fichero: ",stderr);
		else if(nret==ATFILESYS_BADPATH) fputs(u8"La ruta indicada para los ficheros de salida es errónea: ",stderr);
		else fputs(u8"El directorio para los ficheros de salida no se puede crear: ",stderr);
		fputs(pathout,stderr);
		return ERR_OUTPUDIR;
	}
	path_append_sep_ifneeded8(pathout); while(**pfileout!='\0') (*pfileout)++;

	return 0;
}

/* Copia la cadena especificada en inputpath a pathin. Si es un directorio escribe el carácter de separación
de directorios al final si es necesario y deja el puntero fiein apuntando al '\0' del final de dicha cadena; es decir, a
donde comenzaría el nombre del archivo.
Return:
	ATFILETYPE_FILE
	ATFILETYPE_DIRECTORY
	Otro valor: Error

Necesita <ATcrt/ATfiles.h>
*/
int dir_or_file(const char8_t* inputpath, char8_t* pathin, char8_t** pfilein){
	*pfilein=strpcpy8(pathin,inputpath);
	u8int ty=fileclass8(pathin);
	if(ty==ATFILETYPE_FILE) return ty;
	if(ty==ATFILETYPE_DIRECTORY){
		path_append_sep_ifneeded8(pathin);
		 while(**pfilein!='\0') (*pfilein)++;
		 return ty;
	}
	return ty;
}
