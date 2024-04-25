uEarthHeight zmax_agua_borde(const MatrizTierra *matriz){
	uEarthHeight maxz;
	const flagMDT *pflag;
	const uEarthHeight * const pc=&matriz->i.cotaslagos[0];
	const uint salto=matriz->npx-1;

	maxz=0;
	pflag=matriz->i.flags;

	dontimes(salto+1,pflag++){
		if(pflag->lago==0) continue;
		uEarthHeight z=pc[pflag->lago]; maxeq(maxz,z);
	}
	dontimes(matriz->npy-2,){
		if(pflag->lago!=0){uEarthHeight z=pc[pflag->lago]; maxeq(maxz,z);} pflag+=salto;
		if(pflag->lago!=0){uEarthHeight z=pc[pflag->lago]; maxeq(maxz,z);} pflag++;
	}
	dontimes(salto+1,pflag++){
		if(pflag->lago==0) continue;
		uEarthHeight z=pc[pflag->lago]; maxeq(maxz,z);
	}
	return maxz;
}

MatrizMarcadaReturn matrizmarcada___userspecs(MatrizTierra *matriz, const Matriz___Tierra *transf, bint besquina, const DataSets *datasets, Polígono_xy *clip, u8int npols, const OpcionesMDTierra *abst, float npix){
	MatrizMarcadaReturn mret;

	MatrizTierra_setNULL(*matriz);
	matriz->pixel=transf->pixel_m*npix;
	matriz->px=transf->pixel_m;
	matriz->npy=matriz->npx=0; //Indica que ninguna función ha creado la matriz
	matriz->esta.flags=0;
	mret.nret=0;
	mret.nret_edif=mret.nret_fondo=mret.nret_suelo=MTIERRA_DSnotsearched;
	mret.suelo.files=searchedfiles_suelo;			mret.suelo.nfiles=NSearchedFiles_suelo;
	mret.fondo.files=searchedfiles_fondo;		mret.fondo.nfiles=NSearchedFiles_fondo;
	mret.edificios.files=searchedfiles_edificios;	mret.edificios.nfiles=NSearchedFiles_edificios;
	mret.suelo.dsGlobal=NULL;
	mret.suelo.dsLocal=NULL;
	mret.fondo.dsGlobal=NULL;
	mret.fondo.dsLocal=NULL;
	mret.edificios.dset=NULL;

	//Flags. Aplicar el clip
	//Si el área fuera del recorte se va a ignorar, recortamos aquí para que las funciones
	//ignoren el área fuera del recorte. Si no, hay que esperar a después.
	if(clip!=NULL && npols!=0){
		matriz->npx=transf->npx;
		matriz->npy=transf->npy;
		if(besquina) matriz->npx++, matriz->npy++;
		matriz->npuntos=matriz->npx*matriz->npy;
		matriz->i.flags=n_malloc(flagMDT,matriz->npuntos);
		ifunlike(matriz->i.flags==NULL){mret.nret=AT_NOMEM; goto salida;}
		zeroset(matriz->i.flags,usizeof(uint)*matriz->npuntos);
		ifunlike(aplica_clipλφ(matriz,transf,besquina,clip,npols)){mret.nret=AT_NOMEM; goto salida;}
	}

	//El suelo.
	//if(abst->tipo_producto!=EARTH_SOLO_FONDOMAR){
		mret.nret_suelo=matrizsuelo___userspecs(&mret.suelo,matriz,transf,besquina,datasets,abst);
		ifunlike(mret.nret_suelo!=0 && mret.nret_suelo!=MTIERRA_DSnooverlap) goto salida;

		//Copiar los valores de mínimo y máximo a las distintas categorías
		matriz->zbounds.tierra.zmin=matriz->zbounds.zmin;
		matriz->zbounds.tierra.zmax=matriz->zbounds.zmax;

		//Identificar las laminas de agua
		float DS_step;
		if(mret.suelo.dsLocal!=NULL) DS_step=fabsf((float)mret.suelo.dsLocal->φstep)*0.01F;
		else DS_step=PIXEL_METRO___Φ((float)mret.suelo.dsGlobal->φstep);
		mret.nret=interpreta_matriz(matriz,DS_step,&abst->lagos);
		ifnzunlike(mret.nret) goto salida; //nret=AT_NOMEM o nret=MTIERRA_TooManyLakes
	//}

	//El fondo del mar
	if(abst->tipo_producto!=EARTH_SOLO_TIERRA){
		mret.nret_fondo=matriz_fondomar(&mret.fondo,matriz,transf,besquina,datasets,abst);
		ifunlike(mret.nret_fondo==AT_NOMEM) goto salida;
 		if(mret.nret_suelo!=0 && mret.nret_suelo!=MTIERRA_DSnooverlap
			&& mret.nret_fondo!=0 && mret.nret_fondo!=MTIERRA_DSnooverlap
		   ) goto salida;
	}

	//Reservamos pero aún no rellenamos nada.
	if(matriz->esta.nlagos!=0){
		matriz->i.lagos=n_malloc(LagoZ,matriz->esta.nláminas+2);
		ifunlike(matriz->i.lagos==NULL){mret.nret=AT_NOMEM; goto salida;}
		durchlaufep(LagoZ,matriz->i.lagos+1,matriz->esta.nláminas) *p=LagoZVacío;
		matriz->i.lagos[0].depth=matriz->i.lagos[0].cota=0;
		matriz->i.lagos[matriz->esta.nláminas+1]=LagoZNull;
	}

	//Los edificios. Asigna lagos.supe_edif
	if(matriz->pixel<abst->edificios.píxel){
		const PointsDataSet *pt=&datasets->sets.Puntos[0];
		if(pt->folder[0]!='\0'){
			mret.nret_edif=matriz_edificios(&mret.edificios,matriz,transf,besquina,abst,pt);
			ifunlike(mret.nret_edif==AT_NOMEM) goto salida;
		}
	}

	{durchlaufep(flagMDT,matriz->i.flags,matriz->npuntos){
		if(!p->fuera) continue;
		matriz->esta.flags|=FLAG_MATRIZ_HAYFUERA; break;
	}}
	if(matriz->esta.flags&FLAG_MATRIZ_HAYFUERA){
		durchlaufep(flagMDT,matriz->i.flags,matriz->npuntos){if(p->fuera) p->lago=0;}
		ClipModo c=ClipModo_recortar;
		if(abst->fuera_is_agua) c=ClipModo_lago;
		ifnzunlike(mret.nret=trata_recorte(matriz,c)) goto salida;
	}

	//Los datos para cada lago. Esperamos hasta el final para que la escala de la Z de la matriz ya no cambie
	if(matriz->esta.nlagos!=0){ifunlike(explorar_lagos(matriz,abst)==AT_NOMEM) goto salida;}

	//zmax_agua_borde hay que calcularlo al final porque los bounds de matriz los van a ir
	//cambiando cada una de las funciones.
	matriz->zmax_aguaborde=zmax_agua_borde(matriz);

	//Mirar qué hay de cada. Hay que hacerlo al final porque, por ejemplo, un edificio
	//anula que el punto sea de agua
	mret.bflags=0;

	{durchlaufep(flagMDT,matriz->i.flags,matriz->npuntos){
		if(p->fuera) continue;
		if(p->lago==0){mret.bflags|=FLAG_MATRIZ_HAYTIERRA; break;}
	}}
	if(abst->tipo_producto==EARTH_SOLO_TIERRA || matriz->zbounds.maxdp==0){
		durchlaufep(flagMDT,matriz->i.flags,matriz->npuntos){
			if(p->fuera) continue;
			if(p->lago!=0){mret.bflags|=FLAG_MATRIZ_HAYAGUA; break;}
		}
	}else{ //maxdp!=0
		mret.bflags|=FLAG_MATRIZ_HAYAGUA | FLAG_MATRIZ_HAYFONDOMAR;
	}

	matriz->esta.flags|=(umint)mret.bflags;

salida:
	if(mret.nret==AT_NOMEM || mret.nret_suelo==AT_NOMEM || mret.nret_fondo==AT_NOMEM || mret.nret_edif==AT_NOMEM){
		mret.nret=AT_NOMEM;
		free_null_if_MatrizTierra(*matriz);
	}
	return mret;
}
