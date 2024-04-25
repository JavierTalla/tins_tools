//Un "out" en str indica fuera de la Tierra (o de la zona representada)
typedef struct{
	char8_t str[8]; //Ej.: "N40E060". Una cadena vacía indica el primero libre.
	s16int φmin; //En realidad, de la última fila almacenada en el tile
	s16int λmin; //De la primera columna almacenada
} TileUsed;

//nlikely: donde creemos que va a estar el punto
//Si λcentral no es finito se ignora
static u16int determina_dónde(u16int ncheck, double λ, double φ, double λcentral, const ZonalDataSet *DS, TileUsed *tiles){
	s16int λ0, φ0; //Límite inferior de la coordenada en el tile
	TileUsed *pt=tiles;

	if(φ<-90 || φ>90 || (isfinite(λcentral) && (λ>λcentral+180 || λ<λcentral-180))){
		if(tiles[ncheck].str[0]=='o') return ncheck;
		while(pt->str[0]!='\0' && pt->str[0]!='o') pt++;
		if(pt->str[0]=='\0'){pt->str[0]='o'; pt->str[1]='u'; pt->str[2]='t'; pt->str[3]='\0'; pt[1].str[0]='\0';}
		return (u16int)(pt-tiles);
	}
	if(λ>=180) λ-=360;
	elif(λ<-180) λ+=360;

	λ0=(s16int)λ; if(λ<0 && (double)λ0!=λ) λ0--;
	φ0=(s16int)φ; if(φ<0 && (double)φ0!=φ) φ0--;

	 if(DS->WE!=1){
		 s16int k=180; do k-=DS->WE; while(k>λ0);
		 λ0=k;
	 }
	 if(DS->SN!=1){
		 s16int k=90; do k-=DS->SN; while(k>φ0);
		 φ0=k;
	 }

	 if(tiles[ncheck].φmin==φ0 && tiles[ncheck].λmin==λ0) return ncheck;

	 while(pt->str[0]!='\0' && !(pt->φmin==φ0 && pt->λmin==λ0)) pt++;
	 if(pt->str[0]=='\0'){
		 uint8m k;
		 char8_t *s=&pt->str[0];
		 pt->φmin=φ0; pt->λmin=λ0;

		if(φ0<0) *s++='S', k=(uint8m)-φ0;
		else *s++='N', k=(uint8m)φ0;
		s[0]=k/10; s[1]=k-10*s[0];
		*s+++='0'; *s+++='0';
		//
		if(λ0<0) *s++='W', k=(uint8m)-λ0;
		else *s++='E', k=(uint8m)λ0;
		s[0]=k/100; k-=100*s[0];
		s[1]=k/10; s[2]=k-10*s[1];
		*s+++='0'; *s+++='0'; *s+++='0';
		*s='\0';

		 pt[1].str[0]='\0';
	 }
	 return (u16int)(pt-tiles);
}

