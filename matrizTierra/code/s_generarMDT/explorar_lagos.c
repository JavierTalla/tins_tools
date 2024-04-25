//Calcula la cota, la superficie y la profundidad de cada lago. Elimina lagos que han quedado llenos de edificios
//si matriz->pixel<ELIMINA_LAGO_PIXMAX (100 m).
int explorar_lagos(MatrizTierra *matriz, const OpcionesMDTierra *abst){
	struct minmax{
		flagMDT *m, *M;
	} *lagomM; //Solamente hace falta para eliminar lagos.

	aj_malloc_return(lagomM,struct minmax,matriz->esta.nláminas+2);
	{durchlaufep(struct minmax,lagomM,matriz->esta.nláminas+2) p->m=NULL;}

	durchlaufe2(const uEarthHeight,matriz->suelo,matriz->npuntos, flagMDT,matriz->i.flags){
		uLago_t c;
		ifz(c=(uLago_t)ptr_b->lago) continue;
		uLago_t l;
		uEarthHeight z;
		LagoZ *pl;

		l=matriz->i.gruposlago[c];
		z=matriz->i.cotaslagos[c];
		pl=&matriz->i.lagos[l];
		pl->cota=z;
		z-=*ptr; maxeq(pl->depth,z);
		pl->superficie++;

		if(lagomM[l].m==NULL) lagomM[l].m=ptr_b;
		lagomM[l].M=ptr_b;
	}

	if(matriz->pixel<ELIMINA_LAGO_PIXMAX){ //Sumar la superficie edificada y eliminar lagos llenos de edificios
		const u8int K= factorK((uint8m)matriz->pixel);
		for(LagoZ *pl=matriz->i.lagos+1; !LAGOZ_end(*pl); pl++){
			if(LAGOZ_is_empty(*pl)) continue;
			pl->superficie+=pl->supe_edif;
			iflike(K*pl->supe_edif<pl->superficie) continue;

			uLago_t nlago=(uLago_t)(pl-matriz->i.lagos);
			flagMDT *tope=lagomM[nlago].M+1;
			for(flagMDT *pf=lagomM[nlago].m; pf!=tope; pf++){
				if(matriz->i.gruposlago[pf->lago]==nlago) pf->lago=0;
			}
			*pl=LagoZVacío;
		}
	}elif(matriz->pixel<abst->edificios.píxel){ //Sólo sumar la supe. edif.
		for(LagoZ *pl=matriz->i.lagos+1; !LAGOZ_end(*pl); pl++){
			if(LAGOZ_is_empty(*pl)) continue;
			pl->superficie+=pl->supe_edif;
		}
	}

	free(lagomM);
	return 0;
}
