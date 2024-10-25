int escribe_tinplano(const char8_t *ftin, const TINPlano *tin){
	int nret;
	Buffer_bo buf;
	TinCabecera cabe;
	uint tamaño_puntos;

	ifnzunlike(nret=boopen_utf8(&buf,ftin,ATBYTES_LITTLE_ENDIAN)) return nret;

	/* Cabecera */
	cabe.datos.W0=/*0<<24 | */ ('N'<<16U) | ('I'<<8U) | 'T'; //Identificador del formato y número de versión
	cabe.datos.tipos_dato=(tin->uni.tipo<<8U) | tin->tdato; //libre, libre, 'uni_tipo', 'tipo_de_dato'
	if(tin->uni.tipo==UniTin_m_float) IO_SINGLE___float(&cabe.datos.unidades,tin->uni.valor.f);
	else cabe.datos.unidades=tin->uni.valor.u;

	if(TipoDatoTIN_is_integer(tin->tdato)) set_byte0(&cabe.datos.tipos_dato,TIN_sint32);
	elif(tin->tdato==TIN_double) set_byte0(&cabe.datos.tipos_dato,TIN_double);
	else set_byte0(&cabe.datos.tipos_dato,TIN_float);

	//puntos
	cabe.datos.p_retic_pos=Я;	cabe.datos.p_retic_n=0;
	cabe.datos.p_inc_pos=Я;	cabe.datos.p_inc_n=0;
	cabe.datos.p_indiv_pos=uintsizeof(TinCabecera);
	cabe.datos.p_indiv_n=tin->np;
	//
	tamaño_puntos=3*cabe.datos.p_indiv_n;
	if(get_byte0(&cabe.datos.tipos_dato)==TIN_double) tamaño_puntos<<=1;
	//triángulos
	cabe.datos.t_malla_pos=Я;	cabe.datos.t_malla_n=0;
	cabe.datos.t_inc_pos=Я;		cabe.datos.t_inc_n=0;
	cabe.datos.t_indiv_pos=cabe.datos.p_indiv_pos+tamaño_puntos;
	cabe.datos.t_indiv_n=tin->nt;
	//Estilos
	cabe.datos.estilos_p_pos=Я;
	cabe.datos.estilos_t_pos=Я;
	//Clase de cada punto y estilos indiv. de puntos (no hay)
	cabe.datos.clases_p_pos=Я;
	cabe.datos.estilo_pindiv_pos=Я;
	//Clase de cada triángulo y estilos indiv. de triángulos
	cabe.datos.clases_t_pos=Я;
	if(tin->estilos_t==NULL) cabe.datos.estilo_tindiv_pos=Я;
	else cabe.datos.estilo_tindiv_pos=cabe.datos.t_indiv_pos+3*cabe.datos.t_indiv_n;

	bowrite_uints(&buf,cabe.uints,uintsizeof(cabe.uints));
#if ATCRT_DOUBLE
	if(get_byte0(&cabe.datos.tipos_dato)!=TIN_float) bowrite_uints(&buf,(uint*)tin->puntos.in,tamaño_puntos);
#else
	if(get_byte0(&cabe.datos.tipos_dato)==TIN_sint32) bowrite_uints(&buf,(uint*)tin->puntos.in,tamaño_puntos);
	elif(get_byte0(&cabe.datos.tipos_dato)==TIN_double){
		durchlaufep(double,tin->puntos.dbl,tamaño_puntos){
			uint io[2]; IO_DOUBLE___double(&io[0],*p)
			boput_32(&buf,io[0]); boput_32(&buf,io[1]);
		}
	}
#endif
	else{
		durchlaufep(double,tin->puntos.dbl,tamaño_puntos){
			uint io; IO_SINGLE___float(&io,(float)*p);
			boput_32(&buf,io);
	}	}
	bowrite_uints(&buf,tin->triángulos,3*tin->nt);
	//Estilo de cada triángulos
	if(tin->estilos_t!=NULL){
		boput_32(&buf,Я); //No hay bloque siguiente
		boput_8888(&buf,0x80,0,0,1); // 10... (indica que sigue n t1 est1 est2 ...), 0, 0, 1 (indica que el estilo es 'color')
		boput_32(&buf,tin->nt);
		boput_32(&buf,0); //El primer triángulo es el 0
		bowrite_uints(&buf,tin->estilos_t,tin->nt);
	}

	boclose(&buf);
	return buf.error_code;
}

