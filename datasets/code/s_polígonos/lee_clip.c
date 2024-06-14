#define TYPE_FILE 0
#define TYPE_STRING 1

int lee_clip_generic(u8int type, const char8_t *file_or_s, Growing_Polígono_xy *vec, Bufferto8 *log){
	Globals globals;
	int nret;

	globals.blog=log;
	globals.vec=vec;
	globals.file_lev=0;

	Growing_setup(Polígono_xy,*vec,4, goto salida_outofmem);
	Growing_setup(Puntoxy_float,*vec->ppio,10, goto salida_outofmem);
	//next queda apuntando a ppio, pero ya hay un elemento tras él.
	if(type==TYPE_FILE) nret=lee_clip_inline(file_or_s,&globals);
	else nret=lee_clip_scratch(file_or_s,&globals);
	ifunlike(nret!=0){
		if(nret<0 && nret!=AT_NOMEM && log!=NULL) towrite_string(log,u8"El fichero no se ha podido abrir\n");
		goto salida_mala;
	}
	ifunlike((nret=close_polygon(&globals,0))<0) goto salida_mala;
	free(vec->next->ppio); //Si el último polígono se creó y luego se anuló
	return 0;

salida_outofmem:
	nret=AT_NOMEM;
salida_mala:
	if(vec->ppio!=NULL){
		for(Polígono_xy *p=vec->ppio;p!=vec->next;p++) free(p->ppio);
		free(vec->next->ppio); //Si el último polígono se creó y luego se anuló
		free_null(vec->ppio);
	}
	return nret;
}

int lee_clip(const char8_t *file, Growing_Polígono_xy *vec, Bufferto8 *log){
	return lee_clip_generic(TYPE_FILE, file, vec, log);
}

int lee_clip_string(const char8_t *s, Growing_Polígono_xy *vec, Bufferto8 *log){
	return lee_clip_generic(TYPE_STRING, s, vec, log);
}
