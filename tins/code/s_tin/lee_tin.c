#if ATBYTES_MACHINE==ATBYTES_BIG_ENDIAN
#define swap_content(block) {durchlaufep(uints,(uint*)(block),uintsizeof(*(block))) *p=u4___littleend_u4(*p);}
#define swap_nuints(block,n) {durchlaufep(uints,(uint*)(block),n) *p=u4___littleend_u4(*p);}
#else
#define swap_content(block)
#define swap_nuints(block,n)
#endif

#if CHAR_BYTE<=2 && ATBYTES_MACHINE==ATBYTES_LITTLE_ENDIAN
#define copia_n16s(p16,pu,n) memcpy(p16,pu,((n)>>1)*usizeof(uint));
#else
sinline void copia_n16s(sint16m *p16, uint *pu, uint n){
	dontimes(n>>1){
		uint u=*pu++;
		*p16++=p10___littleend_u4(u);
		*p16++=p32___littleend_u4(u);
	}
}
#endif

int lee_puntos_s32(uint *buf, const TinCabecera_Data *cabe, TIN *tin, PLIST plist){
	Puntos_s32 * const pun=&tin->puntos.s32;

	//Puntos en retícula
	if(cabe->p_retic_pos!=Я){
		GStdSetup(PuntosRetícula_s32,pun->p_retículas,4);
		uint *pU=buf+cabe->p_retic_pos;
		do{
			uint m, sig;
			uint *pu=pU;
			m=u4___littleend_u4(*pu); pu++; if(m==Я) break;
			Greserve_n(pun->p_retículas,PuntosRetícula_s32,1,goto salida_outofmem);

			PuntosRetícula_s32 *const pr=pun->p_retículas.next;
			pr->m=m;
			pr->n=u4___littleend_u4(*pu); pu++;
			sig=u4___littleend_u4(*pu); pu++;
			pr->bhalf=u4___littleend_u4(*pu)|1; pu++;
			swap_nuints(pu,12);
			pr->P0.X=*pu++;	pr->P0.Y=*pu++;	pr->P0.Z=*pu++;
			pr->Δ1.X=*pu++;	pr->Δ1.Y=*pu++;	pr->Δ1.Z=*pu++;
			pr->Δn.X=*pu++;	pr->Δn.Y=*pu++;	pr->Δn.Z=*pu++;
			pr->Δu.X=*pu++;	pr->Δu.Y=*pu++;	pr->Δu.Z=*pu++;

			if(pr->Δu.X!=0 || pr->Δu.Y!=0 || pr->Δu.Z!=0 && pr->m!=0 && pr->n!=0){
				uint mn=pr->m*pr->n;
				if(pr->bhalf==0){
					aj_malloc_add(pr->desplaz.pfull,ssint,mn);
					memcpy(pr->desplaz.pfull,pu,mn*usizeof(uint));
					swap_nuints(pr->desplaz.pfull,mn);
				}else{
					if(mn&1) mn++;
					aj_malloc_add(pr->desplaz.phalf,sint16m,mn);
					copia_n16s(pr->desplaz.phalf,pu,mn);
				}
			}else{
				if(pr->bhalf==0) pr->desplaz.pfull=NULL;
				else pr->desplaz.phalf=NULL;
			}

			pun->p_retículas.next++;
			if(sig==Я) break;
			pU+=sig;
		}while(1);
	}

	//Puntos en incrementos
	if(cabe->p_inc_pos!=Я){
		GStdSetup(PuntosIncrementos_s32,pun->p_increm,4);
		uint *pU=buf+cabe->p_inc_pos;
		do{
			uint n, sig;
			uint *pu=pU;
			n=u4___littleend_u4(*pu); pu++; if(n==Я) break;
			ifunlike(n==0){ //Bloque eliminado
				sig=u4___littleend_u4(*pu); pu++;
				if(sig==Я) break;
				pU+=sig; continue;
			}
			Greserve_n(pun->p_increm,PuntosIncrementos_s32,1,goto salida_outofmem);

			PuntosIncrementos_s32 *const pr=pun->p_increm.next;
			pr->n=n;
			sig=u4___littleend_u4(*pu); pu++;
			pr->P0.X=u4___littleend_u4(*pu); pu++;
			pr->P0.Y=u4___littleend_u4(*pu); pu++;
			pr->P0.Z=u4___littleend_u4(*pu); pu++;

			if(pr->n<=1){
				pr->increm=NULL;
			}else{
				uint mn=3*(pr->n-1);
				if(mn&1) mn++;
				aj_malloc_add(pr->increm,sint16m,mn);
				copia_n16s(pr->increm,pu,mn);
			}

			pun->p_increm.next++;
			if(sig==Я) break;
			pU+=sig;
		}while(1);
	}

	//Puntos individuales
	if(cabe->p_indiv_pos!=Я && cabe->p_indiv_n!=0){
		uint mn=3*cabe->p_indiv_n;
		aj_malloc_add(pun->p_indiv,ssint,mn);
		memcpy(pun->p_indiv,buf+cabe->p_indiv_pos,mn*usizeof(uint));
		swap_nuints(pun->p_indiv,mn);
	}

	return 0;
salida_outofmem:
	return AT_NOMEM;
}

