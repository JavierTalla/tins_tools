#define nfiles_left(pfile) (ssint)(debug->nfiles-(pdif)(pfile-debug->files))

//Los puntos que tienen z=NODATA_uEarth los pone con z=z0 y flags.fuera=1
void enmascara_fuera(MatrizTierra *matriz, const Matriz___Tierra *transf){
	if(transf->tipo==TIPO_PROY_PlanoCuadra){
		if(transf->cc.φmax<=90 && transf->cc.φmin>=-90) return;
	}elif(transf->sis.sis.proy!=SIS_Sinusoidal) return;

	uEarthHeight z=MATRIZ_STORED___0(matriz->zbounds);
	durchlaufe2(uEarthHeight,matriz->suelo,matriz->npuntos,flagMDT,matriz->i.flags){
		if(*ptr==NODATA_uEarth) *ptr=z, ptr_b->fuera=1;
	}
}

/*Libera _matriz.m
Reserva matriz->i.flags si era NULL
Escala los valores de Z. Para los que fueran NODATA_ssint pone flag.fuera=1 y la z corresp. a z=0 en el terreno.
Return:
	0
	AT_NOMEM
	MTIERRA_MuchoDesnivel
*/
int pasa_a_matriz_y_enmascara_fuera(PLIST plist, MatrizTierra *matriz, Matriz_ssint _matriz, const Matriz___Tierra *transf){
	int iret=matriz____matriz(plist,matriz,_matriz.m);
	Free_remove(_matriz.m);
	ifnzunlike(iret) return iret;

	if(matriz->i.flags==NULL){
		aj_malloc_add(matriz->i.flags,flagMDT,matriz->npuntos);
		zeroset(matriz->i.flags,usizeof(uint)*matriz->npuntos);
	}
	//Poner flag de fuera a los puntos fuera de la Tierra
	enmascara_fuera(matriz,transf);

	return 0;
salida_outofmem:
	return AT_NOMEM;
}

