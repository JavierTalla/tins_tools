typedef struct{
	uint16m *pb; //Apunta a un píxel con borde izquierdo
	ssint área; //Si área>0 es una isla; si <0, un hueco
} Región;
#define Región_Nula ((Región){NULL,0})

//Definición del tipo "vector de Lagos" El tipo es Vector_Lago
defineVector(Región)
#define Añade_región_nula(regiones,nomem_retval) Vadd(regiones,Región,Región_Nula, return nomem_retval);

//Asigna z=cota a todos los puntos del aujero que tengan z<cota y rodea con las islas (z>cota)
//con borde de MARCA, que añade a la lista de regiones, asignando un área positiva. No cierra la lista de regiones.
//Devuelve 0 o AT_NOMEM
int rellena_aujero_izdo(uint16m *pb, uEarthHeight *ptr, uEarthHeight cota, uint16m MARCA, Vector_Región *regiones){
	if(BORDE_has_TODOS(*pb)){*ptr=cota; return 0;}

	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	do{//Hay que ir recorriendo cada línea horizontal hasta salir del aujero. El último píxel dentro será un píxel con borde
		//izquierdo. Se distingue de uno creado alrededor de una isla dentro del aujero en que estos últimos tienen ptr[1]>cota
		if(*pb & BORDE_IZDO){ //y pb[1] sin borde izquierdo (sin ningún borde, de hecho).
			uEarthHeight *ptr2=ptr-1;
			uint16m *pb2=pb-1;
			do{ ptr2++, pb2++;
				maxeq(*ptr2,cota);
				if(ptr2[1]>cota && !(pb2[1]&BORDE_IZDO)){ //Una isla dentro del hueco (si pb2[1]&_IZDO se trata de una isla o pegada al hueco por fuera)
					if(!(*pb2 & BORDE_DCHO)){ //Aún no se ha creado.
						crea_borde_leq_dcho(pb2,ptr2,cota,MARCA);
						Vadd(*regiones,Región,((Región){pb2,1}),return AT_NOMEM); //Áera 1 para que se sepa que es una isla
					}
					pb2++, ptr2++; //Entrar en la isla
					do pb2++, ptr2++; while(!(*pb2 & BORDE_IZDO)); //Avanzar hasta salir. Al crearla hay que guardar un pb con borde izquierdo
					maxeq(*ptr2,cota); //Procesar el píxel al que hemos salido
				}
			}while(!( (*pb2 & BORDE_DCHO) && (ptr2[1]==cota||(pb2[1]&BORDE_IZDO)) )); //Si hay borde derecho pero ptr2[1]>cota
		}									//estamos a punto de entrar en una isla. Si no está bordeada interiormente se trata de una isla interior previamente creada.
		s=NextEstado(s);
		while(*pb & Borde[s]) s++;
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || PIXEL_IZDO_Continuar(pb,s));
	*pb&=~TABLE_SIGNALLED;
	return 0;
}

//Devuelve 0 o AT_NOMEM. Cierra las regiones de este lago con un NULL
//El borde el lago no puede tener ninguna marca. Las regiones que queden completamente
//dentro de otras regiones no se generan
int obtiene_regiones_interiores(uint16m *pb, const uEarthHeight *ptr, const uEarthHeight cota, Vector_Región *regos){
	if(BORDE_has_TODOS(*pb)){
		Añade_región_nula(*regos,AT_NOMEM); return 0;
	}

	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	do{
		if(*pb & BORDE_IZDO){
			const uEarthHeight *ptr2=ptr-1;
			uint16m *pb2=pb-1;
			do{ ptr2++, pb2++;
				if(*ptr2==cota) continue;
				if(!(*pb2&BORDE_IZDO)){ //Entrada en una isla o hueco aún sin marcar
					Región r; r.pb=pb2;
					if(*ptr2>cota) r.área=crea_borde_gr_izdo(pb2,ptr2,cota); //Una isla
					elif(*ptr2<cota) r.área=-crea_borde_less_izdo(pb2,ptr2,cota); //Un hueco
					Vadd(*regos,Región,r,return AT_NOMEM);
				}									//Se atraviesan ignorándolos los huecos dentro de las islas y las islas dentro de los huecos
				while(!(*pb2&BORDE_DCHO)) pb2++, ptr2++; //Atravesar hasta el último píxel dentro
			}while(!((*pb2 & BORDE_DCHO) && *ptr2==cota)); //Si hay borde derecho pero *ptr2!=cota se trada de una isla o hueco interior
		}
		s=NextEstado(s);
		while(*pb & Borde[s]) s++;
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || PIXEL_IZDO_Continuar(pb,s));
	*pb&=~TABLE_SIGNALLED;

	Añade_región_nula(*regos,AT_NOMEM);
	return 0;
}

