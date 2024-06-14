//Lo dejamos como '\n' para que se procese bien el nombre de un fichero tras !. En otra posición lo tratamos directamente
#undef COMMENT_CHAR
#define COMMENT_CHAR '\n'

//indent: Si =0, no escribe nada.
//Si !=0, escribe "\t...\t", con indent '\t'.
#define do_log_indent(blog, indent) dontimes(indent,) toput_char(blog,'\t')
sinline void log_indent(Bufferto8 *blog, uint8m indent){
	if(blog!=NULL) do_log_indent(blog,indent);
}

#define Write_Diagnostic(blog,on,line,Diag,s,...) towritef(blog, u8"(%u) " Diag ": " s "\n", line, __VA_ARGS__);
#define Write_Error(blog,indent,line,s,...) if(blog!=NULL){do_log_indent(blog,indent); Write_Diagnostic(blog,on,line,"Error",s, __VA_ARGS__)}
#define Write_Warning(blog,indent,line,s,...)  if(blog!=NULL){do_log_indent(blog,indent); Write_Diagnostic(blog,on,line,"Warning",s, __VA_ARGS__)}

#define ERROR_F(s,...)		Write_Error(globals->blog,globals->file_lev,GET_LINE,s,__VA_ARGS__)
#define WARNING_F(s,...)	Write_Warning(globals->blog,globals->file_lev,GET_LINE,s,__VA_ARGS__)
#define ERROR_s(s)			Write_Error(globals->blog,globals->file_lev,GET_LINE,"%s",s)
#define WARNING_s(s)		Write_Warning(globals->blog,globals->file_lev,GET_LINE,s,0)

#define LOG_strings(...)	if(globals->blog!=NULL){do_log_indent(globals->blog,globals->file_lev); towritef(globals->blog, u8"(%u) ",GET_LINE);\
																towrite_many_strings(globals->blog,__VA_ARGS__,NULL);}

//Return 0 o 1.
static inline int lee_two_floats(Puntoxy_float *p, Bufferti8_lc *buffer){
	getfast(*buffer,p->φ,vffloat___str8); if(isnot_st(*buffer->pc)) return 1;
	advanceinline(*buffer); if(*buffer->pc=='\n') return 1;
	getfast(*buffer,p->λ,vffloat___str8); if(isnot_stn(*buffer->pc)) return 1;
	return 0;
}

static void initial_tasks(Globals *globals, const char8_t *fconfig, Bufferti8_lc *buffer, char8_t ruta_this[SHRT_PATH]){
	if(globals->blog!=NULL){
		do_log_indent(globals->blog,globals->file_lev);
		towrite_many_strings(globals->blog,u8"(Archivo ",fconfig,u8"\n",NULL);
	}
	Bufferti_lc_setup(*buffer);
	globals->file_lev++;

	strncpy8(ruta_this,fconfig,SHRT_PATH); ruta_this[SHRT_PATH-1]='\0';
	path_remove_file8(ruta_this);
}

#define GET_LINE lc

/* Cierra el polígono y lo añade. Si había <3 vértices se anula y no se añade.

globals->vec->next apunta al polígono que se ha estado completando, y
globals->vec tiene al menos un elemento tras next.

lc: Número de línea por si hay que mostrar un mensaje
Return:
	AT_NOMEM
	1: No había polígono (<3 vértices). globals->vec->next queda apuntando a
		un polígono incializado, e. d., ->ppio!=NULL, pero vacío: ->next=->ppio.
	0: Había un polígono. Se añade un elemento a globals->vec y globals->vec->next
		queda apuntando a ese polígono, que tiene ->ppio=NULL. Además, hay un
		elemento vacío (NULL,NULL,NULL) tras ese polígono.

Avanza vec->next, que queda apuntando a un polígono recién vacío, y añade
un elemento tras él.
*/
static int close_polygon(Globals *globals, uint lc){
	Polígono_xy *pol=globals->vec->next;
	if(pol->next==pol->ppio) return 1;

	{Puntoxy_float p=*pol->ppio, q=pol->next[-1];
	if(q.x!=p.x || q.y!=p.y) Gadd(*pol,Puntoxy_float,p,return AT_NOMEM);}
	ifunlike(pol->next<pol->ppio+4){
		pol->next=pol->ppio; //Anular el polígono.
		WARNING_s("Polígono de menos de tres vértices ignorado\n");
		return 1;
	}

	Polígono_xy p={NULL,NULL,NULL};
	globals->vec->next++;
	Gadd(*globals->vec,Polígono_xy,p,return AT_NOMEM);
	globals->vec->next--;
	globals->vec->next->next=globals->vec->next->ppio; //This should not be needed here. Just in case.
	return 0;
}

#undef GET_LINE
#define GET_LINE buffer.lc