//Pone flags a 0, salvo por .fuera
int matrizsuelo___userspecs(Debug_matrizsuelo *debug, MatrizTierra *matriz, const Matriz___Tierra *transf, bint besquina, const DataSets *datasets, const OpcionesMDTierra *abst){
	int nret;
	PLIST plist;

	Matriz_ssint _matriz;
	const ZonalDataSet *DS; //El DataSet que emplearemos.
	const LocalDataSet *DSL;
	bint btilelocal; //Si se llega a emplear algún Tile del DSL para la zona
	bint bdone; //Si los tiles del DSL completan toda la zona
	char8_t tilepath[PathLengthFull_Max];
	Tilename tilename;
	SearchedFileInfo *pfile; //No ha de apuntar nunca más allá del último elemento.
	ssint nf; //Número de elementos aún disponibles en el array debug.files

	nret=0;
	debug->dsGlobal=NULL;
	debug->dsLocal=NULL;
	pfile=debug->files; pfile->file[0]='\0';
	nf=(ssint)debug->nfiles;

	//Determinar el DataSet y el valor provisional de zbounds.escala
	{DataSetPair S;
	S=selecciona_dataset(transf,datasets->sets.Gsuelos,datasets->sets.Lsuelos,&datasets->opts,matriz->pixel);
	debug->dsGlobal=DS=S.DS; //Se eliminará más adelante si no se emplea
	debug->dsLocal=DSL=S.DSL;}
	ifunlike(DS==NULL && DSL==NULL) return MTIERRA_DSempty;
	ifunlike(DS==BAD_PTR) return MTIERRA_DSlowres;

	if(abst->lagos.resolución>=3) matriz->zbounds.escala=4; //De momento es un multiplicador respecto a las unidades
	else if(abst->lagos.resolución==2) matriz->zbounds.escala=2; //del dataset, sean estas cuales sean
	else matriz->zbounds.escala=1;
	matriz->Z0=0; //Si se usa un sistema local se modificará

	plist=get_new_plist();
	if(plist==PLIST_OUT_OF_MEM) return AT_NOMEM;

	matriz->npx=transf->npx;
	matriz->npy=transf->npy;
	if(besquina) matriz->npx++, matriz->npy++;
	_matriz.npx=matriz->npx;
	_matriz.npy=matriz->npy;
	_matriz.npixels=matriz->npuntos=matriz->npx*matriz->npy;
	aj_malloc_add(_matriz.m,ssint,matriz->npuntos);
	//Rellenar la matriz con un valor que indica que no hay dato
	{durchlaufep(ssint,_matriz.m,matriz->npuntos) *p=NODATA_ssint;}
	bdone=false;
	tilename.path=&tilepath[0];

	/*DataSet Local, si es necesario y existe para la zona*/
	btilelocal=0;
	if(DSL!=NULL){
		union{uint16_t *u; int16_t *i;} Tile;
		Puntoxy_double *precalculados;
		Puntoxy_double *minmax_fila; //De las coordenadas en geográficas de los puntos de cada fila de la matriz
		float esc;
		const ssint offsetZ0=-DSL->Dato.uniZ*DSL->Dato.offset; //Valor de offset que hay que sumar, en las propias unidades del tile
		Sistema sisDS;
		Extremos2D_dbl geog,UTM;
		struct{		 //Límites de los tiles que hay que procesar. En
			ssint x,X; //coordenadas UTM, en Km. (Por tanto serán
			ssint y,Y; //múltiplos del paso de los tiles).
		} lim;
		uint Tn;
		const float pixY=(float)abs(DSL->φstep)*LOCAL_PIXm_CM,
					   pixX=(float)abs(DSL->λstep)*LOCAL_PIXm_CM,
					   height=((DSL->nrows-1)*abs(DSL->φstep))*LOCAL_PIXm_CM, //Altura del tile entre filas, en metros.
					   width=((DSL->ncols-1)*abs(DSL->λstep))*LOCAL_PIXm_CM;
		u8int huso;
		if(transf->cc.λmax<-12) huso=28;
		else huso=30;

		//Establecer un valor de escala z que abarque el sistema local
		esc=(float)matriz->zbounds.escala; //Valor por el que multiplicaremos los valores del dataset. Lo pasaremos a la función interpola_tile
		matriz->zbounds.escala*=DSL->Dato.uniZ; //Por tanto la escala en la matriz será "esc" multiplicada por uniZ del tile.

		aj_malloc_add(precalculados,Puntoxy_double,matriz->npuntos);
		aj_malloc_add(minmax_fila,Puntoxy_double,2*matriz->npy);

		//Construir la ruta a los tiles
		tilename.name=tilename.namebase=strpcpy8(tilename.path,DSL->folder);
		if(fileclass_utf8(tilename.path)!=ATFILETYPE_DIRECTORY){nret=MTIERRA_DSfolder_notfound; goto salida;}
		path_sep(tilename.name);
		tilename.name=strpcpy8(tilename.name,u8"UTM30" STR8_pathsep "MDT05_UTM30_");
		tilename.ext=strpcpy8(tilename.name,u8"p020_4780.");
		strcpy8(tilename.ext,u8"hgt.gz");
		tilename.ext[3]='\0';
		if(huso==28){
			char8_t *p=tilename.name;
			while(1){
				while(*p!='\0' && *p!='M') p++;
				if(*p=='\0') break;
				p++; *p++='2'; *p++='8';
			}
		}

		Tn=DSL->nrows*DSL->ncols;
		aj_malloc_add(Tile.u,uint16_t,(Tn+1)&~1U);

		setup_Sistema_UTM_Norte(&sisDS,(double)(6*huso)-183);
		geog.mx=transf->cc.radianes.λmin;
		geog.MX=transf->cc.radianes.λmax;
		geog.my=transf->cc.radianes.φmin;
		geog.MY=transf->cc.radianes.φmax;
		UTM=rect_proy___rect_geog(geog,&sisDS);
		lim.x=(ssint)floor(UTM.mx*0.001);		lim.X=(ssint)ceil(UTM.MX*0.001); //Límites en km
		lim.y=(ssint)floor(UTM.my*0.001);		lim.Y=(ssint)ceil(UTM.MY*0.001);
		if(lim.x<0) lim.x-=DSL->WE-1;	lim.x=(lim.x/DSL->WE)*DSL->WE; //Múltiplos del paso del tile
		if(lim.y<0) lim.y-=DSL->SN-1;	lim.y=(lim.y/DSL->SN)*DSL->SN;

		{double Δφ,Δλ;
		Δφ=transf->cc.radianes.pix_φ;
		Δλ=transf->cc.radianes.pix_λ;
		Puntoxy_double *ptr=precalculados;
		if(!transf->b90){
			double φ=transf->cc.radianes.φmax;
			dontimes(matriz->npy,φ-=Δφ){
				double λ=geog.mx;
				dontimes(matriz->npx,λ+=Δλ){
					*ptr++=(Puntoxy_double){λ,φ};
				}
			}
		}else{
			double λ=geog.mx;
			dontimes(matriz->npy,λ+=Δλ){
				double φ=transf->cc.radianes.φmin;
				dontimes(matriz->npx,φ+=Δφ){
					*ptr++=(Puntoxy_double){λ,φ};
				}
			}
		}}
		proy___geo(precalculados,usizeof(Puntoxy_double),matriz->npuntos,&sisDS);
		{Puntoxy_double *ptr=precalculados,
								*pfilas=minmax_fila;
		dontimes(matriz->npy,){
			Puntoxy_double p,q;
			q=p=*ptr;
			dontimes(matriz->npx,ptr++){
				if(ptr->x<p.x) p.x=ptr->x;		else maxeq(q.x,ptr->x);
				if(ptr->y<p.y) p.y=ptr->y;		else maxeq(q.y,ptr->y);
			}
			*pfilas++=p; *pfilas++=q;
		}}
		//A partir de aquí geog, UTM y sisDS ya no hacen falta

		//Ir recorriendo los tiles
		for(ssint x=lim.x; x<lim.X; x+=DSL->WE){ //WE está en km
			double X=(double)(1000*x)+width; //x de la columna de más a la derecha, en metros
			char8_t *pc=tilename.name;
			{uint xx;
			if(x>=0) *pc++='p', xx=x;
			else *pc++='m', xx=-x;
			*pc=(char8_t)(xx/100); xx-=100**pc;
			if(*pc<=9) *pc+='0'; else *pc+='A'-10; pc++;
			*pc=(char8_t)(xx/10); xx-=10**pc; *pc+++='0';
			*pc=(char8_t)(xx+'0'); pc+=2;} //Saltamos la '_'

			for(ssint y=(uint)lim.y; y<lim.Y; y+=DSL->SN, pfile+=(nf>1), nf--){ //SN está en km
				double Y=1000.0*(double)y+height; //y de la fila más alta, en metros
				char8_t *pd=pc;
				{uint xx=y; //y>0 siempre
				*pd=(char8_t)(xx/1000); xx-=1000**pd; *pd+++='0';
				*pd=(char8_t)(xx/100); xx-=100**pd; *pd+++='0';
				*pd=(char8_t)(xx/10); xx-=10**pd; *pd+++='0';
				*pd=(char8_t)(xx+'0');}

				pfile->kind=MatrizSuelo_filekind_Local; strcpy8(pfile->file,tilename.namebase); pfile->found=false;
				int nret=abrir_tile_gz(tilename.path,Tile.u,Tn);
				if(nret==TILE_NONEXISTENT) continue;
				pfile->found=true;
				ifunlike(nret!=TILE_OPENED){
					pfile+=(nf>1), nf--;
					goto salida;
				}
				btilelocal=1;

				interpola_tile_sistema(transf,&sisDS,(uint16_t)DSL->Dato.nodata,offsetZ0,0,esc,_matriz.m,matriz->npx,matriz->npy,Tile.u,1000*x,X,1000*y,Y,pixY,pixX,DSL->ncols,DSL->nrows,precalculados,minmax_fila);
			}//Fin tiles Y
		}//Fin tiles X

		iflike(btilelocal){
			matriz->Z0=DSL->Z0;
			ssint *ptri=_matriz.m, *tope=_matriz.m+_matriz.npixels;
			do{if(*ptri==NODATA_ssint) break; ptri++;}while(ptri<tope);
			bdone=(ptri==tope);
		}
		Free_remove(Tile.u);
	}//Fin DataSet Local

	if(bdone){
		if(debug!=NULL) debug->dsGlobal=NULL;
	}else{
		Suavizado suavizado;
		suavizado.n_corte=MASK_MAX_NHALF; //Lo que se va a emplear. Son medios-píxeles
		if(DS==NULL){nret=MTIERRA_DSpartiallowres; goto salida;}

		const ssint offsetZ0=-DS->Dato.uniZ*DS->Dato.offset;
		if(!btilelocal){
			suavizado.corte=NULL; suavizado.nmarco=0; //=0 para que corte se mantenga en NULL
		}else{
			suavizado.nmarco=MASK_MAX_NHALF<<1;
			uint npixcor=(matriz->npx+suavizado.nmarco)*(matriz->npy+suavizado.nmarco);
			aj_malloc_add(suavizado.corte,umint,npixcor);
			suavizado.nmarco>>=1;
			enmascara_franja(suavizado.corte,_matriz.m,matriz->npx,matriz->npy,halfside___ncorte(suavizado.n_corte),suavizado.nmarco);
			suavizado.corte+=suavizado.nmarco*(1+matriz->npx+2*suavizado.nmarco); //Al primer punto de datos
			suavizado.nmarco<<=1; //el extra total de cada fila/columna
		}

		//Construir la ruta a los tiles
		tilename.name=strpcpy8(tilename.path,DS->folder);
		if(fileclass_utf8(tilename.path)!=ATFILETYPE_DIRECTORY){nret=MTIERRA_DSfolder_notfound; goto salida;}
		path_sep(tilename.name);
		tilename.ext=strpcpy8(tilename.name,u8"W40N100.");
		strcpy8(tilename.ext,u8"hgt");

		if(transf->tipo==TIPO_PROY_PlanoCuadra) nret=matriz_planocuadra(&_matriz,plist,transf,besquina,DS,offsetZ0,-matriz->Z0,(float)matriz->zbounds.escala,&suavizado,&tilename,&pfile,nfiles_left(pfile));
		else nret=matriz_sis(&_matriz,plist,transf,besquina,DS,offsetZ0,-matriz->Z0,(float)matriz->zbounds.escala,&tilename,&pfile,nfiles_left(pfile));
		ifunlike(nret==AT_NOMEM) goto salida_outofmem;
		ifunlike(nret!=0 && nret!=MTIERRA_DSnooverlap) goto salida;
	}

	int iret=pasa_a_matriz_y_enmascara_fuera(plist,matriz,_matriz,transf);
	ifnzunlike(iret){ //nret puede ser 0 o MTIERRA_DSnooverlap, que hay que recordar si iret=0
		nret=iret;
		if(iret==AT_NOMEM) goto salida_outofmem;
		goto salida;
	}
	matriz->zbounds.maxdp=0; //No hay lagos

salida:
	pfile->file[0]='\0'; //Marca el final
	if(nret==0 || nret==MTIERRA_DSnooverlap){
		Remove_from_delete(matriz->i.flags);
		Remove_from_delete(matriz->suelo);
	}
	free_plist(plist);
	return nret;

salida_outofmem:
	nret=AT_NOMEM;
	goto salida;
}

