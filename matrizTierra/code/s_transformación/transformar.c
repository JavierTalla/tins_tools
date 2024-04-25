Puntoxy_float matriz___tierra_planocuadra1(Puntoxy_float p, const Matriz___Tierra *transf, bint besquina){
	float Λ0, Φ0;
	const float _pix_λ=1.0F/transf->cc.pix_λ,
					_pix_φ=1.0F/transf->cc.pix_φ;

	//Cálculo de Λ0 y Φ0. Por definición, medio píxel a la izda. y hacia arriba del primer
	//punto almacenado en la matriz.
	Λ0=transf->cc.λmin;
	if(transf->b90==0) Φ0=transf->cc.φmax;
	else Φ0=transf->cc.φmin;
	if(besquina){
		Λ0-=0.5F*transf->cc.pix_λ;
		if(transf->b90==0) Φ0+=0.5F*transf->cc.pix_φ;
		else Φ0-=0.5F*transf->cc.pix_φ;
	}

	p.λ-=Λ0; p.λ*=_pix_λ;
	p.φ=Φ0-p.φ; p.φ*=_pix_φ; //p.λ y p.φ ya son p.x y p.y.
	if(transf->b90){float x=p.x; p.x=-p.y; p.y=x;}
	return p;
}

void matriz___tierra_planocuadra(Puntoxy_float *p, uint n, const Matriz___Tierra *transf, bint besquina){
	float Λ0, Φ0;
	const float _pix_λ=1.0f/transf->cc.pix_λ,
					_pix_φ=1.0f/transf->cc.pix_φ;

	//Cálculo de Λ0 y Φ0. Por definición, medio píxel a la izda. y hacia arriba del primer
	//punto almacenado en la matriz.
	Λ0=transf->cc.λmin;
	if(transf->b90==0) Φ0=transf->cc.φmax;
	else Φ0=transf->cc.φmin;
	if(besquina){
		Λ0-=0.5F*transf->cc.pix_λ;
		if(transf->b90==0) Φ0+=0.5F*transf->cc.pix_φ;
		else Φ0-=0.5F*transf->cc.pix_φ;
	}

	while(n){ n--;
		p->λ-=Λ0; p->λ*=_pix_λ;
		p->φ=Φ0-p->φ; p->φ*=_pix_φ; //p.λ y p.φ ya son p.x y p.y.
		if(transf->b90){float x=p->x; p->x=-p->y; p->y=x;}
		p++;
	}
}

Puntoxy_float matriz___tierra_sistema1(Puntoxy_float p, const Matriz___Tierra *transf, bint besquina){
	while(p.x-transf->sis.infor.λ0>180) p.x-=360;
	while(p.x-transf->sis.infor.λ0<=-180) p.x+=360;
	Puntoxy_double q={PI_180*p.x,PI_180*p.y};
	q=proy___geo1(&transf->sis,q);
	p.x=(float)q.x; p.y=(float)q.y;
	if(transf->b90){girapunto_proy___tierra(p)}
	p.x-=transf->rectProy.mx;
	p.y=transf->rectProy.MY-p.y;
	p.x/=transf->pix_x;
	p.y/=transf->pix_y;
	if(!besquina){p.x-=0.5F; p.y+=0.5F;}

	return p;
}

//Necesita convertir los puntos a double. Para eso tiene que reservar memoria.
//Return: 0, AT_NOMEM
int matriz___tierra_sistema(Puntoxy_float *p, uint n, const Matriz___Tierra *transf, bint besquina){
	Puntoxy_double *pD, *pd;
	const double λ0=transf->sis.infor.λ0;
	const float _pixx=1.0f/transf->pix_x,
					_pixy=1.0f/transf->pix_y,
					mx=transf->rectProy.mx,
					MX=transf->rectProy.MY;

	aj_malloc_return(pd,Puntoxy_double,n);
	{durchlaufe2(Puntoxy_double,pd,n,Puntoxy_float,p){
		ptr->φ=ptr_b->φ; ptr->λ=ptr_b->λ;
		while(ptr->λ-λ0>180) ptr->λ-=360;
		while(ptr->λ-λ0<=-180) ptr->λ+=360;
		ptr->φ*=PI_180; ptr->λ*=PI_180;
	}}
	proy___geo(pd,usizeof(*pd),n,&transf->sis);
	pD=pd;
	while(n){ n--;
		p->x=(float)pd->x; p->y=(float)pd->y; pd++;
		if(transf->b90){girapunto_proy___tierra(*p)}
		p->x-=mx; p->y=MX-p->y;
		p->x*=_pixx; p->y*=_pixy;
		if(!besquina){p->x-=0.5F; p->y+=0.5F;}
		p++;
	}

	free(pD);
	return 0;
}