#define MAX_DESAGÜES 32
typedef struct{
	uEarthHeight zcero; //z que tienen en la matriz los puntos a cota 0 (nivel del mar)
	uEarthHeight Δplano; //Desnivel necesario en las proximidades de un lago para que no se considere como plano
	uint MGrande; //>= , se ignora su línea de desgüe
	uint MPlano; //Mínimo exigido para los lagos en zonas planas
	uint Mwater, mwater; //>=Mwater: GRANDE; >=mwater: MEDIANO
	uint8m exig_desagües; //<=MAX_DESAGÜES. O significa ignorar los desagües.
} LagosParams;

/* Mirar si es lago por la longitud de sus desagües y su entorno no plano o plano.
	Se permiten hasta sqrtpm(área)/4-1 píxeles de desagüe.
	Para contar cada píxel de desagüe una sola vez podemos mirar los bordes adyacentes a él y
contarlo solamente en el primero de ellos, en un recorrido horario del borde del lago. Pero eso
necesita un montón de if's. Por ello los vamos marcando y luego damos una segunda vuelta para
quitar las marcas.
	Los desagües de las esquinas no los marcamos para ahorrar tiempo a la hora de desmarcar. En
algunos casos esto puede hacer que el píxel se cuente dos o más veces, pero no creo que en la
práctica importe.
	Como además los modelos tienen errores y aparecen falsos lagos en zonas llanas, hay que
mirar también si el lago está en un plano y es pequeño, en cuyo caso no lo tendremos en cuenta
como lago. En la práctica se eliminarán lagos que realmente lo son, pero no queda más remedio.
(Eso, o tener una base de datos con todos los lagos del mundo).
	Return:
		0: Si el lago está bien
		1: Si hay que degradarlo por mucho desagüe
		2: Si hay que degradarlo por ser solo mediano y estar en zona plana
*/
umint degradalago___entorno(uint16m *pb, const uEarthHeight *ptr, uint área, const LagosParams *params){
	umint nret;
	uint	K=0,		//Total de pixeles adyacentes por fuera al borde
			k10=0,	//Número de ellos de borde plano
			kneg=0;	//Número de ellos de desagüe
	const uEarthHeight cota=*ptr;
	uEarthHeight hup=cota+params->Δplano;
	ifunlike(hup<cota) hup=0xFFFE;
	const ssint NPX=PixelOffset[DOWN];

#define Procesa_Borde(dif,hup) \
	K++; if(ptr[dif]<cota) kneg++; else{\
	if(ptr[dif-1]<hup && ptr[dif]<hup && ptr[dif+1]<hup && ptr[dif-npxi]<hup && ptr[dif+npxi]<hup){\
		if(ptr[dif+(dif)]==cota) K--; else k10++;\
	}}\
	pb[dif]|=TABLE_SIGNALLED;

#define Procesa_Esquina(dif,hup) \
	K++;\
	uEarthHeight h=ptr[dif];	if(h<cota) kneg++; else if(h<hup && ptr[(dif)+(dif)]<hup) k10++;

	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	do{
		if((*pb&BORDE_IZDO)	 && !(pb[-1]&TABLE_SIGNALLED)	   ){Procesa_Borde(-1,hup)}
		if((*pb&BORDE_SUPOR)&& !(pb[-npxi]&TABLE_SIGNALLED)){Procesa_Borde(-npxi,hup)}
		if((*pb&BORDE_DCHO)	 && !(pb[1]&TABLE_SIGNALLED)	   ){Procesa_Borde(1,hup)}
		if((*pb&BORDE_INFOR)	 && !(pb[npxi]&TABLE_SIGNALLED)  ){Procesa_Borde(npxi,hup)}
		//Desagües de las esquinas.
		if((*pb&BORDE_IZDO)	 && (*pb & BORDE_SUPOR)){Procesa_Esquina(-npxi-1,hup)}
		if((*pb&BORDE_SUPOR)&& (*pb & BORDE_DCHO)){Procesa_Esquina(-npxi+1,hup)}
		if((*pb&BORDE_DCHO)	 && (*pb & BORDE_INFOR)){Procesa_Esquina(+npxi+1,hup)}
		if((*pb&BORDE_INFOR)	 && (*pb & BORDE_IZDO)){Procesa_Esquina(+npxi-1,hup)}

		s=NextEstado(s);
		while(*pb & Borde[s]) s++;
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || PIXEL_IZDO_Continuar(pb,s));
#undef Procesa_Borde
#undef Procesa_Esquina

	//Desmarcar
	if(*pb & BORDE_DCHO) pb[1]&=~TABLE_SIGNALLED;
	if(*pb & BORDE_INFOR) pb[npxi]&=~TABLE_SIGNALLED;
	else s=UP; //Para que desmarque el píxel izquierdo
	do{
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]!=cota) pb[PixelOffset[s++]]&=~TABLE_SIGNALLED;
		ptr+=PixelOffset[s]; //Siguiente píxel
		pb+=PixelOffset[s];
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || PIXEL_IZDO_Continuar(pb,s));
	*pb&=~TABLE_SIGNALLED;

	if((uint)área<params->MPlano && 6*(K-k10)<K) nret=2; //Lo miramos ahora porque área puede modificarse
	else nret=0;

	if(params->exig_desagües!=0 && área<params->MGrande){ //Las láminas muy grandes no se degradan nunca por desagüe
		uint8m nivel=params->exig_desagües-1;
		uint8m resto=nivel&3;
		nivel>>=2; //Cada cuatro pasos se repiten los valores dividiendo por dos
		if(nivel!=0){
			if(nivel>2){área>>=(nivel-2); nivel=2;}
			kneg<<=nivel;
		}
		switch(resto){
			case 0:
				if(área<150) kneg<<=1;
				else if(área>=300){
					if(área<400) kneg-=kneg>>2;
					else{ kneg>>=1;
						if(área>=600){
							if(área<800) kneg-=kneg>>2;
							else{ kneg>>=1;
								if(área>=2000) kneg>>=1;
					}	}	}
				}
				break;
			case 1:
			case 2:
				if(área<250) kneg<<=1;
				else if(área>=400){
					if(resto==1){
						if(área>=600) kneg-=kneg>>2;
						kneg>>=1;
					}else{ //resto=2
						if(área>=600) kneg>>=1;
						else kneg-=kneg>>2;
					}
					if(área>=2000) kneg>>=1;
				} //El desagüe muchas veces ni aparece en la matriz para lagos de verdad.
				break;
			case 3:
				if(área<350){
					if(área<250) kneg<<=1;
					if(área<100 || área>=250) kneg+=kneg>>1;
				}else if(área>=600){
					if(área<800) kneg-=kneg>>2;
					else{ kneg>>=1;
						if(área>=2000) kneg>>=1;
					}
				}
				break;
		}
		if(kneg>=sqrtpm(área)) return 1;
	}
	return nret;
}

