#include <ATcrt/ATcrt.h>
#include <stdio.h>
#include "../generic/dirs_io.c"
#include "../bmps/include/bmps.h"

#define BMPHGT_NOOPEN_IN 1
#define BMPHGT_NOOPEN_OUT 2
#define BMPHGT_ERROR_WRITE 3
#define BMPHGT_UNKNOWN_BMPTYPE 4

//ncolor:
//	0 : Colores por defecto
//	1-... : Otras coloraciones
int hgt___bmp(const char8_t *fout, const char8_t *fin){
	uint *buf;
	BmpRead bm;
	Buffer_bo bhgt;
	BmpMasks *cols;
	int nret;

	nret=biopen_utf8(&buf,fin);
	ifunlike(nret==AT_NOMEM) return AT_NOMEM;
	else if(nret<0) return BMPHGT_NOOPEN_IN;
	bm=read_bmphead(buf);
	if(bm.planes!=1 || bm.bpp!=16){free(buf); return BMPHGT_UNKNOWN_BMPTYPE;}

	cols=(BmpMasks*)((char*)(bm.puntos-3)-usizeof(char16_t*)); //If cols is not there the next test will be false, which is right.
	if(cols->B!=0xE000 || cols->R!=0x1C00 || cols->G!=0x07FF){ //Else, los bytes ya estaban cambiados
		durchlaufep(uint,bm.puntos,bm.nuints) *p=endianswap_u22(*p);
	}
	if(bm.npx&1){
		uint16_t *pb, *pa;
		pb=pa=(uint16_t*)bm.puntos+bm.npx;
		dontimes(bm.npy-1,){ pa++;
			dontimes(bm.npx,) *pb++=*pa++;
		}
	}

	ifunlike(boopen8(&bhgt,fout,ATFILE_ENDIAN_UNSET)){free(buf); return BMPHGT_NOOPEN_OUT;}
	bowrite_uints(&bhgt,bm.puntos,(bm.npx*bm.npy+1)>>1);
	boclose(&bhgt);
	free(buf);
	ifunlike(bhgt.error_code) return BMPHGT_ERROR_WRITE;
	return 0;
}

void err_message(int nret, const char8_t *filein, const char8_t *fileout){
	if(nret==0) return;
	const char8_t *s1, *s2=NULL;

	switch(nret){
	case AT_NOMEM: s1="El programa se ha quedado sin memoria"; break;
	case BMPHGT_NOOPEN_IN: s1="No se puede abrir el fichero bmp: "; s2=filein; break;
	case BMPHGT_NOOPEN_OUT: s1="No se puede abrir el fichero hgt para escribirlo: "; s2=fileout; break;
	case BMPHGT_ERROR_WRITE: s1="Se produjo algún error en la escritura del fichero "; s2=fileout; break;
	case BMPHGT_UNKNOWN_BMPTYPE: s1="El bitmap no es válido. Tiene que ser un bmp de 16 bits."; break;
	default: s1="Error desconocido";
	}
	fputs(s1,stderr); if(s2!=NULL) fputs(s2,stderr); putc('\n',stderr);
}

int main(int argc, char8_t* argv[]){
	int nret;
	char8_t filein[SHRT_PATH], *pfile1;
	char8_t fileout[SHRT_PATH], *pfile2, *pext;
	char8_t **files;

#ifdef _DEBUG
	getchar();
#endif

	if(argc<2){
		puts(u8"\nUso del programa: bmp2hgt <carpeta o fichero> [<fihero hgt>]\n"
			u8"\nSi en <carpeta o fichero> se indica una carpeta el programa busca en\n"
			u8"ella los fichreros con extensión bmp y los tramsforma en hgt.\n"
			u8"Si se indica un fichero, transforma únicamente ese fichero. En este caso\n"
			u8"puede indicarse opcionalmente el nombre del fichero de salida.\n"
			u8"    Los ficheros bmp tiene que ser de 16 bits."
		);
		return 0;
	}
	nret=dir_or_file(argv[1],filein,&pfile1);
	if(nret!=ATFILETYPE_FILE && nret!=ATFILETYPE_DIRECTORY) return 1;

	if(nret==ATFILETYPE_FILE){
		if(argv[2]==NULL){
			strcpy8(fileout,filein);
			path_get_extension8(fileout,pext);
			if(*pext=='\0') *pext='.', pext[1]='\0';
			pext++;
			if(pext[0]=='h' && pext[1]=='g' && pext[2]=='t'){
				fputs(u8"Por la extensión parece que el fichero de entrada ya es un hgt.\n"
						"Si no es así, indique explícitamente el nombre deseado para el fichero\n"
						"de salida.\n",stderr);
				return 0;
			}
			*pext++='h'; *pext++='g'; *pext++='t'; *pext='\0';
		}else{
			makepath8(fileout,SHRT_PATH,filein,argv[2]);
			ifz(strcmp8(filein,fileout)){
				fputs(u8"Error: Los nombres indicados para los ficheros de entrada y salida son iguales\n",stderr);
				return 1;
			}
		}
		nret=hgt___bmp(fileout,filein);
		err_message(nret,filein,fileout);
		return nret!=0;
	}

	strcpy8(pfile1,u8"*.bmp");
	ifunlike((files=findfiles8(filein))==NULL){
		puts(u8"Se produjo un error al intentar obtener la lista de ficheros de la carpeta de entrada.\n");
		return 1;
	}
	if(files[0]==NULL){
		fprintf(stderr,u8"Ningún fichero de carpeta de entrada: %s , tiene extensión bmp.\n",argv[1]);
		free_filelist8(files); return 0;
	}
	*pfile1='\0';

	pfile2=strpcpy8(fileout,filein);
	for(char8_t **ppfile=files; *ppfile!=NULL;){
		strcpy8(pfile1,*ppfile); ppfile++;
		fputs(pfile1,stderr); putc('\n',stderr);
		pext=strpcpy8(pfile2,pfile1)-3;
		*pext++='h'; *pext++='g'; *pext='t';

		nret=hgt___bmp(fileout,filein);
		err_message(nret,filein,fileout);
	}
	free_filelist8(files);
	return 0;
}