#if !ATCRT_FLOAT
int lee_puntos_fl(uint *buf, const TinCabecera_Data *cabe, TIN *tin, PLIST plist){
	Puntos_fl * const pun=&tin->puntos.fl;

	//Puntos en retícula
	if(cabe->p_retic_pos!=Я){
		GStdSetup(PuntosRetícula_fl,pun->p_retículas,4);
		uint *pU=buf+cabe->p_retic_pos;
		do{
			uint m, sig;
			uint *pu=pU;
			m=u4___littleend_u4(*pu); pu++; if(m==Я) break;
			Greserve_n(pun->p_retículas,PuntosRetícula_fl,1,goto salida_outofmem);

			PuntosRetícula_fl *const pr=pun->p_retículas.next;
			pr->m=m;
			pr->n=u4___littleend_u4(*pu); pu++;
			sig=u4___littleend_u4(*pu); pu++;
			pr->bhalf=u4___littleend_u4(*pu)|1; pu++;
			swap_nuints(pu,12);
			float___IO_SINGLE(&pr->P0.X,*pu); pu++;		float___IO_SINGLE(&pr->P0.Y,*pu); pu++;		float___IO_SINGLE(&pr->P0.Z,*pu); pu++;
			float___IO_SINGLE(&pr->Δ1.X,*pu); pu++;		float___IO_SINGLE(&pr->Δ1.Y,*pu); pu++;		float___IO_SINGLE(&pr->Δ1.Z,*pu); pu++;
			float___IO_SINGLE(&pr->Δn.X,*pu); pu++;	float___IO_SINGLE(&pr->Δn.Y,*pu); pu++;	float___IO_SINGLE(&pr->Δn.Z,*pu); pu++;
			float___IO_SINGLE(&pr->Δu.X,*pu); pu++;	float___IO_SINGLE(&pr->Δu.Y,*pu); pu++;		float___IO_SINGLE(&pr->Δu.Z,*pu); pu++;

			if(pr->Δu.X!=0 || pr->Δu.Y!=0 || pr->Δu.Z!=0 && pr->m!=0 && pr->n!=0){
				uint mn=pr->m*pr->n;
				if(pr->bhalf==0){
					aj_malloc_add(pr->desplaz.pfull,float,mn);
					swap_nuints(pu,mn);
					float_memcpy_IO_SINGLE(pr->desplaz.pfull,pu,mn);
				}else{
					if(mn&1) mn++;
					aj_malloc_add(pr->desplaz.phalf,FloatHalf,mn);
					copia_n16s(pr->desplaz.phalf,pu,mn);
				}
			}else{
				if(pr->bhalf==0) pr->desplaz.pfull=NULL;
				else pr->desplaz.phalf=NULL;
			}

			pun->p_retículas.next++;
			if(sig==Я) break;
			pU+=sig;
		}while(1);
	}

	//Puntos en incrementos
	if(cabe->p_inc_pos!=Я){
		GStdSetup(PuntosIncrementos_fl,pun->p_increm,4);
		uint *pU=buf+cabe->p_inc_pos;
		do{
			uint n, sig;
			uint *pu=pU;
			n=u4___littleend_u4(*pu); pu++; if(n==Я) break;
			ifunlike(n==0){ //Bloque eliminado
				sig=u4___littleend_u4(*pu); pu++;
				if(sig==Я) break;
				pU+=sig; continue;
			}
			Greserve_n(pun->p_increm,PuntosIncrementos_fl,1,goto salida_outofmem);

			PuntosIncrementos_fl *const pr=pun->p_increm.next;
			pr->n=n;
			sig=u4___littleend_u4(*pu); pu++;
			swap_nuints(pu,3);
			float___IO_SINGLE(&pr->P0.X,*pu); pu++;
			float___IO_SINGLE(&pr->P0.Y,*pu); pu++;
			float___IO_SINGLE(&pr->P0.Z,*pu); pu++;

			if(pr->n<=1){
				pr->increm=NULL;
			}else{
				uint mn=3*(pr->n-1);
				if(mn&1) mn++;
				aj_malloc_add(pr->increm,FloatHalf,mn);
				copia_n16s(pr->increm,pu,mn);
			}

			pun->p_increm.next++;
			if(sig==Я) break;
			pU+=sig;
		}while(1);
	}

	//Puntos individuales
	if(cabe->p_indiv_pos!=Я && cabe->p_indiv_n!=0){
		uint mn=3*cabe->p_indiv_n;
		aj_malloc_add(pun->p_indiv,float,mn);
		swap_nuints(buf+cabe->p_indiv_pos,mn);
		float_memcpy_IO_SINGLE(pun->p_indiv,buf+cabe->p_indiv_pos,mn);
	}

	return 0;
salida_outofmem:
	return AT_NOMEM;
}
#endif