#define NO_MARCA 0
/*El borde del lago tiene que estar creado y sin ninguna marca. Devuelve la marca de agua asignada, o MARCA_NOMEM
Recibe, en 'MARCA', la marca que ha de tener de acuerdo a su área en caso de ser lago. Puede decidirse que no es un
lago, en cuyo caso se cambiará MARCA a LAGOKIND_NO, o bien que es solo candidato, en cuyo caso, si MARCA era
LAGOKIND_GRANDE, se degradará a LAGOKIND_MEDIO.
    Para los análisis necesarios para esto se emplean los parámetros área y los de params, que no se emplean para
nada más. Si área es 0 o params es NULL estas comprobaciones se omiten y la MARCA se mantiene.*/
uint16m procesa_lago(uint16m *pb, uEarthHeight *ptr, uint área, uint16m MARCA, Vector_Región *regiones, const LagosParams *params){
	const uEarthHeight cota=*ptr;
	uint nr0=regiones->n; //Número de región dentro del vector en el que empezarán las de este lago

	if(área==0 && BORDE_has_TODOS(*pb)) área=1;

	if(área!=0 && área<9){ //No puede tener nada dentro. área=0 significa que no se sabe el área
		remarca_izdo(pb,MARCA);
		Añade_región_nula(*regiones,AT_NOMEM);
		return MARCA;
	}
	destruye_interior(pb,ptr);
	ifunlike(obtiene_regiones_interiores(pb,ptr,*ptr,regiones)==AT_NOMEM) return MARCA_NOMEM;

	if(área!=0 && params!=NULL){
		//Mirar las áreas de los huecos y decidir si, debido a ello, no se considera un lago
		if(cota!=params->zcero){ //Si es mar ignoramos los huecos
			uint áreaj=0, áreaJ=0;
			for(Región *pr=regiones->ppio+nr0; pr->pb!=NULL; pr++){
				if(pr->área>=0) continue;
				uint a=(uint)-pr->área;
				áreaJ+=a; maxeq(áreaj,a);
			}
			if((área<áreaj*400) || (área<áreaJ*120)) MARCA=LAGOKIND_NO;
		}
		//Mirar si hay que degradarlo en base a su entorno inmediato
		if(MARCA==LAGOKIND_GRANDE && degradalago___entorno(pb,ptr,área,params)) MARCA=LAGOKIND_MEDIO;
	}

	/*Vamos a rodear las islas desde dentro del lago. Puede haber una serie de islas y huecos en corona rodeando una
	laguna a la misma cota que el lago, que a su vez puede tener una isla. Esta última isla está entre las regiones, pero
	si el lago no es GRANDE, al rodear las islas y huecos desde el lago, las primeras se agrupan en una sola y la laguna
	queda fuera del lago, y la última isla no es una isla del lago presente y no se debe rodear. Si el lago es GRANDE
	los huecos se rellenarán y la laguna quedará unida al lago principal, pero puede darse el mismo problema si la
	corona está formada por varias islas tocándose en esquina (o una sola tocándose a sí misma). Así que en cualquier
	caso hay que destruir todas las islas y generarlas de nuevo o bien tener cuidado de eliminar bordes espúreos.
	    No podemos no haberlas generado la primera vez sin más, porque en su interior puede haber muchos puntos
	a cota menor que la del lago que no han de contarse a la hora de cancelar el lago.*/

	/*Solo los grandes son lagos. Los otros no serán nada, salvo que toquen en una esquina a un lago grande, lo que
	se detectará en el proceso de expansión de lagos al final. En los grandes, rellenar los aujeros (y destruirlos) y ya
	generar el borde que rodea las islas, pero aún sin su marca. Pero si se trata del mar, los aujeros seguramente sea
	tierra que no rellenamos.*/
	if(MARCA==LAGOKIND_GRANDE && (params==NULL || cota!=params->zcero)){
		Región *pfin=regiones->ppio+regiones->n;
		regiones->n--; //Para que se sobreescriba el NULL del final al añadir más regiones y no quede un hueco
		for(Región *pr=regiones->ppio+nr0;pr<pfin;pr++){
			if(pr->área>=0) continue; //No es un hueco
			Región *viejo=regiones->ppio;
			ifunlike(rellena_aujero_izdo(pr->pb,ptr+(pr->pb-pb), cota,NO_MARCA,regiones)==AT_NOMEM) return MARCA_NOMEM;
			ifunlike(regiones->ppio!=viejo){pr=regiones->ppio+(pr-viejo); pfin=regiones->ppio+(pfin-viejo);}//Durante la función se ha movido el vector de sitio
			destruye_borde_izdo(pr->pb); pr->área=0; //Para que no se procesen más abajo
		}
		Añade_región_nula(*regiones,MARCA_NOMEM); //Volver a cerrar
		//Estas regiones se distinguirán de las ya existentes porque su borde está a cota
	}

	//Volver a generar las regiones, ahora limitándolas desde dentro del lago
	//Todas las regiones del vector regiones quedan, bien destruídas y con área=0, bien como bordes a cota y área!=0.
	for(Región *pr=regiones->ppio+nr0; pr->pb!=NULL; pr++){ //Quitar el borde interior de las islas y generarlo por fuera
		if(pr->área==0) continue; //Un hueco de un GRANDE ya destruido
		const uEarthHeight *ptr2=ptr+(pr->pb-pb);
		if(*ptr2==cota) break; //Hemos llegado a las islas añadidas en el proceso de eliminación de huecos, en un lago GRANDE
		destruye_borde_izdo(pr->pb); pr->área=0; //Para que no se procese luego
		if(ptr2[-1]!=cota) continue; //Está pegado a otro hueco/isla
		if(pr->pb[-1]&BORDE_DCHO) continue; //Borde ya creado al procesar una isla/hueco anterior, que abarca el presente
		crea_borde_eq_dcho(pr->pb-1,ptr+(pr->pb-pb)-1, NO_MARCA); //Si su área es >0 habría que destruirlo.
		pr->área=1; pr->pb--;												  //Ya se destruirá abajo. Sucederá pocas veces
	}

	remarca_izdo(pb,MARCA);
	//Eliminar los bordes que han quedado fuera del lago; e.d., dentro de una isla o hueco.
	for(Región *pr=regiones->ppio+nr0; pr->pb!=NULL; pr++){
		if(pr->área==0) continue;
		if(is_bad_dcho(pr->pb,ptr+(pr->pb-pb))){destruye_borde_dcho(pr->pb); pr->área=0;}
	}
	//Compactar las regiones y marcar las que quedan
	{Región *pr2=regiones->ppio+nr0;
	for(Región *pr=pr2; pr->pb!=NULL; pr++){
		if(pr->área==0) continue;
		remarca_dcho(pr->pb,MARCA);
		*pr2++=*pr;
	}
	*pr2++=Región_Nula;
	regiones->n=(pdif)(pr2-regiones->ppio);
	}

	//Restituir los lagos
	/* Un lago preexistente tiene todos los píxeles marcados con borde a una misma cota: la cota del lago,
	digamos cota2. Si esa cota es la misma que la del lago que nos ocupa entonces es necesariamente un
	lago dentro de una isla del presente lago. En ese caso se conservará siempre. Si cota2!=cota, entonces
	cuando sea el momento de restituir estos lagos miramos si el píxel sigue teniendo cota2 o por el contrario
	se ha cambiado a cota. En este último caso olvidamos el lago, pues era parte de un hueco que se ha relle-
	nado. Si sigue teniendo cota2 lo conservamos; puede ser porque esté dentro de una isla o porque se haya
	decidido que el presente candidato a lago en realidad no es un lago.*/
	//Falta programar esto. Es muy difícil, aunque teóricamente posible, que un lago dentro de una isla
	//dentro de un lago se detecte antes que el lago principal.

	return MARCA;
}
#undef NO_MARCA

