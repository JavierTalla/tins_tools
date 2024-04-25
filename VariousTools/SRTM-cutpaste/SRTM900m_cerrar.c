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
	char8_t filein2[SHRT_PATH], *pfile2, *pW2;
	char8_t fileout[SHRT_PATH], *pfout;
	Buffer_bo bclosed;
	uint *tile, *tile2;
#define Nf 1200
#define Nc 1200
#define Ncuint 600
#define Ntuints (1200*600)
	uint16_t row[Nc+2];

#ifdef _DEBUG
	getchar();
#endif

	if(argc<3){
		puts(u8"\nUso del programa: srtm900m_cerrar <carpeta de entrada> <carpeta de salida>\n"
			u8"\nEl programa busca en <carpeta de entrada> los fichreros de SRTM900m, de\n"
			u8"10º x 10º, y añade la fila y columna de cierre."
		);
		return 0;
	}
	ifnzunlike(dirs_io(argv[1],argv[2],filein,&pfile1,fileout,&pfout)) return 1;
	pfile2=strpcpy8(filein2,filein);
	strcpy8(pfile1,u8"Sy0Wxx0.hgt"); pW1=pfile1+3;
	strcpy8(pfile2,u8"Sy0Wxx0.hgt"); pW2=pfile2+3;
	strcpy8(pfout,u8"Sy0Wxx0.hgt");

	aj_malloc_return(tile,uint,Ntuints);
	checked_malloc_n(tile2,uint,Ntuints, free(tile); return AT_NOMEM);

	pfile1[1]='7'; //Se empieza en S60
	for(sint8m φ=-7; φ<8;){ //φ empieza atrasado
		if(φ<0) pfile1[1]--; else pfile1[1]++;
		ifonce(++φ==0) *pfile1='N'; //Aquí se pone bien
		*pfile2=*pfile1; pfile2[1]=pfile1[1]; //Copiar la latitud a filein2

		*pW1='W'; pW1[1]='1'; pW1[2]='9'; //Se empieza en W180
		for(sint8m λ=-19; λ<17;){ //λ empieza atrasado
			if(λ<0) dec(pW1+2); else inc(pW1+2);
			ifonce(++λ==0) *pW1='E'; //Aquí se pone bien

			FILE_TYPE hgt;
			//Mirar si existe el ficheo
			puts(filein);
			if(fileclass8(filein)!=ATFILETYPE_FILE){
				pW1[4]=(char)('.'+'\0'-pW1[4]); //Alternar con extensión (SRTM) y sin ella (bathymetry)
				pW2[4]=pW1[4]; //Lo mismo para filein2
				if(fileclass8(filein)!=ATFILETYPE_FILE){puts(u8"  No existe"); continue;}
			}
			*pW2=*pW1; pW2[1]=pW1[1]; pW2[2]=pW1[2]; //Copia de la longitud

			//Abrir el fichero de salida
			strcpy8(pfout,pfile1); //Se llaman igual
			ifunlike(boopen8(&bclosed,fileout,ATFILE_ENDIAN_UNSET)){goto salida_mala;}

			//Escribir toda la primera fila del tile de salida, excepto el último punto
			uint16_t u; //valor del último punto
			if(φ==8){
				zeroset_uint(tile,Ncuint); //En el polo Norte hay mar
				bowrite_uints(&bclosed,tile,Ncuint);
				u=0;
				//Dejar listo el nombre del tile al E
				sint8m l=λ;
				if(l<0) dec(pW2+2); else inc(pW2+2); if(++l==0) *pW2='E'; else if(l==18) *pW2='W';
			}else{
				//Abrir el tile al N del tile en cuestión
				{sint8m l=φ+1;
				if(l<0) *pfile2='S', pfile2[1]='0'-l;
				else *pfile2='N', pfile2[1]='0'+l;}
				//
				FOPEN_B(hgt,filein2);
				if(OPEN_FAILURE(hgt)){puts(u8"  No existe o no se puede abrir el tile al Norte del presente tile"); goto continuar;}
				size_t k=FREAD(tile,hgt,sizeof(uint),Ntuints);
				FCLOSE(hgt);
				if(k!=Ntuints){
					puts(u8"El tile al Norte del presente tile está corrupto"); goto continuar;
				}
				bowrite_uints(&bclosed,tile+(Nf-1)*Ncuint,Ncuint);
				//Abrir el tile al NE del tile en cuestión
				{sint8m l=λ;
				if(l<0) dec(pW2+2); else inc(pW2+2); if(++l==0) *pW2='E'; else if(l==18) *pW2='W';}
				//
				FOPEN_B(hgt,filein2);
				if(OPEN_FAILURE(hgt)){puts(u8"  No existe o no se puede abrir el tile al NE del presente tile"); goto continuar;}
				k=FREAD(tile,hgt,sizeof(uint),Ntuints);
				FCLOSE(hgt);
				if(k!=Ntuints){
					puts(u8"El tile al NE del presente tile está corrupto"); goto continuar;
				}
				u=bytes01(tile[(Nf-1)*Ncuint]);
				//Restaurar la latitud
				*pfile2=*pfile1; pfile2[1]=pfile1[1];
			}

			//Abrir el tile
			FOPEN_B(hgt,filein);
			size_t k=FREAD(tile,hgt,sizeof(uint),Ntuints);
			FCLOSE(hgt);
			if(k!=Ntuints){
				puts(u8"El tile está corrupto"); goto continuar;
			}
			//Abrir el tile al E del tile en cuestión (el nombre ya ha quedado listo de antes)
			FOPEN_B(hgt,filein2);
			if(OPEN_FAILURE(hgt)){puts(u8"  No existe o no se puede abrir el tile al Este del presente tile"); goto continuar;}
			k=FREAD(tile2,hgt,sizeof(uint),Ntuints);
			FCLOSE(hgt);
			if(k!=Ntuints){
				puts(u8"El tile al Este del presente tile está corrupto"); goto continuar;
			}

			//Ir copiando las filas
			uint *pt1=tile, *pt2=tile2;
			dontimes(Nf>>1,){
				uint16_t *pr, *p1;
				//Primera fila (existe u arrastrado)
				pr=row, p1=(uint16_t*)pt1;
				*pr++=u; dontimes(Nc,) *pr++=*p1++; *pr=get_bytes01(pt2);
				bowrite_uints(&bclosed,(uint*)row,Ncuint+1);
				pt1+=Ncuint, pt2+=Ncuint;
				//Segunda fila (dejamos u de arrastre)
				pr=row, p1=(uint16_t*)pt1;
				dontimes(Nc,) *pr++=*p1++; u=get_bytes01(pt2);
				bowrite_uints(&bclosed,(uint*)row,Ncuint);
				pt1+=Ncuint, pt2+=Ncuint;
			}
			boput_1616(&bclosed,0,u);

		continuar:
			boclose(&bclosed);
			ifunlike(bclosed.error_code){
				puts(u8"  Error en la escritura de alguno de los ficheros. Se abrota el proceso");
				goto salida_mala;
			}
		}
	}
	nret=0;
	goto salida;

salida_mala:
	nret=1;
	goto salida;

salida:
	free(tile2);
	free(tile);
	return nret;
}