int lee_puntos_u16(uint *buf, const TinCabecera_Data *cabe, TIN *tin, PLIST plist){
	Puntos_u16 * const pun=&tin->puntos.u16;

	//Puntos en retícula
	if(cabe->p_retic_pos!=Я){
		GStdSetup(PuntosRetícula_u16,pun->p_retículas,4);
		uint *pU=buf+cabe->p_retic_pos;
		do{
			uint m, sig;
			uint *pu=pU;
			m=u4___littleend_u4(*pu); pu++; if(m==Я) break;
			Greserve_n(pun->p_retículas,PuntosRetícula_u16,1,goto salida_outofmem);

			PuntosRetícula_u16 *const pr=pun->p_retículas.next;
			pr->m=m;
			pr->n=u4___littleend_u4(*pu); pu++;
			sig=u4___littleend_u4(*pu); pu++;
			pr->bhalf=u4___littleend_u4(*pu)|1; pu++;
			pr->P0.X=p10___littleend_u4(*pu); pr->P0.Y=p32___littleend_u4(*pu); pu++;
			pr->P0.Z=p10___littleend_u4(*pu); pr->Δ1.X=p32___littleend_u4(*pu); pu++;
			pr->Δ1.Y=p10___littleend_u4(*pu); pr->Δ1.Z=p32___littleend_u4(*pu); pu++;
			pr->Δn.X=p10___littleend_u4(*pu); pr->Δn.Y=p32___littleend_u4(*pu); pu++;
			pr->Δn.Z=p10___littleend_u4(*pu); pr->Δu.X=p32___littleend_u4(*pu); pu++;
			pr->Δu.Y=p10___littleend_u4(*pu); pr->Δu.Z=p32___littleend_u4(*pu); pu++;
			if(pr->Δu.X!=0 || pr->Δu.Y!=0 || pr->Δu.Z!=0 && pr->m!=0 && pr->n!=0){
				uint mn=pr->m*pr->n;
				if(mn&1) mn++;
				aj_malloc_add(pr->desplaz.phalf,uint16m,mn);
				copia_n16s(pr->desplaz.phalf,pu,mn);
			}else{
				pr->desplaz.phalf=NULL;
			}

			pun->p_retículas.next++;
			if(sig==Я) break;
			pU+=sig;
		}while(1);
	}

	//Puntos en incrementos
	if(cabe->p_inc_pos!=Я){
		GStdSetup(PuntosIncrementos_u16,pun->p_increm,4);
		uint *pU=buf+cabe->p_inc_pos;
		do{
			uint n, sig;
			uint *pu=pU;
			n=u4___littleend_u4(*pu); pu++; if(n==Я) break;
			ifunlike(n==0){ //Bloque eliminado
				sig=u4___littleend_u4(*pu); pu++;
				if(sig==Я) break;
				pU+=sig; continue;
			}
			Greserve_n(pun->p_increm,PuntosIncrementos_u16,1,goto salida_outofmem);

			PuntosIncrementos_u16 *const pr=pun->p_increm.next;
			pr->n=n;
			sig=u4___littleend_u4(*pu); pu++;
			uint16_t *ps=(uint16_t*)pu;
			pr->P0.X=u2___littleend_u2(*ps); ps++;
			pr->P0.Y=u2___littleend_u2(*ps); ps++;
			pr->P0.Z=u2___littleend_u2(*ps); ps++;

			if(pr->n<=1){
				pr->increm=NULL;
			}else{
				uint mn=3*(pr->n-1);
				aj_malloc_add(pr->increm,sint16m,mn+(mn&1));
			#if ATBYTES_MACHINE==ATBYTES_LITTLE_ENDIAN
				memcpy(pr->increm,ps,mn*usizeof(uint16_t));
			#else
				uint16_t p16=pr->increm;
				dontimes(mn){*p16++=u2___littleend_u2(*ps); ps++;}
			#endif
			}

			pun->p_increm.next++;
			if(sig==Я) break;
			pU+=sig;
		}while(1);
	}

	//Puntos individuales
	if(cabe->p_indiv_pos!=Я && cabe->p_indiv_n!=0){
		uint mn=3*cabe->p_indiv_n;
		if(mn&1) mn++;
		aj_malloc_add(pun->p_indiv,uint16m,mn);
		copia_n16s(pun->p_indiv,buf+cabe->p_indiv_pos,mn);
	}

	return 0;
salida_outofmem:
	return AT_NOMEM;
}

