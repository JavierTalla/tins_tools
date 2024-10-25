int matriz_edificios(Debug_matrizedificios *debug, MatrizTierra *matriz, const Matriz___Tierra *transf, bint besquina, const OpcionesMDTierra *abst, const PointsDataSet *dset){
	int nret;
	float ΔZ;
	Sistema sis;
	Extremos2D_dbl lims;
	u8int husos[3];
	static char8_t dataset_folder[100];
	char8_t tilepath[PathLengthFull_Max];
	TilenameFolders tilename;
	struct{
		uint x,X;
		uint y,Y;
	} lim;
	double Λ0, Φ0;
	const double _pix_λ=1.0/transf->cc.pix_λ,
					_pix_φ=1.0/transf->cc.pix_φ;
	const double uniz=1.0/dset->Dato.uniZ;
	uEarthHeight ht_error; //Límite absoluto permitido de altura sobre el terreno
	SearchedFileInfo *pfile; //No ha de apuntar nunca más allá del último elemento.
	ssint nf; //Número de elementos aún disponibles en el array debug.files

	dset=selecciona_dataset_puntos(transf,dset,matriz->pixel);
	if(dset==NULL) return MTIERRA_DSnooverlap;

	debug->dset=dset;
	pfile=debug->files; pfile->file[0]='\0';
	nf=debug->nfiles;

	//Construir la ruta a los tiles
	tilename.path=&tilepath[0];
	tilename.folder=tilename.namebase=strpcpy8(tilename.path,dset->folder);
	if(fileclass_utf8(tilename.path)!=ATFILETYPE_DIRECTORY) return MTIERRA_DSfolder_notfound;
	strcpy8(dataset_folder,tilename.path);
	path_sep(tilename.folder);

	/* Valores constantes para todo el cálculo */
	{uint u=abst->edificios.htlim_grupo*(uint8m)matriz->zbounds.escala;
	if(u>=MAX_UEARTHHEIGHT) ht_error=MAX_UEARTHHEIGHT;
	else ht_error=(uEarthHeight)u;}
	ΔZ=dset->Z0-matriz->Z0; //Valor a sumar a las z del dataset
	//Límites de la talla en segundos
	lims.mx=3600.0*transf->cc.λmin;
	lims.my=3600.0*transf->cc.φmin;
	lims.MX=3600.0*transf->cc.λmax;
	lims.MY=3600.0*transf->cc.φmax;

	//Cálculo de Λ0 y Φ0. Por definición, medio píxel a la izda. y hacia arriba del primer
	//punto almacenado en la matriz.
	Λ0=transf->cc.λmin;
	if(transf->b90==0) Φ0=transf->cc.φmax;
	else Φ0=transf->cc.φmin;
	if(besquina){
		Λ0-=0.5F*transf->cc.pix_λ;
		if(transf->b90==0) Φ0+=0.5F*transf->cc.pix_φ;
		else Φ0-=0.5F*transf->cc.pix_φ;
	}
	/*Fin*/

	if(matriz->cielo==NULL){
		uint k=matriz->npx*matriz->npy;
		aj_malloc_return(matriz->cielo,uEarthHeight,k);
		if(sizeof(uEarthHeight)==sizeof(uint)){memcpy_uint(matriz->cielo,matriz->suelo,k);}
		else memcpy_char16((char16_t*)matriz->cielo,matriz->suelo,k);
	}

	setup_Sistema_UTM_Norte(&sis,0); //Λ0 se asignará para cada huso
	{double λcentral=0.5*(transf->cc.λmin+transf->cc.λmax);
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

	nret=MTIERRA_DSnooverlap;
	for(uint *phusos=husos; *phusos!=Я8;){
		Extremos2D_dbl geog, UTM;
		u8int huso=*phusos++;
		sis.sis.Λ0=6.0*(huso-1)-177.0;
		sis.sis.Λ0*=PI_180;

		char8_t h0, h1;
		h1=(char8_t)(huso/10);
		h0=(char8_t)(huso-10*h1);
		h1+='0'; h0+='0';
		tilename.name=tilename.folder;
		*tilename.name++=h1;
		*tilename.name++=h0;
		*tilename.name='\0';

		if(fileclass_utf8(tilename.path)!=ATFILETYPE_DIRECTORY) continue;
		path_sep(tilename.name);
		tilename.ext=strpcpy8(tilename.name,u8"edif_GEO29_476_4750.");
		tilename.name+=strlen8(u8"edif_GEO"); *tilename.name++=h1; *tilename.name=h0; tilename.name+=2;
		strcpy8(tilename.ext,u8"xyz");

		geog.mx=transf->cc.radianes.λmin;
		geog.MX=transf->cc.radianes.λmax;
		geog.my=transf->cc.radianes.φmin;
		geog.MY=transf->cc.radianes.φmax;
		UTM=rect_proy___rect_geog(geog,&sis);
		ifunlike(UTM.mx<0 || UTM.my<0) continue;
		lim.x=(uint)UTM.mx/1000;		lim.X=((uint)UTM.MX+999)/1000;
		lim.y=(uint)UTM.my/1000;		lim.Y=((uint)UTM.MY+999)/1000;
		ifunlike(lim.X>=1000 || lim.Y>=10000) continue;
		lim.x-=lim.x%dset->WE;
		lim.y-=lim.y%dset->SN;

		for(uint x=lim.x; x<lim.X; x+=dset->WE){
			char8_t *pc=tilename.name;
			{uint xx=x;
			*pc=(char8_t)(xx/100); xx-=100**pc; *pc+++='0';
			*pc=(char8_t)(xx/10); xx-=10**pc; *pc+++='0';
			*pc=(char8_t)(xx+'0'); pc+=2;}

			for(uint y=lim.y; y<lim.Y; y+=dset->SN, pfile+=(nf>1), nf--){
				char8_t *pd=pc;
				{uint xx=y;
				*pd=(char8_t)(xx/1000); xx-=1000**pd; *pd+++='0';
				*pd=(char8_t)(xx/100); xx-=100**pd; *pd+++='0';
				*pd=(char8_t)(xx/10); xx-=10**pd; *pd+++='0';
				*pd=(char8_t)(xx+'0'); pd+=2;}

				FILE_TYPE hgt;
				uint cabecera[7], *pn;
				PuntoXYZ_double Q;
				Puntoxy_double p; double Z;
				uint16_t *puntos;
				uint k;

				strcpy8(pfile->file,tilename.namebase); pfile->found=false;
				FOPEN_B(hgt,tilename.path);
				if(OPEN_FAILURE(hgt)) continue;
				pfile->found=true;
				pn=cabecera;

				FREAD(pn,hgt,sizeof(uint),7);
				k=u4___littleend_u4(*pn); pn++;		//Número de registros
				Q.X=(double)(int)u4___littleend_u4(*pn); pn++; //X mín
				Q.Y=(double)(int)u4___littleend_u4(*pn); pn++; //Y mín
				Q.Z=(double)(int)u4___littleend_u4(*pn); pn++;		//Z mín
				if(Q.X>=lims.MX || Q.Y>=lims.MY){FCLOSE(hgt); continue;}

				p.x=(double)(int)u4___littleend_u4(*pn); pn++; //X máx
				p.y=(double)(int)u4___littleend_u4(*pn); pn++; //Y máx
				Z=(double)(int)u4___littleend_u4(*pn); pn++; //Z máx
				if(p.x<lims.mx || p.y<lims.my){FCLOSE(hgt); continue;}

				if(nret==MTIERRA_DSnooverlap) nret=MTIERRA_DSnoextradata;

				k=3*(k+1); //k+1 porque hay un último registro de cierre, con los tres valores a Я,Я,Я.
				puntos=n_malloc(uint16_t,k);
				if(puntos==NULL){FCLOSE(hgt); return AT_NOMEM;}
				{size_t kr=FREAD(puntos,hgt,sizeof(int16_t),k);
				FCLOSE(hgt);
				if(kr!=k) return TILE_BadSize;}

				if(puntos[k-3]!=Я16 || puntos[k-2]!=Я16 || puntos[k-1]!=Я16) return TILE_BadSize; //Cierre del fichero

				Q.Z+=ΔZ;
				for(uint16_t* ptr=puntos; *ptr!=Я16;){
					uEarthHeight z;
					uint ny,nx,n;

					const double gra=1.0/3600;
					p.x=(double)u2___littleend_u2(*ptr); ptr++;
					p.y=(double)u2___littleend_u2(*ptr); ptr++;
					Z=(double)u2___littleend_u2(*ptr); ptr++;
					p.x*=dset->uniXY; //Pasar a segundos
					p.y*=dset->uniXY;
					Z*=uniz; //Pasar a metros
					p.x+=Q.X; p.y+=Q.Y; Z+=Q.Z; //Q.X, Q.Y están en segundos.
					if(p.λ<lims.mx || p.λ>=lims.MX || p.φ<lims.my || p.φ>=lims.MY) continue;
					ifunlike(Z<matriz->zbounds.zmin) continue;
					p.λ*=gra; p.φ*=gra; //Pasar a grados

					p.λ-=Λ0; p.λ*=_pix_λ;
					if(!transf->b90) p.φ=Φ0-p.φ;
					else p.φ-=Φ0;
					p.φ*=_pix_φ;
					ny=(uint)p.φ; nx=(uint)p.λ;
					if(transf->b90) n=nx, nx=ny, ny=n;
					n=ny*matriz->npx+nx;
					if(matriz->i.flags[n].fuera) continue;

					//Si se elimina el "1 || " se eliminarán los puntos de "edificio" sobre lagos. Estos puntos
					//son pantalanes, puentes, embarcaciones, etc.
					if(!matriz->i.flags[n].fuera && (1 || matriz->i.flags[n].lago==0)){
						z=(uEarthHeight)((Z-matriz->zbounds.offset)*matriz->zbounds.escala);
						if(z>matriz->cielo[n] && (uint)(z-matriz->suelo[n])<ht_error){
							nret=0; //Algún punto aporta algo
							matriz->cielo[n]=z;
							EarthHeight zm=(EarthHeight)Z;				//No es exactamente ceil(Z) lo que queremos
							iflike((uEarthHeight)(zm-matriz->zbounds.offset)*matriz->zbounds.escala<z) zm++;
							if(zm>matriz->zbounds.zmax) matriz->zbounds.zmax=zm;
						}
					}
				}

				free(puntos);
			}
		}
	}
	if(nret!=0) goto salida; //Ningún punto aportó nada

	//Eliminar puntos aislados demasiado altos
	//TEST mira si el punto es muy alto y establece el valor de h que al menos uno de sus adyacentes debe de superar
	//El !=0 es para evitar el warning del compilador.
#define NPX (ssint)matriz->npx
#define TEST _unlikely((h=*ptrb-*ptr)>=ht_error) && _likely((h/=3)!=0)
#define test(n) (ptrb[n]-ptr[n]<h)

	{uint u=abst->edificios.htlim_aislado*(uint8m)matriz->zbounds.escala;
	iflike(u<MAX_UEARTHHEIGHT){
		uEarthHeight h;
		ht_error=(uEarthHeight)u;

		uEarthHeight *ptr=matriz->suelo,
						   *ptrb=matriz->cielo;
		//Esquina sup. izda.
		if(TEST && test(1) && test(NPX)) *ptrb=*ptr;			ptr++, ptrb++;
		//Fila superior
		dontimes(matriz->npx-2,(ptr++,ptrb++)){if(TEST && test(-1) && test(1) && test(NPX)) *ptrb=*ptr;}
		//Esquina sup. dcha.
		if(TEST && test(-1) && test(NPX)) *ptrb=*ptr;		ptr++, ptrb++;
		//Bloque
		dontimes(matriz->npy-2,){
			if(TEST && test(-NPX) && test(1) && test(NPX)) *ptrb=*ptr;		ptr++, ptrb++;
			dontimes(matriz->npx-2,(ptr++,ptrb++)){if(_unlikely(TEST) && test(-NPX) && test(-1) && test(1) && test(NPX)) *ptrb=*ptr;}
			if(TEST && test(-NPX) && test(-1) && test(NPX)) *ptrb=*ptr;		ptr++, ptrb++;
		}
		//Esquina inf. izda.
		if(TEST && test(1) && test(-NPX)) *ptrb=*ptr;		ptr++, ptrb++;
		//Fila inferior
		dontimes(matriz->npx-2,(ptr++,ptrb++)){if(_unlikely(TEST) && test(-1) && test(1) && test(-NPX)) *ptrb=*ptr;}
		//Esquina inf. dcha.
		if(TEST && test(-1) && test(-NPX)) *ptrb=*ptr;
	}}
#undef test
#undef TEST
#undef NPX

	//Asignar el flag de edificio y eliminar el de lago donde haya edificio
	{durchlaufe2(uEarthHeight,matriz->suelo,matriz->npuntos,uEarthHeight,matriz->cielo){
		if(*ptr_b==*ptr) continue;
		flagMDT *pf=matriz->i.flags+(ptr-matriz->suelo);
		if(pf->fuera) continue; //Shoud never happen, because of *ptr_b==*ptr above
		pf->edif=1;
		ifnz(pf->lago){
			matriz->i.lagos[matriz->i.gruposlago[pf->lago]].supe_edif++;
			pf->lago=0;
		}
	}}

salida:
	pfile->file[0]='\0';
	return nret;
}
