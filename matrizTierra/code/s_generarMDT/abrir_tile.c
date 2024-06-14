//Intenta abrir el tile. Devuelve cualquiera de los códigos de error TILE_... definidos en interpolar.h o uno de los siguientes:
#define TILE_NONEXISTENT 101
#define TILE_OPENED 0

/*Intenta abrir el tile.
Return:
	TILE_OPENED (=0)
	TILE_NONEXISTENT
	TILE_OpenFailure
	TILE_BadSize

Si se devuelve TILE_OPENED habrá copiado el contenido del fichero en tile */
int abrir_tile(const char8_t *path, uint16_t *tile, uint npoints){
	FILE_TYPE hgt;

	if(fileclass_utf8(path)!=ATFILETYPE_FILE) return TILE_NONEXISTENT;
	FOPEN_B(hgt,(const char*)path);
	if(OPEN_FAILURE(hgt)) return TILE_OpenFailure;
	size_t k=FREAD(tile,hgt,sizeof(uint16_t),npoints);
	FCLOSE(hgt);
	if(k!=npoints) return TILE_BadSize;
	return TILE_OPENED;
}

/* Mira si existe el archivo comprimido <path>.gz. En ese caso se descomprime. Si existen ambos
tal vez el descomprimido esté corrupto (descompresión incompleta, por ejemplo).

Return:
	TILE_OPENED (=0)
	AT_NOMEM
	TILE_NONEXISTENT
	TILE_NoUnzipCommand
	TILE_BadDecompression
	TILE_OpenFailure
	TILE_BadSize

Si se devuelve TILE_OPENED habrá copiado el contenido del fichero en tile */
static int abrir_tile_gz(const char8_t *path, uint16_t *tile, uint npoints){
	char8_t *ext;
#ifdef _WIN32
	const char * const sgz="\"\"C:\\Program Files\\7-Zip\\7z\" e -aoa "; //La comilla de cierre se completará tras escribir el nombre del fichero
#else
	const char * const sgz="gunzip --quiet -f ";
#endif

	ext=path; while(*ext!='\0') ext++;
	*ext='.'; ext[1]='g'; ext[2]='z'; ext[3]='\0';
	if(fileclass_utf8(path)==ATFILETYPE_FILE){ //Expect a compressed (or not) archive to count always as a file for the O.S.
		char SYSCALL[SHRT_PATH+20];
		char *pc=strpcpy8(SYSCALL,sgz);

		//Mirar si existe el comando para descomprimir
	#if SYSTEM_ID==SYSWin_ID
		if(fileclass_utf8("C:\\Program Files\\7-Zip\\7z.exe")!=ATFILETYPE_FILE) return TILE_NoUnzipCommand;
	#elif SYSTEM_ID==SYSUnix_ID
		if(system("gunzip")==-1) return TILE_NoUnzipCommand;
	#endif

		//Descomprimir
		//Indicar la carpeta de salida
	#if SYSTEM_ID==SYSWin_ID
		pc=strpcpy8(pc,"-o\"");
		strcpy8(pc,path);
		char *p2=pc;
		path_get_filename8(p2,pc); *pc++='"'; *pc++=' ';
	#endif
		//El archivo a descomprimir
		*pc++='"'; pc=strpcpy8(pc,path); *pc++='"';
		//El resto
	#if SYSTEM_ID==SYSWin_ID
		*pc++='"';
	#endif
		*pc='\0';
		int nret=system(SYSCALL);

		//Mirar el valor nret
	#if SYSTEM_ID==SYSUnix_ID
		nret>>=8;
		nret=nret==1;  //nret=2 means a warning. It may be about setting access rights on the uncompressed file.
	#else
		nret=nret!=0;
	#endif
		ifnz(nret) return TILE_BadDecompression;

		//Mirar si efectivamente el archivo descomprimido existe ahora
		*ext='\0';
		bint hacked=0;
		if(fileclass_utf8(path)==ATFILETYPE_NOFILE){
			if(ext[-4]=='.'){ //Hack, mientras no arreglen los archivos comprimidos de SRTM30m
				ext-=4; *ext='\0';
				if(fileclass_utf8(path)==ATFILETYPE_NOFILE) return TILE_BadDecompression;
				hacked=1;
			}else{
				return TILE_BadDecompression;
			}
		}

		//No hace falta eliminar el archivo comprimido, porque el comando llamado ya lo hace.
		//Pero 7zip no tiene la opción para ello
	#ifdef _WIN32
		if(!hacked){
			*ext='.';
			pc=strpcpy8(SYSCALL,"del ");
			*pc++='"'; pc=strpcpy8(pc,path); *pc++='"';
			*pc='\0';
			system(SYSCALL); //Ignorar el valor devuelto
			*ext='\0';
		}
	#endif
	}

	*ext='\0';
	return abrir_tile(path,tile,npoints);
}
