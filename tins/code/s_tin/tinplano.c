int tinplano___tin(TINPlano *plano, const TIN *tin){
	plano->np=tin->ns.p_retícula+tin->ns.p_increm+tin->ns.p_indiv;
	plano->nt=tin->ns.t_malla+tin->ns.t_increm+tin->ns.t_indiv;
	plano->tdato=tin->tdato;
	plano->uni=tin->uni;

	aj_malloc_return(plano->triángulos,uint,3*plano->nt);
	if(tin->estilos_t.nestilos==0) plano->estilos_t=NULL;
	else{
		checked_malloc_n(plano->estilos_t,uint,plano->nt, free(plano->triángulos); return AT_NOMEM);
		if(tin->clases_t.nclass<=256){
			if(tin->estilos_t.tipo<2){durchlaufe2(uint,plano->estilos_t,plano->nt,uint8m,tin->clases_t.p.p8) *ptr=tin->estilos_t.p.pest1[*ptr_b].color;}
			else{durchlaufe2(uint,plano->estilos_t,plano->nt,uint8m,tin->clases_t.p.p8) *ptr=tin->estilos_t.p.pest3[*ptr_b].color;}
		}else{
			if(tin->estilos_t.tipo<2){durchlaufe2(uint,plano->estilos_t,plano->nt,uint16m,tin->clases_t.p.p16) *ptr=tin->estilos_t.p.pest1[*ptr_b].color;}
			else{durchlaufe2(uint,plano->estilos_t,plano->nt,uint16m,tin->clases_t.p.p16) *ptr=tin->estilos_t.p.pest3[*ptr_b].color;}
		}
	}

	if(TipoDatoTIN_is_integer(tin->tdato)){
		aj_malloc_n(plano->puntos.in,ssint,3*plano->np);
		ssint *pla=plano->puntos.in;
		switch(tin->tdato){
		case TIN_sint32:{
			if(tin->puntos.s32.p_retículas.ppio!=NULL){
			for(const PuntosRetícula_s32 *p=tin->puntos.s32.p_retículas.ppio; p!=tin->puntos.s32.p_retículas.next; p++){
				PuntoXYZ_ssint p1=p->P0;
				if(p->Δu.X==0 && p->Δu.Y==0 && p->Δu.Z==0){
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_ssint pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))) *pla++=pp.X, *pla++=pp.Y, *pla++=pp.Z;
					}
				}else{
				   const PuntoXYZ_ssint *Δ=&p->Δu;
				   if(p->bhalf){
					sint16m *pde=p->desplaz.phalf;
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_ssint pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))){
							ssint l=*pde++;
							*pla++=pp.X+l*Δ->X;
							*pla++=pp.Y+l*Δ->Y;
							*pla++=pp.Z+l*Δ->Z;
						}
					}
				   }else{
					ssint *pde=p->desplaz.pfull;
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_ssint pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))){
							ssint l=*pde++;
							*pla++=pp.X+l*Δ->X;
							*pla++=pp.Y+l*Δ->Y;
							*pla++=pp.Z+l*Δ->Z;
						}
					}
				   }
				}
			}}
			//
			pla=plano->puntos.in+3*tin->ns.p_retícula;
			if(tin->puntos.s32.p_increm.ppio!=NULL){
			for(const PuntosIncrementos_s32 *p=tin->puntos.s32.p_increm.ppio; p!=tin->puntos.s32.p_increm.next; p++){
				if(p->n==0) continue;
				sint16m *pd=p->increm;
				*pla++=p->P0.X;
				*pla++=p->P0.Y;
				*pla=p->P0.Z;
				dontimes(3*(p->n-1), pla++) pla[1]=*pla+*pd++;
			}}
			//
			pla=plano->puntos.in+3*(tin->ns.p_retícula+tin->ns.p_increm);
			if(tin->puntos.s32.p_indiv!=NULL){
				ssint *pp=tin->puntos.s32.p_indiv;
				dontimes(3*tin->ns.p_indiv,) *pla++=*pp++;
			}
			}
			break;
		case TIN_uint16:{
			if(tin->puntos.u16.p_retículas.ppio!=NULL){
			for(const PuntosRetícula_u16 *p=tin->puntos.u16.p_retículas.ppio; p!=tin->puntos.u16.p_retículas.next; p++){
				PuntoXYZ_uint16m p1=p->P0;
				if(p->Δu.X==0 && p->Δu.Y==0 && p->Δu.Z==0){
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_uint16m pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))) *pla++=pp.X, *pla++=pp.Y, *pla++=pp.Z;
					}
				}else{
					const PuntoXYZ_uint16m *Δ=&p->Δu;
					uint16m *pde=p->desplaz.pfull;
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_uint16m pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))){
							uint16m l=*pde++;
							*pla++=pp.X+l*Δ->X;
							*pla++=pp.Y+l*Δ->Y;
							*pla++=pp.Z+l*Δ->Z;
						}
					}
				}
			}}
			//
			pla=plano->puntos.in+3*tin->ns.p_retícula;
			if(tin->puntos.u16.p_increm.ppio!=NULL){
			for(const PuntosIncrementos_u16 *p=tin->puntos.u16.p_increm.ppio; p!=tin->puntos.u16.p_increm.next; p++){
				if(p->n==0) continue;
				sint16m *pd=p->increm;
				*pla++=p->P0.X;
				*pla++=p->P0.Y;
				*pla=p->P0.Z;
				dontimes(3*(p->n-1), pla++) pla[1]=*pla+*pd++;
			}}
			//
			pla=plano->puntos.in+3*(tin->ns.p_retícula+tin->ns.p_increm);
			if(tin->puntos.u16.p_indiv!=NULL){
				uint16m *pp=tin->puntos.u16.p_indiv;
				dontimes(3*tin->ns.p_indiv,) *pla++=*pp++;
			}
			}
			break;
		case TIN_sint16:{
			if(tin->puntos.s16.p_retículas.ppio!=NULL){
			for(const PuntosRetícula_s16 *p=tin->puntos.s16.p_retículas.ppio; p!=tin->puntos.s16.p_retículas.next; p++){
				PuntoXYZ_sint16m p1=p->P0;
				if(p->Δu.X==0 && p->Δu.Y==0 && p->Δu.Z==0){
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_sint16m pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))) *pla++=pp.X, *pla++=pp.Y, *pla++=pp.Z;
					}
				}else{
					const PuntoXYZ_sint16m *Δ=&p->Δu;
					sint16m *pde=p->desplaz.pfull;
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_sint16m pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))){
							sint16m l=*pde++;
							*pla++=pp.X+l*Δ->X;
							*pla++=pp.Y+l*Δ->Y;
							*pla++=pp.Z+l*Δ->Z;
						}
					}
				}
			}}
			//
			pla=plano->puntos.in+3*tin->ns.p_retícula;
			if(tin->puntos.s16.p_increm.ppio!=NULL){
			for(const PuntosIncrementos_s16 *p=tin->puntos.s16.p_increm.ppio; p!=tin->puntos.s16.p_increm.next; p++){
				if(p->n==0) continue;
				sint16m *pd=p->increm;
				*pla++=p->P0.X;
				*pla++=p->P0.Y;
				*pla=p->P0.Z;
				dontimes(3*(p->n-1), pla++) pla[1]=*pla+*pd++;
			}}
			//
			pla=plano->puntos.in+3*(tin->ns.p_retícula+tin->ns.p_increm);
			if(tin->puntos.s16.p_indiv!=NULL){
				sint16m *pp=tin->puntos.s16.p_indiv;
				dontimes(3*tin->ns.p_indiv,) *pla++=*pp++;
			}
			}
			break;
		}
	}else{
		aj_malloc_n(plano->puntos.dbl,double,3*plano->np);
		double *pla=plano->puntos.dbl;
		switch(tin->tdato){
		case TIN_double:{
			if(tin->puntos.dbl.p_retículas.ppio!=NULL){
			for(const PuntosRetícula_dbl *p=tin->puntos.dbl.p_retículas.ppio; p!=tin->puntos.dbl.p_retículas.next; p++){
				PuntoXYZ_double p1=p->P0;
				if(p->Δu.X==0 && p->Δu.Y==0 && p->Δu.Z==0){
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_double pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))) *pla++=pp.X, *pla++=pp.Y, *pla++=pp.Z;
					}
				}else{
				   const PuntoXYZ_double *Δ=&p->Δu;
				   if(p->bhalf){
					float *pde=p->desplaz.phalf;
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_double pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))){
							float l=*pde++;
							*pla++=pp.X+l*Δ->X;
							*pla++=pp.Y+l*Δ->Y;
							*pla++=pp.Z+l*Δ->Z;
						}
					}
				   }else{
					double *pde=p->desplaz.pfull;
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_double pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))){
							double l=*pde++;
							*pla++=pp.X+l*Δ->X;
							*pla++=pp.Y+l*Δ->Y;
							*pla++=pp.Z+l*Δ->Z;
						}
					}
				   }
				}
			}}
			//
			pla=plano->puntos.dbl+3*tin->ns.p_retícula;
			if(tin->puntos.dbl.p_increm.ppio!=NULL){
			for(const PuntosIncrementos_dbl *p=tin->puntos.dbl.p_increm.ppio; p!=tin->puntos.dbl.p_increm.next; p++){
				if(p->n==0) continue;
				float *pd=p->increm;
				*pla++=p->P0.X;
				*pla++=p->P0.Y;
				*pla=p->P0.Z;
				dontimes(3*(p->n-1), pla++) pla[1]=*pla+*pd++;
			}}
			//
			pla=plano->puntos.dbl+3*(tin->ns.p_retícula+tin->ns.p_increm);
			if(tin->puntos.dbl.p_indiv!=NULL){
				double *pp=tin->puntos.dbl.p_indiv;
				dontimes(3*tin->ns.p_indiv,) *pla++=*pp++;
			}
			}
			break;
		case TIN_float:{
			if(tin->puntos.fl.p_retículas.ppio!=NULL){
			for(const PuntosRetícula_float *p=tin->puntos.fl.p_retículas.ppio; p!=tin->puntos.fl.p_retículas.next; p++){
				PuntoXYZ_float p1=p->P0;
				if(p->Δu.X==0 && p->Δu.Y==0 && p->Δu.Z==0){
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_float pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))) *pla++=pp.X, *pla++=pp.Y, *pla++=pp.Z;
					}
				}else{
				   const PuntoXYZ_float *Δ=&p->Δu;
				   if(p->bhalf){
					FloatHalf *pde=p->desplaz.phalf;
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_float pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))){
							float l=float___FLoatHalf(*pde); pde++;
							*pla++=pp.X+l*Δ->X;
							*pla++=pp.Y+l*Δ->Y;
							*pla++=pp.Z+l*Δ->Z;
						}
					}
				   }else{
					float *pde=p->desplaz.pfull;
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_float pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))){
							float l=*pde++;
							*pla++=pp.X+l*Δ->X;
							*pla++=pp.Y+l*Δ->Y;
							*pla++=pp.Z+l*Δ->Z;
						}
					}
				   }
				}
			}}
			//
			pla=plano->puntos.dbl+3*tin->ns.p_retícula;
			if(tin->puntos.fl.p_increm.ppio!=NULL){
			for(const PuntosIncrementos_float *p=tin->puntos.fl.p_increm.ppio; p!=tin->puntos.fl.p_increm.next; p++){
				if(p->n==0) continue;
				FloatHalf *pd=p->increm;
				*pla++=p->P0.X;
				*pla++=p->P0.Y;
				*pla=p->P0.Z;
				dontimes(3*(p->n-1), pla++) pla[1]=*pla+*pd++;
			}}
			//
			pla=plano->puntos.dbl+3*(tin->ns.p_retícula+tin->ns.p_increm);
			if(tin->puntos.fl.p_indiv!=NULL){
				float *pp=tin->puntos.fl.p_indiv;
				dontimes(3*tin->ns.p_indiv,) *pla++=*pp++;
			}
			}
			break;
		case TIN_half:{
			if(tin->puntos.half.p_retículas.ppio!=NULL){
			for(const PuntosRetícula_half *p=tin->puntos.half.p_retículas.ppio; p!=tin->puntos.half.p_retículas.next; p++){
				PuntoXYZ_half p1=p->P0;
				if(p->Δu.X==0 && p->Δu.Y==0 && p->Δu.Z==0){
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_half pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))) *pla++=pp.X, *pla++=pp.Y, *pla++=pp.Z;
					}
				}else{
					//Programado sólo para FloatHalf realmente un tipo float
					const PuntoXYZ_half *Δ=&p->Δu;
					FloatHalf *pde=p->desplaz.pfull;
					dontimes(p->m, (P_eq(p1,+=,p->Δn))){
						PuntoXYZ_half pp=p1;
						dontimes(p->n, (P_eq(pp,+=,p->Δ1))){
							float l=*pde++;
							*pla++=pp.X+l*Δ->X;
							*pla++=pp.Y+l*Δ->Y;
							*pla++=pp.Z+l*Δ->Z;
						}
					}
				}
			}}
			//
			pla=plano->puntos.dbl+3*tin->ns.p_retícula;
			if(tin->puntos.half.p_increm.ppio!=NULL){
			for(const PuntosIncrementos_half *p=tin->puntos.half.p_increm.ppio; p!=tin->puntos.half.p_increm.next; p++){
				if(p->n==0) continue;
				FloatHalf *pd=p->increm;
				*pla++=p->P0.X;
				*pla++=p->P0.Y;
				*pla=p->P0.Z;
				dontimes(3*(p->n-1), pla++) pla[1]=*pla+*pd++;
			}}
			//
			pla=plano->puntos.dbl+3*(tin->ns.p_retícula+tin->ns.p_increm);
			if(tin->puntos.half.p_indiv!=NULL){
				FloatHalf *pp=tin->puntos.half.p_indiv;
				dontimes(3*tin->ns.p_indiv,) *pla++=*pp++;
			}
			}
			break;
		}
	}

	uint *pt=plano->triángulos;

	if(tin->t_malla.ppio!=NULL){
	for(const TriángulosMalla *p=tin->t_malla.ppio; p!=tin->t_malla.next; p++){
		uint a0=p->a0, b0=p->b0, c0=p->c0;
		dontimes(p->m, (a0+=p->Δan, b0+=p->Δbn, c0+=p->Δcn)){
			uint a=a0, b=b0, c=c0;
			dontimes(p->n, (a+=p->Δa1, b+=p->Δb1, c+=p->Δc1)) *pt++=a, *pt++=b, *pt++=c;
		}
	}}

	if(tin->t_increm.ppio!=NULL){
	for(const TriángulosIncrementos *p=tin->t_increm.ppio; p!=tin->t_increm.next; p++){
		*pt++=p->a0;
		sint16m *pi=p->increm;
		dontimes(3*p->n-1, pi++){*pt=pt[-1]+*pi; pt++;}
	}}

	{durchlaufep(uint,tin->t_indiv,3*tin->ns.t_indiv) *pt++=*p;}

	return 0;

salida_outofmem:
	free(plano->estilos_t);
	free(plano->triángulos);
	return AT_NOMEM;
}
