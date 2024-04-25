#include <ATcrt/ATcrt.h>
#include <stdio.h>
#include <ATcrt/file_fix.h>
#include <ATsistemas/ATsistemas.h>
#include <PI.h>
#include "../generic/dirs_io.c"

int main(int argc, char8_t* argv[]){
	int nret;
	uint8m huso;
	char8_t filein[SHRT_PATH+20], *pfile1;
	char8_t fileout[SHRT_PATH+20], *pfile2;
	char8_t **files;
	Buffer_bo bgeo;
	Sistema sis;

#ifdef _DEBUG
	getchar();
#endif

	huso=255;
	if(argc>=2){
		char *pc=argv[1];
		iflike(*pc=='h'){ pc++;
			iflike(*pc=='u'){ pc++;
				iflike(*pc=='s'){ pc++;
					iflike(*pc=='o'){do pc++; while(*pc==' ');}
		}	}	}
		iflike(pc-argv[1]>=4 && *pc=='='){ //Tiene que haber un signo =
			do pc++; while(*pc==' ');
			huso=*pc++-'0';
			ifunlike(huso>'9') huso=255;
			else if(*pc>='0' && *pc<='9'){
				huso*=10; huso+=*pc++-'0';
				ifunlike(huso>60 || (*pc>='0' && *pc<='9')) huso=255;
			}
		}
	}
	if(argc<4 || huso==255){
		puts(u8"\nUso del programa: Lidar-geog___UTM huso=<n> <carpeta de entrada> <carpeta de salida>\n"
			u8"\nEl programa busca en <carpeta de entrada> todos los ficheros con extensión xyz,\n"
			u8"que tienen que ser los que generamos. El programa transformará su contenido de\n"
			u8"UTM a geográficas y los escribirá en <carpeta de salida>, con un nombre como el\n"
			u8"original pero substituyendo \"UTM\", si existe esa cadena dentro del nombre, por\n"
			u8"\"GEO\". Si la cadena \"UTM\" no existe deja el nombre como está.\n"
		     u8"El huso tiene que ser un valor entre 1 y 60."
		);
		return 0;
	}

	dirs_io(argv[2],argv[3],filein,&pfile1,fileout,&pfile2);
	strcpy8(pfile1,u8"*.xyz");
	ifunlike((files=findfiles8(filein))==NULL){
		puts(u8"Se produjo un error al intentar obtener la lista de ficheros de la carpeta de entrada.\n");
		return 1;
	}
	if(files[0]==NULL){
		fputs(u8"Ningún fichero de carpeta de entrada: ",stdout); fputs(argv[2],stdout); puts(", tiene extensión xyz.\n");
		free_filelist8(files); return 0;
	}

	sis.infor.λ0=(double)(6*((int)huso-30)-3);
	setup_Sistema_UTM_Norte(&sis,sis.infor.λ0);

	for(char8_t **ppfile=files; *ppfile!=NULL;){
		strcpy8(pfile1,*ppfile); ppfile++;
		putchar('\n'); puts(pfile1);
		//Copiar pfile1 en pfile2, cambiando UTM por GEO
		{char8_t* c2=pfile2;
		for(char8_t* c1=pfile1; *c1!='\0';){
			if(_unlikely(*c1=='U') && c1[1]=='T' && c1[2]=='M'){
				*c2++='G'; *c2++='E'; *c2++='O';
				c1+=3;
			}else{
				*c2++=*c1++;
			}
		}
		*c2='\0';}

		FILE_TYPE hgt;
		uint cabecera[7], *pn;
		PuntoXYZ_double Q;
		uint mz, MZ;
		uint16_t *puntos;
		PuntoXYZ_double *puntos_geo;
		uint k;

		/*Abrir, leer la cabecera y reservar memoria*/

		FOPEN_B(hgt,filein);
		if(OPEN_FAILURE(hgt)) continue;
		pn=cabecera;
		FREAD(pn,hgt,sizeof(uint),7); //Big endian
		k=u4___bigend_u4(*pn); pn++;		//Número de registros
		if(k==0) continue; //No se procesan los ficheros vacíos
		putchar(' '); puts(pfile2);

		Q.X=(double)u4___bigend_u4(*pn); pn++; //X mín
		Q.Y=(double)u4___bigend_u4(*pn); pn++; //Y mín
		mz=u4___bigend_u4(*pn);			//Z mín. Lo dejamos tal como está en el fichero
		MZ=u4___bigend_u4(pn[3]);			//Los máximos de X e Y los ignoramos

		k++; //Porque hay un último registro de cierre, con los tres valores a Я,Я,Я.
		puntos_geo=n_malloc(PuntoXYZ_double,k);
		k*=3; 
		puntos=n_malloc(uint16_t,k);
		if(_unlikely(puntos_geo==NULL) || _unlikely(puntos==NULL)){
			freeif(puntos);
			freeif(puntos_geo);
			FCLOSE(hgt);
			goto salida_outofmem;
		}
		{size_t kr=FREAD(puntos,hgt,sizeof(int16_t),k);
		FCLOSE(hgt);
		if(kr!=k) goto salida_mala;}

		if(puntos[k-3]!=Я16 || puntos[k-2]!=Я16 || puntos[k-1]!=Я16) goto salida_mala; //Cierre del fichero

		/*Leer los puntos e ir pasándolos a un array*/

		PuntoXYZ_double *pgeo=puntos_geo;
		for(uint16_t* ptr=puntos; *ptr!=Я16; pgeo++){
			pgeo->X=(double)u2___bigend_u2(*ptr); ptr++;		pgeo->X*=0.125;
			pgeo->Y=(double)u2___bigend_u2(*ptr); ptr++;		pgeo->Y*=0.125;
			*(uint16m*)&pgeo->Z=u2___bigend_u2(*ptr); ptr++; //La Z simplemente se pasa al endianness de la máquina
			pgeo->X+=Q.X; pgeo->Y+=Q.Y;
		}
		NOTFINITE_d(pgeo->X);
		free(puntos); //Not needed anymore

		geo___proy((Puntoxy_double*)puntos_geo,usizeof(PuntoXYZ_double),(pdif)(pgeo-puntos_geo),&sis); //La transformación

		/*Pasar de radianes a grados, sumar λcentral y obtener los valores extremos en geográficas*/
		struct{double mx, MX, my, MY;	}mm;
		struct{int mx,MX, my,MY;}mmi;

		mm.MX=mm.mx=puntos_geo[0].X;
		mm.MY=mm.my=puntos_geo[0].Y;
		for(pgeo=puntos_geo; isfinite(pgeo->X); pgeo++){
			if(pgeo->X<mm.mx) mm.mx=pgeo->X;		else if(pgeo->X>mm.MX) mm.MX=pgeo->X;
			if(pgeo->Y<mm.my) mm.my=pgeo->Y;		else if(pgeo->Y>mm.MY) mm.MY=pgeo->Y;
			pgeo->X*=PI_180_PI;
			pgeo->Y*=PI_180_PI;
		}
		mm.mx*=PI_180_PI; mm.MX*=PI_180_PI;
		mm.my*=PI_180_PI; mm.MY*=PI_180_PI;
		//Pasar a enteros en segundos, que es como irán en el fichero
		mmi.mx=(int)(mm.mx*3600)-(mm.mx<0);		mmi.MX=(int)(mm.MX*3600)+(mm.MX>=0);
		mmi.my=(int)(mm.my*3600)-(mm.my<0);		mmi.MY=(int)(mm.MY*3600)+(mm.MY>=0);
		//Volverlos a double, pues estos son los valores de offset que hay que aplicar
		mm.mx=(double)mmi.mx;		mm.MX=(double)mmi.MX; //MX y MY no hacen falta y
		mm.my=(double)mmi.my;		mm.MY=(double)mmi.MY; //se podrían omitir

		/*Escribir el fichero*/
		const double UNI=200; //Multiplicador respecto a segundos de arco. =3/600, 3 décimas de tercero (15 cm en latitud)

		ifunlike(boopen8(&bgeo,fileout,ATBYTES_LITTLE_ENDIAN)){free(puntos_geo); goto salida_mala;}
		//Cabecera
		{uint n; n=(pdif)(pgeo-puntos_geo); boput_32(&bgeo,n);} //k
		boput_32(&bgeo,mmi.mx); //Los valores extremos se escriben en segundos
		boput_32(&bgeo,mmi.my);
		boput_32(&bgeo,mz); //Este se ha mantenido como estaba en el fichero. El uint almacena un entero con signo, en metros
		boput_32(&bgeo,mmi.MX);
		boput_32(&bgeo,mmi.MY);
		boput_32(&bgeo,MZ); //Lo mismo que mz

		//Puntos
	#define tofile_x(X) (uint16m)(UNI*(3600*X-mm.mx))
	#define tofile_y(Y) (uint16m)(UNI*(3600*Y-mm.my))
	#define tofile_z(Z) *(uint16m*)&(Z) //La Z se ha almacenado tal cual

		uint16m u2=0; //Declarado fuera para detectar tras el bucle si se llegó a escribir un Я16
		for(pgeo=puntos_geo; isfinite(pgeo->X);){
			uint16m u1;
				u1=tofile_x(pgeo->X);
				u2=tofile_y(pgeo->Y);
			boput_1616(&bgeo,u2,u1);
				u1=tofile_z(pgeo->Z);
			pgeo++;
			iflike(isfinite(pgeo->X)){
					u2=tofile_x(pgeo->X);
				boput_1616(&bgeo,u2,u1);
					u1=tofile_y(pgeo->Y);
					u2=tofile_z(pgeo->Z);
				boput_1616(&bgeo,u2,u1);
				pgeo++;
			}else{
					u2=Я16;
				boput_1616(&bgeo,u2,u1);
			}
		}
		//El cierre. Se ha escrito, como mucho, un Я16
		boput_32(&bgeo,Я);
		if(u2!=Я16){boput_32(&bgeo,Я);}

		free(puntos_geo);
		boclose(&bgeo);
		if(bgeo.error_code) goto salida_mala;
	}
	nret=0;
	goto salida;

salida_mala:
	puts(u8"Error en el procesado del fichero\n");
	nret=1;
	goto salida;

salida_outofmem:
	puts(u8"El programa se ha quedado sin memoria\n");
	nret=AT_NOMEM;
	goto salida;

salida:
	free_filelist8(files);
	return nret;
}