int lee_puntos_dbl(uint *buf, const TinCabecera_Data *cabe, TIN *tin, PLIST plist){
	Puntos_dbl * const pun=&tin->puntos.dbl;

	//Puntos en retícula
	if(cabe->p_retic_pos!=Я){
		GStdSetup(PuntosRetícula_dbl,pun->p_retículas,4);
		uint *pU=buf+cabe->p_retic_pos;
		do{
			uint m, sig;
			uint *pu=pU;
			m=u4___littleend_u4(*pu); pu++; if(m==Я) break;
			Greserve_n(pun->p_retículas,PuntosRetícula_dbl,1,goto salida_outofmem);

			PuntosRetícula_dbl *const pr=pun->p_retículas.next;
			pr->m=m;
			pr->n=u4___littleend_u4(*pu); pu++;
			sig=u4___littleend_u4(*pu); pu++;
			pr->bhalf=u4___littleend_u4(*pu)|1; pu++;
			swap_nuints(pu,24);
			double___IO_DOUBLE(&pr->P0.X,*pu); pu+=2;		double___IO_DOUBLE(&pr->P0.Y,*pu); pu+=2;		double___IO_DOUBLE(&pr->P0.Z,*pu); pu+=2;
			double___IO_DOUBLE(&pr->Δ1.X,*pu); pu+=2;		double___IO_DOUBLE(&pr->Δ1.Y,*pu); pu+=2;		double___IO_DOUBLE(&pr->Δ1.Z,*pu); pu+=2;
			double___IO_DOUBLE(&pr->Δn.X,*pu); pu+=2;		double___IO_DOUBLE(&pr->Δn.Y,*pu); pu+=2;		double___IO_DOUBLE(&pr->Δn.Z,*pu); pu+=2;
			double___IO_DOUBLE(&pr->Δu.X,*pu); pu+=2;		double___IO_DOUBLE(&pr->Δu.Y,*pu); pu+=2;		double___IO_DOUBLE(&pr->Δu.Z,*pu); pu+=2;

			if(pr->Δu.X!=0 || pr->Δu.Y!=0 || pr->Δu.Z!=0 && pr->m!=0 && pr->n!=0){
				uint mn=pr->m*pr->n;
				if(pr->bhalf==0){
					aj_malloc_add(pr->desplaz.pfull,double,mn);
					swap_nuints(pu,mn<<1);
					double_memcpy_IO_DOUBLE(pr->desplaz.pfull,pu,mn);
				}else{
					aj_malloc_add(pr->desplaz.phalf,float,mn);
					swap_nuints(pu,mn);
					float_memcpy_IO_SINGLE(pr->desplaz.phalf,pu,mn);
				}
			}else{
				if(pr->bhalf==0) pr->desplaz.pfull=NULL;
				else pr->desplaz.phalf=NULL;
			}

			pun->p_retículas.next++;
			if(sig==Я) break;
			pU+=sig;
		}while(1);
	}

	//Puntos en incrementos
	if(cabe->p_inc_pos!=Я){
		GStdSetup(PuntosIncrementos_dbl,pun->p_increm,4);
		uint *pU=buf+cabe->p_inc_pos;
		do{
			uint n, sig;
			uint *pu=pU;
			n=u4___littleend_u4(*pu); pu++; if(n==Я) break;
			ifunlike(n==0){ //Bloque eliminado
				sig=u4___littleend_u4(*pu); pu++;
				if(sig==Я) break;
				pU+=sig; continue;
			}
			Greserve_n(pun->p_increm,PuntosIncrementos_dbl,1,goto salida_outofmem);

			PuntosIncrementos_dbl *const pr=pun->p_increm.next;
			pr->n=n;
			sig=u4___littleend_u4(*pu); pu++;
			swap_nuints(pu,6);
			double___IO_DOUBLE(&pr->P0.X,*pu); pu+=2;
			double___IO_DOUBLE(&pr->P0.Y,*pu); pu+=2;
			double___IO_DOUBLE(&pr->P0.Z,*pu); pu+=2;

			if(pr->n<=1){
				pr->increm=NULL;
			}else{
				uint mn=3*(pr->n-1);
				aj_malloc_add(pr->increm,float,mn);
				float_memcpy_IO_SINGLE(pr->increm,pu,mn);
			}

			pun->p_increm.next++;
			if(sig==Я) break;
			pU+=sig;
		}while(1);
	}

	//Puntos individuales
	if(cabe->p_indiv_pos!=Я && cabe->p_indiv_n!=0){
		uint mn=3*cabe->p_indiv_n;
		aj_malloc_add(pun->p_indiv,double,mn);
		swap_nuints(buf+cabe->p_indiv_pos,mn<<1);
		double_memcpy_IO_DOUBLE(pun->p_indiv,buf+cabe->p_indiv_pos,mn);
	}

	return 0;
salida_outofmem:
	return AT_NOMEM;
}

