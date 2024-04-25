//d0: distancia a aplicar al punto del centro de la máscara. d0 se sumará a todos los valores de la máscara
//wd_mask tiene que ser el semi-ancho. El ancho total es 2*wd_mask+1.
void remascara(umint *pcor,const umint *mask,uint8m wd_mask,uint NPX, umint d0){
	pcor-=wd_mask*(NPX+1);
	wd_mask<<=1; wd_mask++; //el ancho total
	const uint salto_m=MASK_MAX_N-wd_mask; //El salto en la máscara
	const uint salto=NPX-wd_mask;

	dontimes(wd_mask,(mask+=salto_m,pcor+=salto)){
		dontimes(wd_mask,(mask++,pcor++)){
			mineq(*pcor,d0+*mask);
		}
	}
}

/*Los puntos fuera del DS-local tendrán un valor de *corte=0. Los puntos de la franja en la que habrá que
interpolar tendrán un 'c' >=1. Mayor cuanto más lejos estén del borde. Los que no han entrado dentro de
ninguna máscara quedarán con Я.
wd_mask:   La distancia al píxel central del borde de la máscara; es decir, la máscara tendrá 2*wd_mask+1 píxeles
extra_side: El número de píxeles de margen que incluye 'corte' en cada lado.
*/
void enmascara_franja(umint *corte, const ssint *matriz,ssint npx, ssint npy, umint wd_mask, umint extra_side){
	corte+=extra_side*(1+npx+2*extra_side); //Al primer punto de datos
	ifunlike(wd_mask>MASK_MAX_NHALF) wd_mask=MASK_MAX_NHALF;
	ifunlike(wd_mask>extra_side) wd_mask=extra_side;
	extra_side<<=1;
	const umint * const mask=&Mask[0]+(MASK_MAX_NHALF-wd_mask)*(MASK_MAX_N+1); //Puntero a la esquina
																							//de la máscara que vamos a emplear
	const uint NPX=(uint)npx+extra_side;

	{uint n=(npy-1)*NPX+npx; //Número de píxeles desde el primero hasta el último con datos
	iflike(extra_side>=1){ //Hay margen tras el último punto con datos (al menos cinco puntos).
		uint k=(usizeof(uint)/usizeof(*corte)); //sizeof(uint)<=4;
		n=(n+k)/k;
		oneset_uint(corte,n);
	}else{ //Esto no va a ser true nunca
		oneset(corte,n*uintsizeof(*corte));
	}}

	//Primera fila
	//Pimero
	if(*matriz==NODATA_ssint) *corte=0;
	else{
		umint d=0;
		if(matriz[1]==NODATA_ssint || matriz[npx]==NODATA_ssint) d=1;
		else if(matriz[npx+1]==NODATA_ssint) d=2;
		if(d!=0) remascara(corte,mask,wd_mask,NPX,d);
	}
	matriz++,corte++;
	//Línea
	dontimes(npx-2,(matriz++,corte++)){
		umint d;
		if(*matriz==NODATA_ssint){*corte=0; continue;}
		if(matriz[-1]==NODATA_ssint || matriz[1]==NODATA_ssint || matriz[npx]==NODATA_ssint) d=1;
		else if(matriz[npx-1]==NODATA_ssint || matriz[npx+1]==NODATA_ssint) d=2;
		else continue;
		remascara(corte,mask,wd_mask,NPX,d);
	}
	//Último
	if(*matriz==NODATA_ssint) *corte=0;
	else{
		umint d=0;
		if(matriz[-1]==NODATA_ssint || matriz[npx]==NODATA_ssint) d=1;
		else if(matriz[npx-1]==NODATA_ssint) d=2;
		if(d!=0) remascara(corte,mask,wd_mask,NPX,d);
	}
	matriz++, corte++;
	corte+=extra_side;

	//Intermedidas
	dontimes(npy-2,corte+=extra_side){
		//Pimero
		if(*matriz==NODATA_ssint) *corte=0;
		else{
			umint d=0;
			if(matriz[1]==NODATA_ssint || matriz[-npx]==NODATA_ssint || matriz[npx]==NODATA_ssint) d=1;
			else if(matriz[-npx+1]==NODATA_ssint || matriz[npx+1]==NODATA_ssint) d=2;
			if(d!=0) remascara(corte,mask,wd_mask,NPX,d);
		}
		matriz++,corte++;
		//Línea
		dontimes(npx-2,(matriz++,corte++)){
			umint d;
			if(*matriz==NODATA_ssint){*corte=0; continue;}
			if(matriz[-npx]==NODATA_ssint || matriz[-1]==NODATA_ssint || matriz[1]==NODATA_ssint || matriz[npx]==NODATA_ssint) d=1;
			else if(matriz[-npx-1]==NODATA_ssint || matriz[-npx+1]==NODATA_ssint || matriz[npx-1]==NODATA_ssint || matriz[npx+1]==NODATA_ssint) d=2;
			else continue;
			remascara(corte,mask,wd_mask,NPX,d);
		}
		//Último
		if(*matriz==NODATA_ssint) *corte=0;
		else{
			umint d=0;
			if(matriz[-1]==NODATA_ssint || matriz[-npx]==NODATA_ssint || matriz[npx]==NODATA_ssint) d=1;
			else if(matriz[-npx-1]==NODATA_ssint || matriz[npx-1]==NODATA_ssint) d=2;
			if(d!=0) remascara(corte,mask,wd_mask,NPX,d);
		}
		matriz++,corte++;
	}

	//Última fila
	//Pimero
	if(*matriz==NODATA_ssint) *corte=0;
	else{
		umint d=0;
		if(matriz[1]==NODATA_ssint || matriz[-npx]==NODATA_ssint) d=1;
		else if(matriz[-npx+1]==NODATA_ssint) d=2;
		if(d!=0) remascara(corte,mask,wd_mask,NPX,d);
	}
	//Línea
	matriz++,corte++;
	dontimes(npx-2,(matriz++,corte++)){
		umint d;
		if(*matriz==NODATA_ssint){*corte=0; continue;}
		if(matriz[-npx]==NODATA_ssint || matriz[-1]==NODATA_ssint || matriz[1]==NODATA_ssint) d=1;
		else if(matriz[-npx-1]==NODATA_ssint || matriz[-npx+1]==NODATA_ssint) d=2;
		else continue;
		remascara(corte,mask,wd_mask,NPX,d);
	}
	//Último
	if(*matriz==NODATA_ssint) *corte=0;
	else{
		umint d=0;
		if(matriz[-1]==NODATA_ssint || matriz[-npx]==NODATA_ssint) d=1;
		else if(matriz[-npx-1]==NODATA_ssint) d=2;
		if(d!=0) remascara(corte,mask,wd_mask,NPX,d);
	}
	//matriz++, corte++;
	//corte+=extra_side;
}
