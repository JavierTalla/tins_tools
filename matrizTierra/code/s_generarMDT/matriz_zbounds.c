//Asignar el nuevo valor de minz manteniendo maxz y si hace falta reajustar offset
void matriz_cambia_minz(MatrizTierra *matriz,EarthHeight min){
	EarthHeight offset; //nuevo offset
	matriz->zbounds.zmin=min;
	if(min<0) offset=matriz->zbounds.zmin;
	else if(matriz->zbounds.zmax*matriz->zbounds.escala>0xFFFE) offset=matriz->zbounds.zmax;
	else offset=0;

	//Si el offset ha cambiado hay que adecuar los valores de matriz al nuevo offset
	if(offset!=matriz->zbounds.offset){ //Tiene que ser offset<matriz->zbounds.offset
		uEarthHeight off=(uEarthHeight)(matriz->zbounds.escala*(matriz->zbounds.offset-offset));
		{durchlaufep(uEarthHeight,matriz->suelo,matriz->npuntos) *p+=off;}
		for(uEarthHeight *ptr=matriz->i.cotaslagos;*ptr!=MAX_UEARTHHEIGHT;ptr++) *ptr+=off;
		matriz->zbounds.offset=offset;
	}
}

/*Copiar los datos a matriz->suelo pasándolos a uint16m, apliclando el escalado
y offset necesarios, en su caso.
matriz->flags puede ser NULL. Si no es NULL, los puntos con flag.fuera se ignoran.
Si matriz->suelo es NULL, reserva para ello.
matriz->zbounds.escala tiene que estar asignado. Es la escala de _matriz.
_matriz no es const porque puede ser necesario reescalar, en cuyo caso zbounds.escala
	se ajusta.
El offset ha de ser un número exacto de metros, así que permitimos alturas solamente
	hasta 0xFFF0 para que haya margen.
Return:
	0
	AT_NOMEM
	MTIERRA_MuchoDesnivel
*/
int matriz____matriz(PLIST plist, MatrizTierra *matriz, ssint *_matriz){
	ssint min, max;
	uint esc;
	uint Δ, lim;
	esc=matriz->zbounds.escala;

	max=INT32_MIN;
	min=INT32_MAX;
	if(matriz->i.flags==NULL){
		durchlaufep(ssint,_matriz,matriz->npuntos){
			if(*p==NODATA_ssint) continue;
			mineq(min,*p); maxeq(max,*p);
		}
	}else{
		durchlaufe2(ssint,_matriz,matriz->npuntos,flagMDT,matriz->i.flags){
			if(ptr_b->fuera) continue;
			if(*ptr==NODATA_ssint) continue;
			mineq(min,*ptr); maxeq(max,*ptr);
		}
	}
	ifunlike(max<=min){
		min=0; max=1;
	}
	Δ=max-min;
	if(min>0) lim=max;
	else if(max<0) lim=-min;
	else lim=Δ;

	ifunlike(lim>0xFFF0*esc){ //Sólo puede suceder si el desnivel de la zona (o mín., o máx.) es mayor de ¡64 Km!
		return MTIERRA_MuchoDesnivel;
	}
	//Establecer zbounds.escala
	ifunlike(Δ>0xFFF0){
		//Parte entera al redondear. Los valores negativos quedan negativos.
		do{Δ>>=1, esc>>=1;
			min>>=1, max>>=1;
			durchlaufep(ssint,_matriz,matriz->npuntos) *p>>=1;
		}while(Δ>0xFFF0);
		matriz->zbounds.escala=esc;
	}
	//Establecer zbounds.min y max.
	while(esc>1) min>>=1, max=(max+1)>>1, esc>>=1;
	matriz->zbounds.zmin=(EarthHeight)min;
	matriz->zbounds.zmax=(EarthHeight)max;
	//zbounds.offset
	max*=matriz->zbounds.escala;
	if(min<0) matriz->zbounds.offset=matriz->zbounds.zmin;
	else if(max>0xFFFE) matriz->zbounds.offset=matriz->zbounds.zmax;
	else matriz->zbounds.offset=0;

	//Copia a matriz, aplicando offset.
	if(matriz->suelo==NULL){
		aj_malloc_add(matriz->suelo,uEarthHeight,matriz->npuntos);
	}
	{const ssint off=matriz->zbounds.offset*matriz->zbounds.escala;
	durchlaufe2(uEarthHeight,matriz->suelo,matriz->npuntos,ssint,_matriz){
		if(*ptr_b==NODATA_ssint) *ptr=NODATA_uEarth;
		else *ptr=(uEarthHeight)(*ptr_b-off);
	}}

	return 0;

salida_outofmem:
	return AT_NOMEM;
}
