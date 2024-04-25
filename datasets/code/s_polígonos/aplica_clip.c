void aplica_clip(umint *state, uint npx, uint npy, Polígono_xy *clip, u8int npols){
	ifunlike(npols==0 || npx==0 || npy==0) return;
	const uint nx=npx; //Elementos de cada fila de state. No son npx+1 porque
	const float fnpx=(float)(npx-1); //los cortes a la derecha del máximo no hace falta registrarlos.

	//En state, el bit bajo se emplea para el polígonno que en cada momento se esté procesando.
	//Un 2 indica que es punto de dentro. Por tanto, una vez que se hace |=2, el 2 ya no desaparece.
	while(npols){ npols--;
		Puntoxy_float *p=clip->ppio;
		Puntoxy_float p1=*p++;
		while(p!=clip->next){
			Puntoxy_float p0=p1;
			p1=*p++;
			if(p0.y==p1.y) continue;
			if(p0.x>=fnpx && p1.x>=fnpx) continue; //Los cortes a la derecha no se registran

			Puntoxy_float a,b;
			if(p0.y<p1.y) a=p0, b=p1;
			else a=p1, b=p0;
			if(a.y>npy || b.y<=0) continue;

			float x; uint y;
			uint ymax=(uint)b.y; ifunlike((float)ymax==b.y) ymax--;
			if(ymax>=npy) ymax=npy-1;
			float δx=(b.x-a.x)/(b.y-a.y); //δx al aumentar 1 en y.
			x=a.x;
			if(a.y<0){y=0; x+=-a.y*δx;}
			else{float f=ceilf(a.y); x+=(f-a.y)*δx; y=(uint)f;}

			umint *pf=state+y*nx;
			if(δx>=0){
			   while(y<=ymax && x<0){*pf^=2; y++, pf+=nx, x+=δx;}
			   for(;y<=ymax && x<fnpx; y++,pf+=nx, x+=δx) pf[1+(uint)x]^=2;
			}else{
			   while(y<=ymax && x>=fnpx) y++,pf+=nx, x+=δx;
			   for(;y<=ymax && x>=0; y++,pf+=nx, x+=δx) pf[1+(uint)x]^=2;
			   while(y<=ymax){*pf^=2; y++, pf+=nx;}
			}
		}

		//Deja state limpio para la siguiente iteración (e.d., sus 2nd-bit bajos a 0).
		{umint *p=state;
		dontimes(npy,){ uint b=0;
			dontimes(npx,p++){
				if(*p&2) b=~b;
				*p&=0xFD; //Clear the 2nd-lowest bit.
				if(b) *p|=1;
			}
		}}

		clip++;
	}
}