int lee_fichero_tin(const char8_t *ftin, TIN *tin){
	int nret;
	uint *buf;
	TinCabecera_Data *cabe;
	PLIST plist;

	ifunlike((uint)(nret=biopen_utf8(&buf,ftin))>ATFILEI_MAXSIZE) return nret;
	{uint u=buf[0];
	if(p0___uint(u)!='T' || p1___uint(u)!='I' || p2___uint(u)!='N'){free(buf); return FILEREAD_BADFORMAT;}}

	nret=0;
	*tin=(TIN){0};
	cabe=(TinCabecera_Data*)buf;

	tin->tdato=p0___uint(buf[1]);
	if(tin->tdato!='s' && tin->tdato!='S' && tin->tdato!='i' && tin->tdato!='h' && tin->tdato!='f' && tin->tdato!='d'){
		free(buf); return FILEREAD_BADFORMAT;
	}
	tin->uni.tipo=p1___uint(buf[1]);

	swap_content(cabe);
	{uint u=cabe->unidades;
	if(tin->uni.tipo==UniTin_m_float) float___IO_SINGLE(&tin->uni.valor.f,u);
	else tin->uni.valor.u=u;}

	plist=get_new_plist();
	ifunlike(plist==PLIST_NULL || plist==PLIST_OUT_OF_MEM){
		free(buf); return AT_NOMEM;
	}

	//Rellenar ns
	tin->ns.p_retícula=cabe->p_retic_n;
	tin->ns.p_increm=cabe->p_inc_n;
	tin->ns.p_indiv=cabe->p_indiv_n;
	tin->ns.t_malla=cabe->t_malla_n;
	tin->ns.t_increm=cabe->t_inc_n;
	tin->ns.t_indiv=cabe->t_indiv_n;

	switch(tin->tdato){
#if ATCRT_FLOAT
	case 'f': case 'i': nret=lee_puntos_s32(buf,cabe,tin,plist); break;
#else
	case 'i': nret=lee_puntos_s32(buf,cabe,tin,plist); break;
	case 'f': nret=lee_puntos_fl(buf,cabe,tin,plist); break;
#endif
	case 'h': case 's': case 'S': nret=lee_puntos_u16(buf,cabe,tin,plist); break;
	case 'd': nret=lee_puntos_dbl(buf,cabe,tin,plist); break;
	}
	ifunlike(nret==AT_NOMEM) goto salida_outofmem;

	//Triángulos en progresión aritmética
	if(cabe->t_malla_pos!=Я){
		GStdSetup(TriángulosMalla,tin->t_malla,4);
		uint *pU=buf+cabe->t_malla_pos;
		do{
			uint m;
			uint *pu=pU;
			m=u4___littleend_u4(*pu); pu++; if(m==Я) break;
			Greserve_n(tin->t_malla,TriángulosMalla,1,goto salida_outofmem);

			TriángulosMalla *const pr=tin->t_malla.next;
			pr->m=m;
			pr->n=u4___littleend_u4(*pu); pu++;
			swap_nuints(pu,9);
			pr->a0=*pu++;		pr->b0=*pu++;		pr->c0=*pu++;
			pr->Δa1=*pu++;	pr->Δb1=*pu++;	pr->Δc1=*pu++;
			pr->Δan=*pu++;	pr->Δbn=*pu++;	pr->Δcn=*pu++;

			tin->t_malla.next++;
			pU=pu;
		}while(1);
	}

	//Triángulos en incrementos
	if(cabe->t_inc_pos!=Я){
		GStdSetup(TriángulosIncrementos,tin->t_increm,4);
		uint *pU=buf+cabe->t_inc_pos;
		do{
			uint n, sig;
			uint *pu=pU;
			n=u4___littleend_u4(*pu); pu++; if(n==Я) break;
			ifunlike(n==0){ //Bloque eliminado
				sig=u4___littleend_u4(*pu); pu++;
				if(sig==Я) break;
				pU+=sig; continue;
			}
			Greserve_n(tin->t_increm,TriángulosIncrementos,1,goto salida_outofmem);

			TriángulosIncrementos *const pr=tin->t_increm.next;
			pr->n=n;
			sig=u4___littleend_u4(*pu); pu++;
			pr->a0=u4___littleend_u4(*pu); pu++;
			uint mn=3*pr->n-1;
			if(mn&1) mn++;
			aj_malloc_add(pr->increm,sint16m,mn);
			copia_n16s(pr->increm,pu,mn);

			tin->t_increm.next++;
			if(sig==Я) break;
			pU+=sig;
		}while(1);
	}

	//Triángulos individuales
	if(cabe->t_indiv_pos!=Я && cabe->t_indiv_n!=0){
		uint mn=3*cabe->t_indiv_n;
		aj_malloc_add(tin->t_indiv,uint,mn);
		memcpy(tin->t_indiv,buf+cabe->t_indiv_pos,mn*usizeof(uint));
		swap_nuints(tin->t_indiv,mn);
	}

	//Estilos de los triángulos
	if(cabe->estilos_t_pos!=Я){
		uint *pu=buf+cabe->estilos_t_pos;
		tin->estilos_t.tipo=p0___uint(*pu); pu++;
		tin->estilos_t.nestilos=u4___littleend_u4(*pu); pu++;
		ifunlike(tin->estilos_t.tipo==0 || tin->estilos_t.tipo>3 || tin->estilos_t.nestilos==0){
			tin->estilos_t.nestilos=0;
			tin->estilos_t.p.pest1=NULL;
		}else{
			uint mn=tin->estilos_t.nestilos;
			switch(tin->estilos_t.tipo){
			//case 1: case 2; mn*=1; break;
			case 3: mn*=2;
			}
			uint *pes_u;
			aj_malloc_add(pes_u,uint,mn);
			memcpy(pes_u,pu,mn*usizeof(uint));
			swap_nuints(pes_u,mn);
			tin->estilos_t.p.pest1=(EstiloTriángulos1*)pes_u;
		}
	}

	//Clase a la que pertenece cada triángulo
	if(cabe->clases_t_pos!=Я){
		uint *pu=buf+cabe->clases_t_pos;
		uint bpc=u4___littleend_u4(*pu); pu++;
		switch(bpc){
		case 0: tin->clases_t.nclass=1; break;
		case 1: tin->clases_t.nclass=2; break;
		case 2: tin->clases_t.nclass=4; break;
		case 4: tin->clases_t.nclass=16; break;
		case 8: tin->clases_t.nclass=256; break;
		case 16: tin->clases_t.nclass=0xFFff; break;
		default: tin->clases_t.nclass=1;
		}
		uint mn=tin->ns.t_malla+tin->ns.t_increm+tin->ns.t_indiv;
		if(mn&1) mn++;

		if(tin->clases_t.nclass==1){
			tin->clases_t.p.p8=NULL;
		}elif(tin->clases_t.nclass<=256){
			if(mn&2) mn+=2;

			if(tin->clases_t.nclass==256 && CHAR_BYTE==1 && ATBYTES_MACHINE==ATBYTES_LITTLE_ENDIAN){
				aj_malloc_add(tin->clases_t.p.p8,uint8m,mn);
				memcpy(tin->clases_t.p.p8,pu,mn);
			}else{
				uint8m *p8;
				switch(bpc){
				case 1:
					mn=divup(mn,32); //Cada uint guarda 32 datos
					aj_malloc_add(tin->clases_t.p.p8,uint8m,mn<<5);
					p8=tin->clases_t.p.p8;
					dontimes(mn,){
						uint u=*pu++;
						u8int u8;

						u8=p0___uint(u);
						*p8++=u8&1;	*p8++=u8&2;	*p8++=u8&4;	*p8++=u8&8;
						*p8++=u8&16;	*p8++=u8&32;	*p8++=u8&64;	*p8++=u8&128;
						u8=p1___uint(u);
						*p8++=u8&1;	*p8++=u8&2;	*p8++=u8&4;	*p8++=u8&8;
						*p8++=u8&16;	*p8++=u8&32;	*p8++=u8&64;	*p8++=u8&128;
						u8=p2___uint(u);
						*p8++=u8&1;	*p8++=u8&2;	*p8++=u8&4;	*p8++=u8&8;
						*p8++=u8&16;	*p8++=u8&32;	*p8++=u8&64;	*p8++=u8&128;
						u8=p3___uint(u);
						*p8++=u8&1;	*p8++=u8&2;	*p8++=u8&4;	*p8++=u8&8;
						*p8++=u8&16;	*p8++=u8&32;	*p8++=u8&64;	*p8++=u8&128;
					}
					break;
				case 2:
					mn=divup(mn,16); //Cada uint guarda 16 datos
					aj_malloc_add(tin->clases_t.p.p8,uint8m,mn<<4);
					p8=tin->clases_t.p.p8;
					dontimes(mn,){
						uint u=*pu++;
						u8int u8;

						u8=p0___uint(u);
						*p8++=u8&3;	*p8++=u8&12;	*p8++=u8&0x30;	*p8++=u8&0xC0;
						u8=p1___uint(u);
						*p8++=u8&3;	*p8++=u8&12;	*p8++=u8&0x30;	*p8++=u8&0xC0;
						u8=p2___uint(u);
						*p8++=u8&3;	*p8++=u8&12;	*p8++=u8&0x30;	*p8++=u8&0xC0;
						u8=p3___uint(u);
						*p8++=u8&3;	*p8++=u8&12;	*p8++=u8&0x30;	*p8++=u8&0xC0;
					}
					break;
				case 4:
					mn=divup(mn,8); //Cada uint guarda 8 datos
					aj_malloc_add(tin->clases_t.p.p8,uint8m,mn<<3);
					p8=tin->clases_t.p.p8;
					dontimes(mn,){
						uint u=*pu++;
						u8int u8;

						u8=p0___uint(u);	*p8++=u8&0xF; *p8++=u8&0xF0;
						u8=p1___uint(u);	*p8++=u8&0xF; *p8++=u8&0xF0;
						u8=p2___uint(u);	*p8++=u8&0xF; *p8++=u8&0xF0;
						u8=p3___uint(u);	*p8++=u8&0xF; *p8++=u8&0xF0;
					}
					break;
				case 8:
					mn=divup(mn,4); //Cada uint guarda 4 datos. (mn ya era múltiplo de 4)
					aj_malloc_add(tin->clases_t.p.p8,uint8m,mn<<2);
					p8=tin->clases_t.p.p8;
					dontimes(mn,){
						uint u=*pu++;
						*p8++=p0___uint(u);
						*p8++=p1___uint(u);
						*p8++=p2___uint(u);
						*p8++=p3___uint(u);
					}
					break;
				}
			}
		}else{
			aj_malloc_add(tin->clases_t.p.p16,uint16m,mn);
			copia_n16s(tin->clases_t.p.p16,pu,mn);
		}
	}else{
		tin->clases_t.nclass=0;
		tin->clases_t.p.p8=NULL;
	}

salida:
	free(buf);
	release_plist(plist);
	return nret;

salida_outofmem:
	free_plist(plist);
	nret=AT_NOMEM;
	goto salida;

//salida_badformat:
}

