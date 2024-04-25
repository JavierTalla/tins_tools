#define PlanoCuadra 0
#define Mundo 1
#define EstereoNorte 2
#define EstereoSur 3
#define Estéreo 4

//Si el área pasada en geográficas es muy alargada es posible que la función escoja planocuadra
//pero luego al expandir el área resulte un área para la que habría sido mejor una proyección.
//Que no pasen regiones tan alargadas.
static umint decide_proyección(const RECT_ΛΦ *geo, float *φ0){
	float Δφ, Δλ, c,C;
	Δφ=geo->Φ-geo->φ;
	Δλ=geo->Λ-geo->λ;

	*φ0=0.5F*(geo->Φ+geo->φ);

	if(Δφ>=MΦ && Δλ>=180+(ESδΛ<<1)) return Mundo;
	if(geo->Φ>=0 && geo->φ<=0){
		c=max(geo->Φ,-geo->φ);
		if(c==90) c=0; else c=cosf(c*(float)PI_180);
		C=1;
	}else{
		if(geo->φ==-90) C=0; else C=cosf(geo->φ*(float)PI_180);
		if(geo->Φ==90) c=0; else c=cosf(geo->Φ*(float)PI_180);
		if(c>C){float aux=c; c=C; C=aux;}
	}
	if(c*PlCu>C) return PlanoCuadra;
	if(Δλ==360){
		if(Δφ>=90+ESδΛ) return Mundo;
		if(geo->Φ==90) return EstereoNorte;
		if(geo->φ==-90) return EstereoSur;
	}

	//Una proyección no polar
	float s1,c1, s2,c2, N,D;
	N=geo->φ*(float)PI_180;	s1=sinf(N); c1=cosf(N);
	N=geo->Φ*(float)PI_180;	s2=sinf(N); c2=cosf(N);
	Δφ*=(float)PI_180;
	Δλ*=(float)PI_180;

	N=(s1+s2)*(s2-s1);
	D=Δφ+s2*c2-s1*c1;
	N*=Δλ; Δλ*=0.5F; //A partir de aquí, Δλ/2;
	D*=2*sinf(Δλ);
	*φ0=atan2f(N,D); //De la proyección estereográfica

	//Mirar que el punto más alejado no esté a más de 90+ESδΛ del centro
	float s0,c0;
	N=-sinf(ESδΛ*(float)PI_180);
	c=cosf(Δλ);
	s0=sinf(*φ0);
	c0=cosf(*φ0);
	C=s1*s0+c*c1*c0; //sin(φ1)*sin(φ0)+cos(Δλ)*cos(φ1)*cos(φ0)
	if(C<N) return Mundo;
	C=s2*s0+c*c2*c0;
	if(C<N) return Mundo;

	*φ0*=(float)PI_180_PI;
	return Estéreo;
}

static void cc_radianes(Matriz___Tierra *transf){
	if(transf->tipo==TIPO_PROY_PlanoCuadra){
		//Valores en radianes
		transf->cc.radianes.λmin=transf->cc.λmin*(float)PI_180;
		transf->cc.radianes.λmax=transf->cc.λmax*(float)PI_180;
		transf->cc.radianes.φmin=transf->cc.φmin*(float)PI_180;
		transf->cc.radianes.φmax=transf->cc.φmax*(float)PI_180;
		transf->cc.radianes.pix_λ=transf->cc.pix_λ*(float)PI_180;
		transf->cc.radianes.pix_φ=transf->cc.pix_φ*(float)PI_180;
	}
}

static void calcula_info(Matriz___Tierra *transf){
	//Calcular φ0 y λ0 (el centro del primer píxel)
	transf->info.x0=transf->rectProy.mx+0.5F*transf->pix_x;
	transf->info.y0=transf->rectProy.my+0.5F*transf->pix_y;

	//En info, que es meramente informativo, hay que dejar las coordenadas
	//anteriores al giro de 90º.
	if(transf->b90){
		giraxy_tierra___proy(float,transf->info.x0,transf->info.y0);
	}
	//Los valores de rectProy y pix_x, pix_y están siempre en metros.
	//En caso de PlanoCuadra, pasarlos a grados.
	if(transf->tipo==TIPO_PROY_PlanoCuadra){
		float k=1.0F/(float)(PI_180*R_TIERRA);
		transf->info.y0*=k;
		transf->info.x0*=k*transf->cc._cosφ0;
	}
}

