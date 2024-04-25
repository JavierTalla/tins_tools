//Multiplica todo el tin por esc
void reescala_tin(TINMalla* tin, float esc){
	if(esc==1.0F) return;
	{durchlaufep(sint16m,tin->malla.z.ppio,tin->malla.z.n) *p=(sint16m)((float)*p*esc);}
	{durchlaufep(ssint,&tin->p_indiv.ppio[0].X,3*tin->p_indiv.n) *p=(ssint)((float)*p*esc);}
	tin->minmax.mx=(ssint)((float)tin->minmax.mx*esc);
	tin->minmax.MX=(ssint)((float)tin->minmax.MX*esc);
	tin->minmax.my=(ssint)((float)tin->minmax.my*esc);
	tin->minmax.MY=(ssint)((float)tin->minmax.MY*esc);
}

//Escala los valores de Z
void reescalaZ_tin(TINMalla* tin, float esc){
	if(esc==1.0F) return;
	{durchlaufep(sint16m,tin->malla.z.ppio,tin->malla.z.n) *p=(sint16m)((float)*p*esc);}
	durchVectorp(PuntoXYZ_ssint,tin->p_indiv) p->Z=(ssint)((float)p->Z*esc);
}

//Desplaza en Z
void desplazaZ_tin(TINMalla* tin, ssint Δz){
	if(Δz==0) return;
	sint16m δz=(sint16m)Δz;
	{durchlaufep(sint16m,tin->malla.z.ppio,tin->malla.z.n) *p+=δz;}
	durchVectorp(PuntoXYZ_ssint,tin->p_indiv) p->Z+=Δz;
}