/*La función tiene que asignar la cota de los DataSet de fondo marino allá donde haya lámina de agua y la cota encontrada
sea menor que la que hay. Mantiene el matriz->Z0 que haya.
    Se complica porque la matriz que se recibe tiene los datos respecto a un cero en zbounds que habrá que cambiar a consecuencia
de los valores del fondo del mar. Así que lo que hacemos es crear una matriz provisional con los valores del fondo del mar y una
marca de ausencia de dato, luego se ajusta el cero absoluto y se mueven los valores de la matriz original en consecuencia y por
último se funden ambas matrices.
    En este ajuste matriz.cielo se ignora, por lo que sus valores pueden dejar de ser válidos tras la llamada a esta función.*/
int matriz_fondomar(Debug_matrizsuelo *debug, MatrizTierra *matriz, const Matriz___Tierra *transf, bint besquina, const DataSets *datasets, const OpcionesMDTierra * _unused(abst)){
	int nret;
	PLIST plist;

	Matriz_ssint _matriz;
	const ZonalDataSet *DS; //El DataSet que emplearemos.
	const LocalDataSet *DSL;
	char8_t tilepath[PathLengthFull_Max];
	Tilename tilename;
	SearchedFileInfo *pfile; //No ha de apuntar nunca más allá del último elemento.
	bint bdone;

	/*Ninguna de las variables de esta función depende en absoluto de la organización interna de la información
	dentro del tile (p.e., no depende de si los datos se almacenan en líneas de latitud o de longitud, o en qué sentido).*/
	nret=0;
	debug->dsGlobal=NULL;
	debug->dsLocal=NULL;
	pfile=debug->files; pfile->file[0]='\0';

	//Determinar el DataSet.
	{DataSetPair S;
	S=selecciona_dataset(transf,datasets->sets.Gfondos,datasets->sets.Lfondos,&datasets->opts,matriz->pixel);
	debug->dsGlobal=DS=S.DS; //Se eliminará más adelante si no se emplea
	debug->dsLocal=DSL=S.DSL;}
	ifunlike(DS==NULL && DSL==NULL) return MTIERRA_DSempty;
	ifunlike(DS==BAD_PTR) return MTIERRA_DSlowres;

	/*Suavizado suavizado;
	suavizado.n_corte=MASK_MAX_NHALF; //Lo que se va a emplear. Son medios-píxeles
	suavizado.corte=NULL; suavizado.nmarco=0;*/
	const ssint offsetZ0=-DS->Dato.uniZ*DS->Dato.offset;

	//Construir la ruta a los tiles
	tilename.path=&tilepath[0];
	tilename.name=strpcpy8(tilename.path,DS->folder);
	if(fileclass_utf8(tilename.path)!=ATFILETYPE_DIRECTORY) return MTIERRA_DSfolder_notfound;
	path_sep(tilename.name);
	tilename.ext=strpcpy8(tilename.name,u8"W40N100");
	*tilename.ext='\0';

	plist=get_new_plist();
	if(plist==PLIST_OUT_OF_MEM) return AT_NOMEM;

	if(matriz->suelo==NULL){
		matriz->npx=transf->npx;
		matriz->npy=transf->npy;
		if(besquina) matriz->npx++, matriz->npy++;
	}
	_matriz.npx=matriz->npx;
	_matriz.npy=matriz->npy;
	_matriz.npixels=matriz->npuntos=matriz->npx*matriz->npy;
	aj_malloc_add(_matriz.m,ssint,matriz->npuntos);
	//Rellenar la matriz con un valor que indica que no hay dato
	{durchlaufep(ssint,_matriz.m,matriz->npuntos) *p=NODATA_ssint;}
	bdone=false;

	if(transf->tipo==TIPO_PROY_PlanoCuadra) nret=matriz_planocuadra(&_matriz,plist,transf,besquina,DS,offsetZ0,-matriz->Z0,(float)matriz->zbounds.escala,NULL,&tilename,&pfile,nfiles_left(pfile));
	else nret=matriz_sis(&_matriz,plist,transf,besquina,DS,offsetZ0,-matriz->Z0,(float)matriz->zbounds.escala,&tilename,&pfile,nfiles_left(pfile));
	ifunlike(nret==AT_NOMEM) goto salida_outofmem;
	ifunlike(nret!=0 && nret!=MTIERRA_DSnooverlap) goto salida;

	matriz->zbounds.maxdp=0;
	if(matriz->suelo==NULL){
		int iret=pasa_a_matriz_y_enmascara_fuera(plist,matriz,_matriz,transf);
		ifnzunlike(iret){ //nret puede ser 0 o MTIERRA_DSnooverlap, que hay que recordar si iret=0
			nret=iret;
			if(iret==AT_NOMEM) goto salida_outofmem;
			goto salida;
		}
	}elif(nret!=MTIERRA_DSnooverlap){
		//Actualilzar zbounds y combinar ambas matrices
		ssint min, max;
		uint Δ, lim;
		u8int esc;

		//Obtener el nuevo valor de min y los nuevos Δ y lim para recalcular, si es necesario, zbounds.escala y .offset
		esc=matriz->zbounds.escala;
		max=matriz->zbounds.zmax*esc;
		min=matriz->zbounds.zmin*esc;
		{durchlaufe2(ssint,_matriz.m,matriz->npuntos,flagMDT,matriz->i.flags){
			if(!ptr_b->fuera && ptr_b->lago!=0 && *ptr<min) min=*ptr;
		}}
		Δ=max-min;
		if(min>0) lim=max;
		else if(max<0) lim=-min;
		else lim=Δ;

		ifunlike(lim>0xFFF0*esc){ //Sólo puede suceder si el desnivel de la zona (o mín., o máx.) es mayor de ¡64 Km!
			nret=MTIERRA_MuchoDesnivel;
			goto salida;
		}
		//Establecer el nuevo zbounds.escala si es necesario
		ifunlike(Δ>0xFFF0){
			//Parte entera al redondear. Los valores negativos quedan negativos.
			do{Δ>>=1, esc>>=1;
				min>>=1, max>>=1;
				{durchlaufep(uEarthHeight,matriz->suelo,matriz->npuntos) *p>>=1;}
				for(uEarthHeight *p=matriz->i.cotaslagos;*p!=MAX_UEARTHHEIGHT;p++) *p>>=1;
				{durchlaufep(ssint,_matriz.m,matriz->npuntos){if(*p!=MAX_UEARTHHEIGHT) *p>>=1;}}
			}while(Δ>0xFFF0);
			matriz->zbounds.escala=esc;
		}
		//Obtener el nuevo min (.max no cambiará, por definición de lo que hace esta función)
		while(esc>1) min>>=1, esc>>=1; //Pasar min a metros
		//min ha cambiado. Hence min<zbounds.zmin. Cambiar offset si es necesario y en ese caso recalcular
		//los valores almacenados en matriz.
		if(min!=matriz->zbounds.zmin) matriz_cambia_minz(matriz,(EarthHeight)min);

		//Combinar ambas matrices. matriz está ya en sus unidades y con offset; _matriz está en sus unidades,
		//pero sin el offset.
		//De paso, calcular maxdp
		nret=MTIERRA_DSnoextradata;
		{const ssint off=matriz->zbounds.offset*matriz->zbounds.escala;
		flagMDT *pflag=matriz->i.flags;
		durchlaufe2(uEarthHeight,matriz->suelo,matriz->npuntos,ssint,_matriz.m){
			uLago_t c;
			if(pflag->fuera){pflag++; continue;}
			ifz(c=(uLago_t)pflag++->lago) continue;
			uEarthHeight h=(uEarthHeight)(*ptr_b-off);
			if(h<*ptr){nret=0; *ptr=h;}
			//
			h=matriz->i.cotaslagos[c]-h; //h is now dp
			maxeq(matriz->zbounds.maxdp,h);
		}}

		Free_remove(_matriz.m);
	}

salida:
	pfile->file[0]='\0'; //Marca el final
	free_plist(plist);
	return nret;

salida_outofmem:
	nret=AT_NOMEM;
	goto salida;
}
