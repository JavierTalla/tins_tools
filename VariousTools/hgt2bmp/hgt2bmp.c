#include <ATcrt/ATcrt.h>
#include <stdio.h>
#include <ATcrt/file_fix.h>
#include "../generic/dirs_io.c"
#include "../bmps/include/bmps.h"

#define BMPHGT_NOOPEN_IN 1
#define BMPHGT_NOOPEN_OUT 2
#define BMPHGT_ERROR_WRITE 3
#define BMPHGT_UNKNOWN_HGTSIZE 4

//ncolor:
//	0 : Colores por defecto
//	1-... : Otras coloraciones
int bmp___hgt(const char8_t *fout, const char8_t *fin, u8int ncolor){
	uint *puntos;
	FILE_TYPE hgt;
	uint k=12967202;
	Bitmap16 bmp;
	BmpMasks cols;

//MDT05: 4004002			2001 x 2001
//SRTM30m: 12967201		3601 x 3601
//SRTM90m: 1442401		1201 x 1201
//SRTM270m: 1442402		1201 x 1201
//SRTM900m(o): 1440000  1200 x 1200
//SRTM900m(c): 1442402  1201 x 1201
//SRTM3.6k: 811802		901 x 901
//SRTM18k: 2336042		1081 x 2161

	FOPEN_B(hgt,fin);
	if(OPEN_FAILURE(hgt)) return BMPHGT_NOOPEN_IN;
	checked_malloc_n(puntos,uint,k>>1, FCLOSE(hgt); return AT_NOMEM);
	k=FREAD(puntos,hgt,usizeof(char16_t),k);
	FCLOSE(hgt);
	if((k&3)==2) k--; //k son words

	switch(k){
		case 4004001: bmp.npy=bmp.npx=2001; break;
		case 12967201: bmp.npy=bmp.npx=3601; break;
		case 1442401: bmp.npy=bmp.npx=1201; break;
		case 1440000: bmp.npy=bmp.npx=1200; break;
		case 811801: bmp.npy=bmp.npx=901; break;
		case 2336041: bmp.npy=1081; bmp.npx=2161; break;
		default: free(puntos); return BMPHGT_UNKNOWN_HGTSIZE;
	}
	k=(k+1)>>1; //k son uints
	if(ncolor!=0){ //Else, mantenemos adrede los bytes sin cambiar
		durchlaufep(uint,puntos,k) *p=endianswap_u22(*p);
	}

	if(!(bmp.npx&1)){
		bmp.values=(IO2char16*)puntos;
		bmp.nuints=k;
	}else{
		bmp.nuints=((bmp.npx+1)>>1)*bmp.npy;
		bmp.values=n_malloc(uint,bmp.nuints);
		ifunlike(bmp.values==NULL){
			free(puntos); return AT_NOMEM;
		}
		char16_t *pv=(char16_t*)bmp.values;
		char16_t *pp=(char16_t*)puntos;
		dontimes(bmp.npy,pv++){
			dontimes(bmp.npx,) *pv++=*pp++;
		}
		free_null(puntos);
	}

	//Para case 0 serían conveniente flags no continuas, pero los visores no lo aplican bien.
	switch(ncolor){
	case 0: cols.B=0xE000; cols.R=0x1C00; cols.G=0x07FF; break;
	case 1: cols.G=0x03; cols.R=0x0C; cols.B=0x30; break;
	default: cols.R=0xFFFF; cols.G=0x3FFF; cols.B=0x3FFF;
	}

	int nret=escribe16(fout,bmp,&cols);
	if(nret==BMP_WRITE_NOOPEN) nret=BMPHGT_NOOPEN_OUT;
	else if(nret>0) nret=BMPHGT_UNKNOWN_HGTSIZE;

	free(bmp.values);
	return nret;
}

