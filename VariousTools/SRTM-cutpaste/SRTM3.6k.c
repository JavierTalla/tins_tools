#include <ATcrt/ATcrt.h>
#include <stdio.h>
#include <ATcrt/file_fix.h>
#include "../generic/dirs_io.c"

#define NOMEM_RETURN_CODE AT_NOMEM
#define inc(s) if(*(s)!='9') (*(s))++; else (s)[-1]++, *(s)='0'
#define dec(s) if(*(s)!='0') (*(s))--; else (s)[-1]--, *(s)='9'
#define inc3(s) if(*(s)<'7') (*(s))+=3; else (s)[-1]++, *(s)-=7
#define dec3(s) if(*(s)>='3') (*(s))-=3; else (s)[-1]--, *(s)+=7

typedef struct{
	char8_t path[SHRT_PATH];
	char8_t *pfile,*pW;
} Filename;

int main(int argc, char8_t* argv[]){
	int nret;
	Filename filein[3];
	char8_t fileout[SHRT_PATH], *pfile2, *pW2;
	Buffer_bo boutput;
	uint *tile; //De entrada
	uint *Tile; //De salida
#define Nin_f 1200 //Son 1201
#define Nin_c 1200 //Son 1201
#define Nin_u16s ((Nin_f+1)*(Nin_c+1)+1)
#define Nin_tuints (Nin_u16s>>1)
#define No_f1 300 //Número de filas del tile de salida correspondientes a cada tile de entrada
#define No_c1 300 //Lo mismo para las columnas
#define No_uints ((901*901+1)>>1)
	const uint salto=3*(Nin_c+1); //Tras cada fila copiada Se saltan tres filas de los tles de entrada

#ifdef _DEBUG
	getchar();
#endif

	if(argc<3){
		puts(u8"\nUso del programa: srtm3.6k <carpeta de entrada> <carpeta de salida>\n"
			u8"\nEl programa busca en <carpeta de entrada> los fichreros de SRTM900m, de\n"
			u8"10º x 10º, con fila y columna de cierre, y crea ficheros de 30º x 30º de 2' de paso."
		);
		return 0;
	}
	ifnzunlike(dirs_io(argv[1],argv[2],filein[0].path,&filein[0].pfile,fileout,&pfile2)) return 1;
	filein[1].pfile=strpcpy8(filein[1].path,filein[0].path);
	filein[2].pfile=strpcpy8(filein[2].path,filein[0].path);
	strcpy8(filein[0].pfile,u8"S60W180.hgt"); filein[0].pW=filein[0].pfile+3;
	strcpy8(filein[1].pfile,u8"Sy0Wxx0.hgt"); filein[1].pW=filein[1].pfile+3;
	strcpy8(filein[2].pfile,u8"Sy0Wxx0.hgt"); filein[2].pW=filein[2].pfile+3;
	strcpy8(pfile2,u8"Sy0Wxx0.hgt"); pW2=pfile2+3;

	//Mirar si son ficheros con o sin extensión
	if(fileclass8(filein[0].path)!=ATFILETYPE_FILE){
		filein[0].pW[4]='\0'; //Probar sin extensión
		if(fileclass8(filein[0].path)!=ATFILETYPE_FILE){
			fputs(u8"En la carpeta de entrada no existe el fichero ",stdout); puts(filein[0].path);
			fputs(u8"  ni el fichero ",stdout); filein[0].pW[4]='.'; puts(filein[0].path);
			return 1;
		}
		filein[1].pW[4]='\0'; filein[2].pW[4]='\0';
		pW2[4]='\0';
	}

	aj_malloc_return(tile,uint,3*Nin_tuints); //Espacio para tres ficheros de entrada
	checked_malloc_n(Tile,uint,No_uints, free(tile); return AT_NOMEM);

	pfile2[1]='9'; //Se empieza en S60
	for(sint8m φ=-9; φ<6;){ //φ empieza atrasado
		if(φ<0) pfile2[1]-=3; else pfile2[1]+=3;
		ifonce((φ+=3)==0) *pfile2='N'; //Aquí se pone bien

		*pW2='W'; pW2[1]='2'; pW2[2]='1'; //Se empieza en W180
		for(sint8m λ=-21; λ<15;){ //λ empieza atrasado
			if(λ<0) dec3(pW2+2); else inc3(pW2+2);
			ifonce((λ+=3)==0) *pW2='E'; //Aquí se pone bien
			//Dejar listas las latitudes en filein
			filein[1].pW[0]=filein[0].pW[0]=pW2[0];
			filein[1].pW[1]=filein[0].pW[1]=pW2[1];
			filein[1].pW[2]=filein[0].pW[2]=pW2[2];
			if(λ<0) dec(filein[1].pW+2); else inc(filein[1].pW+2);
			filein[2].pW[0]=filein[1].pW[0];
			filein[2].pW[1]=filein[1].pW[1];
			filein[2].pW[2]=filein[1].pW[2];
			if(λ+1<0) dec(filein[2].pW+2); else inc(filein[2].pW+2);
			if(λ<0 && λ+2>=0){
				*filein[2].pW='E';
				if(λ+1==0) *filein[1].pW='E';
			}

			puts(pfile2);
			uint16_t *pt=(uint16_t*)Tile;

			for(sint8m l=3;l>=1;){ l--;
				{sint8m k=φ+l;
				char8_t *s=filein[0].pfile;
				if(k<0) *s++='S', k=-k;
				else *s++='N';
				*s='0'+(char8_t)k;} //La latitud ha quedado escrita en filein[0]
				filein[2].pfile[0]=filein[1].pfile[0]=filein[0].pfile[0]; //Copiar la latitud a
				filein[2].pfile[1]=filein[1].pfile[1]=filein[0].pfile[1]; //filein[1] y filein[2]

				FILE_TYPE hgt;
				size_t k;
				uint16_t *p1, *p2, *p3;
				p1=(uint16_t*)tile;
				p2=p1+Nin_u16s; p3=p2+Nin_u16s;

				//Abrir y leer el fichero 1º
				FOPEN_B(hgt,filein[0].path);
				if(OPEN_FAILURE(hgt)){
					fputs(u8"  Uno de los fichero de entrada necesarios no existe o no se puede abrir: ",stdout); puts(filein[0].path);
					goto salida_mala;
				}
				k=FREAD((uint*)p1,hgt,sizeof(uint),Nin_tuints);
				FCLOSE(hgt); if(k!=Nin_tuints){puts(u8"  Fichero corrupto"); goto salida_mala;}
				//Abrir y leer el fichero 2º
				FOPEN_B(hgt,filein[1].path);
				if(OPEN_FAILURE(hgt)){
					fputs(u8"  Uno de los fichero de entrada necesarios no existe o no se puede abrir: ",stdout); puts(filein[1].path);
					goto salida_mala;
				}
				k=FREAD((uint*)p2,hgt,sizeof(uint),Nin_tuints);
				FCLOSE(hgt); if(k!=Nin_tuints){puts(u8"  Fichero corrupto"); goto salida_mala;}
				//Abrir y leer el fichero 3º
				FOPEN_B(hgt,filein[2].path);
				if(OPEN_FAILURE(hgt)){
					fputs(u8"  Uno de los fichero de entrada necesarios no existe o no se puede abrir: ",stdout); puts(filein[2].path);
					goto salida_mala;
				}
				k=FREAD((uint*)p3,hgt,sizeof(uint),Nin_tuints);
				FCLOSE(hgt); if(k!=Nin_tuints){puts(u8"  Fichero corrupto"); goto salida_mala;}

				//En la primera iteración emepezamos leyendo la fila superior de los tiles. En las siguientes
				//saltamos las cuatro primeras filas
				const uint Salto=salto+Nin_c+1;
				if(l!=2) p1+=Salto, p2+=Salto, p3+=Salto;
				dontimes(No_f1+(l==2),(p1+=salto, p2+=salto, p3+=salto)){
					dontimes(No_c1,p1+=4) *pt++=*p1; p1++;
					dontimes(No_c1,p2+=4) *pt++=*p2; p2++;
					dontimes(No_c1,p3+=4) *pt++=*p3; *pt++=*p3++;
				}
			}

			//Abrir y escribir el fichero de salida
			ifunlike(boopen8(&boutput,fileout,ATFILE_ENDIAN_UNSET)){
				puts(u8"  No se puede abrir el fichero para escriir"); goto salida_mala;
			}
			bowrite_uints(&boutput,Tile,No_uints);
			boclose(&boutput);
			ifunlike(boutput.error_code){
				puts(u8"  Error en la escritura del fichero"); goto salida_mala;
			}
		}
	}
	nret=0;
	goto salida;

salida_mala:
	nret=1;
	goto salida;

salida:
	free(Tile);
	free(tile);
	return nret;
}
