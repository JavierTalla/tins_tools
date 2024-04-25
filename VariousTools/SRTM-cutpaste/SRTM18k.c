#include <ATcrt/ATcrt.h>
#include <stdio.h>
#include <ATcrt/file_fix.h>
#include "../generic/dirs_io.c"

#define NOMEM_RETURN_CODE AT_NOMEM

typedef struct{
	char8_t path[SHRT_PATH];
	char8_t *pfile,*pW;
} Filename;

int main(int argc, char8_t* argv[]){
	int nret;
#define Nio_row 12 //Número de imput tiles que entran por cada fila del output tile
#define Nio_filas 6
#define Npo_pi 5 //Relación entre el paso de salida y el de entrada
	Filename filein[Nio_row];
	char8_t fileout[SHRT_PATH], *pfile2, *pW2;
	Buffer_bo boutput;
	uint *tile; //De entrada
	uint *Tile; //De salida
#define Nin_f 900 //Son 901
#define Nin_c 900 //Son 901
#define Nin_u16s ((Nin_f+1)*(Nin_c+1)+1)
#define Nin_tuints (Nin_u16s>>1)
#define No_f1 180 //Número de filas del tile de salida correspondientes a cada tile de entrada
#define No_c1 180 //Lo mismo para las columnas
#define No_c (No_c1*Nio_row+1) //Número de columnas del tile de salida
#define No_u16s (No_c*(No_f1*Nio_filas+1)+1)
#define No_uints (No_u16s>>1)
	const uint salto=(Npo_pi-1)*(Nin_c+1); //Tras cada fila copiada Se saltan cuatro filas de los tiles de entrada

#ifdef _DEBUG
	getchar();
#endif

	if(argc<3){
		puts(u8"\nUso del programa: srtm18k <carpeta de entrada> <carpeta de salida>\n"
			u8"\nEl programa busca en <carpeta de entrada> los fichreros de SRTM3.6k, de\n"
			u8"30º x 30º y crea un único fichero para toda la Tierra de 10' de paso. Al sur del\n"
			u8"60ºS rellena con ceros."
		);
		return 0;
	}
	ifnzunlike(dirs_io(argv[1],argv[2],filein[0].path,&filein[0].pfile,fileout,&pfile2)) return 1;
	{durchlaufep(Filename,&filein[1],Nio_row-1) p->pfile=strpcpy8(p->path,filein[0].path);}
	//Dejar escrita la longitud para todo el array de ficheros de entrada. No cambiará
	{Filename *p=&filein[0];
	for(sint8m λ=-18; λ<18;λ+=3){ //λ empieza atrasado
		sint8m l=λ;
		p->pW=p->pfile+3;
		char8_t *pW=p++->pfile;
		*pW++='N'; *pW++='6'; *pW++='0';

		if(λ<0) l=-l, *pW='W';
		else *pW='E'; pW++;
		*pW++='0'+(l>=10); if(l>=10) l-=10;
		*pW++='0'+l;
		*pW++='0';
		strcpy8(pW,".hgt");
	}}
	strcpy8(pfile2,u8"S90W180.hgt"); pW2=pfile2+3;

	//Mirar si son ficheros con o sin extensión
	if(fileclass8(filein[0].path)!=ATFILETYPE_FILE){
		filein[0].pW[4]='\0'; //Probar sin extensión
		if(fileclass8(filein[0].path)!=ATFILETYPE_FILE){
			fputs(u8"En la carpeta de entrada no existe el fichero ",stdout); puts(filein[0].path);
			fputs(u8"  ni el fichero ",stdout); filein[0].pW[4]='.'; puts(filein[0].path);
			return 1;
		}
		{durchlaufep(Filename,&filein[1],Nio_row-1) p->pW[4]='\0';}
		pW2[4]='\0';
	}

	aj_malloc_return(tile,uint,Nio_row*Nin_tuints); //Espacio para Nio_row ficheros de entrada
	checked_malloc_n(Tile,uint,No_uints, free(tile); return AT_NOMEM);

	uint16_t *pT=(uint16_t*)Tile;
	for(sint8m φ=6; φ>=-6; φ-=3){
		{char8_t c='0';
		if(φ>=0) c+=φ; else c-=φ;
		durchlaufep(Filename,&filein[0],Nio_row) p->pfile[1]=c;}
		ifonce(φ==-3){durchlaufep(Filename,&filein[0],Nio_row) *p->pfile='S';}

		//Abrir y leer la fila de ficheros
		{uint *pt=tile;
		durchlaufep(Filename,&filein[0],Nio_row){
			FILE_TYPE hgt;
			puts(p->pfile);
			FOPEN_B(hgt,p->path);
			if(OPEN_FAILURE(hgt)){
				fputs(u8"  Uno de los fichero de entrada necesarios no existe o no se puede abrir: ",stdout); puts(p->path);
				goto salida_mala;
			}
			size_t k=FREAD(pt,hgt,sizeof(uint),Nin_tuints);
			FCLOSE(hgt); if(k!=Nin_tuints){puts(u8"  Fichero corrupto"); goto salida_mala;}
			pt+=Nin_tuints;
		}}

		uint16_t *p1=(uint16_t*)tile;
		//En la primera iteración emepezamos leyendo la fila superior de los tiles. En las siguientes
		//saltamos las cinco primeras filas
		if(φ!=6) p1+=salto+Nin_c+1;
		dontimes(No_f1+(φ==6),p1+=Npo_pi*(Nin_c+1)){
			uint16_t *pi=p1;
			dontimes(Nio_row,pi+=Nin_u16s){
				uint16_t *pin=pi; dontimes(No_c1,pin+=Npo_pi) *pT++=*pin;
			}
			*pT++=*p1; //Al ser toda la Tierra El cierre coincide con el primer punto
		}
	}
	//Rellenar con ceros al sur de 60S
	*pT++='0'; //Estaba en posición impar
	const uint falta=(No_f1*No_c)>>1; //No_f1 porque lo que falta es el equivalente a una fila de tiles de entrada
	zeroset_uint((uint*)pT,falta);			//Incluye el cero húerfano al final.

	//Abrir y escribir el fichero de salida
	ifunlike(boopen8(&boutput,fileout,ATFILE_ENDIAN_UNSET)){
		puts(u8"  No se puede abrir el fichero para escriir"); goto salida_mala;
	}
	bowrite_uints(&boutput,Tile,No_uints);
	boclose(&boutput);
	ifunlike(boutput.error_code){
		puts(u8"  Error en la escritura del fichero"); goto salida_mala;
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
