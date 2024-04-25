/* Las funciones de este fichero efectúan el (re)muestreo en _un_ tile, recibiendo ya punteros a memoria y todos los valores
de desplazamientos necesario al efecto.

interpola_tile		El tile está en el mismo sistema que la matriz (e.g., geográficas)
interpola_sistema	El tile está en el sistema 'sis' y los puntos se pasan de geográficas a matriz según indica 'transf'
*/

//Los puntos en el mar tienen que quedar a 0. Para ello suavizamos el ajuste de Z0
//en torno a la cota 0, en el intervalo [-2*ajusteZ0,2*ajusteZ0].
#define apply_ajusteZ0_valor(valor,ajusteZ0) \
	if(offset!=0 && valor!=0){\
		float off=fabsf(ajusteZ0);\
		if(valor<-2*off || valor>2*off) valor+=ajusteZ0;\
		else ifnz(cmpsign(valor,ajusteZ0)) valor*=0.5F;\
		else valor*=1.5F;\
	}

//vhabía tiene que ser un valor válido
ssint promediar_con_lo_que_había(ssint v, ssint vhabía, uint8m pcorte, uint8m ncorte){
	v=v*(ncorte-pcorte)+vhabía*pcorte;
	v+=ncorte>>1;
	ssint b=(v<0);
	v/=ncorte;
	return v-b;
}

