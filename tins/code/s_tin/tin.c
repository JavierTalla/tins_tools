#undef VectorGrowingFactor
#define VectorGrowingFactor 4

u16int tclass___vclass(Tclass___Vclass kak_tclass, uint8m v0, uint8m v1, uint8m v2, tclass_from_vclasses func){
	uint8m v12;

	switch(kak_tclass){
	case Tclass___Vclass_2min:
	case Tclass___Vclass_2max:
		if(v0==v1) return v0;
		if(v0==v2) return v0;
		if(v1==v2) return v1;
		if(kak_tclass==Tclass___Vclass_2max) goto MAX;
	case Tclass___Vclass_min: v12=min(v0,v1); return min(v12,v2);
	case Tclass___Vclass_max: MAX: v12=max(v0,v1); return max(v12,v2);
	case Tclass___Vclass_3func:
		if(v0==v1 && v1==v2 && v2==v1) return v0;
	case Tclass___Vclass_func:
		return func(v0,v1,v2);
	}
	//Unreachable
	return 0;
}

int tin___matriz16(TINMalla *tin, const sint16m *matriz, uint pix, uint npx, uint npy, bint bcompacto, const uint8m *clases, Tclass___Vclass kak_tclass, tclass_from_vclasses func){
	const umint *pflags;

	*tin=(TINMalla){0};
	if(npx<2 || npy<2) return 0;

	tin->malla.y0=tin->malla.x0=0;
	tin->malla.nx=npx;
	tin->malla.ny=npy;
	tin->malla.Δx=(ssint)pix; tin->malla.Δy=-(ssint)pix;

	tin->triangles.N=(npx-1)*(npy-1)*2; //>=2, par
	tin->malla.z.N=(npx*npy+1)&~1U; //>=4, par.
	Vsetup(sint16m,tin->malla.z,tin->malla.z.N, goto salida_outofmem);
	Vsetup(TinTriangle,tin->triangles,tin->triangles.N, goto salida_outofmem);

	//Rellenar las z de la malla y minmax
	memcpy(tin->malla.z.ppio,matriz,npx*npy*usizeof(tin->malla.z.ppio[0]));
	tin->malla.z.n=npx*npy;
	tin->minmax.my=(ssint)(npy-1)*pix; //=ptr->Y
	tin->minmax.mx=0;
	tin->minmax.MY=0;
	tin->minmax.MX=(ssint)(npx-1)*pix;

	//Rellenar las clases de los puntos seleccionados, si ngrueso!=1
	if(clases==NULL) pflags=NULL;
	else pflags=clases;

	//Los triángulos
	npx--, npy--; //Ahora indican número de triángulos
	{uint n0=0, n1=npx+1;
	TinTriangle *ptr=tin->triangles.ppio;
	dontimes(npy,){
		ssint z1, z3;
		umint f1, f3;

		z1=tin->malla.z.ppio[n0];
		z3=tin->malla.z.ppio[n1];
		if(pflags!=NULL){
			f1=pflags[n0];
			f3=pflags[n1];
		}
		n0++, n1++;

		dontimes(npx,(n0++,n1++)){
			ssint z0,z2; //0,1 arriba; 2,3, abajo
			umint f0,f2;

			//n0 y n1 están apuntando al punto delantero
			z0=z1;	z1=tin->malla.z.ppio[n0];
			z2=z3;	z3=tin->malla.z.ppio[n1];
			if(pflags!=NULL){
				f0=f1;	f1=pflags[n0];
				f2=f3;	f3=pflags[n1];
			}

			//Mantendremos la diagonal más baja del cuadrado, para que las
			//láminas de agua salgan mejor
			if(bcompacto || z0+z3<=z2+z1){	//La diagonal 0-3 queda por debajo. Triángulos 230, 103
				ptr->a=n1-1; ptr->b=n1; ptr++->c=n0-1;
				ptr->a=n0; ptr->b=n0-1; ptr--->c=n1;
				if(pflags==NULL){
					ptr++->class=0;
					ptr++->class=0;
				}else{
					ptr++->class=tclass___vclass(kak_tclass,f2,f3,f0,func);
					ptr++->class=tclass___vclass(kak_tclass,f1,f0,f3,func);
				}
			}else{			//La diagonal 1-2 queda por debajo. Triángulos 210, 123
				ptr->a=n1-1; ptr->b=n0; ptr++->c=n0-1;
				ptr->a=n0; ptr->b=n1-1; ptr--->c=n1;
				if(pflags==NULL){
					ptr++->class=0;
					ptr++->class=0;
				}else{
					ptr++->class=tclass___vclass(kak_tclass,f2,f1,f0,func);
					ptr++->class=tclass___vclass(kak_tclass,f1,f2,f3,func);
				}
			}
		}
	}
	tin->triangles.n=(pdif)(ptr-tin->triangles.ppio);
	}

	tin->bdiag=bcompacto;
	return 0;

salida_outofmem:
	free_null_if(tin->triangles.ppio);
	free_null_if(tin->malla.z.ppio);
	return AT_NOMEM;
}

//Calcula con cuántos puntos queda una línea que tenía np puntos al pasarla a un grueso ngrueso.
#define npgrueso___np(np,ngrueso) (((np)-1)/ngrueso + 1)

//size es el tamaño de cada punto en matriz. El dato 1 está en matriz[size].
int tingrueso___matriz16_size(TINMalla *tin, uint8m ngrueso, const sint16m *matriz, uint8m size, uint pix, uint npx, uint npy, bint bcompacto, const uint8m *clases, Tclass___Vclass kak_tclass, tclass_from_vclasses func){
	ifunlike(ngrueso>=npx) ngrueso=(uint8m)(npx-1);
	ifunlike(ngrueso>=npy) ngrueso=(uint8m)(npy-1);
	uint ntx=npgrueso___np(npx,ngrueso);
	uint nty=npgrueso___np(npy,ngrueso);
	uint inc_nextrow;
	pix*=ngrueso;
	*tin=(TINMalla){0};

	sint16m *M=n_malloc(sint16m,ntx*nty);
	umint *F=NULL;
	if(clases!=NULL) F=n_malloc(umint,ntx*nty);
	ifunlike(M==NULL || (clases!=NULL && F==NULL)){
		freeif(F); freeif(M);
		*tin=(TINMalla){0};
		return AT_NOMEM;
	}
	inc_nextrow=ngrueso*(npx-ntx);

	//Copiar las clases
	if(F!=NULL){
		const umint *pm=clases;
		umint *pM=F;
		dontimes(nty, pm+=inc_nextrow){
			dontimes(ntx,pm+=ngrueso) *pM++=*pm;
		}
	}

	//Copiar la matriz
	ngrueso*=size;
	inc_nextrow*=size;
	{const sint16m *pm=matriz;
	sint16m *pM=M;
	dontimes(nty, pm+=inc_nextrow){
		dontimes(ntx,pm+=ngrueso) *pM++=*pm;
	}}

	int nret=tin___matriz16(tin,M,pix,ntx,nty,bcompacto,F,kak_tclass,func);
	freeif(F);
	free(M);
	return nret;
}

#undef VectorGrowingFactor
#define VectorGrowingFactor 1 //El valor por defecto
