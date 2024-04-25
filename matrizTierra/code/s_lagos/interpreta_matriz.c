/* interpreta_matriz

		La matriz bwater consta de un byte por cada posición (píxel) de la matriz de cotas "matriz". Los bits de estos
	bytes son flags que indican si el píxel correspondiente en "matriz" forma parte de una lámina así como si está en
	el borde de la lámina, y el tipo de borde: izquierdo, derecho, etc.
		El algortimo va recorriendo el borde de la región con cota constante, marcando las posiciones en bwater
	con el correspondiente código de borde y llevando la cuenta del área de la región. Al llegar al punto inicial y
	cerrar el polígono se mira su área total, y se decide su status en función de su área.
		Para ver si es un lago y no el borde de una depresión mayor se miran todos los píxeles de su interior. Si
	muchos son < entonces no es lago y se descarta. Ahora bien, no se ha de analizar el interior de las islas dentro del
	lago, ya que podrían tener a su vez otro lago o depresión a cota menor que el lago principal. Recuérdese al efecto
	que el "lago" principal puede ser el mar, y hay en la Tierra islas con puntos bajo el nivel del mar. Por ello, mientras
	se va recorriendo el interior de un lago se delimitan sus islas, que quedan así "fuera" del lago y y son susceptibles
	de ser analizadas a su vez para buscar láminas de agua.
		Estas islas se contabilizan adrede en el número de píxeles de la región, pues lo que queremos es el tamaño de
	la lámina, tenga o no islas en el medio.
		Una lámina de agua tiene que estar formada por un núcleo de al menos Mwater píxeles. Esta es un bloque "Grande".
	A este se pueden añadir otros bloques de cualquier tamaño (por tanto también píxeles aislados) que toquen en una
	esquina el bloque principal, o un bloque que a su vez toca en esquina al bloque principal, etc. Pero para evitar una
	propagación a lo largo de un río, un conjunto de píxeles que toque en una esquina a otro solamente puede propagar
	la condición de 	agua a otros bloques si tiene al menos mwater píxeles (bloque "Mediano").
		Para poder llevar a cabo esta expansión de la lámina principal a traves de sus esquinas a otras laminas menores,
	en cuanto se crea una lámina con área>mwater, por tanto Mediana o Grande, se guarda en un registro de láminas
	para su posterior recorrido del borde una vez que se ha recorrido toda la matriz y se han delimitado todos los bloques.

		Hay un casos que está adrede ignorado, pues es posible que en toda la Tierra nunca se de y la complicación añadida
	no merece la pena: Un lago que desagüe por todos sus píxeles con BORDE_IZDO. Un lago así tendría que ser extrema-
	damente alargado en dirección E-O para superar el umbral de área en relación al número de píxeles de desagüe, a
	menos que desagüe solamente a través de uno o dos píxeles. En este caso más plausible el lago tendría que tener un
	borde izquierdo recto compuesto por no más de cuatro píxeles (en la práctica) y extenderse hacia la derecha hasta
	alcanzar un área mímima de 20, siempre sin salirse de la banda horizontal delimitada por los píxeles de su borde
	izquierdo.

		El algoritmo, en cuando detecta una posible lámina de agua recorre su borde delimitándola. Tras ello:
	1º: Aparta los lagos que hubiera dentro. Normalmente no va a haber ninguo. Si hay se detecta un lago, él y todo
		su contenido de eliminan de la matriz y quedan guardados provisionalmente en otra parte.
	2º: Se identifican todas sus islas y huecos. Una isla es una agruación de píxeles a cota mayor que el lago; un hueco,
		una agrupación a cota menor. Puede suceder, y suele suceder, que los huecos aparecen pegados a islas.
	3º: En función del tamaño de los huecos se decide si el candidato a lago es realmente un lago.
		a) Si es que sí:
			* Los huecos se rellenan con la cota del lago, dejando como islas lo que, dentro de los huecos,
			estuviera a cota mayor que la del lago.
			* Las islas se mantienen. Las generadas en el paso anterior y estas no se distinguen ya unas de
			otras. Todo lo que hubiera dentro de ellas se ignora.
			* Los lagos con su contenido que se habían apartado en el paso 1º vuelven si la cota de un píxel
			de su borde sigue siendo la misma que antes de rellenar los huecos. Esto elimina los lagos que
			estaban en los huecos que se han rellenado y mantiene los demás. (Un lago a la misma cota que
			el lago que se está procesando necesariamente estaba contenido dentro de una isla de este último).
		b) Si es que no:
			* Los huecos e islas se reprocesan, desechando los que se habían generado. Se generará ahora un
			borde desde el interior del lago, que abarca a huecos e islas indistintamente; es decir, se crean los
			bordes interiores de lago. Estos huecos/islas se llaman genéricamente islas.
			* Los lagos apartados en el paso 1º se restituyen, con todo su contenido.

		En ambos casos los bordes generados constituyen el borde exterior y los bordes interiores del (no-)lago,
		estando todos aplicados a píxeles a la cota del lago, por tanto a la cota del lago, delimitando una región
		en la que todos los píxeles están a la cota del lago, que queda así aislada del exterior, formado por el exterior
		en el sentido habitual de la palabra y sus islas. En el caso a) se tratará siempre de islas reales; en el segundo
		caso serán islas desde el punto de vista lógico, sean o no reales (puede ser un hueco). Cualquier píxel del
		exterior del lago que esté tocando a un borde del lago estará a cota != a la del lago.

	Las islas están fuera del lago a todos los efectos. En particular, los bordes interiores de los lagos han de
	marcarse igual que el borde exterior. Por tanto:

	4º: Al borde exterior del lago y a sus borde interiore se añade la marca de NO, GRANDE, MEDIO o PEQ.

		Al continuar procesando la matriz, al llegar a un borde de lago este se atraviesa hasta dar con el borde
	de salida, que puede ser de una isla. Según el método seguido para procesar el lago, el primer píxel fuera
	del lago será el primero a una cota distinta de la del lago (o el borde derecho de la matriz). Para no tener
	que hacer una excepción continua con esto, la matriz se rodea de un marco a cota MAX_MEDIDAS.

		Una vez procesado un lago, la información que se recuerda de él es un puntero a uno de sus píxeles
	con borde izquierdo del borde exterior (el que sirvió de inicio para recorrer su borde) y, en el caso de los
	GRANDEs, los bordes interiores.
*/