//Calcula transf->info y, si es planocuadra, la copia en radianes de los valores en grados
sinline void calcula_secundarios(Matriz___Tierra *transf){
	cc_radianes(transf);
	calcula_info(transf);
}

//Calcular sus valores específicos cuando todo lo demás ya está calculado
//Recibe unos valores de pix_x, pix_y y rectProy en grados, con λ ya ajustado *cosφ0, y los pasa a metros.
static void calcula_PlanoCuadra(Matriz___Tierra *transf){
	if(!transf->b90){
		transf->cc.npλ=transf->npx;
		transf->cc.npφ=transf->npy;
		transf->cc.λmin=transf->rectProy.mx*transf->cc._cosφ0;
		transf->cc.λmax=transf->rectProy.MX*transf->cc._cosφ0;
		transf->cc.φmin=transf->rectProy.my;
		transf->cc.φmax=transf->rectProy.MY;
		//Al emplear cuando se rellene la matriz el valor pix_λ estamos perdiendo el ajuste por redondeo
		transf->cc.pix_λ=transf->pix_x*transf->cc._cosφ0; //llevado a cabo en rectTalla___rectProy
		transf->cc.pix_φ=transf->pix_y;
	}else{
		transf->cc.npλ=transf->npy;
		transf->cc.npφ=transf->npx;
		transf->cc.λmin=-transf->rectProy.MY*transf->cc._cosφ0;
		transf->cc.λmax=-transf->rectProy.my*transf->cc._cosφ0;
		transf->cc.φmin=transf->rectProy.mx;
		transf->cc.φmax=transf->rectProy.MX;
		//Al emplear cuando se rellene la matriz el valor pix_λ estamos perdiendo el ajuste por redondeo
		transf->cc.pix_λ=transf->pix_y*transf->cc._cosφ0; //llevado a cabo en rectTalla___rectProy
		transf->cc.pix_φ=transf->pix_x;
	}
	//Dejar valores en metros
	float k=(float)(PI_180*R_TIERRA);
	transf->rectProy.mx*=k;		transf->rectProy.MX*=k;
	transf->rectProy.my*=k;		transf->rectProy.MY*=k;
	transf->pix_x*=k;				transf->pix_y*=k;
}

//1º: Gira transf->rectProy si b90
//2º: Ajuste del rectángulo de talla a la forma del área de la Tierra pedida, a aprtir de transf->rectProy
//	  En concreto calcula, en transf, npx, npy, pix_x, pix_y.
//3º Ajusta rectProy por los redondeos para que se corresponda exactamete con transf->npx, transf->npy.
//npx y npy: Valores deseados por defecto
static void rectTalla___rectProy(const uint npx, const uint npy, bint bgirar, Matriz___Tierra *transf){
	float ΔX,ΔY; //En la matriz
	float npx_d;	//Para encajar exactamente el área de la Tierra pedida en rect.ht se deberían tener npx_d.

	ΔX=transf->rectProy.MX-transf->rectProy.mx;
	ΔY=transf->rectProy.MY-transf->rectProy.my;
	if(!bgirar) transf->b90=0;
	else{
		if(ΔX>=ΔY) transf->b90=(npx<npy);
		else transf->b90=(npx>npy);
	}
	if(transf->b90){
		girarect_proy___tierra(&transf->rectProy);
		float aux=ΔX; ΔX=ΔY; ΔY=aux;
	}

	npx_d=npy*ΔX/ΔY;
	if((float)npx>=npx_d){ //Estrechar en x
		transf->npy=npy;
		transf->npx=(uint)(npx_d+0.7F);
	}else{ //Estrechar en y
		npx_d=npx*ΔY/ΔX; //npx_d. pasa a indicar npy_d
		transf->npx=npx;
		transf->npy=(uint)(npx_d+0.7F);
	}

	//pix_x, pix_y y ajuste de redondeos
	float Δ;
	transf->pix_x=ΔX/(float)transf->npx;
	transf->pix_y=ΔY/(float)transf->npy;
	//
	Δ=transf->pix_x*transf->npx;
	if(ΔX!=Δ){
		ΔX-=Δ; ΔX/=2; //Corrección
		transf->rectProy.mx+=ΔX;
		transf->rectProy.MX=transf->rectProy.mx+Δ;
	}
	Δ=transf->pix_y*transf->npy;
	if(ΔY!=Δ){
		ΔY-=Δ; ΔY/=2; //Corrección
		transf->rectProy.my+=ΔY;
		transf->rectProy.MY=transf->rectProy.my+Δ;
	}
}

