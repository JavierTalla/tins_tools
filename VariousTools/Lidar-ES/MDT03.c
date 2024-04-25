#include <ATcrt/ATcrt.h>
#include <stdio.h>
#include <ATsistemas/ATsistemas.h>
#include <PI.h>
#include <ATcrt/definesBufferto8.h>
#define NOMEM_RETURN_CODE AT_NOMEM
#include <ATcrt/file_fix.h>
#define READ_BADFILE 2

typedef uint16_t uEarthHeight;

typedef struct{
	uint kind; //Tipo de fichero. Los posibles valores los define cada función que rellene una estructura de este tipo
	char8_t file[100]; //Solamente se debe guardar la parte variable, desde la raíz del dataset
	bint found;
} SearchedFileInfo;

typedef struct{
	//ssint xmin, xmax, ymin, ymax;
	char8_t fnombre[SHRT_PATH];
	char8_t *varnombre; //Puntero a la parte variable del nombre.
	uint16_t *T;
	uint16_t *pos0, *pfinal;
			//pos0: Posición del primer elemento de la fila que se está rellenando. Puede ser el primer de la fila
				//o uno intermedio si el fichero que estamos leyendo no llega hasta el extremo izquierdo del tile.
			//pfinal: Bien el final de la fila del tile, bien (para el último tile) la posición que corresponde al final
} Tile;			//de la fila en el fichero que se está leyendo

//#define NODATA 0x8000 //El de los tiles generados

//Devuelve 0 si se puede rellenar con ceros, 1 en caso contrario.
bint zerofill(ssint x,ssint X,ssint y,ssint Y){
	if(x>=107900 && (Y<4132000 || y>=3977000)) return 0; //Sur: Atlántico y Medi. al N del estrecho
	if(x>=500000 && Y<4670000) return 0; //Bloque SE
	if(x>=1000000 && Y<4716500) return 0; //Esquina NE
	if(y>=4690000 && X<598000) return 0; //Cantábrico. Bloque NO
	if(X<30620 && y>=4665330) return 0; //Esquina SO de Galicia, 1.
	if(X<12730 && y>=4634000) return 0; //Esquina SO de Galicia, 2.
	return 1;
}

