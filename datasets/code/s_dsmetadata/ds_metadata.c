#undef COMMENT_CHAR
#define COMMENT_CHAR '%'

#ifndef elif
#define elif else if
#endif
#define sinline static inline

//These are for buffer.pc pointing to a ' ','\t','\n'-ended word
//La palabra no puede estar terminada por '%'; es decir, no puede aparecer un comentario
//inmediatamente pegado a una palabra, pues se tomaría como parte de ella.
#define isVALb(s) (strcmp8_b(buf->pc,s)==0)
#define ifVALb(s) if(isVALb(s))
#define elifVALb(s) else ifVALb(s)

//These are to be used for ptr pointing to a '\0'-ended word
#define isKEY(s) (strcmp8(ptr,s)==0)
#define ifKEY(s) if(isKEY(s))
#define elifKEY(s) else ifKEY(s)

/*
pc apunta a una clave (la primera palabra de una línea) y el resto de la línea.
Deja la clave terminada por '\0' o '\n'. Este último caso solamente si ya estaba terminada por '\n'.
Si encuentra un signo igual, '=', tras la clave, lo borra.
Avanza pc hasta la palabra siguiente o el '\n' o COMMENT_CHAR del final de línea si no hay
más palabras. Un comentario pegado a la clave no se tratará como comentario sino que se
tomará como parte de la palabra.
Return:
	true: Se encontró un '=', que fue borrado.
	false: No se encontró '='.

En caso de que devuelva true, la clave estará terminada por '\0'.
*/
bint isolate_key_advance(char8_t **pc){
	while(isnot_stn(**pc) && **pc!='=') (*pc)++;
	if(**pc!='=' && **pc!='\n'){*(*pc)++='\0'; Advanceinline_pc(*pc);}
	if(**pc=='='){*(*pc)++='\0'; Advanceinline_pc(*pc); return true;}
	return false;
}

#define buffer (*buf)

//Devuelven 1 si el valor de alguna clave es erróneo
int lee_Dato(Bufferti8 *buf, uint8m *type){
	uint n;
	getfast_advanceinline(buffer,n,uint___str8);
	if(n==8) *type=INTDATA_TYPE_8U; //Defaults to unsigned
	elif(n==16) *type=INTDATA_TYPE_16U;
	elif(n==32) *type=INTDATA_TYPE_32U;
	else return 1;

	if_Moreinl(buffer){
		ifVALb("bit"){str_stn(buffer.pc); //Optional "bit" word
			Advanceinline(buffer); if_Nomore(buffer) return 0;
		}
		ifVALb("signed") *type|=1;
	}
	return 0;
}

//El valor de las unidades tiene que ser una fracción exacta de metro, al menos de momento
int lee_UnidadesZ(Bufferti8 *buf, uint8m *uni){
	uint n;
	getfast_advanceinline(buffer,n,uint___str8);
	ifunlike_Nomore(buffer) return 1;
	ifunlike(n==0) return 1;

	if(*buf->pc=='c' && buf->pc[1]=='m'){
		buf->pc+=2;
		ifunlike(isnot_stn(*buf->pc) && *buf->pc!=COMMENT_CHAR) return 1;
		*uni=(uint8m)(100/n);
		ifunlike(100!=*uni*n) return 1;
		return 0;
	}
	ifunlike(n!=1) return 1;
	if(*buf->pc=='/'){
		buf->pc++; Advanceinline(buffer);
		ifunlike_Nomore(buffer) return 1;
		getfast(buffer,n,uint___str8);
		ifunlike(n==0 || n>=256) return 1;
		*uni=(uint8m)n;
	}else{
		*uni=1;
	}
	Advanceinline(buffer);
	ifunlike(*buf->pc!='m') return 1;
	buf->pc++;
	ifunlike(isnot_stn(*buf->pc) && *buf->pc!=COMMENT_CHAR) return 1;

	return 0;
}

//El valor de las unidades tiene que ser una fracción exacta de metro, al menos de momento
int lee_UnidadesXY_puntos(Bufferti8 *buf, double *uni){
	int nret;
	uint n;
	getfast_advanceinline(buffer,n,uint___str8);
	ifunlike_Nomore(buffer) return 1;
	ifunlike(n==0) return 1;

	*uni=(double)n;
	if(*buf->pc=='/'){
		buf->pc++; Advanceinline(buffer);
		ifunlike_Nomore(buffer) return 1;
		getfast_advanceinline(buffer,n,uint___str8);
		if(n==0) return 1;
		*uni/=(double)n;
	}
	if(*buf->pc=='\"') return 0;

	nret=!isVALb("dter");
	ifz(nret) *uni=n/600.0; //dter
	return nret;
}

