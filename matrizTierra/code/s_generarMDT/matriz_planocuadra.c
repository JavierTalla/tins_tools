typedef struct{
	float height; //En grados
	float φstep, λstep; //En segundos
	uint npoints; //=nrows*ncols
	uint usize;
} TileInfo; //Información obtenida a partir del DataSet DS

typedef struct{
	ssint MINX; //Coordenada (φ,λ,X o Y) del límite inferior del primer tile de la serie que se abrirá
	uint ntiles;		//Número de tiles de la serie.
	uint *mat_nline;	//El número de fila (o columna) con menor valor de 'X', de entre las
							//filas (o columnas) de la matriz, de las que entran en cada tile de la serie,
	float *mat_Xnline; //El valor de la coordenada 'X' de la fila (o columna) mat_nline.
} XSteps;

/* Calcula los valores de los arrays mat_nline y mat_Xline. Tienen que tener espacio para xsteps->ntiles+1
elementos. El primero de ambos arrays contiene el número de fila (o columna) con menor valor de 'X',
de entre las filas (o columnas) de la matriz, de las que entran en cada tile de la serie. mat_Xline contiene
la coordenada 'X' de la fila (o columna) indicada en mat_nline. En todos los casos se cuentan las
filas (o columnas de la matriz) comenzando en la de menor coordenada 'X', hacia arriba. Las filas
(o columnas) en la matriz podrán realmente estar almacenadas así o al revés.
    En ambos arrays se añade un elemento de cierre que contiene el nº de línea y la coordenada 'X' de la
que sería la siguiente línea en la matriz. Por ejemplo, si la matriz tiene 40 líneas, la coordenada 'X'
de la última es 1300 y el paso de una a otra es de 32.5, se tendrá

	mat_nline[xsteps->ntiles]=40;			mat_Xline[xsteps->ntiles]=1332.5;

    Si un valor de 'X' coincide exactamente en la divisoria de dos tiles se asignará al tile superior, salvo la
última fila (o columna) de la matriz, que siempre irá en el tile inferior para no tener que emplear un tile
más solamente para ella.

	mat_xmin:	La coordenada del borde izquierdo o superior de la matriz (el que corresponda a las coordendas 'X').
	pix_x:			El paso en la matriz de una línea a otra.
	mat_nlines:	Hace falta solamente para el último elemento del array mat_nline: En el ejemplo de
		arriba sería mat_nlines=40.
	besquina:		Si es true, la primera fila o columna de la matriz a calcular tiene por coordenada 'X'
		mat_xmin; si es false, tiene mat_xmin+0.5*pix_x. En ambos caso de una línea a otra se avanza pix_x.
	tile_step:		Lo que va de un tile al siguiente en el sentido de la coordenada 'X' (es decir, su alto o su ancho),
		en las mismas unidades que mat_xmin y pix_x.
	*/
void setup_Xsteps_serie(XSteps *xsteps, float mat_xmin, float pix_x, uint mat_nlines, bint besquina, ssint tile_step){
	uint *pcol=xsteps->mat_nline;
	float *pλ0=xsteps->mat_Xnline;
	float λ0=mat_xmin;
	if(!besquina) λ0+=0.5F*pix_x;
	*pcol++=0;	//Valores para el primer tile
	*pλ0++=λ0; //
	ssint tile_λmin=xsteps->MINX+tile_step; //Empezar ya en el segundo tile
	dontimes(xsteps->ntiles-1,(pcol++,pλ0++,tile_λmin+=tile_step)){
		*pcol=(uint)(((float)tile_λmin-λ0)/pix_x);
		*pλ0=λ0+(float)*pcol*pix_x;
		if(*pλ0<tile_λmin) (*pcol)++, *pλ0+=pix_x;
	}
	//El valor de cierre es el de la columna que sería la siguiente
	*pcol=mat_nlines;
	*pλ0=λ0+(float)*pcol*pix_x;
}