int main(int argc, char8_t* argv[]){
	int nret=0;
	Bufferto8 _blog;
	Bufferto8 * const blog=&_blog;
	const char8_t *opción;
	char8_t *DSfolder;
	struct{
		double λmin, λmax;
		double φmin, φmax;
		double pix_λ, pix_φ;	//Guardarán los inversos de los tamaños de píxel
		uint npφ, npλ;
	} transf;
	typedef struct{
		uint16m max_t,
					median_t,
					median_agua;
		uint8m n_t, n_agua;
	} PixelMDT4;
	PixelMDT4 *matriz;

	Sistema sis;
	const double H0=-1.6; //Origen de las alturas en España respecto al nivel del Geoide
	u8int husos[3];
	static char8_t dataset_folder[100];
	char8_t tilepath[SHRT_PATH];
	struct{
		char8_t *path;
		char8_t *folder; //Raíz de todas las carpetas UTM.
		char8_t *name;
		char8_t *ext;
	} tilename;
	struct{
		ssint x,X;
		ssint y,Y;
	} lim;
	uint16m ht_error; //Límite absoluto permitido de altura sobre el terreno

#ifdef _DEBUG
	getchar();
#endif

	//Command line processing
	{const char8_t *prog=u8"uso del programa:\n"
			u8"MDT03 -files φ λ path\n"
			u8"\tGenera un archivo \"φ-λ.log\" con la lista de archivos de lidar necesarios\n"
			u8"\nMDT03 φ λ path\n "
			u8"\tGenera el archivo de MDT03 correspondiente\n"
			u8"\nEn ambos casos (φ,λ) es un punto cualquiera del área abarcada por el archivo.\n"
			u8"path es la ruta a los ficheros de lidar, que a su vez tienen que estar agrupados en carpetas según los husos UTM.";
	const char8_t *endptr;

	if(argc<2 || (argc-(argv[1][0]=='-') != 4)){
error_cmdline:
		puts(prog); return 0;
	}
	if(argv[1][0]=='-'){
		opción=*++argv;
		ifnz(strcmp8(opción,"-files")){
			printf(u8"Opción de línea de comandos no reconocida: %s",opción);
			return 0;
		}
	}
	else opción=NULL;

	argv++;
	transf.φmin=vfdouble___str8_p(*argv,&endptr);
	ifunlike(*endptr!='\0'){
		fputs("Error: El valor indicado para φ, ",stderr); fputs(*argv,stderr); fputs(" no tiene el formato correcto para un número.\n\n",stderr);
		goto error_cmdline;
	}
	argv++;
	transf.λmin=vfdouble___str8_p(*argv,&endptr);
	ifunlike(*endptr!='\0'){
		fputs("Error: El valor indicado para λ, ",stderr); fputs(*argv,stderr); fputs(" no tiene el formato correcto para un número.\n\n",stderr);
		goto error_cmdline;
	}
	ifunlike(transf.φmin<-80 || transf.φmin>=80){
		fputs("Error: El valor indicado para φ, ",stderr); fputs(*argv,stderr); fputs(" está fuera de rango.\n\n",stderr);
		goto error_cmdline;
	}
	ifunlike(transf.λmin<-180 || transf.λmin>=180){
		fputs("Error: El valor indicado para λ, ",stderr); fputs(*argv,stderr); fputs(" está fuera de rango.\n\n",stderr);
		goto error_cmdline;
	}
	argv++;

	DSfolder=*argv;
}

	{char8_t fout[SHRT_PATH];
	strcpy8(fout,"files.log");
	toopen8(blog,fout);}

	//Truncar a un múltiplo de 3 minutos
	transf.φmin+=90;		transf.φmin=(int)(transf.φmin*20);		transf.φmax=transf.φmin+1;
	transf.λmin+=180;		transf.λmin=(int)(transf.λmin*20);		transf.λmax=transf.λmin+1;
	transf.φmin=transf.φmin/20.0-90;
	transf.φmax=transf.φmax/20.0-90;
	transf.λmin=transf.λmin/20.0-180;
	transf.λmax=transf.λmax/20.0-180;
	//Los valores anteriores han quedado en grados. Los siguientes los guardamos ya en radianes.
	//Guardamos los inversos del tamaño de pixel (=píxeles que caben en un radián).
	transf.pix_λ=10*3600*PI_180_PI/4; // 1/6'"
	transf.pix_φ=50*3600*PI_180_PI/4; // 1/4.8'"  (60/4.8 = 50/4)
	//
	transf.npφ=2251;
	transf.npλ=1801;
	const uint N=2251*1801;

	aj_malloc_return(matriz,PixelMDT4,N);

	ht_error=400;

	setup_Sistema_UTM_Norte(&sis,0); //Λ0 se asignará para cada huso
	{double λcentral=0.5*(transf.λmin+transf.λmax);
	ifunlike(λcentral>=180) λcentral-=360;
	else ifunlike(λcentral<-180) λcentral+=360;
	λcentral/=6.0;
	λcentral+=31;
	husos[0]=(uint)λcentral; //[1,60]
	λcentral-=(uint)λcentral;
	if(λcentral<0.05){
		if((husos[1]=husos[0]-1)==0) husos[1]=60;	husos[2]=Я8;
	}else if(λcentral>0.95){
		if((husos[1]=husos[0]+1)==61) husos[1]=1;		husos[2]=Я8;
	}else{
		husos[1]=Я8;
	}}
	//Ya no son necesrios los valores en grados. Pasarlos a radianes
	transf.λmin*=PI_180;	     transf.λmax*=PI_180;
	transf.φmin*=PI_180;	     transf.φmax*=PI_180;

	tilename.path=&tilepath[0];
	tilename.folder=strpcpy8(tilename.path,DSfolder); path_sep(tilename.folder);
	tilename.folder=strpcpy8(tilename.folder,"UTM");
	strcpy8(dataset_folder,tilename.path);
	path_sep(tilename.folder);

	for(uint *ptr=husos; *ptr!=Я8;){
		Extremos2D_dbl geog,UTM;
		u8int huso=*ptr++;
		sis.sis.Λ0=6.0*huso-183.0;
		sis.sis.Λ0*=PI_180;

		char8_t h0, h1;
		h1=(char8_t)(huso/10);
		h0=(char8_t)(huso-10*h1);
		h1+='0'; h0+='0';
		tilename.name=tilename.folder;
		*tilename.name++=h1;
		*tilename.name++=h0;
		*tilename.name='\0';

		ifunlike(fileclass8(tilename.path)!=ATFILETYPE_DIRECTORY){
			fprintf(stderr,"\nError: La carpeta %s no existe",tilename.path);
			nret=1; goto salida;
		}
		path_sep(tilename.name);
		tilename.ext=strpcpy8(tilename.name,u8"lidar_UTM29_476_4750.");
		tilename.name+=strlen8(u8"lidar_UTM"); *tilename.name++=h1; *tilename.name=h0; tilename.name+=2;
		strcpy8(tilename.ext,u8"xyz");

		geog.mx=transf.λmin;	geog.MX=transf.λmax;
		geog.my=transf.φmin;	geog.MY=transf.φmax;
		UTM=rect_proy___rect_geog(geog,&sis);
		lim.x=(ssint)(UTM.mx/1000);		lim.X=((ssint)UTM.MX+999)/1000;
		lim.y=(ssint)(UTM.my/1000);		lim.Y=((ssint)UTM.MY+999)/1000;
		lim.x&=~1U;	//Los tiles van de 2 en 2 Km
		lim.y&=~1U;

		for(uint x=(uint)lim.x; x<(uint)lim.X; x+=2){
			char8_t *pc=tilename.name;
			{uint xx=x;
			*pc=(char8_t)(xx/100); xx-=100**pc; *pc+++='0';
			*pc=(char8_t)(xx/10); xx-=10**pc; *pc+++='0';
			*pc=(char8_t)(xx+'0'); pc+=2;}

			for(uint y=(uint)lim.y; y<(uint)lim.Y; y+=2){
				char8_t *pd=pc;
				{uint xx=y;
				*pd=(char8_t)(xx/1000); xx-=1000**pd; *pd+++='0';
				*pd=(char8_t)(xx/100); xx-=100**pd; *pd+++='0';
				*pd=(char8_t)(xx/10); xx-=10**pd; *pd+++='0';
				*pd=(char8_t)(xx+'0'); pd+=2;}

				FILE_TYPE hgt;
				uint cabecera[7], *pn; //Big endian
				PuntoXYZ_double Q;
				Puntoxy_double p;
				uint16_t *puntos;
				uint k;

				towrite_string(blog,tilename.folder);
				ifz(strcmp8(opción,u8"-files")){toput_char(blog,'\n'); continue;}
				FOPEN_B(hgt,tilename.path);
				if(OPEN_FAILURE(hgt)){towrite_string(blog," X\n"); continue;}
				toput_char(blog,'\n');
				nret=0; //Indica que algún fichero de los que entran en el tile existe;
				pn=cabecera;

				FREAD(pn,hgt,sizeof(uint),7);
				k=u4___bigend_u4(*pn); pn++;		//Número de registros
				Q.X=(double)u4___bigend_u4(*pn); pn++; //X mín
				Q.Y=(double)u4___bigend_u4(*pn); pn++; //Y mín
				{uint z0=u4___bigend_u4(*pn); pn++;		//Z mín
				Q.Z=(double)(int)z0;}	//z0 puede ser negativo
				if(Q.X>=UTM.MX || Q.Y>=UTM.MY){FCLOSE(hgt); continue;}

				{PuntoXYZ_double P;
				P.X=(double)u4___bigend_u4(*pn); pn++; //X máx
				P.Y=(double)u4___bigend_u4(*pn); pn++; //Y máx
				P.Z=(double)u4___bigend_u4(*pn); pn++; //Z máx
				if(P.X<UTM.mx || P.Y<UTM.my){FCLOSE(hgt); continue;}}

				k=4*(k+1); //k+1 porque hay un último registro de cierre, con los tres valores a Я,Я,Я.
				puntos=n_malloc(uint16_t,k);
				if(puntos==NULL){FCLOSE(hgt); nret=AT_NOMEM; goto salida;}
				{size_t kr=FREAD(puntos,hgt,sizeof(int16_t),k);
				FCLOSE(hgt);
				if(kr!=k) return READ_BADFILE;}

				if(puntos[k-3]!=Я16 || puntos[k-2]!=Я16 || puntos[k-1]!=Я16) return READ_BADFILE; //Cierre del fichero

				Q.Z+=H0;
				for(uint16_t* ptr=puntos; *ptr!=Я16;){
					union{PuntoXYZ_double P; Puntoxy_double p;} P;
					uint nλ, nφ, n;

					P.P.X=(double)u2___bigend_u2(*ptr); ptr++;
					P.P.Y=(double)u2___bigend_u2(*ptr); ptr++;
					P.P.Z=(double)u2___bigend_u2(*ptr); ptr++;
					P_mul(P.P,0.125); //El fichero guarda ocatvos de metros
					P_eq(P.P,+=,Q);

					p=geo___proy1(&sis,P.p);
					p.λ-=transf.λmin; p.λ*=transf.pix_λ;		p.λ+=0.5;
					p.φ-=transf.φmin; p.φ*=transf.pix_φ;	p.φ+=0.5;
					if(p.λ<0 || p.λ>=transf.npλ || p.φ<0 || p.φ>=transf.npφ) continue;

					nλ=(uint)p.λ;
					nφ=transf.npφ-1-(uint)p.φ;
					n=transf.npλ*nφ+nλ;
				}

				free(puntos);
			}
		}
	}

	ifz(strcmp8(opción,u8"-files")) goto salida;

	//Eliminar puntos aislados demasiado altos
	//TEST mira si el punto es muy alto y establece el valor de h que al menos uno de sus adyacentes debe de superar
	//El !=0 es para evitar el warning del compilador.
#if 0
#define NPX (ssint)matriz->npx
#define TEST _unlikely((h=*ptrb-*ptr)>=ht_error) && _likely((h/=3)!=0)
#define test(n) (ptrb[n]-ptr[n]<h)
	{uint u=400*8;
	iflike(u<MAX_UEARTHHEIGHT){
		uEarthHeight h;
		ht_error=(uEarthHeight)u;

		uEarthHeight *ptr=matriz->suelo,
						   *ptrb=matriz->cielo;
		//Esquina sup. izda.
		if(_unlikely(TEST) && test(1) && test(NPX)) *ptrb=*ptr;		ptr++, ptrb++;
		//Fila superior
		dontimes(matriz->npx-2,(ptr++,ptrb++)){if(TEST && test(-1) && test(1) && test(NPX)) *ptrb=*ptr;}
		//Esquina sup. dcha.
		if(_unlikely(TEST) && test(-1) && test(NPX)) *ptrb=*ptr;		ptr++, ptrb++;
		//Bloque
		dontimes(matriz->npy-2,){
			if(_unlikely(TEST) && test(-NPX) && test(1) && test(NPX)) *ptrb=*ptr;		ptr++, ptrb++;
			dontimes(matriz->npx-2,(ptr++,ptrb++)){if(_unlikely(TEST) && test(-NPX) && test(-1) && test(1) && test(NPX)) *ptrb=*ptr;}
			if(_unlikely(TEST) && test(-NPX) && test(-1) && test(NPX)) *ptrb=*ptr;		ptr++, ptrb++;
		}
		//Esquina inf. izda.
		if(_unlikely(TEST) && test(1) && test(-NPX)) *ptrb=*ptr;		ptr++, ptrb++;
		//Fila inferior
		dontimes(matriz->npx-2,(ptr++,ptrb++)){if(_unlikely(TEST) && test(-1) && test(1) && test(-NPX)) *ptrb=*ptr;}
		//Esquina inf. dcha.
		if(_unlikely(TEST) && test(-1) && test(-NPX)) *ptrb=*ptr;
	}}
#undef test
#undef TEST
#undef NPX
#endif

salida:
	toclose(blog);
	free(matriz);
	return nret;
}