int lee_float_uni(Bufferti8 *buf, float *dato){
	getfast_advanceinline(buffer,*dato,vffloat___str8);
	ifunlike_Nomore(buffer) return 1;
	ifVALb("m") return 0;
	elifVALb("cm"){*dato/=100; return 0;}
	return 1;
}

sinline int lee_pasoxy_local(Bufferti8 *buf, sint16m *dato){
	sMagnitudValidada d=lee_valida_sLongitudTierraPeq((econst char8_t**)&buf->pc,0x7FFF,0x7FFF,UNI_CM);
	*dato=(sint16m)d.val;
	return d.err;
}

#undef buffer
#undef isVALb
#define isVALb(s) (strcmp8_b(buffer.pc,s)==0)

//Devuelve 1 si el valor de alguna clave es erróneo
//Las claves que no reconozca las ingora.
int lee_dsMETA_global(ZonalDataSet *ds, const char8_t *fichero){
	Bufferti8 buffer;
	int nret;

	nret=tiopen8(&buffer,fichero);
	ifunlike(nret<0) return nret;
	nret=0;

	ds->Dato.endianness=ATBYTES_LITTLE_ENDIAN;
	ds->Dato.uniZ=1;
	ds->Dato.type=0; //Bad value
	ds->Dato.offset=0;
	ds->Dato.nodata=0; //No value for nodata point
	ds->φmin=-90;
	ds->φmax=90;
	ds->px=0;
	ds->SN=0;	ds->WE=0;
	ds->φstep=0; ds->λstep=0;
	ds->nrows=0; ds->ncols=0;

	Advance(buffer);
	if(*buffer.pc==0xef && buffer.pc[1]==0xbb && buffer.pc[2]==0xbf) buffer.pc+=3; //Byte order mask
	goto first_entry;
continuar:
	finishline_Advance(buffer);
first_entry:
	if(*buffer.pc=='\0') goto end;{
		const char8_t *ptr=buffer.pc;
		if(!isolate_key_advance(&buffer.pc) || NOMORE_INL(buffer)) goto continuar;

		ifKEY(u8"Endianness"){
			ifVALb("BigEndian") ds->Dato.endianness=ATBYTES_BIG_ENDIAN;
			elifVALb("LittleEndian") ds->Dato.endianness=ATBYTES_LITTLE_ENDIAN;
			else goto bad_line;
		}elifKEY(u8"Dato"){ifunlike(lee_Dato(&buffer,&ds->Dato.type)) goto bad_line;}
		elifKEY(u8"UnidadesZ"){ifunlike(lee_UnidadesZ(&buffer,&ds->Dato.uniZ)) goto bad_line;}
		elifKEY(u8"φmin"){getfast(buffer,ds->φmin,vffloat___str8);}
		elifKEY(u8"φmax"){getfast(buffer,ds->φmax,vffloat___str8);}
		elifKEY(u8"TileJump"){
			getfast_advanceinline(buffer,ds->SN,(uint16m)uint___str8); //Lo que abarca el tile, SN, en grados
			ifunlike(strbeginsby8(buffer.pc,u8"°")) goto bad_line;
			buffer.pc+=strlen8(u8"°"); Advanceinline(buffer);

			ifunlike(*buffer.pc!='x'&& *buffer.pc!='X') goto bad_line; //tanto x cuanto
			buffer.pc++; Advanceinline(buffer); ifunlike_Nomore(buffer) goto bad_line;

			getfast(buffer,ds->WE,(uint16m)uint___str8); //Lo que abarca en WE, en grados
				Advanceinline(buffer);
				ifunlike(strbeginsby8(buffer.pc,u8"°")) goto bad_line;
		}elifKEY(u8"Dims"){ //ny x nx
			getfast_advanceinline(buffer,ds->nrows,(uint16m)uint___str8);
			ifunlike(*buffer.pc!='x'&& *buffer.pc!='X') goto bad_line;
				buffer.pc++; Advanceinline(buffer); ifunlike_Nomore(buffer) goto bad_line;
			getfast(buffer,ds->ncols,(uint16m)uint___str8);
		}elifKEY(u8"Pasoφ"){
			getfast_advanceinline(buffer,ds->φstep,(sint16m)ssint___str8);
			ifunlike(*buffer.pc!='\"') goto bad_line;
		}
		elifKEY(u8"Pasoλ"){
			getfast_advanceinline(buffer,ds->λstep,(sint16m)ssint___str8);
			ifunlike(*buffer.pc!='\"') goto bad_line;
		}
		goto continuar;
	bad_line:
		nret=1; goto continuar;
	}
end:
	ticlose(buffer);

	ifunlike(ds->SN==0 || ds->WE==0 || ds->φstep==0 || ds->λstep==0 || ds->nrows==0 || ds->ncols==0) nret=1;
	ds->px=30*(uint16m)abs(ds->φstep);  //Si no estaba establecido, asignarlo. Cada segundo son 30 metros

	if(nret) ds->md_state=METADATA_Wrong;
	else ds->md_state=METADATA_Right;
	return nret;
}

