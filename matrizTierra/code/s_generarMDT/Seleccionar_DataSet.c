
/*pixel es el tmaño efectivo del píxel sobre el terreno, en metros
Gsets y Lsets pueden ser NULL
ds se emplea para los valores de pixeljump y lastpixeljump
Devuelve una pareja de datasets (DS,DSL), en la que cada uno de esos elementos es:
	NULL: No hay ningún dataset que abarque la zona
	BAD_PTR: (Solamente en DS). No hay ningún dataset con resolución suficiente
	else: Un DS con METADATA_Right.
*/
static DataSetPair selecciona_dataset(const Matriz___Tierra *transf, const ZonalDataSet *Gsets, const LocalDataSet *Lsets, const DSopts *ds, float pixel){
	const ZonalDataSet *DS;
	const LocalDataSet *DSL;

	pixel/=ds->pixeljump; //El DataSet puede tener hasta pixel de grande el píxel

	if(Gsets!=NULL){
		for(DS=Gsets; DS->folder[0]!='\0'; DS++){
			if(DS->md_state!=METADATA_Right) continue;
			if(DS->px<pixel) break;
		}
	}else DS=NULL;

	if(DS==NULL || DS->folder[0]=='\0'){ //Nos hemos quedado con ganas de emplear un DS con píxel más pequeño
		ifunlike(DS==Gsets) DS=NULL; //No ∃ningún dataset
		if(DS!=NULL){ //Nos quedamos con el último, que es lo menos malo
			do(--DS); while(DS!=Gsets && DS->md_state!=METADATA_Right);
			if(DS->md_state!=METADATA_Right) DS=NULL;
		}

		//Determinar el DataSet Local, si lo hay. La condición rect_in_rect2(transf,DSL)
		//indica que es posible (pero no seguro) que el DSL cubra la zona del trabajo
		if(Lsets!=NULL){
			for(DSL=Lsets; DSL->folder[0]!='\0'; DSL++){
				if(DSL->md_state!=METADATA_Right) continue;
				if(DSL->px<pixel*LOCAL_PIXUNI_CM && rect_in_rect2(transf->cc,DSL)) break;
			}
			if(DSL->folder[0]=='\0'){ //También aquí nos habría gustado encontrar uno más preciso
				//Quedarnos con lo mejor que haya disponible
				while(DSL--!=Lsets){if(DSL->md_state==METADATA_Right && rect_in_rect2(transf->cc,DSL)) break;}
				if(DSL<Lsets) DSL=NULL;
			}
		}else DSL=NULL;

		pixel*=ds->pixeljump/ds->lastpixeljump; //Permitir más holgura respecto a la resolución deseada si no queda más remedio

		//En este punto, si los punteros son NULL es porque no hay ningún DataSet
		if(DS==NULL && DSL==NULL) goto salida;
		if(DS!=NULL && DS->px>pixel) DS=NULL;
		if(DSL!=NULL && DSL->px>pixel*LOCAL_PIXUNI_CM) DSL=NULL;
		if(DS==NULL && DSL==NULL) DS=BAD_PTR;
		else if(_likely(DS!=NULL) && DSL!=NULL && _unlikely(DSL->px>=DS->px*LOCAL_PIXUNI_CM)) DSL=NULL; //No emplear un DS Local si su resolución no es mejor que la del global
	}else{
		DSL=NULL;
	}

salida:
	return (DataSetPair){DS,DSL};
}

/*pixel es el tmaño efectivo del píxel sobre el terreno, en metros
Return:
	NULL: No hay ningún dataset que abarque la zona
	BAD_PTR: No hay ningún dataset con resolución suficiente
	else: Un DS con METADATA_Right.
*/
static const PointsDataSet* selecciona_dataset_puntos(const Matriz___Tierra *transf, const PointsDataSet *Psets, float pixel){
	const PointsDataSet *pd;

	for(pd=Psets;pd->folder[0]!='\0';pd++){
		if(pd->md_state!=METADATA_Right) continue;
		if(/* pd->px<pixel*LOCAL_PIXUNI_CM && */ rect_in_rect2(transf->cc,pd)) return pd;
	}
	for(pd=Psets;pd->folder[0]!='\0';pd++){
		if(pd->md_state!=METADATA_Right) continue;
		if(rect_in_rect2(transf->cc,pd)) return pd;
	}
	for(pd=Psets;pd->folder[0]!='\0';pd++){
		if(pd->md_state!=METADATA_Right) continue;
		if(/* pd->px<pixel*LOCAL_PIXUNI_CM && */ rect_cut_rect2(transf->cc,pd)) return pd;
	}
	for(pd=Psets;pd->folder[0]!='\0';pd++){
		if(pd->md_state!=METADATA_Right) continue;
		if(rect_cut_rect2(transf->cc,pd)) return pd;
	}

	return NULL;
}