void setup_λsteps_min_y_n(const Matriz___Tierra *transf, XSteps *λsteps, ssint tile_step){
	λsteps->MINX=(ssint)transf->cc.λmin;	if(transf->cc.λmin<0 && (float)λsteps->MINX!=transf->cc.λmin) λsteps->MINX--;

	 if(tile_step==1){
		 λsteps->ntiles=(ssint)transf->cc.λmax;	if((float)(ssint)λsteps->ntiles!=transf->cc.λmax && transf->cc.λmax>=0) λsteps->ntiles++;
		 λsteps->ntiles-=λsteps->MINX;
	}else{
		ssint k;
		if(λsteps->MINX>=180) k=180+360;
		else if(λsteps->MINX>=-180) k=180;
		else k=-180;
		do k-=tile_step; while(k>λsteps->MINX);
		λsteps->MINX=k;

		k=λsteps->MINX+tile_step;
		λsteps->ntiles=1;
		while(k<transf->cc.λmax) k+=tile_step, λsteps->ntiles++;
	}
}
void setup_φsteps_min_y_n(const Matriz___Tierra *transf, XSteps *φsteps, ssint tile_step){
	φsteps->MINX=(ssint)transf->cc.φmin;	if(transf->cc.φmin<0 && (float)φsteps->MINX!=transf->cc.φmin) φsteps->MINX--;

	if(tile_step==1){
		φsteps->ntiles=(ssint)transf->cc.φmax;	if((float)(ssint)φsteps->ntiles!=transf->cc.φmax && transf->cc.φmax>=0) φsteps->ntiles++;
		φsteps->ntiles-=φsteps->MINX;
	}else{
		ssint k=90;
		do k-=tile_step; while(k>φsteps->MINX);
		φsteps->MINX=k;
		k=φsteps->MINX+tile_step;
		φsteps->ntiles=1;
		while(k<transf->cc.φmax) k+=tile_step, φsteps->ntiles++;
	}
}