//Todos los valores de φ y λ (offet, paso del tile, etc) han de estar en las mismas unidades (p. e. segundos)
void interpola_tile(
	ssint offset,		//A los valores de z del tile se les sumará este offset, en las mismas unidades que los datos del tile.
	float ajusteZ0,	//Después se les sumará este otro valor, pero modificándolo cerca del 0 para mantener fija esa cota 0.
	float escala,			//Los valores de z del tile se multiplicarán por escala al pasarlos a la matriz
	ssint* pmatriz,	//pmatriz ha de apuntar a la primera posición a interpolar; es decir, a (φtop_offset, λleft_offset) respecto al
					//primer píxel en memoria del tile. Además ha de ser el de dirección de memoria más baja de los que se van a calcular.
		ssint Δlineaφ,		//Lo que se avanza en matriz al avanzar una línea en φ hacia valores de φ menores.
		ssint Δlineaλ,		//Lo que se avanza en matriz al avanzar una línea en λ hacia valores de λ mayores.
		float pix_λ,			//Tamaño de píxel λ de la matriz, >0.
		float pix_φ,			//Tamaño de píxel φ de la matriz, >0.
		float φtop_offset,	//φ offset de la primera línea de φ de la matriz a calcular respecto a la línea más al N del tile. Ha de ser <=0
		float λleft_offset,	//λ offset de la primera línea de λ de la matriz a calcular respecto a la línea más al W del tile. Ha de ser >=0
		uint nlíneasφ,		//Número de líneas de φ de la matriz cuyos valores se obtendrán interpolando en este tile
		uint nlíneasλ,		//Número de líneas de λ de la matriz cuyos valores se obtendrán interpolando en este tile
	int16_t* Tile,
		float tile_φstep,	//Paso de φ en el tile. Positivo hacia el N, negativo hacia el S.
		float tile_λstep,		//Paso de λ en el tile
		uint tile_ncols,		//Número de columnas del tile
		uint tile_nrows,	//Número de filas del tile
	const umint *pcorte,	//Si es !=NULL indica una franja en la que hay que promediar el valor existente en matriz con el obtenido de este tile
		ssint Δlineaφ_corte,		//Lo que se avanza en corte al avanzar una línea en φ hacia valores de φ menores.
		ssint Δlineaλ_corte,		//Lo que se avanza en matriz al avanzar una línea en λ hacia valores de λ mayores.
		umint ncorte				//Se pondera lo que ya exista en la matriz con peso *corte/ncorte. Si *corte es >=ncorte, su peso será 1
										//(el valor nuevo se ignora). Si es 0, el valor existente se ignora
){
	const float foffset=(float)offset;
	//Cambio de unidades a "tile_φstep's" y "tile_λstep's".
	//pix_φ y pix_λ tienen que ser de manera que al ir recorriendo el tile se avance
	//siempre hacia valores de φ menores y, dentro de cada fila, hacia valores de λ mayores.
	φtop_offset/=tile_φstep; //Queda >=0 si tile_φstep<0
	pix_φ/=-tile_φstep;
	λleft_offset/=tile_λstep;
	pix_λ/=tile_λstep;

	if(tile_φstep>0) φtop_offset+=(float)(tile_nrows-1); //Era negativo. Ahora es postitivo y es un bottom_offset.
	if(tile_λstep<0) λleft_offset+=(float)(tile_ncols-1); //Era negativo. Ahora es postitivo y es un right_offset.

	if(pcorte==NULL) Δlineaφ_corte=0, Δlineaλ_corte=0; //Para que se mantenga en NULL

	dontimes(nlíneasφ,(pmatriz+=Δlineaφ,φtop_offset+=pix_φ, pcorte+=Δlineaφ_corte)){
		int16_t *ptilerow0, *ptilerow1;
		float f0, f1;	//Pesos de las filas 0 y 1.

		ifunlike(φtop_offset<0) φtop_offset=0;	//Si los valores que se han pasado a la función son correctos podría suceder esto
		uint r0=(uint)φtop_offset;			//solamente si pix_φ es negativo, en las unidades "tile_φstep's", en la última iteración
		f1=φtop_offset-(float)r0;			//debido a errores de redondeo de la coma flotante. O bien en un sistema de ties !closed.
		f0=1.0F-f1;

		ptilerow0=Tile+r0*tile_ncols;
		if(r0+1<tile_nrows) ptilerow1=ptilerow0+tile_ncols;
		else ptilerow1=ptilerow0;

		float λoffset=λleft_offset;
		ssint *pC=pmatriz;
		const umint *pcor=pcorte;
		dontimes(nlíneasλ,(pC+=Δlineaλ,λoffset+=pix_λ, pcor+=Δlineaλ_corte)){
			uint col0;
			float valor;

			if(*pC!=INT32_MIN && (pcor==NULL || *pcor>=ncorte)) continue;
			ifunlike(λoffset<0) λoffset=0;	//Si los valores que se han pasado a la función son correctos podría suceder esto
			col0=(uint)λoffset;			//solamente si pix_λ es negativo, en las unidades "tile_λstep's", en la última iteración
												//debido a errores de redondeo de la coma flotante.
			if(col0>=tile_ncols) col0=tile_ncols-1; //Con valores correctos, esto solamente puede suceder si el sistema de tiles es
															 //!closed y el píxel derecho coincide justo con el borde derecho del tile.
			valor=(int16_t)u2___bigend_u2(ptilerow0[col0])*f0 + (int16_t)u2___bigend_u2(ptilerow1[col0])*f1;
			if(col0+1!=tile_ncols){
				float g1;	//Peso de la columna 1.
				float v;

				g1=λoffset-(float)col0;
				col0++;
				v=(int16_t)u2___bigend_u2(ptilerow0[col0])*f0 + (int16_t)u2___bigend_u2(ptilerow1[col0])*f1;
				valor=valor*(1.0F-g1)+v*g1;
			}
			valor+=foffset;
			apply_ajusteZ0_valor(valor,ajusteZ0)
			valor*=escala;
			valor+=0.5F; //Si es negativo, se corrige en la instrucción siguiente
			ssint v2=(ssint)valor-(valor<0);
			//Promediar con lo que ya había si hace falta
			if(*pC==INT32_MIN) *pC=v2;
			else *pC=promediar_con_lo_que_había(v2,*pC,*pcor,ncorte); //Hence pcor!=NULL
	}	}
}