void err_message(int nret, const char8_t *filein, const char8_t *fileout){
	if(nret==0) return;
	const char8_t *s1, *s2=NULL;

	switch(nret){
	case AT_NOMEM: s1="El programa se ha quedado sin memoria"; break;
	case BMPHGT_NOOPEN_IN: s1="No se puede abrir el fichero hgt: "; s2=filein; break;
	case BMPHGT_NOOPEN_OUT: s1="No se puede abrir el fichero bmp para escribirlo: "; s2=fileout; break;
	case BMPHGT_ERROR_WRITE: s1="Se produjo algún error en la escritura del fichero "; s2=fileout; break;
	case BMPHGT_UNKNOWN_HGTSIZE: s1="Tamaño de fichero hgt desconocido"; break;
	default: s1="Error desconocido";
	}
	fputs(s1,stderr); if(s2!=NULL) fputs(s2,stderr); putc('\n',stderr);
}

int main(int argc, char8_t* argv[]){
	int nret;
	char8_t filein[SHRT_PATH], *pfile1;
	char8_t fileout[SHRT_PATH], *pfile2, *pext;
	char8_t **files;
	char8_t ncol; //Juego de colores escogido.

#ifdef _DEBUG
	getchar();
#endif

	if(argc<2){
		puts(u8"\nUso del programa: hgt2bmp <carpeta o fichero> [<fihero bmp>]\n"
			u8"\nSi en <carpeta o fichero> se indica una carpeta el programa busca en\n"
			u8"ella los fichreros con extensión hgt y los tramsforma en bmp.\n"
			u8"Si se indica un fichero, transforma únicamente ese fichero. En este caso\n"
			u8"puede indicarse opcionalmente el nombre del fichero de salida.\n"
			u8"    Los ficheros hgt no indican el número de filas y de columnas. El programa"
			u8"busca los tamaños conocidos. Si no es ninguno de ellos mostrará un\n"
			u8"mensaje de error."
		);
		return 0;
	}
	nret=dir_or_file(argv[1],filein,&pfile1);
	if(nret!=ATFILETYPE_FILE && nret!=ATFILETYPE_DIRECTORY) return 1;

	ncol=argv[0][7]-'0';
	if(ncol>9) ncol=0;

	if(nret==ATFILETYPE_FILE){
		if(argv[2]==NULL){
			strcpy8(fileout,filein);
			path_get_extension8(fileout,pext);
			if(*pext=='\0') *pext='.', pext[1]='\0';
			pext++;
			if(pext[0]=='b' && pext[1]=='m' && pext[2]=='p'){
				fputs(u8"Por la extensión parece que el fichero de entrada ya es un bmp.\n"
						"Si no es así, indique explícitamente el nombre deseado para el fichero\n"
						"de salida.\n",stderr);
				return 0;
			}
			*pext++='b'; *pext++='m'; *pext++='p'; *pext='\0';
		}else{
			makepath8(fileout,SHRT_PATH,filein,argv[2]);
			ifz(strcmp8(filein,fileout)){
				fputs(u8"Error: Los nombres indicados para los ficheros de entrada y salida son iguales\n",stderr);
				return 1;
			}
		}
		nret=bmp___hgt(fileout,filein,ncol);
		err_message(nret,filein,fileout);
		return nret!=0;
	}

	strcpy8(pfile1,u8"*.hgt");
	ifunlike((files=findfiles8(filein))==NULL){
		puts(u8"Se produjo un error al intentar obtener la lista de ficheros de la carpeta de entrada.\n");
		return 1;
	}
	if(files[0]==NULL){
		fprintf(stderr,u8"Ningún fichero de carpeta de entrada: %s , tiene extensión hgt.\n",argv[1]);
		free_filelist8(files); return 0;
	}
	*pfile1='\0';

	pfile2=strpcpy8(fileout,filein);
	for(char8_t **ppfile=files; *ppfile!=NULL;){
		strcpy8(pfile1,*ppfile); ppfile++;
		fputs(pfile1,stderr); putc('\n',stderr);
		pext=strpcpy8(pfile2,pfile1)-3;
		*pext++='b'; *pext++='m'; *pext='p';

		nret=bmp___hgt(fileout,filein,ncol);
		err_message(nret,filein,fileout);
	}
	free_filelist8(files);
	return 0;
}