/*Calcula la transformación, que almacena en transf.
1º Se escoge una proyección
2º Se calcula rectProy a partir de geo y la proyección escogida
3º: rectTalla___rectProy
	a. El rectángulo npx x npy se se hace más pequeño en horiz. o en vert. para ajustarse al área pedida en geo,
		y los valores se guardan en transf->npx y transf->npy.
	b. Se calcula pix_x y pix_y y los límites de rectProy se ajustan por redondeo para que coincidan exactamente
		con pix_x*npx y pix_y*npy.
4º Se calculan los valores específicos que sea necesario para la proyección escogida

bgirar indica si se permite un giro de 90º

pix_x, pix_y y rectProy quedan siempre en metros, también para PlanoCuadra.
*/
void calcula_matriz___tierra(const uint npx, const uint npy, const RECT_ΛΦ *geográficas, Matriz___Tierra *transf, bint bgirar){
	RECT_ΛΦ geo=*geográficas;
	while(geo.λ<-180) geo.λ+=360;
	while(geo.λ>=180) geo.λ-=360;
	while(geo.Λ<-180) geo.Λ+=360;
	while(geo.Λ>=180) geo.Λ-=360;
	if(geo.Λ<=geo.λ) geo.Λ+=360;
	float φ0;
	float δφ; //Para las proyecciones de Mundo

	umint t=decide_proyección(&geo,&φ0);

	/*Cálculo del rectángulo rectProy y δφ*/

	//Cálculo de rectProy para PlanoCuadra. Para Mundo se entra aquí
	//para calcular δφ; el rectProy se recalculará más abajo.
	//pix_x, pix_y, y el rectProy que resultan de aquí están en grados (con λ *cosφ0).
	if(t==PlanoCuadra || t==Mundo){
		if(t!=Mundo) transf->tipo=TIPO_PROY_PlanoCuadra;
		transf->cc.cosφ0=cosf((float)(0.5F*(float)(geo.φ+geo.Φ)*PI_180));
		transf->rectProy.mx=transf->cc.cosφ0*geo.λ;
		transf->rectProy.MX=transf->cc.cosφ0*geo.Λ;
		transf->rectProy.my=geo.φ;
		transf->rectProy.MY=geo.Φ;
		//El escalado cosφ0 en λ perjudica los valores de φ mayores.
		//Multiplicando por sin(δφ)/δφ (que es <1) se obtiene algo más compensado,
		//pero no conviene estirar mucho el centro en proyecciones que abarquen el Ecuador,
		//porque queda mal. Así que el escalado a aplicar a λ estará entre cosφ0 y
		//cosφ0*sin(δφ)/δφ. Buscaremos el que mejor se ajuste al rectángulo de la matriz
		float Δxproy, Δyproy, δproym, δproyM, δmatriz;
		Δxproy=transf->rectProy.MX-transf->rectProy.mx;
		Δyproy=transf->rectProy.MY-transf->rectProy.my;
		δφ=0.5F*(float)(geo.Φ-geo.φ)*(float)PI_180;
		δφ=sinf(δφ)/δφ;
		maxeq(δφ,0.75f);
		float c_antes=transf->cc.cosφ0; //Recordarlo
		if(δφ<1.0F){
			δproyM=Δxproy/Δyproy; //Rect. con δφ=1
			δproym=δproyM*δφ;	//Rect. con δφ el mín. posible
			δmatriz=(float)npx/(float)npy; //Rect. que se quiere para la matriz (talla)
			{float δav=0.5F*(δproym+δproyM);
			if((δmatriz>=1 && δav<1) || (δmatriz<1 && δav>=1)) δmatriz=1/δmatriz;} //Interca. x<->y en el rect. de la matriz
			if(δmatriz<=δproym) transf->cc.cosφ0*=δφ; //El extremo inferior
			elif(δmatriz<δproyM) transf->cc.cosφ0*=δmatriz/δproyM; //Un valor intermedio
			transf->rectProy.mx=transf->cc.cosφ0*geo.λ;
			transf->rectProy.MX=transf->cc.cosφ0*geo.Λ;
		}
		if(t==Mundo) δφ=c_antes/transf->cc.cosφ0; //>=1
		transf->cc._cosφ0=1.0F/transf->cc.cosφ0;
	}
	//Cálculo de rectProy para Sistema (incl. Mundo).
	if(t!=PlanoCuadra){
		transf->tipo=TIPO_PROY_Sistema;
		float λ=0.5F*(geo.Λ+geo.λ);
		if(t==Mundo) setup_Sistema_Sinusoidal_ex(&transf->sis,λ,δφ,0.5);
		elif(t==EstereoNorte) setup_Sistema_Estereo_Norte(&transf->sis,λ);
		elif(t==EstereoSur) setup_Sistema_Estereo_Sur(&transf->sis,λ);
		else setup_Sistema_Estereográfica(&transf->sis,φ0,λ); //Estéreo

		Extremos2D_dbl proy;
		proy.mx=geo.λ*(float)PI_180;
		proy.MX=geo.Λ*(float)PI_180;
		proy.my=geo.φ*(float)PI_180;
		proy.MY=geo.Φ*(float)PI_180;
		proy=rect_proy___rect_geog(proy,&transf->sis);
		transf->rectProy.mx=(float)proy.mx;
		transf->rectProy.MX=(float)proy.MX;
		transf->rectProy.my=(float)proy.my;
		transf->rectProy.MY=(float)proy.MY;
	}
	rectTalla___rectProy(npx,npy,bgirar,transf); //Aquí se decide y aplica b90.

	if(transf->tipo==TIPO_PROY_PlanoCuadra){
		calcula_PlanoCuadra(transf); //Pasa  pix_x, pix_y y rectProy de grados a metros
	}
	transf->pixel_m=transf->pix_y;
	//Ajustar el píxel nominal por el factor de escala de la proyección
	if(transf->tipo==TIPO_PROY_Sistema){
		float Δx=transf->rectProy.MX-transf->rectProy.mx;
		float Δ=transf->rectProy.MY-transf->rectProy.my;
		//Para una proyección estereográfica tomamos el punto a 0.6 del lado largo
		//Para una con una línea de k=1, se tomará a 0.7 del lado corto
		maxeq(Δx,Δ); Δx*=0.3F;
		Δx/=(2.0F*R_TIERRA);
		Δ=1.0F+Δx*Δx;
		transf->pixel_m/=Δ;
	}

	calcula_secundarios(transf);
}

#undef PlanoCuadra
#undef Mundo
#undef EsteroNorte
#undef EsteroSur
#undef Estéreo

int calcula_transf_plani_bb(const uint npx, const uint npy, const RECT_ΛΦ *geográficas, Matriz___Tierra *transf, bint bgirar, bint bajustar){
	ifunlike(npx<2 || npy<2) return MATRIZ_TOO_SMALL;
	{uint x=npx, y=npy;
	while(x && !(y&0x80000000)) x>>=1, y<<=1;
	ifnz(x) return MATRIZ_TOO_BIG;}

	calcula_matriz___tierra(npx,npy,geográficas,transf,bgirar); //asigna npx, npy que mejor se ajustan a geográficas.
	if(bajustar){
		if(transf->npx<npx){ //La función ha "mandado" expandir el área de la Tierra a lo ancho del tablón
			uint l=npx-transf->npx; //exceso
			uint de=l>>1;
			expande_MatrizTierra_x(transf,de,l-de);
		}else if(transf->npy<npy){ //La función ha "mandado" etc., a lo alto.
			uint l=npy-transf->npy; //exceso
			uint de=l>>1;
			expande_MatrizTierra_y(transf,de,l-de);
		}
	}

	return 0;
}