/*suavizado puede ser NULL
La función va recorriendo ordenadamente los tiles que intervienen. La organización de este recorrido depende
exclusivamente de las medidas de la matriz y el área abarcada por cada tile. No depende de la organización interna
de la información dentro del tile. Esa información está en DS.
    Los puntos fuera de la Tierra quedan con NODATA_ssint.
    Los puntos para los que no hay dato quedan con cota "cero": escala*offset. Esto es así porque los óceanos no
tienen tile en el dataset, entendiéndose que están a cota 0.

Return:
	0
	AT_NOMEM
	MTIERRA_DSnooverlap:  Ningún tile del dataset entra en la zona
	TILE_... Códigos de error definidos en MatrizMarcadaReturn.h

En caso de que el return sea MTIERRA_DSnooverlap, toda la matriz habrá quedado con el valor escala*offset.
*/
static int matriz_sis(const Matriz_ssint *matriz, PLIST plist, const Matriz___Tierra *transf, bint besquina, const ZonalDataSet *DS,
	ssint offset,		//A los valores de z del tile se les sumará este offset, en las mismas unidades que los datos del tile.
	float ajusteZ0,	//Después se les sumará este otro valor, pero modificándolo cerca del 0 para mantener fija esa cota 0.
	float escala,		//Los valores de z del tile se multiplicarán por escala al pasarlos a la matriz
	Tilename *tilename, SearchedFileInfo **pfile, ssint nf
){
	union{uint16_t *u; int16_t *i;} Tile;
	const uint φstep=-DS->φstep; //φstep desde φ0 (de la primera fila) a la siguiente, en segundos
	const float _φstep=1.0F/(float)φstep;
	const float _λstep=1.0F/(float)DS->λstep;
	const uint npoints=DS->nrows*DS->ncols;
	//Los que siguen podrían ser float, pero como geo___proy requiere double, así los tendremos
	double *xvals, *yvals;
	typedef struct{
		u16int nt; //Número de tile en la serie tiles;
		Puntoxy_double p; //(λ,φ)
	} Dónde;
	int nret=MTIERRA_DSnooverlap;

	typedef struct{
		uint min, max;
	} MinMax;
	Dónde *dónde;
	double λcentral;
	const float foffset=(float)offset;
	TileUsed *tiles;
	MinMax *Tilemm;
	u16int Ntiles;

	//Rellenar tileinfo
	aj_malloc_add(Tile.i,int16_t,(npoints+1)&~1U);
	aj_malloc_add(xvals,double,matriz->npx+1); //un NAN marcará el final
	aj_malloc_add(yvals,double,matriz->npy+1);
	aj_malloc_add(dónde,Dónde,matriz->npixels);

	//Cálculo de λcentral
	if(transf->sis.sis.proy==SIS_Estereográfica || transf->sis.sis.proy==SIS_Estereográfica_Polar){
		NOTFINITE_d(λcentral); //En estas proyecciones no hay repetición de puntos
	}else{
		Puntoxy_double p;
		p.x=0.5*(transf->rectProy.mx+transf->rectProy.MX);
		p.y=0.5*(transf->rectProy.my+transf->rectProy.MY);
		if(transf->b90){girapunto_tierra___proy(p)}
		p=geo___proy1(&transf->sis,p);
		λcentral=p.λ*PI_180_PI;
	}

	//x y δ serían double igualmente aunque xvals e yvals fueran float, para que el error de redondeo
	//sea menor (aunque daría igual).
	{double x=transf->rectProy.mx, δ=transf->pix_x;
	if(!besquina) x+=0.5*δ;
	double *pf=xvals; dontimes(matriz->npx-1,x+=δ) *pf++=x;
	*pf++=transf->rectProy.MX; NOTFINITE_f(*pf);}
	//
	{double x=transf->rectProy.MY, δ=transf->pix_y;
	if(!besquina) x-=0.5*δ;
	double *pf=yvals; dontimes(matriz->npy-1,x-=δ) *pf++=x;
	*pf++=transf->rectProy.my; NOTFINITE_f(*pf);}

	{Dónde *pd=dónde;
	for(double *py=yvals; isfinite(*py);){
		double y=*py++;
		for(double *px=xvals; isfinite(*px);pd++){
			pd->p.x=*px++; pd->p.y=y;
			if(transf->b90){girapunto_tierra___proy(pd->p)}
		}
	}}
	geo___proy(&dónde->p,usizeof(Dónde),matriz->npixels,&transf->sis);
	Free_remove(yvals);
	Free_remove(xvals);

	Ntiles=100;
	aj_malloc_add_ind(tiles,TileUsed,Ntiles+1);
	aj_malloc_add_ind(Tilemm,MinMax,Ntiles);
	tiles[0].str[0]='\0'; //Un '\0' marca el final
	{durchlaufep(MinMax,Tilemm,Ntiles) p->min=Я, p->max=0;}

	{u16int nt=0; tiles[0].φmin=200; //latest value;
	durchlaufei_fwd(Dónde,dónde,matriz->npixels){
		ptri->p.λ*=PI_180_PI; ptri->p.φ*=PI_180_PI;
		nt=ptri->nt=determina_dónde(nt,ptri->p.λ,ptri->p.φ,λcentral,DS,tiles);
		mineq(Tilemm[nt].min,i); Tilemm[nt].max=i;
		if(ptri->p.λ>=180) ptri->p.λ-=360;
		elif(ptri->p.λ<-180) ptri->p.λ+=360;
		//Hay que reservar _antes_ de que haga falta más elementos de los que hay
		if(nt==Ntiles-1){
			Ntiles<<=1;
			void *prov;

			prov=realloc(tiles,(Ntiles+1)*usizeof(TileUsed));
			ifunlike(prov==NULL) goto salida_outofmem;
			tiles=prov;

			prov=realloc(Tilemm,Ntiles*usizeof(MinMax));
			ifunlike(prov==NULL) goto salida_outofmem;
			Tilemm=prov; //nt+1=Ntiles>>1
			durchlaufep(MinMax,Tilemm+(Ntiles>>1),Ntiles>>1) p->min=Я, p->max=0;
		}
	}}

	/*** IR RECORRIENDO LOS TILES ***/

	u16int i=0;
	for(const TileUsed *pt=tiles; pt->str[0]!='\0'; pt++, i++){
		int ret;
		if(pt->str[0]=='o') ret=TILE_NONEXISTENT;
		else{
			char8_t c=tilename->name[(umint)sizeof(pt->str)-1];
			char8_t *p=strpcpy8(tilename->name,pt->str); *p=c;
			ret=abrir_tile(tilename->path,Tile.u,npoints);
			strcpy8((*pfile)->file,tilename->name);
			(*pfile)->kind=MatrizSuelo_filekind_Global;
			(*pfile)->found=(ret!=TILE_NONEXISTENT);
			(*pfile)+=(nf>1), nf--;
		}
		if(ret!=TILE_NONEXISTENT && ret!=TILE_OPENED) return ret;
		Dónde *pd=dónde+Tilemm[i].min;
		ssint *pm=matriz->m+Tilemm[i].min;
		uint dif=Tilemm[i].max-Tilemm[i].min+1;
		if(ret==TILE_NONEXISTENT){
			ssint z;
			if(pt->str[0]=='o') z=NODATA_ssint;
			else{
				z=0;
				ifunlike(pt->φmin==47 && pt->λmin==-87) z=179; //Lago Superior
				else ifunlike(pt->φmin>=37 && pt->φmin<47 && pt->λmin>=48 && pt->λmin<54) z=(uint16m)-29; //Mar Caspio
			}
			dontimes(dif,(pd++,pm++)){if(pd->nt==i) *pm=z;}
		}else{
			nret=0; //Hata que no se abre uno es MTIERRA_DSnooverlap
			ssint φmax=3600*pt->φmin+(DS->nrows-1)*φstep;
			ssint λmin=3600*pt->λmin;
			dontimes(dif,(pd++,pm++)){
				if(pd->nt!=i) continue;
				pd->p.φ=φmax-3600*pd->p.φ;	ifunlike(pd->p.φ<0) pd->p.φ=0;
				pd->p.λ=3600*pd->p.λ-λmin;		ifunlike(pd->p.λ<0) pd->p.λ=0; //Podría omitirse
				uint mf=(uint)pd->p.φ/φstep;
				pd->p.φ-=mf*φstep;					ifunlike(mf==(uint)DS->nrows-1) pd->p.φ=0; //Podría omitirse
				uint mc=(uint)pd->p.λ/DS->λstep;
				pd->p.λ-=mc*DS->λstep;			ifunlike(mc>=(uint)DS->ncols-1) mc=DS->ncols-1, pd->p.λ=0; //Para que no vaya a mirar la columna siguiente
				int16_t *pi=Tile.i+mf*DS->ncols+mc;

				float v, v0, v1;
				float f=(float)pd->p.λ*_λstep;
				//
				v0=(int16_t)u2___bigend_u2(pi[0]);
				if(f!=0){v1=(int16_t)u2___bigend_u2(pi[1]); v0=v0*(1.F-f)+f*v1;} //v0 queda con el resultado de ambos
				v=v0;
				if(pd->p.φ!=0){
					pi+=DS->ncols;
					v0=(int16_t)u2___bigend_u2(pi[0]);
					if(f!=0){v1=(int16_t)u2___bigend_u2(pi[1]); v0=v0*(1.F-f)+f*v1;} //v0 queda con el resultado de ambos
					f=(float)pd->p.φ*_φstep;
					v=v*(1.F-f)+f*v0;
				}
				v+=foffset;
				apply_ajusteZ0_valor(v,ajusteZ0)
				v*=escala;
				v+=0.5F; //Si es negativo, se corrige en la instrucción siguiente
				ssint v2=(ssint)v-(v<0);

				*pm=v2;
		}	}
	}

	Free_remove(Tilemm);
	Free_remove(tiles);
	return nret;

salida_outofmem:
	plist_to_immediate(plist);
	return AT_NOMEM;
}
