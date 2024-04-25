//Return 0, AT_NOMEM.
int aplica_clipλφ(MatrizTierra *matriz, const Matriz___Tierra *transf, const bint besquina, Polígono_xy *clip, u8int npols){
	//Los intervalos en y de las líneas se toman [a,b), siendo a<=b. Los segmentos horizontales se ignoran
	umint *state; //De cada punto de la matriz
	ifunlike(npols==0) return 0;

	state=calloc(matriz->npx*matriz->npy,sizeof(state[0]));
	ifunlike(state==NULL) return AT_NOMEM;
	{durchlaufep(Polígono_xy,clip,npols){
		ifunlike(matriz___tierra(p->ppio,(pdif)(p->next-p->ppio),transf,besquina)) return AT_NOMEM;
	}}

	aplica_clip(state,matriz->npx,matriz->npy,clip,npols);
	durchlaufe2(flagMDT,matriz->i.flags,matriz->npuntos,umint,state){
		ptr->fuera=!(*ptr_b&1);
	}

	free(state);
	return 0;
}

//Return 0, AT_NOMEM
int trata_recorte(MatrizTierra *matriz, ClipModo clip_modo){
	switch(clip_modo){
	case ClipModo_nada: break;
	case ClipModo_recortar: default:
		//La z de los puntos 'fuera' debería ignorarse siempre. Para las funciones que no lo hagan
		//asignamos a esos puntos la z mínima.
		{uEarthHeight zmin=MATRIZ_MIN_STORED(matriz->zbounds);
		{durchlaufe2(uEarthHeight,matriz->suelo,matriz->npuntos,flagMDT,matriz->i.flags){if(ptr_b->fuera) *ptr=zmin;}}
		if(matriz->cielo!=NULL){
			durchlaufe2(uEarthHeight,matriz->cielo,matriz->npuntos,flagMDT,matriz->i.flags){if(ptr_b->fuera) *ptr=zmin;}
		}}
		break;
	case ClipModo_lago:
		//Buscar la z mínima de los puntos del borde del área de fuera
		{uEarthHeight zmin=MATRIZ_MAX_STORED_TIERRA(matriz->zbounds);
		const uEarthHeight ZMIN=MATRIZ_MIN_STORED(matriz->zbounds);
		//Búsqueda en horizontal
		uEarthHeight *p=matriz->suelo, *p1;
		flagMDT *pf=matriz->i.flags, *pf1;
		dontimes(matriz->npy,(p++,pf++)){
			dontimes(matriz->npx-1,(p++,pf++)){
				if(pf[1].fuera!=pf->fuera){
					uEarthHeight z;
					ifz(pf->fuera) z=*p; else z=p[1];
					mineq(zmin,z);
			}	}
			if(zmin==ZMIN) break;
		}
		//En vertical
		p=matriz->suelo; p1=p+matriz->npx;
		pf=matriz->i.flags; pf1=pf+matriz->npx;
		dontimes(matriz->npy-1,){
			if(zmin==ZMIN) break;
			dontimes(matriz->npx,(p++,p1++,pf++,pf1++)){
				if(pf->fuera!=pf1->fuera){
					uEarthHeight z;
					ifz(pf->fuera) z=*p; else z=*p1;
					mineq(zmin,z);
			}	}
		}

		void *voidp;
		ifunlike((voidp=realloc(matriz->i.cotaslagos,(matriz->esta.nláminas+1+2)*usizeof(*matriz->i.cotaslagos)))==NULL) return AT_NOMEM;
		matriz->i.cotaslagos=(uEarthHeight*)voidp;
		ifunlike((voidp=realloc(matriz->i.gruposlago,(matriz->esta.nláminas+1+2)*usizeof(*matriz->i.gruposlago)))==NULL) return AT_NOMEM;
		matriz->i.gruposlago=(uLago_t*)voidp;
		ifunlike((voidp=realloc(matriz->i.lagos,(matriz->esta.nláminas+1+2)*usizeof(*matriz->i.lagos)))==NULL) return AT_NOMEM;
		matriz->i.lagos=(LagoZ*)voidp;
		matriz->esta.nlagos++;
		matriz->esta.nláminas++;
		matriz->esta.ncomponentes++;
		const uLago_t nlam=(uLago_t)matriz->esta.nláminas;
		//copiar el cierre a la nueva última posición
		matriz->i.cotaslagos[nlam+1]=matriz->i.cotaslagos[nlam];			matriz->i.cotaslagos[nlam]=zmin;
		matriz->i.gruposlago[nlam+1]=matriz->i.gruposlago[nlam];		matriz->i.gruposlago[nlam]=nlam;
		matriz->i.lagos[nlam+1]=matriz->i.lagos[nlam];						matriz->i.lagos[nlam]=LagoZVacío;
		durchlaufe2(uEarthHeight,matriz->suelo,matriz->npuntos,flagMDT,matriz->i.flags){
			if(ptr_b->fuera){*ptr=zmin; ptr_b->fuera=0; ptr_b->lago=nlam;}
		}
		if(matriz->cielo!=NULL){ //pf->fuera se ha hecho 0. Los identificamos por el númeo de lago
			durchlaufe2(uEarthHeight,matriz->cielo,matriz->npuntos,flagMDT,matriz->i.flags){
				if(ptr_b->lago==nlam) *ptr=zmin;
			}
		}
		matriz->esta.flags&=~FLAG_MATRIZ_HAYFUERA;
		}
		break;
	}

	return 0;
}
