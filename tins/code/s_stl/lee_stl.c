#define P_op(P, EQ, Q,OP,R) (P).X EQ ((Q).X OP (R).X),   (P).Y EQ ((Q).Y OP (R).Y),   (P).Z EQ ((Q).Z OP (R).Z)

int lee_fichero_stl(const char8_t *ftin, TINPlano *tin){
	int nret;
	uint *buf;
	uint *ptr;
	Hash_hPuntoXYZ_floatuint puntos;

	ifunlike((uint)(nret=biopen_utf8(&buf,ftin))>ATFILEI_MAXSIZE) return nret;

	*tin=(TINPlano){0};
	ptr=buf+STL_CABECERA_UINTSIZEOF;
	nret-=STL_CABECERA_UINTSIZEOF+1;
	tin->nt=*ptr++;

	if((tin->nt>0xFFffFFffU/24) || ((uint)nret<<1 < 25*tin->nt)){free(buf); return FILEREAD_BADFORMAT;}

	tin->triángulos=NULL;
	tin->puntos.dbl=NULL;
	tin->uni.tipo=UniTin_MM; tin->uni.valor.u=1;
	tin->tdato=TIN_float;
	Hkey_setup(hPuntoXYZ_floatuint,puntos,(tin->nt>>1)+(tin->nt>>2),(Punto_EMPTY),goto salida_outofmem);

	aj_malloc_n(tin->triángulos,uint,3*tin->nt);

	uint np=0; //Número del siguiente punto a añadir
	uint *tri=tin->triángulos;
	dontimes(tin->nt,){
		ptr+=3; //vector
		PuntoXYZ_float P;
		hPuntoXYZ_floatuint *pP;

		float___IO_SINGLE(&P.X,*ptr);	ptr++;
		float___IO_SINGLE(&P.Y,*ptr);	ptr++;
		float___IO_SINGLE(&P.Z,*ptr);	ptr++;
		pP=geth_hPuntoXYZ_floatuint(&puntos,P);
		if(pP!=NULL) *tri=pP->data;
		else{addh_keydata(hPuntoXYZ_floatuint,&puntos,P,np,goto salida_outofmem); *tri=np++;}
		tri++;
		float___IO_SINGLE(&P.X,*ptr);	ptr++;
		float___IO_SINGLE(&P.Y,*ptr);	ptr++;
		float___IO_SINGLE(&P.Z,*ptr);	ptr++;
		pP=geth_hPuntoXYZ_floatuint(&puntos,P);
		if(pP!=NULL) *tri=pP->data;
		else{addh_keydata(hPuntoXYZ_floatuint,&puntos,P,np,goto salida_outofmem); *tri=np++;}
		tri++;
		float___IO_SINGLE(&P.X,*ptr);	ptr++;
		float___IO_SINGLE(&P.Y,*ptr);	ptr++;
		float___IO_SINGLE(&P.Z,*ptr);	ptr++;
		pP=geth_hPuntoXYZ_floatuint(&puntos,P);
		if(pP!=NULL) *tri=pP->data;
		else{addh_keydata(hPuntoXYZ_floatuint,&puntos,P,np,goto salida_outofmem); *tri=np++;}
		tri++;

		ptr=(uint*)((char*)ptr+usizeof(uint_least16_t)); //2 bytes &#$! al final
	}

	tin->np=np;
	aj_malloc_n(tin->puntos.dbl,double,3*tin->np);
	durchHashp(hPuntoXYZ_floatuint,puntos){
		if(PuntoXYZ_float_is_empty(p->key)) continue;
		uint n=3*p->data;
		tin->puntos.dbl[n++]=p->key.X;
		tin->puntos.dbl[n++]=p->key.Y;
		tin->puntos.dbl[n]=p->key.Z;
	}

	tin->estilos_t=NULL;

	free(buf);
	return 0;

salida_outofmem:
	free(tin->triángulos);
	free(puntos.ppio);
	free(buf);
	return AT_NOMEM;
}