void free_TIN(TIN *tin){
	switch(tin->tdato){
	case 'i': {
		Puntos_s32 *ps=&tin->puntos.s32;
		if(ps->p_retículas.ppio!=NULL){
			for(PuntosRetícula_s32 *p=ps->p_retículas.ppio; p!=ps->p_retículas.next; p++){
				if(p->bhalf) free(p->desplaz.phalf);
				else free(p->desplaz.pfull);
			}
			free_null(ps->p_retículas.ppio);
		}
		if(ps->p_increm.ppio!=NULL){
			for(PuntosIncrementos_s32 *p=ps->p_increm.ppio; p!=ps->p_increm.next; p++) free(p->increm);
			free_null(ps->p_increm.ppio);
		}
		free_null(ps->p_indiv);
	} break;
	case 's': case 'S': {
	Puntos_u16 *ps=&tin->puntos.u16;
		if(ps->p_retículas.ppio!=NULL){
			for(PuntosRetícula_u16 *p=ps->p_retículas.ppio; p!=ps->p_retículas.next; p++){
				if(p->bhalf) free(p->desplaz.phalf);
				else free(p->desplaz.pfull);
			}
			free_null(ps->p_retículas.ppio);
		}
		if(ps->p_increm.ppio!=NULL){
			for(PuntosIncrementos_u16 *p=ps->p_increm.ppio; p!=ps->p_increm.next; p++) free(p->increm);
			free_null(ps->p_increm.ppio);
		}
		free_null(ps->p_indiv);
	} break;
	case 'f': {
		Puntos_fl *ps=&tin->puntos.fl;
		if(ps->p_retículas.ppio!=NULL){
			for(PuntosRetícula_float *p=ps->p_retículas.ppio; p!=ps->p_retículas.next; p++){
				if(p->bhalf) free(p->desplaz.phalf);
				else free(p->desplaz.pfull);
			}
			free_null(ps->p_retículas.ppio);
		}
		if(ps->p_increm.ppio!=NULL){
			for(PuntosIncrementos_float *p=ps->p_increm.ppio; p!=ps->p_increm.next; p++) free(p->increm);
			free_null(ps->p_increm.ppio);
		}
		free_null(ps->p_indiv);
	} break;
	case 'd': {
		Puntos_dbl *ps=&tin->puntos.dbl;
		if(ps->p_retículas.ppio!=NULL){
			for(PuntosRetícula_dbl *p=ps->p_retículas.ppio; p!=ps->p_retículas.next; p++){
				if(p->bhalf) free(p->desplaz.phalf);
				else free(p->desplaz.pfull);
			}
			free_null(ps->p_retículas.ppio);
		}
		if(ps->p_increm.ppio!=NULL){
			for(PuntosIncrementos_dbl *p=ps->p_increm.ppio; p!=ps->p_increm.next; p++) free(p->increm);
			free_null(ps->p_increm.ppio);
		}
		free_null(ps->p_indiv);
	} break;
	case 'h': {
		Puntos_half *ps=&tin->puntos.half;
		if(ps->p_retículas.ppio!=NULL){
			for(PuntosRetícula_half *p=ps->p_retículas.ppio; p!=ps->p_retículas.next; p++){
				if(p->bhalf) free(p->desplaz.phalf);
				else free(p->desplaz.pfull);
			}
			free_null(ps->p_retículas.ppio);
		}
		if(ps->p_increm.ppio!=NULL){
			for(PuntosIncrementos_half *p=ps->p_increm.ppio; p!=ps->p_increm.next; p++) free(p->increm);
			free_null(ps->p_increm.ppio);
		}
		free_null(ps->p_indiv);
	} break;
	}

	free_null(tin->t_malla.ppio);
	if(tin->t_increm.ppio!=NULL){
		for(TriángulosIncrementos *p=tin->t_increm.ppio; p!=tin->t_increm.next; p++) free(p->increm);
		free_null(tin->t_increm.ppio);
	}
	free_null(tin->t_indiv);

	switch(tin->estilos_t.tipo){
	case 0: free_null(tin->estilos_t.p.pest1); break;
	case 1: free_null(tin->estilos_t.p.pest2); break;
	case 2: free_null(tin->estilos_t.p.pest3); break;
	}

	if(tin->clases_t.nclass<=256) free_null(tin->clases_t.p.p8);
	else free_null(tin->clases_t.p.p16);

	zeroset_uint(&tin->ns,uintsizeof(tin->ns));
}

void free_TINplano(TINPlano* tin){
	if(TipoDatoTIN_is_integer(tin->tdato)) free(tin->puntos.in);
	else free(tin->puntos.dbl);
	free(tin->triángulos);
	free(tin->estilos_t);
}
