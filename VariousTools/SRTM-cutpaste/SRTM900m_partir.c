#include <ATcrt/ATcrt.h>
#include <stdio.h>
#include <ATcrt/file_fix.h>
#include "../generic/dirs_io.c"

#define NOMEM_RETURN_CODE AT_NOMEM
#define inc(s) if(*(s)!='9') (*(s))++; else (s)[-1]++, *(s)='0'
#define dec(s) if(*(s)!='0') (*(s))--; else (s)[-1]--, *(s)='9'

int main(int argc, char8_t* argv[]){
	int nret;
	char8_t filein[SHRT_PATH], *pfile1, *pW1;
	char8_t fileout[SHRT_PATH], *pfile2, *pW2;
	Buffer_bo bsplit[4];
	uint *tile;
#define Nfuint 3000
#define Ncols 4800
#define Ntuints (3000*4800)
#define Nf10uint 600
#define Nc10 1200

#ifdef _DEBUG
	getchar();
#endif

	if(argc<3){
		puts(u8"\nUso del programa: srtm900m_partir <carpeta de entrada> <carpeta de salida>\n"
			u8"\nEl programa busca en <carpeta de entrada> los fichreros de SRTM900m, de\n"
			u8"50º x 40º, y los parte en ficheros de 10º x 10º."
		);
		return 0;
	}
	ifnzunlike(dirs_io(argv[1],argv[2],filein,&pfile1,fileout,&pfile2)) return 1;
	strcpy8(pfile1,u8"Sy0Wxx0.hgt"); pW1=pfile1+3;
	strcpy8(pfile2,u8"Sy0Wxx0.hgt"); pW2=pfile2+3;

	aj_malloc_return(tile,uint,Ntuints);

	for(sint8m φ=-6; φ<9; φ+=5){
		{uint8m k;
		char8_t *s=pfile1;
		if(φ<0) *s++='S', k=-φ;
		else *s++='N', k=φ;
		*s='0'+k;}
		*pW1='W';

		for(sint8m λ=-18; λ<18; λ+=4){
			ifonce(λ==2) *pW1='E';
			{uint8m k;
			char8_t *s=pW1+1;
			if(λ<0) k=-λ; else k=λ;
			s[0]=k/10; s[1]=k-10*s[0];
			*s+++='0'; *s+='0';}

			FILE_TYPE hgt;
			//Abrir y leer el fichero
			puts(pfile1);
			{FOPEN_B(hgt,filein);
			if(OPEN_FAILURE(hgt)){
				pW1[4]=(char)('.'+'\0'-pW1[4]); //Alternar con extensión (SRTM) y sin ella (bathymetry)
				pW2[4]=pW1[4];
				FOPEN_B(hgt,filein);
				if(OPEN_FAILURE(hgt)){puts(u8"  No existe o no se puede abrir"); continue;}
			}
			size_t k=FREAD(tile,hgt,sizeof(uint),Ntuints);
			FCLOSE(hgt);
			if(k!=Ntuints){
				puts(u8"  Fichero corrupto"); continue;
			}}

			sint8m k=φ+5;
			uint *pt=tile;
			dontimes(5,){
				if(--k<0) *pfile2='S', pfile2[1]='0'-k;
				else *pfile2='N', pfile2[1]='0'+k;
				sint8m l=λ;
				*pW2=*pW1; pW2[1]=pW1[1]; pW2[2]=pW1[2];

				ifunlike(boopen8(&bsplit[0],fileout,ATFILE_ENDIAN_UNSET)){goto salida_mala;} if(l<0) dec(pW2+2); else inc(pW2+2); if(++l==0) *pW2='E';
				ifunlike(boopen8(&bsplit[1],fileout,ATFILE_ENDIAN_UNSET)){goto salida_mala;} if(l<0) dec(pW2+2); else inc(pW2+2); if(++l==0) *pW2='E';
				ifunlike(boopen8(&bsplit[2],fileout,ATFILE_ENDIAN_UNSET)){goto salida_mala;} if(l<0) dec(pW2+2); else inc(pW2+2); if(++l==0) *pW2='E';
				ifunlike(boopen8(&bsplit[3],fileout,ATFILE_ENDIAN_UNSET)){goto salida_mala;}
				dontimes(Nc10,){
					bowrite_uints(&bsplit[0],pt,Nf10uint); pt+=Nf10uint;
					bowrite_uints(&bsplit[1],pt,Nf10uint); pt+=Nf10uint;
					bowrite_uints(&bsplit[2],pt,Nf10uint); pt+=Nf10uint;
					bowrite_uints(&bsplit[3],pt,Nf10uint); pt+=Nf10uint;
				}
				boclose(&bsplit[0]); boclose(&bsplit[1]);
				boclose(&bsplit[2]); boclose(&bsplit[3]);
				if(bsplit[0].error_code || bsplit[1].error_code || bsplit[2].error_code || bsplit[3].error_code){
					puts(u8"  Error en la escritura de alguno de los ficheros. Se abrota el proceso");
					goto salida_mala;
				}
			}
		}
	}
	nret=0;
	goto salida;

salida_mala:
	nret=1;
	goto salida;

salida:
	free(tile);
	return nret;
}