//El borde del lago tiene que estar creado. Se asignará la marca MARCA y se da por sentado
//que es un lago. Por tanto los huecos se rellenarán, también con MARCA
//Devuelve MARCA o MARCA_NOMEM
sinline uint16m reprocesa_lago(uint16m *pb, uEarthHeight *ptr, uint16m MARCA, Vector_Región *regiones){
	destruye_bordes_interiores(pb,ptr); //Solamente destruye los bordes de este propio lago
	remarca_izdo(pb,0);
	return procesa_lago(pb,ptr,0,MARCA,regiones,NULL);
}

/*Propaga el lago desde un píxel de su borde a una lámina que lo toque en una esquina y esté a la misma
cota. Se miran las cuatro esquinas, pudiendo tratarse por tanto hasta cuatro láminas. Se exige que sea
una lámina; e. d., no se ampliará a puntos a la misma cota que no sean el borde de una lámina ya
detectada. Se llevarán a cabo distintas acciones según la naturaleza de esa lámina:
1. La lámina ya tiene número de lago asignado: Se trata de una lámina que ya se ha procesado.
	No se hace nada más que incluir esa lámina y el presente lago en el mismo grupo (vector lgrupos).
2. La lámina es GRANDE: Se trata de un lago que está en la lista de lagos y que se procesará cuando
	le llegue su turno. No se hace nada. Podría incluso tratarse de otro borde de este mismo lago
3. El PEQUEÑA: Se deja marcada con el número de lago.
4. Es MEDIANA: Se remarca su borde como GRANDE y se añade al final de la lista de lagos para que se
	procese cuando se llegue a él al ir recorriendo la lista. Sus bordes interiores se añadirán al vector
	"regiones", como para cualquier otro lago GRANDE.

Devuelve 0 o AT_NOMEM
bwater solo hace falta para asignar el campo 'ind' de un lago añadido (pb-bawater)
*/
int expande_desde_borde(uint16m * const pb, uEarthHeight * const ptr, uLago_t klago, uLago_t ngrupo, Vector_Lago *lagos, Vector_Región *regiones, uLago_t *lgrupos, uint16m *bwater){
	uint16m *pb4[5];
	const uint NPX=PixelOffset[2];
	const uEarthHeight cota=*ptr;

	//if(TSTATE_has_LAGO(*pc)) lo añadimos solamente para formar gruposlago
	pb4[3]=pb4[2]=pb4[1]=pb4[0]=NULL; pb4[4]=pb;
	if((*pb & BORDE_IZDO) && (*pb & BORDE_SUPOR) && ptr[-npxi-1]==cota){
		uint16m *pc=pb+(-npxi-1);
		if(TSTATE_has_LAGO(*pc) || TSTATE_is_MED_OR_PEQ(*pc)) pb4[0]=pc;
	}
	if((*pb & BORDE_SUPOR) && (*pb & BORDE_DCHO) && ptr[-npxi+1]==cota){
		uint16m *pc=pb+(-npxi+1);
		if(TSTATE_has_LAGO(*pc) || TSTATE_is_MED_OR_PEQ(*pc)) pb4[1]=pc;
	}
	if((*pb & BORDE_DCHO) && (*pb & BORDE_INFOR) && ptr[npxi+1]==cota){
		uint16m *pc=pb+(npxi+1);
		if(TSTATE_has_LAGO(*pc) || TSTATE_is_MED_OR_PEQ(*pc)) pb4[2]=pc;
	}
	if((*pb & BORDE_INFOR) && (*pb & BORDE_IZDO) && ptr[npxi-1]==cota){
		uint16m *pc=pb+(npxi-1);
		if(TSTATE_has_LAGO(*pc) || TSTATE_is_MED_OR_PEQ(*pc)) pb4[3]=pc;
	}

	for(uint16m **pbb=&pb4[0]; *pbb!=pb;){
		uint16m *pc=*pbb++;
		if(pc==NULL) continue;

		if(TSTATE_has_LAGO(*pc)){ //Un lago que ya se ha procesado
			uLago_t i=LAGO_get_n(*pc);
			while(lgrupos[i]!=i) i=lgrupos[i];
			lgrupos[i]=ngrupo;
			continue;
		}
		if(TSTATE_marca_AGUA(*pc)==LAGOKIND_GRANDE) continue; //Es el mismo que el de otra esquina ya procesada

		//Ir a un punto del borde exterior del lago. Hay que aplicar get_borde_leftmost a todos los punteros pc,
		//también a los que están en una esquina sup.-izda. e inf.-izda. de su lámina, ya que el lago principal
		//(el que estamos procesando) puede estar incluído en una isla de su satélite, al menos en teoría.
		pc=get_borde_leftmost(pc);
		bint bmedio=(TSTATE_marca_AGUA(*pc)==LAGOKIND_MEDIO);
		uint nreg=regiones->n; //recordarlo
		ifunlike(reprocesa_lago(pc,ptr+(pc-pb),LAGOKIND_GRANDE,regiones)==MARCA_NOMEM) return AT_NOMEM;
		if(bmedio){
			Lago *pl;
			Venlarge_one(*lagos,Lago,pl,return AT_NOMEM);
			pl->klago=klago;
			pl->ind=(pdif)(pc-bwater);
			pl->internos=nreg;
		}else{ //Dejar el número de lago y eliminar la marca GRANDE
			crea_marcalago_izdo(pc,ptr+(pc-pb),klago); //Remarcar el lago y sus islas
			for(Región *pr=regiones->ppio+nreg;pr->pb!=NULL;pr++) crea_marcalago_dcho(pr->pb,ptr+(pr->pb-pb),klago);
			regiones->n=nreg; //Olvidar las regiones
		}
	}

	return 0;
}