void setup_params(LagosParams *params, MatrizTierra *matriz, float DS_step, const Opciones_Lagos *lagos){
	/*Si matriz->zbounds.offset>0, zcero tiene que ser un valor que ningún píxel de la matriz tenga.
	La función que crea la matriz garantiza que el valor MAX_UEARTHHEIGHT (0xFFFF)
	nunca se asignará. Si no se quiere depender de esta suposición basta declararlo uint
	y asignar un valor fuera del rango del dato uEarthHeight. Por ejemplo 0x8000.*/
	if(matriz->zbounds.offset>0) params->zcero=MAX_UEARTHHEIGHT;
	else params->zcero=(uEarthHeight)(matriz->zbounds.escala*(-matriz->zbounds.offset));

	//MGrande
	params->MGrande=lagos->grandes;
	//Mpequeños
	float Mpeq=(float)lagos->pequeños/matriz->pixel;

	float d1=matriz->pixel/DS_step;
	//No es necesario mirar la relación con DS->λstep porque pasado a metros será menor,
	//de donde d2>d1, y queremos el menor de los dos.
	if(d1<=1.0f){
		params->Mwater=MinWaterTablePixels[MinWaterTablePixels_Index1].M;
		params->mwater=MinWaterTablePixels[MinWaterTablePixels_Index1].m;
		d1=1.0f/d1;
		Mpeq*=d1; //Sólo uno porque abajo ya se penaliza inv. prop. a píxel
		d1*=d1;
		params->MGrande=(uint)((float)params->MGrande*d1+0.5f);
		params->Mwater=(uint)((float)params->Mwater*d1+0.5f);
		params->mwater=(uint)((float)params->mwater*d1+0.5f);
	}else{
		for(u8int i=0;;i++){
			if(d1<MinWaterTablePixels[i].step) continue;
			params->Mwater=MinWaterTablePixels[i].M;
			params->mwater=MinWaterTablePixels[i].m;
			break;
		}
		d1=1.0;
	}
	params->Mwater=(params->Mwater*lagos->base)>>2; //>>2 porque el parámetro 'base' se toma con origen en 4.
	//Mirar si hay que hacer Mwater más grande porque el píxel sea muy pequeño.
	uint M=(uint)(Mpeq+0.5f);
	maxeq(params->Mwater,M);

	//Ajustar a la resolución real de la matriz.
	if(matriz->pixel!=matriz->px){ //Mwater, etc. se miden en píxeles de la matriz
		float npix=matriz->pixel/matriz->px; //Número de píxeles de la matriz que constituyen un píxel efectivo
		npix*=npix;
		params->MGrande=(uint)((float)params->MGrande*npix+0.5F);
		params->Mwater=(uint)((float)params->Mwater*npix+0.5F);
		params->mwater=(uint)((float)params->mwater*npix+0.5F);
		d1*=npix; //d1 definitivo
	}

	//Reasignar valores imposibles
	maxeq(params->Mwater,4);
	maxeq(params->MGrande,params->Mwater);
	mineq(params->mwater,params->Mwater);
	//Expansión
	if(lagos->expansión==1) params->mwater=(uint)(d1+0.5F);
	else if(lagos->expansión>=2) params->mwater=1; //Todos los <GRANDE son medianos

	//Para lagos planos
	params->MPlano=((lagos->planos+1)*params->Mwater)>>1; //Si una lámina de agua está en medio de una zona plana, tendrá
	if(matriz->pixel<9.F) params->MPlano=(uint)(3.F*(float)params->MPlano/sqrtf(matriz->pixel));	//que ser al menos MPlano de grande para que se considere como grande.

	params->Δplano=(uEarthHeight)(10*matriz->zbounds.escala); //=10 metros
	d1=matriz->pixel*4;
	if(d1<256.0f){ //hence matriz->pixel<64. Tomar Δplano más pequeño.
		params->Δplano*=(EarthHeight)sqrtpm((uint)d1);
		params->Δplano/=16;
	}

	params->exig_desagües=min(lagos->exig_desagües,MAX_DESAGÜES);
	if(matriz->px<DS_step){
		float f=DS_step/matriz->px;
		uint u=(uint)(f*(float)params->exig_desagües+0.5F);
		params->exig_desagües= u>=256? 255 : (uint8m)u;
	}
}