//Recorre todos lo píxeles de la matriz que pueden entrar en el tile, obteniendo en su caso
//el valor de z. Se supone que las filas están almacenadas de mayor a menor coordenada Y.
//precalc ha de apuntar a un array de npx*npy Puntoxy_double con las coordenadas de cada
//píxel de la matriz en el sistema del tile. minmax_fila ha de apuntar a un array de 2*npyx
void interpola_tile_precalc(
	const Matriz___Tierra *transf,
	const Sistema *sis,
	const uint16_t NoDataVal,
	ssint offset,		//A los valores de z del tile se les sumará este offset, en las mismas unidades que los datos del tile.
	float ajusteZ0,	//Después se les sumará este otro valor, pero modificándolo cerca del 0 para mantener fija esa cota 0.
	float escala,		//Los valores de z del tile se multiplicarán por escala al pasarlos a la matriz
	ssint* _matriz,	//La matriz
	uint npx, uint npy,	//de matriz
	uint16_t* Tile,
		double xm, double Xm,	//Valores mín y máx de este tile en el sistema sis, en metros
		double ym, double Ym,
		float tile_ystep,		//Paso de y en el tile. Positivo hacia valores de Y mayores; negativo hacia menores.
		float tile_xstep,		//Paso de x en el tile.
		uint Tnr,		//Número de filas del tile
		uint Tnc,		//Número de columnas del tile
	const Puntoxy_double *precalc, //Coordenadas (x,y) en el sistema del tile de cada punto de la matriz
	const Puntoxy_double *minmax_fila //mín. y máx. de cada fila de precalc
){
	ssint *pmatriz;
	const Puntoxy_double *pcalc;
	const double _ystep=1.0/fabsf(tile_ystep);
	const double _xstep=1.0/tile_xstep;
	uint mf, mc; //mc: Columna mínima de cada fila de la matriz que entra en el tile
	const float foffset=(float)offset;

	{Puntoxy_double p;
	if(!transf->b90){//Mirar la primera fila de la matriz cuya y_min entra en el tile
		p.y=Ym;		//Buscar la φ máxima del tile
		if(Ym>=0){if(-xm>Xm) p.x=xm; else p.x=Xm;} //El más alejado del meridiano central
		else{
			if(signbit(xm)!=signbit(Xm)) p.x=0; //El punto más próximo al meridiano central.
			else if(xm>=0) p.x=xm; else p.x=Xm;
		}
		p=geo___proy1(sis,p); p.φ*=PI_180_PI;
		if(p.φ>=transf->cc.φmax) mf=0;
		else mf=(uint)ceil((transf->cc.φmax-p.φ)/transf->cc.pix_φ);
	}else{ //Mirar la primera fila de la matriz cuya x_max entra en el tile
		p.x=xm; //Buscar la λ mínima del tile
		if(xm>=0){if(-ym>Ym) p.y=ym; else p.y=Ym;} //El más alejado del Ecuador
		else{
			if(signbit(ym)!=signbit(Ym)) p.y=0; //El punto más próximo al Ecuador.
			else if(ym>=0) p.y=ym; else p.y=Ym;
		}
		p=geo___proy1(sis,p);
		p.λ*=PI_180_PI; p.λ+=sis->infor.λ0;
		if(p.λ<transf->cc.λmin) mf=0;
		else mf=(uint)ceil((p.λ-transf->cc.λmin)/transf->cc.pix_λ);
	}}
	if(mf>=npy) return;
	mc=mf*npx; //auxi.
	minmax_fila+=mf<<1;
	pmatriz=_matriz+mc;
	pcalc=precalc+mc;

	if(!transf->b90) minmax_fila++; //Solo necesitaremos los valores de ymax. Si transf->b90, los de ymin
	mc=0;
	dontimes(npy-mf,minmax_fila+=2){
		//ajustar mc
		pcalc+=mc; //mc variará poco entre una fila y otra
		if(!transf->b90){ //Las filas de la matriz son filas de λ. El bucle externo avanza en φ.
			if(minmax_fila->y<ym) break; //No hemos salido del tile por abajo
			while(mc<npx && pcalc->x<xm) pcalc++, mc++;
			do pcalc--, mc--; while(ispos(mc) && pcalc->x>=xm);
			pcalc++, mc++;
		}else{ //Las filas de la matriz son filas de φ. El bucle externo avanza en λ.
			if(minmax_fila->x>Xm) break; //Nos hemos salido del tile por la derecha
			while(mc<npx && pcalc->y<ym) pcalc++, mc++;
			do pcalc--, mc--; while(ispos(mc) && pcalc->y>=ym);
			pcalc++, mc++;
		}

		pmatriz+=mc;
		fordown(j,npx-mc,pmatriz++){ //Dentro del bucle j ya va disminuido
			Puntoxy_double q;
			float φtop_offset, λoffset;
			uint r0,col0;
			uint16_t *pr0, *pr1;
			float f0, f1;	//Pesos de las filas 0 y 1.
			float valor;
			uint16_t v0, v1;

			q=*pcalc++;
			if(!transf->b90 ? q.x>Xm : q.y>Ym){
				pcalc+=j; pmatriz+=j+1; //j vale uno menos de la iteraciones que quedan contando la presente.
				break;
			}
			ifunlike(q.x<xm || q.x>Xm || q.y<ym || q.y>Ym) continue;

			φtop_offset=(float)((Ym-q.y)*_ystep);
			ifunlike(φtop_offset<0) φtop_offset=0;			//Guard frente a posibles errores de coma flotante y a tiles
			ifunlike(φtop_offset>=Tnr-1) φtop_offset=(float)(r0=Tnr-1); //!closed. Quedará f0=0, f1=1.
			else r0=(uint)φtop_offset;
			f1=φtop_offset-(float)r0;
			f0=1.0F-f1;
			pr0=Tile+Tnc*r0;
			pr1=pr0;
			iflike(r0+1!=Tnr) pr1+=Tnc;

			λoffset=(float)((q.x-xm)*_xstep);
			ifunlike(λoffset<0) λoffset=0;
			col0=(uint)λoffset;
			if(col0>=Tnc) col0=Tnc-1;

			v0=u2___bigend_u2(pr0[col0]);
			v1=u2___bigend_u2(pr1[col0]);
			if(v0==NoDataVal) valor=v1;
			else if(v1==NoDataVal) valor=v0;
			else valor=v0*f0+v1*f1;

			if(col0+1!=Tnc){
				float g1;
				float v;

				g1=λoffset-(float)col0;
				col0++;
				v0=u2___bigend_u2(pr0[col0]);
				v1=u2___bigend_u2(pr1[col0]);
				if(v0==NoDataVal) v=v1;
				else if(v1==NoDataVal) v=v0;
				else v=v0*f0+v1*f1;

				if(valor==NoDataVal) valor=v;
				else if(v==NoDataVal);
				else valor=valor*(1.0F-g1)+v*g1;
			}
			if(valor!=NoDataVal){
				valor+=foffset;
				apply_ajusteZ0_valor(valor,ajusteZ0)
				if(valor<0) valor=0; //En España no hay
				valor*=escala;
				valor+=0.5F;
				*pmatriz=(ssint)valor-(valor<0);
			}
		}//Fin matriz x
	}
}