//Obtener los satélites y, si son medianos, añadirlos al vector de lagos, al final, transformados en grandes.
//Devuelve 0 o AT_NOMEM
//bwater solo hace falta para asignar el campo 'ind' de un lago añadido (pb-bawater)
int expande_borde_izdo(uint16m *pb, uEarthHeight *ptr, uLago_t klago, uLago_t ngrupo, Vector_Lago *lagos, Vector_Región *regiones, uLago_t *lgrupos, uint16m *bwater){
	*pb|=TABLE_SIGNALLED; //Marca que indica el píxel de comienzo
	uint8m borde;		//Para llevar el control de los bordes que se recorren y poder detectar correctamente el final del bucle do{}while();
	uint8m s=RIGHT;	//Nos situamos en la esquina superior izquierda
	ptr--, pb--;
	do{
		ptr+=PixelOffset[s]; //Siguiente píxel
		pb+=PixelOffset[s];
		s=NextEstado(s);
		ifunlike(expande_desde_borde(pb,ptr,klago,ngrupo,lagos,regiones,lgrupos,bwater)) return AT_NOMEM;
		borde=0;
		while(ptr[PixelOffset[s]]!=*ptr) borde|=Borde[s++];	//Se termina cuando se escribiría el borde IZDO del píxel de comienzo
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || !(borde&BORDE_IZDO));
	*pb&=~TABLE_SIGNALLED;
	return 0;
}

int expande_borde_dcho(uint16m *pb, uEarthHeight *ptr, uLago_t klago, uLago_t ngrupo, Vector_Lago *lagos, Vector_Región *regiones, uLago_t *lgrupos, uint16m *bwater){
	*pb|=TABLE_SIGNALLED; //Marca que indica el píxel de comienzo
	uint8m borde;		//Para llevar el control de los bordes que se recorren y poder detectar correctamente el final del bucle do{}while();
	uint8m s=LEFT;	//Nos situamos en la esquina inferior derecha
	ptr++, pb++;
	do{
		ptr+=PixelOffset[s]; //Siguiente píxel
		pb+=PixelOffset[s];
		s=NextEstado(s);
		ifunlike(expande_desde_borde(pb,ptr,klago,ngrupo,lagos,regiones,lgrupos,bwater)) return AT_NOMEM;
		borde=0;
		while(ptr[PixelOffset[s]]!=*ptr) borde|=Borde[s++];	//Se termina cuando se escribiría el borde DCHO del píxel de comienzo
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || !(borde&BORDE_DCHO));
	*pb&=~TABLE_SIGNALLED;
	return 0;
}