int escribe_fichero_tin(const char8_t *ftin, const TINMalla *tin, const EstiloTriángulos1 *estilos, uint nestilos){
	int nret;
	Buffer_bo buf;
	TinCabecera cabe;
	TinPuntosRetícula pmalla;
	TriángulosMalla tmalla[2];
	uint npmalla;
	struct{
		uint malla;
		uint incr;
		uint indiv;
	} nT;
	struct{
		uint p_malla;
		uint p_indiv;
		uint t_malla;
		uint t_incs;
		uint t_indiv;
		uint estilos_t;
		uint clases_t;
	} tamaños;

	ifnzunlike(nret=boopen_utf8(&buf,ftin,ATBYTES_LITTLE_ENDIAN)) return nret;

	npmalla=tin->malla.ny*tin->malla.nx; //Número de puntos en la malla

	/* Lo que ocupará cada bloque de elementos */

	//Lo que ocupa en el fichero la malla de puntos: Su cabecera más los puntos (2 por uint)
	tamaños.p_malla=uintsizeof(TinPuntosRetícula)+((npmalla+1)>>1);
	//Puntos individuales. Cada punto indiv. son 3 ssints
	tamaños.p_indiv=(tin->p_indiv.n)*3;
	//t_malla, t_incs y t_indiv
	if(tin->bdiag!=0){
		uint ntmalla=(tin->malla.nx-1)*(tin->malla.ny-1)*2; //Número de triángulos de la malla
		if(tin->bdiag==2){
			nT.malla=ntmalla;
			nT.incr=0;
			//Triángulos en malla regular. Dos series más el EOUIA del final
			tamaños.t_malla=2*uintsizeof(TriángulosMalla)+1;
			tamaños.t_incs=0;
		}else{ //1
			nT.malla=0;
			nT.incr=ntmalla;
			tamaños.t_malla=0;
			//Triángulos en modo incrementos: La cabecera son 2 words; a0 otro, b0 y c0 en otro,
			//y para los demás triángulos, 3 s16int por triángulo.
			tamaños.t_incs=3+((3*ntmalla)>>1);
		}
		nT.indiv=tin->triangles.n-ntmalla;
	}else{
		nT.incr=nT.malla=0;
		tamaños.t_incs=tamaños.t_malla=0;
		nT.indiv=tin->triangles.n;
	}
	//Triángulos individuales. Cada triángulo son 3 uints (los vértices)
	tamaños.t_indiv=3*nT.indiv;

	//Estilos de los triángulos: Un uint para indicar el formato, otro para el número de estilos
	//definidos, y luego los estilos.
	if(estilos==NULL || nestilos==0) tamaños.estilos_t=0;
	else tamaños.estilos_t=2+nestilos*uintsizeof(EstiloTriángulos1);
	//Clase a la que pertenece cada triángulo: Un uint que indica que la clase de cada triángulo
	//ocupa 2 bits. Luego divup(nt,16) uints con los números de clase.
	tamaños.clases_t=1+divup(tin->triangles.n,16);


	/** Rellenar estructuras con datos que se escribirán al fichero **/

	/* Cabecera */
	cabe.datos.W0=uint___u1111(0,'N','I','T'); //Identificador del formato y número de versión
	cabe.datos.tipos_dato=uint___u1111(0,0,tin->uni.tipo,TIN_sint32); //libre, libre, 'uni_tipo', 'sint32'
	if(tin->uni.tipo==UniTin_m_float) IO_SINGLE___float(&cabe.datos.unidades,tin->uni.valor.f);
	else cabe.datos.unidades=tin->uni.valor.u;
	//puntos en retícula
	cabe.datos.p_retic_pos=uintsizeof(TinCabecera);
	cabe.datos.p_retic_n=npmalla;
	//puntos en incrementos (no hay)
	cabe.datos.p_inc_pos=Я;
	cabe.datos.p_inc_n=0;
	//puntos individuales
	cabe.datos.p_indiv_pos=cabe.datos.p_retic_pos+tamaños.p_malla;
	cabe.datos.p_indiv_n=tin->p_indiv.n;

	//triángulos
	cabe.datos.t_malla_n	=nT.malla;
	cabe.datos.t_inc_n		=nT.incr;
	cabe.datos.t_indiv_n	=nT.indiv;
	cabe.datos.t_malla_pos	=cabe.datos.p_indiv_pos+tamaños.p_indiv;
	cabe.datos.t_inc_pos		=cabe.datos.t_malla_pos+tamaños.t_malla;
	cabe.datos.t_indiv_pos	=cabe.datos.t_inc_pos+tamaños.t_incs;
	//Estilos de los puntos (no hay)
	cabe.datos.estilos_p_pos=Я;
	//Estilos de los triángulos
	cabe.datos.estilos_t_pos=cabe.datos.t_indiv_pos+tamaños.t_indiv;
	//Clase de cada punto y estilos indiv. de puntos (no hay)
	cabe.datos.clases_p_pos=Я;
	cabe.datos.estilo_pindiv_pos=Я;
	//Clase de cada triángulo y estilos indiv. de triángulos
	cabe.datos.clases_t_pos=cabe.datos.estilos_t_pos+tamaños.estilos_t;
	cabe.datos.estilo_tindiv_pos=Я; //No hay

	if(tamaños.t_malla==0) cabe.datos.t_malla_pos=Я;
	if(tamaños.t_incs==0) cabe.datos.t_inc_pos=Я;
	if(tamaños.t_indiv==0) cabe.datos.t_inc_pos=Я;
	if(tamaños.estilos_t==0) cabe.datos.estilos_t_pos=Я;

	/* Encabezado de los puntos en retícula */
	pmalla.m=tin->malla.ny;
	pmalla.n=tin->malla.nx;
	pmalla.siguiente=Я; //No hay más mallas
	//Los datos serán sin16
	pmalla.dato=1;
	pmalla.X0=tin->malla.x0;
	pmalla.Y0=tin->malla.y0;
	pmalla.Z0=0;
	//El incremento dentro de una fila
	pmalla.ΔX1=tin->malla.Δx; pmalla.ΔY1=0; pmalla.ΔZ1=0;
	//El incremento de una fila a otra
	pmalla.ΔXn=0; pmalla.ΔYn=tin->malla.Δy; pmalla.ΔZn=0;
	//El vector de desplazamiento desde cada punto de la malla.
	pmalla.ΔX=0; pmalla.ΔY=0; pmalla.ΔZ=1;

	/* Triángulos en malla, si hay */
	if(nT.malla!=0){
		tmalla[0].m=pmalla.m-1;
		tmalla[0].n=pmalla.n-1;
		tmalla[0].a0=pmalla.n; tmalla[0].b0=pmalla.n+1; tmalla[0].c0=0;
		tmalla[0].Δc1=tmalla[0].Δb1=tmalla[0].Δa1=1;
		tmalla[0].Δcn=tmalla[0].Δbn=tmalla[0].Δan=pmalla.n;
		//
		tmalla[1]=tmalla[0];
		tmalla[1].a0=1; tmalla[1].b0=0; tmalla[1].c0=pmalla.n+1;
	}


	/** Escribir los elementos al fichero **/

	//Escribir la cabecera
	bowrite_uints(&buf,cabe.uints,uintsizeof(cabe.uints));

	//Escribir los puntos en retícula
	/*En cada uint del fichero van dos puntos (dos valores de Z). Aunque el valor de npmalla sea
	impar podemos tomar el siguiente punto 	en tin->malla.z para completar el último uint, ya
	que tras los valores de Z hay padding bytes o el vector p_indiv, y se puede leer un punto más.*/
	bowrite_uints(&buf,(uint*)&pmalla,uintsizeof(pmalla));
#if !defined(NON_STD_INTEGERS) && INT_LEAST16_MAX==0x7fFF
	bowrite_uints(&buf,(uint*)tin->malla.z.ppio,(tin->malla.z.n+1)>>1);
#else
	{Durchlaufep(const sint16m,tin->malla.z.ppio,(tin->malla.z.n+1)>>1){
		uint16m s1=*p++;
		uint16m s2=*p++;
		boput_1616(&buf,s2,s1);
	}}
#endif

	//Escribir los puntos individuales
	{durchVectorp(const PuntoXYZ_ssint,tin->p_indiv){
		boput_32(&buf,(uint)p->X);
		boput_32(&buf,(uint)p->Y);
		boput_32(&buf,(uint)p->Z);
	}}

	if(nT.malla!=0){
		/*Escribir los triángulos en malla*/
		bowrite_uints(&buf,(uint*)&tmalla,uintsizeof(tmalla));
		boput_32(&buf,Я);
	}
	if(nT.incr!=0){
		/*Escribir los triángulos en incrementos*/
		//La cabecera
		boput_32(&buf,nT.incr); //Número de triángulos definidos
		boput_32(&buf,Я); //Siguiente (no hay siguiente)
		//Los puntos. Como el formato establece que han de representarse en complemento a 2,
		//hacemos la resta y la reducimos a uint16m sin más.
		uint d; //El último vértice escrito
		const TinTriangle *p=tin->triangles.ppio;
		//Primer triángulo
		boput_32(&buf,p->a);
		boput_1616(&buf,(uint16m)(p->c-p->b),(uint16m)(p->b-p->a));
		d=p++->c;
		//resto de triángulos
		dontimes((nT.incr-1)>>1,){ //No pasarse del último triángulo
			boput_1616(&buf,(uint16m)(p->b-p->a),(uint16m)(p->a-d));
			uint16m s1=(uint16m)(p->c-p->b);
			d=p++->c;
			uint16m s2=(uint16m)(p->a-d);
			boput_1616(&buf,s2,s1);
			boput_1616(&buf,(uint16m)(p->c-p->b),(uint16m)(p->b-p->a));
			d=p++->c;
		}
		//Si (nT.incr-1) es impar falta el último
		if((nT.incr-1)&1){
			boput_1616(&buf,(uint16m)(p->b-p->a),(uint16m)(p->a-d));
			boput_1616(&buf,0,(uint16m)(p->c-p->b));
			p++;
		}
	}
	if(nT.indiv!=0){
		durchlaufep(const TinTriangle,tin->triangles.ppio+nT.malla+nT.incr, nT.indiv){
			boput_32(&buf,p->a); boput_32(&buf,p->b); boput_32(&buf,p->c);
	}}

	//Escribir los estilos de los triángulos
	if(estilos!=NULL && nestilos!=0){
		boput_32(&buf,1); //Tipo de información de estilo presente
		boput_32(&buf,nestilos); //Siguen 'nestilos' estilos
		bowrite_uints(&buf,(uint*)&estilos[0],nestilos*uintsizeof(EstiloTriángulos1));
	}

	//Escribir la clase a que pertenece cada triángulo
	const TinTriangle *p, *pend;
	boput_32(&buf,2); //2 bits per triángulos
	if(nT.malla!=0){ //Los triángulos no se han escrito al fichero en el orden en que están en tin->triangles
		uint u;
		p=tin->triangles.ppio;
		pend=p+nT.malla;
		//La primera malla (pares)
		dontimes((nT.malla>>1)/16,){
			u=0;
			u|=(p->class&3);		p+=2;		u|=(p->class&3)<<2;	p+=2;
			u|=(p->class&3)<<4;	p+=2;		u|=(p->class&3)<<6;	p+=2;
			u|=(p->class&3)<<8;	p+=2;		u|=(p->class&3)<<10;	p+=2;
			u|=(p->class&3)<<12;	p+=2;		u|=(p->class&3)<<14;	p+=2;
			u|=(p->class&3)<<16;	p+=2;		u|=(p->class&3)<<18;	p+=2;
			u|=(p->class&3)<<20;	p+=2;		u|=(p->class&3)<<22;	p+=2;
			u|=(p->class&3)<<24;	p+=2;		u|=(p->class&3)<<26;	p+=2;
			u|=(p->class&3)<<28;	p+=2;		u|=(p->class&3)<<30;	p+=2;
			boput_32(&buf,u);
		}

		//El final de una malla y el principio de la otra
		u=0;
		{uint8m shift;
		for(shift=0; p!=pend; p+=2, shift+=2){
			u|=(p->class&3)<<shift;
		}
		pend++; //Hence, points two positions past the last one
		p=tin->triangles.ppio+1;
		for(;shift!=32 && p!=pend; p+=2, shift+=2){
			u|=(p->class&3)<<shift;
		}}
		boput_32(&buf,u);

		//La segunda malla (impares)
		dontimes(((pdif)(pend-p)>>1)/16,){
			u=0;
			u|=(p->class&3);		p+=2;		u|=(p->class&3)<<2;	p+=2;
			u|=(p->class&3)<<4;	p+=2;		u|=(p->class&3)<<6;	p+=2;
			u|=(p->class&3)<<8;	p+=2;		u|=(p->class&3)<<10;	p+=2;
			u|=(p->class&3)<<12;	p+=2;		u|=(p->class&3)<<14;	p+=2;
			u|=(p->class&3)<<16;	p+=2;		u|=(p->class&3)<<18;	p+=2;
			u|=(p->class&3)<<20;	p+=2;		u|=(p->class&3)<<22;	p+=2;
			u|=(p->class&3)<<24;	p+=2;		u|=(p->class&3)<<26;	p+=2;
			u|=(p->class&3)<<28;	p+=2;		u|=(p->class&3)<<30;	p+=2;
			boput_32(&buf,u);
		}

		//El final de la segunda malla y el principio del resto de triángulos
		u=0;
		{uint8m shift;
		for(shift=0; p!=pend; p+=2, shift+=2){
			u|=(p->class&3)<<shift;
		}
		pend=tin->triangles.ppio+tin->triangles.n;
		p=tin->triangles.ppio+nT.malla;
		for(;shift!=32 && p!=pend; p++, shift+=2){
			u|=(p->class&3)<<shift;
		}}
		boput_32(&buf,u);

		//El resto de triángulos
		dontimes((pdif)(pend-p)/16,){
			u=0;
			u|=(p++->class&3);			u|=(p++->class&3)<<2;
			u|=(p++->class&3)<<4;		u|=(p++->class&3)<<6;
			u|=(p++->class&3)<<8;		u|=(p++->class&3)<<10;
			u|=(p++->class&3)<<12;	u|=(p++->class&3)<<14;
			u|=(p++->class&3)<<16;	u|=(p++->class&3)<<18;
			u|=(p++->class&3)<<20;	u|=(p++->class&3)<<22;
			u|=(p++->class&3)<<24;	u|=(p++->class&3)<<26;
			u|=(p++->class&3)<<28;	u|=(p++->class&3)<<30;
			boput_32(&buf,u);
		}
	}else{
		p=tin->triangles.ppio;
		pend=p+tin->triangles.n;
		dontimes(tin->triangles.n/16,){ //Los que ocupan un número entero de palabras
			uint u=0;
			u|=(p++->class&3);			u|=(p++->class&3)<<2;
			u|=(p++->class&3)<<4;		u|=(p++->class&3)<<6;
			u|=(p++->class&3)<<8;		u|=(p++->class&3)<<10;
			u|=(p++->class&3)<<12;	u|=(p++->class&3)<<14;
			u|=(p++->class&3)<<16;	u|=(p++->class&3)<<18;
			u|=(p++->class&3)<<20;	u|=(p++->class&3)<<22;
			u|=(p++->class&3)<<24;	u|=(p++->class&3)<<26;
			u|=(p++->class&3)<<28;	u|=(p++->class&3)<<30;
			boput_32(&buf,u);
		}
	}
	if(p!=pend){ //El resto
		uint u=0;
		for(uint8m c=0; p!=pend; p++, c+=2){
			u|=(p->class&3)<<c;
		}
		boput_32(&buf,u);
	}

	boclose(&buf);
	return buf.error_code;
}
