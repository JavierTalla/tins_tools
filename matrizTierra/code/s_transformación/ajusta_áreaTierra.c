//Expande ex por cada lado de x y ey por cada lado de y
void expande_MatrizTierra(Matriz___Tierra *transf, uint ex, uint ey){
	transf->rectProy.mx-=ex*transf->pix_x;
	transf->rectProy.MX+=ex*transf->pix_x;
	transf->npx+=ex<<1;
	transf->rectProy.my-=ey*transf->pix_y;
	transf->rectProy.MY+=ey*transf->pix_y;
	transf->npy+=ey<<1;

	if(transf->tipo==TIPO_PROY_PlanoCuadra){
		uint eλ, eφ;
		float d;

		if(!transf->b90) eλ=ex, eφ=ey;
		else eλ=ey, eφ=ex;

		d=eλ*transf->cc.pix_λ;
		transf->cc.λmin-=d;
		transf->cc.λmax+=d;
		transf->cc.npλ+=eλ<<1;

		d=eφ*transf->cc.pix_φ;
		transf->cc.φmin-=d;
		transf->cc.φmax+=d;
		transf->cc.npφ+=eφ<<1;
	}

	calcula_secundarios(transf);
}

//iz, de: El número de píxeles a añadir por la izquierda y por la derecha
void expande_MatrizTierra_x(Matriz___Tierra *transf, uint iz, uint de){
	transf->rectProy.mx-=iz*transf->pix_x;
	transf->rectProy.MX+=de*transf->pix_x;
	transf->npx+=iz+de;

	if(transf->tipo==TIPO_PROY_PlanoCuadra){
		float d1, d2;
		if(!transf->b90){
			d1=iz*transf->cc.pix_λ;
			d2=de*transf->cc.pix_λ;

			transf->cc.λmin-=d1;
			transf->cc.λmax+=d2;
			transf->cc.npλ+=iz+de;
		}else{
			d1=iz*transf->cc.pix_φ;
			d2=de*transf->cc.pix_φ;

			transf->cc.φmin-=d1;
			transf->cc.φmax+=d2;
			transf->cc.npφ+=iz+de;
		}
	}

	calcula_secundarios(transf);
}

//ab, ar: El número de píxeles a añadir por la abajo y por arriba
void expande_MatrizTierra_y(Matriz___Tierra *transf, uint ab, uint ar){
	transf->rectProy.my-=ab*transf->pix_y;
	transf->rectProy.MY+=ar*transf->pix_y;
	transf->npy+=ab+ar;

	if(transf->tipo==TIPO_PROY_PlanoCuadra){
		float d1, d2;
		if(!transf->b90){
			d1=ab*transf->cc.pix_φ;
			d2=ar*transf->cc.pix_φ;

			transf->cc.φmin-=d1;
			transf->cc.φmax+=d2;
			transf->cc.npφ+=ab+ar;
		}else{
			d1=ar*transf->cc.pix_λ;
			d2=ab*transf->cc.pix_λ;

			transf->cc.λmin-=d1;
			transf->cc.λmax+=d2;
			transf->cc.npλ+=ab+ar;
		}
	}

	calcula_secundarios(transf);
}