//Recorre todos lo píxeles de la matriz que pueden entrar en el tile, obteniendo en su caso
//el valor de z. Se supone que las filas están almacenadas de mayor a menor coordenada Y.
//precalc puede ser NULL. Si no es null se supone que apunta a un array de npx*npy Puntoxy_double
//con las coordenadas de cada píxel de la matriz en el sistema del tile, y minmax_fila a un array
//de 2*npx con los mín. y máx. de cada fila de precalc
void interpola_tile_sistema(
	const Matriz___Tierra *transf,
	const Sistema *sis,	//Del tile
	const uint16_t NoDataVal,
	ssint offset,		//A los valores de z del tile se les sumará este offset, en las mismas unidades que los datos del tile.
	float ajusteZ0,	//Después se les sumará este otro valor, pero modificándolo cerca del 0 para mantener fija esa cota 0.
	float escala,		//Los valores de z del tile se multiplicarán por escala al pasarlos a la matriz
	ssint* _matriz,	//La matriz
	uint npx, uint npy,	//de matriz
	uint16_t* Tile,
		double xm, double Xm,	//Valores mín y máx de este tile en el sistema sis
		double ym, double Ym,
		float tile_ystep,		//Paso de y en el tile. Positivo hacia valores de Y mayores; negativo hacia menores.
		float tile_xstep,		//Paso de x en el tile.
		uint Tnr,		//Número de filas del tile
		uint Tnc,		//Número de columnas del tile
	Puntoxy_double *precalc, //Coordenadas (x,y) en el sistema del tile de cada punto de la matriz
	Puntoxy_double *minmax_fila
){
	if(precalc!=NULL){
		interpola_tile_precalc(transf,sis,NoDataVal,offset,ajusteZ0,escala,_matriz,npx,npy,Tile,xm,Xm,ym,Ym,tile_ystep,tile_xstep,Tnr,Tnc,precalc,minmax_fila);
		return;
	}

	float row_Δφ, row_Δλ, col_Δφ, col_Δλ; //En la matriz
	ssint *pmatriz;
	Puntoxy_double p;
	const double _ystep=1.0/fabsf(tile_ystep);
	const double _xstep=1.0/tile_xstep;
	const float foffset=(float)offset;

	if(!transf->b90){
		row_Δφ=-transf->cc.pix_φ;	row_Δλ=0;
		col_Δφ=0;						col_Δλ=transf->cc.pix_λ;
	}else{
		row_Δφ=0;						row_Δλ=transf->cc.pix_λ;
		col_Δφ=transf->cc.pix_φ;		col_Δλ=0;
	}
	row_Δφ*=(float)PI_180;	row_Δλ*=(float)PI_180;
	col_Δφ*=(float)PI_180;	col_Δλ*=(float)PI_180;

	if(!transf->b90) p.φ=transf->cc.φmax*PI_180;
	else p.λ=(transf->cc.λmin-sis->infor.λ0)*PI_180;

	pmatriz=_matriz;
	dontimes(npy,(p.φ+=row_Δφ, p.λ+=row_Δλ)){ //Uno de los dos, p.φ o p.λ, está de sobra aquí
		bint b=0;
		Puntoxy_double q, r;

		//Establecer p.λ o p.φ y mirar si esta fila de la matriz entra siquiera en parte en el tile.
		if(col_Δλ!=0){ //Las filas de la matriz son filas de λ. El bucle externo avanza en φ.
			//Probar el primer punto de la fila
			p.λ=(transf->cc.λmin-sis->infor.λ0)*PI_180;
			q=proy___geo1(sis,p);
			if(q.y>=ym && q.y<=Ym) b=1; //Si entra en los márgenes de latitud del tile

			else{ //Probar el punto del final de la fila
				r.φ=p.φ;
				r.λ=(transf->cc.λmax-sis->infor.λ0)*PI_180;
				q=proy___geo1(sis,r);
				if(q.y>=ym && q.y<=Ym) b=1;

				else if(p.λ<0 && r.λ>=0){ //Probar el punto del meridiano central
					r.λ=0;
					q=proy___geo1(sis,r);
					if(q.y>=ym && q.y<=Ym) b=1;
				}
			}
		}else{ //Las filas de la matriz son filas de φ. El bucle externo avanza en λ.
			//Probar el primer punto de la fila
			p.φ=transf->cc.φmin*PI_180;
			q=proy___geo1(sis,p);
			if(q.x>=xm && q.x<=Xm) b=1; //Si entra en los márgenes de longitud del tile

			else{ //Probar el punto del final de la fila
				r.λ=p.λ;
				r.φ=transf->cc.φmax*PI_180;
				q=proy___geo1(sis,r);
				if(q.x>=xm && q.x<=Xm) b=1;
				else if(p.φ<0 && r.φ>=0){ //Probar el punto del ecuador
					r.φ=0;
					q=proy___geo1(sis,r);
					if(q.x>=xm && q.x<=Xm) b=1;
				}
			}
		}
		if(!b){pmatriz+=npx; continue;}

		dontimes(npx,(p.φ+=col_Δφ, p.λ+=col_Δλ,pmatriz++)){
			Puntoxy_double q;
			float φtop_offset, λoffset;
			uint r0,col0;
			uint16_t *pr0, *pr1;
			float f0, f1;	//Pesos de las filas 0 y 1.
			float valor;
			uint16_t v0, v1;

			q=proy___geo1(sis,p);
			if(q.x<xm || q.x>Xm || q.y<ym || q.y>Ym) continue;

			φtop_offset=(float)((Ym-q.y)*_ystep);
			ifunlike(φtop_offset<0) φtop_offset=0;			//Guard frente a posibles errores de coma flotante y a tiles
			ifunlike(φtop_offset>=Tnr-1) φtop_offset=(float)(r0=Tnr-1); //!closed. Quedará f0=0, f1=1.
			else r0=(uint)φtop_offset;
			f1=φtop_offset-(float)r0;
			f0=1.0F-f1;
			pr0=Tile+Tnc*r0;
			pr1=pr0;
			iflike(r0+1!=Tnr) pr1+=Tnc;

			λoffset=(float)((q.x-xm)*_xstep);
			ifunlike(λoffset<0) λoffset=0;
			col0=(uint)λoffset;
			if(col0>=Tnc) col0=Tnc-1;

			v0=u2___bigend_u2(pr0[col0]);
			v1=u2___bigend_u2(pr1[col0]);
			if(v0==NoDataVal) valor=v1;
			else if(v1==NoDataVal) valor=v0;
			else valor=v0*f0+v1*f1;

			if(col0+1!=Tnc){
				float g1;
				float v;

				g1=λoffset-(float)col0;
				col0++;
				v0=u2___bigend_u2(pr0[col0]);
				v1=u2___bigend_u2(pr1[col0]);
				if(v0==NoDataVal) v=v1;
				else if(v1==NoDataVal) v=v0;
				else v=v0*f0+v1*f1;

				if(valor==NoDataVal) valor=v;
				else if(v==NoDataVal);
				else valor=valor*(1.0F-g1)+v*g1;
			}
			if(valor!=NoDataVal){
				valor+=foffset;
				apply_ajusteZ0_valor(valor,ajusteZ0)
				if(valor<0) valor=0; //En España no hay
				valor*=escala;
				valor+=0.5F;
				*pmatriz=(ssint)valor-(valor<0);
			}
		}//Fin matriz x
	}
}