/*suavizado puede ser NULL
La función que realmente interpola es interpola_tile. Esta simplemente va recorriendo ordenadamente los tiles
que intervienen, llamando a interpola_tile para cada uno de ellos. La organización de este recorrido depende
exclusivamente de las medidas de la matriz y el área abarcada por cada tile. Por eso ninguna de las variables de
esta función depende en absoluto de la organización interna de la información dentro del tile (p.e., no depende
de si los datos se almacenan en líneas de latitud o de longitud, o en qué sentido).
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
static int matriz_planocuadra(const Matriz_ssint *matriz, PLIST plist, const Matriz___Tierra *transf, bint besquina, const ZonalDataSet *DS,
	ssint offset,		//A los valores de z del tile se les sumará este offset, en las mismas unidades que los datos del tile.
	float ajusteZ0,	//Después se les sumará este otro valor, pero modificándolo cerca del 0 para mantener fija esa cota 0.
	float escala,		//Los valores de z del tile se multiplicarán por escala al pasarlos a la matriz
	const Suavizado *suavizado,
	Tilename *tilename, SearchedFileInfo **pfile, ssint nf
){
	XSteps φsteps, λsteps;
	ssint Δlineaφ, //Lo que varía la posición dentro de matriz al avanzar una línea de φ hacia valores de φ menores.
			Δlineaλ; //Lo que varía la posición dentro de matriz al avanzar una línea de λ hacia valores de λ mayores.
	ssint Δlineaφ_corte, Δlineaλ_corte; //Lo mismo, pero para la matriz de suavizado, que tiene las filas más largas.
	union{uint16_t *u; int16_t *i;} Tile;
	TileInfo TI;
	float pix_φs, pix_λs; //De la matriz (e.d., = a los de transf). Valores en segundos
	uint npλ,npφ;
	umint n_corte;
	if(!transf->b90) npλ=matriz->npx, npφ=matriz->npy;
	else npλ=matriz->npy, npφ=matriz->npx;
	int nret=MTIERRA_DSnooverlap;

	//Rellenar tileinfo
	TI.φstep=(float)DS->φstep;
	TI.λstep=(float)DS->λstep;
	TI.height=((DS->nrows-1)*abs(DS->φstep))/3600.0F; //Altura entre la primera y última filas, en grados
	TI.npoints=DS->nrows*DS->ncols;
	TI.usize=(TI.npoints+1)*usizeof(uint16_t);
	aj_malloc_add(Tile.i,int16_t,(TI.npoints+1)&~1U);

	setup_φsteps_min_y_n(transf,&φsteps,(ssint)DS->SN);
	aj_malloc_add(φsteps.mat_nline,uint,φsteps.ntiles+1);
	aj_malloc_add(φsteps.mat_Xnline,float,φsteps.ntiles+1);	//El elemento +1 en estas dos no se empleará para interpolar
	setup_Xsteps_serie(&φsteps,transf->cc.φmin,transf->cc.pix_φ,npφ,besquina,(ssint)DS->SN);

	setup_λsteps_min_y_n(transf,&λsteps,(ssint)DS->WE);
	aj_malloc_add(λsteps.mat_nline,uint,λsteps.ntiles+1);
	aj_malloc_add(λsteps.mat_Xnline,float,λsteps.ntiles+1);	//pero sí transitoriamente para calcular φsteps.nline[ntiles_φ] y λsteps.nline[ntiles_λ]
	setup_Xsteps_serie(&λsteps,transf->cc.λmin,transf->cc.pix_λ,npλ,besquina,(ssint)DS->WE);

	/*** IR RECORRIENDO LOS TILES ***/

	if(!transf->b90){Δlineaφ=npλ; Δlineaλ=1;}
	else{Δlineaλ=npφ; Δlineaφ=-1;}

	if(suavizado!=NULL && suavizado->corte!=NULL){
		Δlineaφ_corte=Δlineaφ;
		Δlineaλ_corte=Δlineaλ;
		if(!transf->b90) Δlineaφ_corte+=suavizado->nmarco;
		else Δlineaλ_corte+=suavizado->nmarco;
		n_corte=suavizado->n_corte;
	}else{
		Δlineaφ_corte=0;
		Δlineaλ_corte=0;
		n_corte=0;
	}

	pix_λs=transf->cc.pix_λ*3600; //A partir de ahora necesitamos sus valores en segundos,
	pix_φs=transf->cc.pix_φ*3600;
	uint *pnlineφ=φsteps.mat_nline;
	float *pφ0=φsteps.mat_Xnline; //De la primera fila de la matriz que entra en el tile siguiente (la de mín. φ)
	ssint tile_bottomφ=φsteps.MINX;	//e.g. N36, 37, 38 ...
	uint nlineφ_top=*pnlineφ++; //De la última fila de la matriz que entra en cada tile
	pφ0++;
	dontimes(φsteps.ntiles,tile_bottomφ+=(ssint)DS->SN){
		uint nlineφ_bottom; //De la primera fila de la matriz que entra en el tile
		float φtop_offset; //φ offset de nlineφ_top respecto al borde superior del tile. <=0
		ssint *plineφ_beginning; //Al punto de la matriz correspondiente a nlineφ_top y mín. λ.
		umint *plineφ_corte;

		nlineφ_bottom=nlineφ_top;			//nlineφ_bottom y nlineφ_top se cuentan desde la línea con mínimo φ de la matriz hacia arriba.
		nlineφ_top=*pnlineφ++;
		φtop_offset=*pφ0++-transf->cc.pix_φ;	//Provisionalmente es el valor de φ de la primera líneaφ (máx. φ) de matriz de las que entran en el tile.
														//Se convertirá en offset dos líneas más abajo
		ifunlike(nlineφ_bottom==nlineφ_top) continue;	//Nos saltamos la fila de tiles. Sucede si de una líneaφ de matriz a la siguiente saltamos más de un tile.
		φtop_offset-=((float)tile_bottomφ+TI.height);	// -= φ(parte superior del tile). Queda un valor negativo.
		φtop_offset*=3600; //En segundos
		{uint k;
		if(tile_bottomφ>=0) tilename->name[0]='N', k=tile_bottomφ;
		else tilename->name[0]='S', k=-tile_bottomφ;
		tilename->name[1]=(char8_t)(k/10);
		tilename->name[2]=(char8_t)(k-tilename->name[1]*10);
		tilename->name[1]+='0'; tilename->name[2]+='0';}

		if(!transf->b90) plineφ_beginning=matriz->m+npλ*(npφ-nlineφ_top);
		else plineφ_beginning=matriz->m+(nlineφ_top-1);

		if(suavizado!=NULL && suavizado->corte!=NULL){
			if(!transf->b90) plineφ_corte=suavizado->corte+(npλ+suavizado->nmarco)*(npφ-nlineφ_top);
			else plineφ_corte=suavizado->corte+(nlineφ_top-1);
		}else{
			plineφ_corte=0;
		}

		//Número de líneas de la matriz que se rellenarán en esta iteración del bucle
		uint nlinesφ_interp=nlineφ_top-nlineφ_bottom; //nlineφ_bottom not needed any more.

		if(tile_bottomφ>=90 || tile_bottomφ<-90){
			if(!transf->b90){
				ssint *p=plineφ_beginning;
				uint n=nlinesφ_interp*npλ;
				dontimes(n,) *p++=NODATA_ssint;
			}else{
				ssint *p=plineφ_beginning-(nlinesφ_interp-1);
				const uint salto=npφ-nlinesφ_interp;
				dontimes(npλ,p+=salto){
					dontimes(nlinesφ_interp,) *p++=NODATA_ssint;
				}
			}
			continue;
		}

		uint *pnlineλ=λsteps.mat_nline;
		float *pλ0=λsteps.mat_Xnline;
		ssint tile_leftλ=λsteps.MINX;		//e.g.  W9, 8, 7 ...
		uint nlineλ_right=*pnlineλ++;
		dontimes(λsteps.ntiles,(tile_leftλ+=(ssint)DS->WE, (*pfile)+=(nf>1), nf--)){
			uint nlineλ_left;
			float λleft_offset; //λ offset de ncol_left respecto al borde izquierdo del tile. >=0

			nlineλ_left=nlineλ_right;	//Número de la primera línea de la matriz que entra en el tile
			nlineλ_right=*pnlineλ++;	//Número de la última línea de la matriz que entran en el tile
			λleft_offset=*pλ0++;	//Provisionalmente es el valor de λ de la primera líneaλ de matriz de las que entran en el tile.
			if(nlineλ_left==nlineλ_right) continue;	//Nos saltamos el tile
			λleft_offset-=tile_leftλ; //Ahora ya es el offset de λ de la primera líneaλ respecto al tile.
			λleft_offset*=3600; //En segundos

			{ssint k=tile_leftλ;
			if(k>=180) k-=360;  else if(k<-180) k+=360;
			if(k>=0) tilename->name[3]='E';
			else tilename->name[3]='W', k=-k;
			tilename->name[4]=(char8_t)(k/100);
			k-=100*tilename->name[4];	tilename->name[4]+='0';
			tilename->name[5]=(char8_t)(k/10);
			tilename->name[6]=(char8_t)(k-tilename->name[5]*10);
			tilename->name[5]+='0'; tilename->name[6]+='0';}

			(*pfile)->kind=MatrizSuelo_filekind_Global; strcpy8((*pfile)->file,tilename->name);
			int ret=abrir_tile_gz(tilename->path,Tile.u,TI.npoints);
			if(ret==TILE_NONEXISTENT){
				(*pfile)->found=false;
				uint16m z=0;
				ifunlike(tile_bottomφ==47 && tile_leftλ==-87) z=179; //Lago Superior
				else ifunlike(tile_bottomφ>=37 && tile_bottomφ<47 && tile_leftλ>=48 && tile_leftλ<54) z=(uint16m)-29; //Mar Caspio
				//En el lago Victoia no falta ningún tile
				iflike(z==0){zeroset(Tile.u,TI.usize);}
				else{
					z=bigend_u2___u2(z)&0xFFFF;
					uint zz=(z<<16) | z;
					memset_uint(Tile.u,zz,TI.usize/usizeof(uint));
				}
			}else{
				nret=0; //Hata que no se abre uno es MTIERRA_DSnooverlap
				(*pfile)->found=true;
				ifunlike(ret!=TILE_OPENED){
					pfile+=(nf>1), nf--;
					return ret;
				}
			}

			ssint *pmatriz=plineφ_beginning+Δlineaλ*nlineλ_left;
			interpola_tile(offset,ajusteZ0,escala,pmatriz,Δlineaφ,Δlineaλ, pix_λs,pix_φs, φtop_offset, λleft_offset, nlinesφ_interp, nlineλ_right-nlineλ_left,
								Tile.i, TI.φstep, TI.λstep, DS->ncols, DS->nrows,
								plineφ_corte+Δlineaλ_corte*nlineλ_left,Δlineaφ_corte,Δlineaλ_corte,n_corte);
		}
	}

	return nret;

salida_outofmem:
	return AT_NOMEM;
}