//Devuelve 1 si el valor de alguna clave es erróneo
//Las claves que no reconozca las ingora.
int lee_dsMETA_local(LocalDataSet *ds, const char8_t *fichero){
	Bufferti8 buffer;
	int nret;

	nret=tiopen8(&buffer,fichero);
	ifunlike(nret<0) return nret;
	nret=0;

	ds->Dato.endianness=ATBYTES_LITTLE_ENDIAN;
	ds->Dato.uniZ=1;
	ds->Dato.type=0; //Bad value
	ds->Dato.offset=0;
	ds->Dato.nodata=0; //No value for nodata point
	ds->φmax=ds->φmin=0;
	ds->λmax=ds->λmin=0;
	ds->px=0;
	ds->SN=0;		ds->WE=0;
	ds->φstep=0; ds->λstep=0;
	ds->nrows=0; ds->ncols=0;
	//
	ds->Z0=0; //If not present, is zero.

	Advance(buffer);
	if(*buffer.pc==0xef && buffer.pc[1]==0xbb && buffer.pc[2]==0xbf) buffer.pc+=3; //Byte order mask
	goto first_entry;
continuar:
	finishline_Advance(buffer);
first_entry:
	if(*buffer.pc=='\0') goto end;{
		const char8_t *ptr=buffer.pc;
		if(!isolate_key_advance(&buffer.pc) || NOMORE_INL(buffer)) goto continuar;

		ifKEY(u8"Endianness"){
			ifVALb("BigEndian") ds->Dato.endianness=ATBYTES_BIG_ENDIAN;
			elifVALb("LittleEndian") ds->Dato.endianness=ATBYTES_LITTLE_ENDIAN;
			else nret=1;
		}elifKEY(u8"Dato"){ifunlike(lee_Dato(&buffer,&ds->Dato.type)) goto bad_line;}
		elifKEY(u8"UnidadesZ"){ifunlike(lee_UnidadesZ(&buffer,&ds->Dato.uniZ)) goto bad_line;}
		elifKEY(u8"Offset"){getfast(buffer,ds->Dato.offset,(uint16m)uint___str8);}
		elifKEY(u8"NoDataValue"){getfast(buffer,ds->Dato.nodata,basedinteger___str8);}
		elifKEY(u8"φmin"){getfast(buffer,ds->φmin,vffloat___str8);}
		elifKEY(u8"φmax"){getfast(buffer,ds->φmax,vffloat___str8);}
		elifKEY(u8"λmin"){getfast(buffer,ds->λmin,vffloat___str8);}
		elifKEY(u8"λmax"){getfast(buffer,ds->λmax,vffloat___str8);}
		elifKEY(u8"TileJump"){
			getfast_advanceinline(buffer,ds->SN,(uint8m)uint___str8); //Lo que abarca el tile, SN, en minutos o km.
			if(*buffer.pc=='\''){buffer.pc++; Advanceinline(buffer);}

			ifunlike(*buffer.pc!='x'&& *buffer.pc!='X') goto bad_line; //tanto x cuanto
			buffer.pc++; Advanceinline(buffer); ifunlike_Nomore(buffer) goto bad_line;

			getfast(buffer,ds->WE,(uint8m)uint___str8); //Lo que abarca en WE, en grados
			//Ignorar las unidades
		}elifKEY(u8"Dims"){ //ny x nx
			getfast_advanceinline(buffer,ds->nrows,(uint16m)uint___str8);
			ifunlike(*buffer.pc!='x'&& *buffer.pc!='X') goto bad_line;
				buffer.pc++; Advanceinline(buffer); ifunlike_Nomore(buffer) goto bad_line;
			getfast(buffer,ds->ncols,(uint16m)uint___str8);
		}elif(isKEY(u8"Pasoφ") || isKEY(u8"PasoY")){ifunlike(lee_pasoxy_local(&buffer,&ds->φstep)) goto bad_line;}
		elif(isKEY(u8"Pasoλ") || isKEY(u8"PasoX")){ifunlike(lee_pasoxy_local(&buffer,&ds->λstep)) goto bad_line;}
		elifKEY("Zzero"){ifunlike(lee_float_uni(&buffer,&ds->Z0)) goto bad_line;}
		goto continuar;
	bad_line:
		nret=1; goto continuar;
	}
end:
	ticlose(buffer);

	ifunlike(ds->SN==0 || ds->WE==0 || ds->φstep==0 || ds->λstep==0 || ds->nrows==0 || ds->ncols==0) nret=1;
	if(nret) ds->md_state=METADATA_Wrong;
	else ds->md_state=METADATA_Right;
	return nret;
}