//DS_step en metros. Solamente se emplea para obtener Mwater
int interpreta_matriz(MatrizTierra *matriz, float DS_step, const Opciones_Lagos *olagos){
	int nret=0;
	PLIST plist;

	uint NPX;
	uEarthHeight *pmatriz;
	uint16m *bwater;	//El bit bajo guarda las marcas. El alto, el número de lago
	Vector_Región regiones; //El primer y último corte en cada fila con el borde de un lago
	Vector_Lago lagos;
	LagosParams params;

	ifunlike(matriz->zbounds.zmax==matriz->zbounds.zmin){
		aj_malloc_return(matriz->i.cotaslagos,uEarthHeight,1+2);
		checked_malloc_n(matriz->i.gruposlago,uLago_t,1+2, free_null(matriz->i.cotaslagos); return AT_NOMEM);

		durchlaufep(flagMDT,matriz->i.flags,matriz->npuntos) p->lago=1;
		matriz->i.cotaslagos[0]=0;
		matriz->i.cotaslagos[1]=matriz->zbounds.zmin;
		matriz->i.cotaslagos[2]=MAX_UEARTHHEIGHT;
		matriz->i.gruposlago[0]=0;
		matriz->i.gruposlago[1]=1;
		matriz->i.gruposlago[2]=0;
		matriz->esta.nlagos=matriz->esta.nláminas=matriz->esta.ncomponentes=1;
		matriz->esta.nislas=0;

		return 0;
	}

	plist=get_new_plist();
	if(plist==PLIST_OUT_OF_MEM) return AT_NOMEM;

	/* Setup: Reserva memoria y rellenar los que sea necesario. */
	NPX=matriz->npx+2;
	{uint pixcount_extended=NPX*(matriz->npy+2);
	aj_malloc_add(pmatriz,uEarthHeight,pixcount_extended);
	aj_malloc_add(bwater,uint16m,pixcount_extended);
	VStdSetup(Lago,lagos,20);
	VStdSetup(Región,regiones,((matriz->npx+matriz->npy)<<1));
	Set_PixelOffset(NPX); //Inicializar PixelOffset

	//Poner un marco alrededor de la matriz de altura MAX_UEARTHHEIGHT
	uEarthHeight *p1, *p2;
	p1=pmatriz;
	p2=matriz->suelo;
	dontimes(NPX,p1++) *p1=MAX_UEARTHHEIGHT;
	dontimes(matriz->npy,){
		*p1++=MAX_UEARTHHEIGHT;
		dontimes(matriz->npx,) *p1++=*p2++;
		*p1++=MAX_UEARTHHEIGHT;
	}
	dontimes(NPX,p1++) *p1=MAX_UEARTHHEIGHT;

	zeroset(bwater,pixcount_extended*usizeof(uint16m));}

	//Params: Valores constantes a lo largo de la función.
	setup_params(&params,matriz,DS_step,olagos);


	/****---  Identificar las láminas de agua  ---****/

	{uEarthHeight *ptr, *tope;
	uint16m *pb;
	ptr=pmatriz+NPX+1;
	tope=pmatriz+NPX*(matriz->npy+1);
	pb=bwater+NPX+1;
	while(ptr<tope){
		if(*ptr==MAX_UEARTHHEIGHT){ //Final de fila
			ptr+=2, pb+=2;
			continue;
		}
		if(*pb&BORDE_IZDO){ //Este píxel es del borde izquierdo de un lago ya creado. Cruzarlo.
			while(!(*pb&BORDE_DCHO)) ptr++, pb++;
			ptr++, pb++;
			continue;
		}

		uEarthHeight cota=*ptr; //La cota del posible lago

		//Mirar si puede ser el píxel izquierdo de un lago sin desagüe aquí
		if(cota>=ptr[-1] || cota>ptr[1] || cota>ptr[-npxi-1] || cota>ptr[-npxi] || cota>ptr[-npxi+1]
							|| cota>ptr[npxi-1] || cota>ptr[npxi] || cota>ptr[npxi+1]
		   ){ptr++, pb++; continue;}

		//Crear un nuevo lago.
		ssint área=crea_borde_eq_izdo(pb,ptr);
		if(área<0){					//Esto es cierto en algunos casos de píxel demasiado grande, y en cualquier caso teóricamente es posible
			destruye_borde_izdo(pb);	//y si sucediese podría generar un crash en el programa. No puede quedar un BORDE_IZDO desparejado.
			ptr++, pb++;
			continue;
		}

		uint nreg=regiones.n; //Recordarlo porque n aumentará
		uint16m MARCA;
			if((uint)área<params.mwater) MARCA=LAGOKIND_PEQ;
			else if((uint)área<params.Mwater) MARCA=LAGOKIND_MEDIO;
			else MARCA=LAGOKIND_GRANDE;
		MARCA=procesa_lago(pb,ptr,área,MARCA,&regiones,&params);
		ifunlike(MARCA==MARCA_NOMEM) goto salida_outofmem;

		if(MARCA<LAGOKIND_GRANDE){
			regiones.n=nreg; //Olvidar las regiones
		}else{
			Lago *pl;
			Venlarge_one(lagos,Lago,pl,goto salida_outofmem);
			pl->ind=(pdif)(pb-bwater);
			pl->internos=nreg;
			pl->klago=0;
		}
		//Dejamos ptr y pb donde empezaron, para que en la siguiente iteración se detecte el BORDE_IZDO y se
		//entre en el proceso que cruza el lago
	}}
	//Compactar los lagos (elimina los lagos grandes que hubieran quedado incluidos dentro de otro lago mayor)
	{Lago *p2, *p1;
	p2=p1=lagos.ppio;
	dontimes(lagos.n,p1++){
		ifz(bwater[p1->ind]&BORDE_IZDO) continue;
		*p2++=*p1;
	}
	lagos.n=(pdif)(p2-lagos.ppio);
	}


	/***----  Expandir los lagos y asignar el número del lago al borde  ----***/

	/* En lugar de una expansión recursiva, que es más complicado, cada polígono incorporado al expandir se
	añade al final del vector de lagos para que se le aplique el algoritmo cuando se llegue a él.
	    Los lagos del vector de lagos comienzan sin número de lago y con LAGOKIND_GRANDE. Además puede
	haber por la matriz otras regiones abandonadas con otro LAGOKIND. Una vez que se procese adquiere un
	número de lago y LAGOKIND ya no hace falta. Esto permite emplear también esos bits para el número de
	lago. Por otra parte, una vez que su borde (el exterior y los interiores) ya está marcado con el número de lago,
	solamente son necesarios los bordes izdo. y dcho. para rellenarlo, y estos a su 	vez se pueden reemplazar por
	el hecho de tener número de lago y mirando *ptr. Así pues, podemos emplear todos los bits para el número
	de lago, excepto el bit alto, que se pone a 1 para indicar que el resto de bits son un número de lago.
	    En cuanto un lago se procese todos sus satélites pequeños que aún no hubieran sido procesados quedarán
	con su número de lago y no se distinguen del lago principal. Los satélites medianos se dejan sin procesar, se
	convierten a un lago grande y se añaden al final del vector de lagos, como cualquier otro lago, pero en este
	vector con el campo klago recordando el número del lago original, para que no adquieran erróneamente un
	número de lago nuevo.
	    Se indicarán los conjuntos de lagos que se tocan en diagonal en gruposlago:  i -> while(grupo(i)!=i) i=grupo(i).
	    Se asignan aquí cotaslagos.*/

	matriz->esta.nláminas=lagos.n; //= al valor de Klago al terminar el bucle
	ifunlike(lagos.n>max_nlago){
		nret=MTIERRA_TooManyLakes;
		goto salida;
	}
	aj_malloc_add(matriz->i.cotaslagos,uEarthHeight,lagos.n+2);
	aj_malloc_add(matriz->i.gruposlago,uLago_t,lagos.n+2);
	matriz->i.cotaslagos[0]=0;
	matriz->i.gruposlago[0]=0;

#define lgrupos matriz->i.gruposlago
	uLago_t Klago=0;
	{Lago *ptri=lagos.ppio;
	for(uint ilago=0; ilago<lagos.n;ilago++,ptri++){ //lagos.n puede aumentar en el bucle al añadirse más lagos al vector
		uint16m *pb=bwater+ptri->ind;
		uEarthHeight *ptr=pmatriz+ptri->ind;
		uLago_t klago;
		uLago_t ngrupo; //Números a guardar en matriz
		Lago *pold; //Por si el vector lagos.ppio se mueve de sitio al añadir más componentes

		if(ptri->klago!=0) klago=ptri->klago; //Un lago mediano añadido
		else{klago=++Klago; lgrupos[klago]=klago; //Un lago grande, que ya estaba al empezar este bucle
			   matriz->i.cotaslagos[klago]=*ptr;		//y que es la primera vez que se procesa
		}
		ngrupo=klago; while(lgrupos[ngrupo]!=ngrupo) ngrupo=lgrupos[ngrupo];

		pold=lagos.ppio;
		ifunlike(expande_borde_izdo(pb,ptr,klago,ngrupo,&lagos,&regiones,&lgrupos[0],bwater)) goto salida_outofmem;
		ifunlike(lagos.ppio!=pold) ptri=lagos.ppio+(ptri-pold);
		pold=lagos.ppio;
		for(Región *pr=regiones.ppio+ptri->internos;pr->pb!=NULL;pr++){
			ifunlike(expande_borde_dcho(pr->pb,pmatriz+(pr->pb-bwater),klago,ngrupo,&lagos,&regiones,&lgrupos[0],bwater)) goto salida_outofmem;
		}
		ifunlike(lagos.ppio!=pold) ptri=lagos.ppio+(ptri-pold);

		//Remarcar el lago y sus islas. Los satélites pequeños ya se han marcado en expande_borde_izdo.
		crea_marcalago_izdo(pb,ptr,klago);
		for(Región *pr=regiones.ppio+ptri->internos;pr->pb!=NULL;pr++) crea_marcalago_dcho(pr->pb,pmatriz+(pr->pb-bwater),klago);
	}}
	matriz->i.cotaslagos[++Klago]=MAX_UEARTHHEIGHT; //Marcar el final
	lgrupos[Klago]=0; //Marcar el final

	//Asignar el grupolago definitivo a cada lago y obtener nlagos
	matriz->esta.nlagos=0;
	for(uLago_t i=1; i<Klago;i++){
		uLago_t u;
		if((u=lgrupos[i])==i){matriz->esta.nlagos++; continue;}
		while(lgrupos[u]!=u) u=lgrupos[u];
		lgrupos[i]=u;
	}
	matriz->esta.ncomponentes=lagos.n; //Las pequeñas no se cuentan
#undef lgrupos

	/* Rellenar los lagos con su número de lago, ya en matriz.i.flags
	En pmatriz se han eliminado los espúreos al procesar cada lago, pero en matriz, no. Tenemos
	cuidado de actualizar matriz. */
	{Durchlaufei(uint16m,bwater+2*NPX-1,matriz->npy){*ptri=0xFFFF; ptri+=NPX;}} //Emplearemos *pb para detectar el final de cada fila.
	{uint16m *pb, *tope;
	flagMDT *pflags;
	pb=bwater+NPX+1;
	tope=bwater+NPX*(matriz->npy+1);
	pflags=matriz->i.flags;
	while(pb<tope){
		uint16m *pbold=pb;
		while(*pb!=0xFFFF && !TSTATE_has_LAGO(*pb)) pb++;
		pflags+=pb-pbold; //Mantener pflags en su sitio
		if(*pb==0xFFFF){pb+=2; continue;} //Final de fila
													//else, has_LAGO
		uLago_t l=LAGO_get_n(*pb);
		const uEarthHeight *ptr=pmatriz+(pb-bwater);
		uEarthHeight cota=*ptr;
		uEarthHeight *ppio=matriz->suelo+(pflags-matriz->i.flags); //Para rellenarlo, por si había espúreos
			while(*ptr==cota) pb++, ptr++, pflags++->lago=l;
		uEarthHeight * const pfin=matriz->suelo+(pflags-matriz->i.flags);
		do *ppio++=cota; while(ppio<pfin); //Rellenarlo
	}}

	//Número de islas
	matriz->esta.nislas=0;
	{durchVectori(Región,regiones) matriz->esta.nislas+=(ptri->pb!=NULL);}
	goto salida;

salida_outofmem:
	nret=AT_NOMEM;
	goto salida;

salida:
	Remove_from_delete(matriz->i.cotaslagos); //Si aún no se habían añadido no se hace nada
	Remove_from_delete(matriz->i.gruposlago);
	free_plist(plist);
	return nret;
}

#undef npxi
#undef Set_NextPixelOffset
#undef Δz
#undef Borde
#undef Multiplicador