static inline int lee_clip_inline(const char8_t* file, Globals *globals);
/* Return:
	0: Bien
	AT_NOMEM: Error de memoria o de apertura de archivos
	>0: Otro error importante.
*/
static int lee_clip_buffer(Bufferti8_lc buffer, Globals *globals, const char8_t *file){
	int nret;
	char8_t ruta_this[SHRT_PATH];
	Polígono_xy *pol;

	initial_tasks(globals,file,&buffer,ruta_this);
	nret=0;
	pol=globals->vec->next;

skip_blanks:
	advance(buffer); while(*(buffer).pc=='%'){finishline_advance(buffer);}
	goto entry_in_line;
continuar:
	do{finishline_advanceinline(buffer);}while(*buffer.pc=='%');
entry_in_line:
	if(*buffer.pc=='\0') goto salir;
	if(*buffer.pc=='\n'){ //Se acabó un polígono
		if(pol->next==pol->ppio) goto skip_blanks; //Sólo se pasó por líneas vacías
		ifunlike((nret=close_polygon(globals,buffer.lc))<0) goto salida_outofmem;
		nret=0;
		pol=globals->vec->next;
		if(pol->ppio==NULL){ //Si se añadió el polígono recién terminado, ppio será NULL. Si se anuló, no.
			Growing_setup(Puntoxy_float,*globals->vec->next,10,goto salida_outofmem);
		}
		goto skip_blanks;
	}
	if(*buffer.pc=='!'){ //Incluír otro fichero
		char8_t newfile[SHRT_PATH];
		buffer.pc++; advanceinline(buffer);
		if(is_stn(*buffer.pc)){
			ERROR_s("Un '!' debe ir inmediatamente seguido del nombre del archivo, que no pude comenzar por un blanco");
			nret=1;
			goto cerrar_fichero;
		}
		Prepare_line(buffer);
		makepath8(newfile,SHRT_PATH,ruta_this,buffer.pc);
		resume(buffer);

		LOG_strings(u8"> Buscando el archivo de configuración ",newfile,"\n");
		nret=lee_clip_inline(newfile,globals);
		pol=globals->vec->next; //pol may have advanced
		ifunlike(nret){
			ifunlike(nret==AT_NOMEM) goto salida_outofmem;
			const char8_t *s;
			if(nret==ATFILEI_BADPATH || nret==ATFILEI_NOFILE) s=u8"  ... no encontrado";
			else s=u8"  ... no se ha podido abrir";
			ERROR_s(s);
			nret=2;
			goto cerrar_fichero;
		}
		goto continuar;
	}
	if((*buffer.pc>='0' && *buffer.pc<='9') || *buffer.pc=='-' || *buffer.pc=='.' || *buffer.pc=='+'){
		Puntoxy_float p;
		ifnz(lee_two_floats(&p,&buffer)){
			nret=1; ERROR_s("No son dos números. Se detiene la lectura.");
			goto cerrar_fichero;
		}
		Gadd(*pol,Puntoxy_float,p,goto salida_outofmem);
		goto continuar;
	}
	//Ya no queda nada conocido
	WARNING_s("Línea con un formato no reconocido ignorada");
	goto continuar;

salir:
	//El último polígono no se puede cerrar, porque este fichero puede ser solamente una parte de él
	goto cerrar_fichero;

salida_outofmem:
	nret=AT_NOMEM;
cerrar_fichero:
	ticlose(buffer);
	globals->file_lev--;
	if(globals->blog!=NULL){
		do_log_indent(globals->blog,globals->file_lev);
		const char8_t *s;
		if(nret==0) s=u8"fin del archivo ";
		else s=u8"fin de la lectura del archivo ";
		towrite_many_strings(globals->blog,s,file,")\n\n",NULL);
	}
	return nret;
}

/*
globals->file_lev ha de ser 0 en una llamada a esta función desde fuera.
globals->nfile tiene que venir asignado. Es quien llama quien decide el número de fichero
Return:
	0: Bien
	<0: Error de memoria o de apertura del archivo file
	>0: Otro error importante.

Si el valor devuelto es !=0 la lectura se detuvo.
*/
static inline int lee_clip_inline(const char8_t* file, Globals *globals){
	int nret;
	Bufferti8_lc buffer;
	ifunlike((nret=tiopen_utf8((Bufferti8*)&buffer,file))<0) return nret;
	return lee_clip_buffer(buffer,globals,file);
}
static inline int lee_clip_scratch(const char8_t* s, Globals *globals){
	int nret;
	Bufferti8_lc buffer;
	ifunlike((nret=tiopen_mem8((Bufferti8*)&buffer,s,strlen8(s)))<0) return nret;
	return lee_clip_buffer(buffer,globals,u8"Anonimous file");
}