//Devuelve 1 si el valor de alguna clave es erróneo
//Las claves que no reconozca las ingora.
int lee_dsMETA_puntos(PointsDataSet *ds, const char8_t *fichero){
	Bufferti8 buffer;
	int nret;

	nret=tiopen8(&buffer,fichero);
	ifunlike(nret<0) return nret;
	nret=0;

	ds->Dato.endianness=ATBYTES_LITTLE_ENDIAN;
	ds->Dato.uniZ=1;
	ds->Dato.type=0; //Bad value
	ds->uniXY=0; //Bad value
	ds->φmax=ds->φmin=0;
	ds->λmax=ds->λmin=0;
	ds->SN=0;	ds->WE=0;
	//
	ds->Z0=0; //If not present, is zero.

	Advance(buffer);
	if(*buffer.pc==0xef && buffer.pc[1]==0xbb && buffer.pc[2]==0xbf) buffer.pc+=3; //Byte order mask
	goto first_entry;
continuar:
	finishline_Advance(buffer);
first_entry:
	if(*buffer.pc=='\0') goto end;{
		const char8_t *ptr=buffer.pc;
		if(!isolate_key_advance(&buffer.pc) || NOMORE_INL(buffer)) goto continuar;

		ifKEY(u8"Endianness"){
			ifVALb("BigEndian") ds->Dato.endianness=ATBYTES_BIG_ENDIAN;
			elifVALb("LittleEndian") ds->Dato.endianness=ATBYTES_LITTLE_ENDIAN;
			else nret=1;
		}elifKEY(u8"Dato"){ifunlike(lee_Dato(&buffer,&ds->Dato.type)) goto bad_line;}
		elifKEY(u8"UnidadesZ"){ifunlike(lee_UnidadesZ(&buffer,&ds->Dato.uniZ)) goto bad_line;}
		elifKEY(u8"φmin"){getfast(buffer,ds->φmin,vffloat___str8);}
		elifKEY(u8"φmax"){getfast(buffer,ds->φmax,vffloat___str8);}
		elifKEY(u8"λmin"){getfast(buffer,ds->λmin,vffloat___str8);}
		elifKEY(u8"λmax"){getfast(buffer,ds->λmax,vffloat___str8);}
		elifKEY(u8"TileJump"){
			getfast_advanceinline(buffer,ds->SN,(uint8m)uint___str8); //Lo que abarca el tile, SN, en minutos o km.
			if(*buffer.pc=='\''){buffer.pc++; Advanceinline(buffer);}

			ifunlike(*buffer.pc!='x'&& *buffer.pc!='X') goto bad_line; //tanto x cuanto
			buffer.pc++; Advanceinline(buffer); ifunlike_Nomore(buffer) goto bad_line;

			getfast(buffer,ds->WE,(uint8m)uint___str8); //Lo que abarca en WE, en grados
			//Ignorar las unidades
		}
		elifKEY(u8"UnidadesXY"){ifunlike(lee_UnidadesXY_puntos(&buffer,&ds->uniXY)) goto bad_line;}
		elifKEY("Zzero"){ifunlike(lee_float_uni(&buffer,&ds->Z0)) goto bad_line;}
		goto continuar;
	bad_line:
		nret=1; goto continuar;
	}
end:
	ticlose(buffer);

	ifunlike(ds->SN==0 || ds->WE==0 || ds->uniXY==0) nret=1;

	if(nret) ds->md_state=METADATA_Wrong;
	else ds->md_state=METADATA_Right;
	return nret;
}
#undef buffer

#undef isVALb
#undef ifVALb
#undef elifVALb
