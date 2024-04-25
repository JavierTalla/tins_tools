//#include "MDT5cutpaste.h"
//#include "MDT5Península.h" //u otro

int main(int argc, char8_t* argv[]){
	int nret;
	Bufferti8 buffer;
	char8_t outpath[SHRT_DIR];
	Tile tiles[5]; //Con 4 ficheros tiene que ser suficiente. Un T=NULL en marcará el final por si acaso
	ssint Xmin, Ymin; //X: del primero de los tiles; Y: del tile que se está rellenando.
	const struct{
		uint xlines;//=2001
		uint ylines;//2001
		uint size16;//=xlines*ylines+1;
		uint size32;//=size16>>1;
		float zoffset;//=500; //En metros.
		float esc;//=4; //Cuartos de metro
		ssint Δx; //10000 = (xlines-1)*5m
		ssint Δy; //10000 = (ylines-1)*5m
	} TILE={
		2001,2001,2001*2001+1,(2001*2001+1)>>1,500,4,
		10000,10000
	};

	struct{
		uint ncols, nrows, pix; //Del fichero que leemos
		ssint xmin, xmax, ymin, ymax;
		float noval;
		uint row, col; //fila y columna en la que se está, del fichero que leemos. Empieza en 0,0.
	} file;
	u8int ntiles; //que se escribirán simultáneamente
	ssint y; //de cada instante.

	if(argc<3){
		fputs(u8"\nUso del programa: ",stdout); fputs(argv[0],stdout); puts( " <ficheroascii> <carpeta de salida>\n"
			u8"\n<ficheroascii> es un fichero de una malla MDT en formato ASCII. En la carpeta\n"
			u8"en la que se encuentre el fichero <ficheroascii> tiene que existir una carpeta de\n"
			u8"nombre \"generados\" en la cual se escribirán los archivos de salida. Estos serán\n"
			u8"ficheros de MDT binarios, cuyos bordes coincidirán con múltiplos de 10 Km\n"
			u8"tanto en X como en Y. Si un fichero binario ya existe se actualiza, ya que cada\n"
			u8"fichero de salida se forma a partir de varios ficheros ASCII distintos. A su vez,\n"
			u8"cada fichero ASCII puede repartirse entre varios ficheros de salida.\n"
			u8"    Los ficheros de salida se llamarán MDT05_UTM" strHUSO "_pxxx_yyyy, o bien\n"
			u8"MDT05_UTM" strHUSO "_mxxx_yyyy, según la coordenada X sea positiva o negativa.\n"
			u8"pxxx/mxxx e yyyy son las coordenadas de la esquina SO del fichero, en kilómetros.\n"
			u8"La coordenada Y siempre es positiva. La última cifra 'x' así como la última cifra 'y'\n"
			u8"serán siempre 0, al ser las coordenadas de las esquinas múltiplos de 10 Km.\n"
			u8"    La coordenada X puede exceder de 999 Km, en cuyo caso para la primera cifra\n"
			u8"se empleará A,B,C, etc. Para la Península y Baleares lo máximo necesario es B.\n"
		);
		return 0;
	}
#ifdef _DEBUG
	getchar();
#endif

	nret=tiopen8(&buffer,argv[1]);
	if(nret<0){
		puts(u8"No se puede abrir el primer fichero para leer");
		return 1;
	}
	strcpy8(outpath,argv[2]);
	path_append_sep_ifneeded8(outpath);

	file.pix=file.nrows=file.ncols=0;
	file.ymin=file.xmin=0x7FFFffff;
	file.noval=0.0F;
	file.row=0;

	advance(buffer);
	while(*buffer.pc>='A' && *buffer.pc<='Z'){
		const char8_t *ptr;
		ssint u;

		prepare_string(buffer);
		ptr=buffer.pc;
		buffer.pc=buffer.next+1;
		if(buffer.savedchar!='\n')  advanceinline(buffer);
		if(buffer.savedchar=='\n' || *buffer.pc=='\0' || *buffer.pc=='\n'){
			puts(u8"Fichero corrupto en su cabecera");
			return 1;
		}

		nocheck_get_stay(buffer,u,ssint___str8);
		ifWORD("NCOLS") file.ncols=u;
		elifWORD("NROWS") file.nrows=u;
		elifWORD("CELLSIZE"){
			if(u!=5){puts(u8"El tamaño de píxel del fichero no está establecido a 5m"); return 1;}
			file.pix=u;
		}
		elifWORD("XLLCENTER") file.xmin=u;
		elifWORD("YLLCENTER") file.ymin=u;
		elifWORD("NODATA_VALUE") file.noval=(float)u;

		finishline_advance(buffer); file.row++;
	}

	if(file.nrows==0 || file.ncols==0 ||file. pix==0 || file.xmin==0x7FFFffff || file.ymin==0x7FFFffff || file.noval==0.0F){
		puts(u8"La cabecera está incompleta. Falta alguno de los siguientes valores:\n"
				u8"\tNCOLS, NROWS, CELLSIZE, XLLCENTER, YLLCENTER, NODATA_VALUE");
		return 1;
	}
	file.xmax=file.xmin+(file.ncols-1)*file.pix;
	file.ymax=file.ymin+(file.nrows-1)*file.pix;

	//Determinar cuántos tiles abarca cada linea horiz.
	{ssint Xmax;
	Xmin=file.xmin/TILE.Δx; Xmin-=(TILE.Δx*Xmin>file.xmin); //Añade 1 si Xmin<0 no es múltiplo de 10.
	Xmax=file.xmax/TILE.Δx; Xmax-=(TILE.Δx*Xmax>file.xmax);
	ntiles=(u8int)(Xmax-Xmin)+1; //+1 incondicionalmente aunque TILE.Δx*Xmax==xmax. No haría falta porque los ficheros
	Xmin*=TILE.Δx;}					//que se leen se solapan, pero es más fácil así que añadir una excepcion al código.

	//Reservar memoria
	{durchlaufep(Tile,tiles,ntiles){p->T=n_malloc(uint16_t,TILE.size16);}}
	{durchlaufep(Tile,tiles,ntiles){
		ifunlike(p->T==NULL){
			p=tiles; dontimes(ntiles,p++){freeif(p->T);}
			puts(u8"El programa no tiene memoria suficiente para ejecutarse");
			return 1;
		}
		p->varnombre=strbuild8(p->fnombre,outpath,"MDT05_UTM" strHUSO "_",NULL);
	}}

#undef MORE_INL
#define MORE_INL(buf) ((buf).pc+=*(buf).pc=='\r', *(buf).pc!='\n')
	y=file.ymax+file.pix; //+pix porque y va retrasada.
	Ymin=y; //Será la coordenada Y de la línea inferior de los tiles que se van a rellenar.
				 //Pero inicialmente lo establecemos así porque interesa.
	uint ROW0=file.row;
	file.row=0;
	while(1){
		//y guarda la coordenada de lo que acabamos de leer
		if(y==Ymin || *buffer.pc=='\0'){ //Final de tile o primera iteración del bucle o final del fichero
			if(file.row!=0){ //Final de tile o de fichero
				if(y==Ymin){//Final de tile (aunque también sea final de fichero)
					//Rewind una fila. La fila límite se incluye en ambas líneas de tiles
					do buffer.pc--; while(*buffer.pc!='\n'); advance(buffer);
					y+=file.pix;
				}
				//Escribir a disco los tiles con los que acabamos de terminar.
				Buffer_bo buf;
				{durchlaufep(Tile,tiles,ntiles){
					boopen8(&buf,p->fnombre,ATFILE_ENDIAN_UNSET); //Que no haga nada al escribir a disco
					bowrite_uints(&buf,(uint*)p->T,TILE.size32);			//Los valores ya los tenemos en big-endian en memoria
					boclose(&buf);
				}}
				if(*buffer.pc=='\0') break; //SALIDA DEL BUCLE while
				Ymin-=TILE.Δy;
			}else{ //Primera iteración
				/*Si la línea que nos disponemos a leer coincide realmente con el final
				de un tile se entrará otra vez en (y==Ymin) en la iteración siguiente del
				blucle while*/
				Ymin=y-file.pix; //y-pix porque y va retrasada
				Ymin-=(uint)(y-file.pix)%(uint)TILE.Δy; //Establecer el valor de de Ymin para los tiles a que pertenece
			}												   //la 1ª fila del fichero que vamos a leer

			//Construir la ruta de los siguientes tiles a abrir
			char8_t sy[4]; //Parte 'Y' de la ruta
			{uint yl, yy;
			char8_t *s;
			yl=Ymin/1000; s=sy; //yl: km

			yy=yl/1000; *s++=(char8_t)(yy+'0'); yl-=yy*1000;
			yy=yl/100; *s++=(char8_t)(yy+'0'); yl-=yy*100;
			yy=yl/10; *s++=(char8_t)(yy+'0'); yl-=yy*10;
			*s=(char8_t)(yl+'0');}

			ssint xleft=Xmin/1000; //km
			{durchlaufep(Tile,tiles,ntiles){
				uint xl;
				char8_t *s;

				s=p->varnombre;
				if(xleft>=0){*s++='p'; xl=(uint)xleft;}
				else{*s++='m'; xl=(uint)-xleft;}
				xleft+=TILE.Δx/1000; //km
				uint xx=xl/100;
				if(xx<=9) *s=(char8_t)(xx+'0'); else *s=(char8_t)(xx-10+'A'); s++;
				xl-=xx*100; xx=xl/10;
				*s++=(char8_t)(xx+'0'); xl-=xx*10;
				*s++=(char8_t)(xl+'0');

				*s++='_'; *s++=sy[0]; *s++=sy[1]; *s++=sy[2]; *s++=sy[3];
				*s++='.'; *s++='h'; *s++='g'; *s++='t'; *s='\0';
			}}

			//Abrir los tiles si ya existen; crearlos en caso contrario.
			{ssint xleft=Xmin;
			durchlaufep(Tile,tiles,ntiles){
				FILE_TYPE hgt=BAD_FILE;
				if(fileclass8(p->fnombre)==ATFILETYPE_FILE) FOPEN_B(hgt,p->fnombre);
				if(OPEN_FAILURE(hgt)){
					uint16_t /*fill=(uint16_t)zerofill(xleft,xleft+TILE.Δx,Ymin,Ymin+TILE.Δy); //Hay zonas de España que
					ifnz(fill)*/ fill=bigend_u2___u2(NODATA);						//no están cubiertas por los tiles
					durchlaufej(uint16_t,p->T,TILE.size16) *ptrj=fill;
				}else{
					size_t k=FREAD((uint*)p->T,hgt,sizeof(uint),TILE.size32);
					FCLOSE(hgt);
					if(k!=TILE.size32){
						printf("error: El fichero %s no tiene el tamaño de un tile de MDT05",p->fnombre);
						nret=1; goto salida;
					}
				}
				xleft+=TILE.Δx;
			}}

			//Establecer pos0 y pfinal
			uint k0=TILE.ylines-(y-Ymin)/file.pix; //El -1 y el -pix de "ylines-1" y "(y-pix)" se anulan
			k0*=TILE.xlines;
			{durchlaufep(Tile,tiles,ntiles){p->pos0=p->T+k0; p->pfinal=p->pos0+TILE.ylines;}}
			tiles[0].pos0+=(file.xmin-Xmin)/file.pix;
			//No hace falta ajustar tile[ntiles-1].pfinal porque se detectará el final de la fila que se lee.
		}

		y-=file.pix; //Avanzar y. De aquí para abajo ya no está retrasada; es la y de la fila que se va a leer.

		file.row++;
		file.col=0;
		Tile *ptile=&tiles[0];
		uint16_t *p=ptile->pos0;
		while(MORE_INL(buffer)){ file.col++;
			float z;
			uint16_t Z;

			nocheck_get_advanceinline(buffer,z,vffloat___str8);
			if(z==file.noval) Z=*p; //Mantener lo que hubiera (NODATA o 0, según)
			else{
				if(z<0) z=0; //España no tiene valores negativos
				Z=(uint16_t)((z+TILE.zoffset)*TILE.esc+0.5F);
				Z=bigend_u2___u2(Z);
			}
			*p++=Z;
			ifunlike(p==ptile->pfinal){ //Pasar al tile siguiente en X. Aunque sea el último punto de la línea que estamos leyendo
				ptile++;			 //y por tanto el tile que abrimos podría omitirse, hay memoria reservada para él
				ifunlike(ptile->T==NULL) break; //Sólo puede suceder si el fichero está mal (tiene demasiados elementos en una fila)
				p=ptile->pos0;
				*p++=Z;
			}
		}
		while(MORE_INL(buffer)){ignore_advanceinline(buffer); file.col++;} //Se sale en algún caso si se detecta un exceso de elementos
		ifunlike(file.col!=file.ncols){
			printf("error: La fila %u no tiene %u datos sino %u\n",ROW0+file.row,file.ncols,file.col);
			nret=1; goto salida;
		}

		do{finishline_advance(buffer);}while(*buffer.pc==26);
		//Avanzar pos0
		{durchlaufep(Tile,tiles,ntiles){p->pos0+=TILE.xlines; p->pfinal+=TILE.xlines;}}
	}
	ifunlike(file.row!=file.nrows){
		printf("error: El fichero no contiene %u filas de datos sino %u\n",file.nrows,file.row);
		nret=1; goto salida;
	}

salida:
	{durchlaufep(Tile,tiles,ntiles){freeif(p->T);}}
	return nret;
}
