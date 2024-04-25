/* Logging */
#if FILE_PREFERRED_ENCODING==FILE_ENCODING_BIT8
	#if FILE8BITS_ARE_UTF8
	#define towrite_filename towrite_string
	#else
	#define towrite_filename towrite_string_utf8
	#endif
#else
	#define towrite_filename towrite_string16_utf8
#endif

//For the ensuing arguments
#if FILE_PREFERRED_ENCODING==FILE_ENCODING_BIT8
	#define path_get_filename path_get_filename8
	#define path_get_extension path_get_extension8
	#define path_clean path_clean8
	#define strcmpargv(a,b) strcmp8(a,u8""b)
#else
	#define path_get_filename path_get_filename16
	#define path_get_extension path_get_extension16
	#define path_clean path_clean16
	#define strcmpargv(a,b) strcmp16(a,u""b)
#endif

#define PUTerr(s)  fputs(s,stderr)
#define PUTerrnl(s) PUTerr(s), fputc('\n',stderr)

struct OutDir{
	char8_t fout8[SHRT_PATH], *ext;
};

static inline charfile_t* get_progname(charfile_t *path){
	charfile_t *s;
	path_get_filename(path,s);
	path_get_extension(s,path); *path='\0';
	return s;
}

/*  Compone el nombre del fichero consumiendo cuantas palabras de argv sea necesario.

Return:
	0: todo bien
	1: Se ha indicado un nombre de fichero vacío
	2: El nombre indicado para el fichero no es el de un fichero existente

Escribe los mensajes de error / avisos que sea necesario.
Si el valor devuelto es 0 copia el nombre del fichero a od->fout8, en utf8, y deja
od->ext apuntando al punto de la extensión o al '\0' del final si no hay extensión.
*/
static int procesa_filename(charfile_t const *const **argv, struct OutDir *od){
	int nret;
	charfile_t fout[SHRT_PATH];

	nret=cmdline_compose(argv,fout,NULL);
	if(nret==1) PUTerr(u8"Warning: No hay una comilla de cierre correspondiente a la comilla \" de apertura."
							" Se entiende que el nombre del fichero se extiende hasta el final de la línea de comandos.\n");
	ifunlike(fout[0]==0){
		PUTerr(u8"Se ha indicado un nombre de fichero vacío. Nada que hacer\n");
		return 1;
	}
	nret=0;

	path_clean(fout);
	if(fileclass(fout)!=ATFILETYPE_FILE) nret=2;
#if FILE_PREFERRED_ENCODING==FILE_ENCODING_BIT8
	strcpy8(od->fout8,fout);
#else
	stru8___str16(od->fout8,fout);
#endif
	if(nret){
		PUTerr(u8"No existe ningún fichero de nombre "); PUTerrnl(od->fout8);
		return 2;
	}
	path_get_extension8(od->fout8,od->ext);
	return 0;
}

/*  Compone el nombre del fichero consumiendo cuantas palabras de argv sea necesario.

Return:
	0: todo bien
	1: Se ha indicado un nombre de fichero vacío

Escribe los mensajes de error / avisos que sea necesario.
Si el valor devuelto es 0 copia el nombre del fichero a od->fout8, en utf8, y deja
od->ext apuntando al punto de la extensión o al '\0' del final si no hay extensión.
*/
static int procesa_filename_out(charfile_t const *const **argv, struct OutDir *od){
	int nret;
	charfile_t fout[SHRT_PATH];

	nret=cmdline_compose(argv,fout,NULL);
	if(nret==1) PUTerr(u8"Warning: No hay una comilla de cierre correspondiente a la comilla \" de apertura."
							" Se entiende que el nombre del fichero se extiende hasta el final de la línea de comandos.\n");
	ifunlike(fout[0]==0) return 1;

#if FILE_PREFERRED_ENCODING==FILE_ENCODING_BIT8
	strcpy8(od->fout8,fout);
#else
	stru8___str16(od->fout8,fout);
#endif
	path_clean(fout);
	path_get_extension8(od->fout8,od->ext);
	return 0;
}
