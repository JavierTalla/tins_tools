#include <stdio.h>

#define P_op(P, EQ, Q,OP,R) (P).X EQ ((Q).X OP (R).X),   (P).Y EQ ((Q).Y OP (R).Y),   (P).Z EQ ((Q).Z OP (R).Z)

PuntoXYZ_ssint getP(const TINMalla *tin, uint np){
	if(np<tin->malla.z.n){
		PuntoXYZ_ssint P;
		div_t d;
		d=div((int)np,(int)tin->malla.nx);
		P.X=tin->malla.x0+tin->malla.Δx*d.rem;
		P.Y=tin->malla.y0+tin->malla.Δy*d.quot;
		P.Z=tin->malla.z.ppio[np];
		return P;
	}
	return tin->p_indiv.ppio[np-tin->malla.z.n];
}

//If Vmm is not 1.0, coordinates are multiplied by Vmm
//Escribe 12 words
void write_triangle(Buffer_bo *buf, PuntoXYZ_ssint A, PuntoXYZ_ssint B, PuntoXYZ_ssint C, float Vmm){
	PuntoXYZ_ssint v1, v2, P;
	IO_SINGLE trian[4*3], *pt; //vector y los tres vértices

	P_op(v1,=,B,-,A);
	P_op(v2,=,C,-,A);
	pt=&trian[0];

	P.X=v1.Y*v2.Z-v1.Z*v2.Y;
	P.Y=v1.Z*v2.X-v1.X*v2.Z;
	P.Z=v1.X*v2.Y-v1.Y*v2.X;
	if(Vmm==1.0f){
		IO_SINGLE___float(pt,(float)P.X); pt++;	IO_SINGLE___float(pt,(float)P.Y);  pt++;	IO_SINGLE___float(pt,(float)P.Z);  pt++;
		IO_SINGLE___float(pt,(float)A.X); pt++;	IO_SINGLE___float(pt,(float)A.Y);  pt++;	IO_SINGLE___float(pt,(float)A.Z);  pt++;
		IO_SINGLE___float(pt,(float)B.X); pt++;	IO_SINGLE___float(pt,(float)B.Y);  pt++;	IO_SINGLE___float(pt,(float)B.Z);  pt++;
		IO_SINGLE___float(pt,(float)C.X); pt++;	IO_SINGLE___float(pt,(float)C.Y);  pt++;	IO_SINGLE___float(pt,(float)C.Z);  pt++;
	}else{
		IO_SINGLE___float(pt,(float)P.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)P.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)P.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)A.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)A.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)A.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)B.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)B.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)B.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)C.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)C.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)C.Z*Vmm);  pt++;
	}

	bowrite_SINGLEs(buf,trian,12);
}

//If Vmm is not 1.0, coordinates are multiplied by Vmm
//resto: primer u16 a escribir.
//color: último u16 a escribir
//Escribe 13 words
void write_triangle_resto(Buffer_bo *buf, PuntoXYZ_ssint A, PuntoXYZ_ssint B, PuntoXYZ_ssint C, float Vmm, u16int resto, u16int color){
	PuntoXYZ_ssint v1, v2, P;
	IO_SINGLE trian[4*3], *pt; //vector y los tres vértices

	P_op(v1,=,B,-,A);
	P_op(v2,=,C,-,A);
	pt=&trian[0];

	P.X=v1.Y*v2.Z-v1.Z*v2.Y;
	P.Y=v1.Z*v2.X-v1.X*v2.Z;
	P.Z=v1.X*v2.Y-v1.Y*v2.X;
	if(Vmm==1.0f){
		IO_SINGLE___float(pt,(float)P.X); pt++;	IO_SINGLE___float(pt,(float)P.Y);  pt++;	IO_SINGLE___float(pt,(float)P.Z);  pt++;
		IO_SINGLE___float(pt,(float)A.X); pt++;	IO_SINGLE___float(pt,(float)A.Y);  pt++;	IO_SINGLE___float(pt,(float)A.Z);  pt++;
		IO_SINGLE___float(pt,(float)B.X); pt++;	IO_SINGLE___float(pt,(float)B.Y);  pt++;	IO_SINGLE___float(pt,(float)B.Z);  pt++;
		IO_SINGLE___float(pt,(float)C.X); pt++;	IO_SINGLE___float(pt,(float)C.Y);  pt++;	IO_SINGLE___float(pt,(float)C.Z);  pt++;
	}else{
		IO_SINGLE___float(pt,(float)P.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)P.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)P.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)A.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)A.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)A.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)B.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)B.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)B.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)C.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)C.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)C.Z*Vmm);  pt++;
	}

#if COMPILER_ID==CLANG_ID  /* workaround a bug in clang*/
	IO_SINGLE trian2[13], *pt2=&trian2[0];

	pt=&trian[0];
	dontimes(12,pt++){
		*pt2++=((*pt&0xFFFF)<<16) | resto;
		resto=*pt>>16;
	}
	*pt2=(color<<16) | resto;

	pt2-=12;
	bowrite_SINGLEs(buf,pt2,13);
#else
	pt=&trian[0];
	dontimes(12,pt++){
		boput_1616(buf,*pt&0xFFFF,resto);
		resto=*pt>>16;
	}
	boput_1616(buf,color,resto); //attr. count (color) y último resto
#endif
}

//If Vmm is not 1.0, coordinates are multiplied by Vmm
//Escribe 12 words
void write_triangle_dbl(Buffer_bo *buf, PuntoXYZ_double A, PuntoXYZ_double B, PuntoXYZ_double C, float Vmm){
	PuntoXYZ_float v1, v2, P;
	IO_SINGLE trian[4*3], *pt; //vector y los tres vértices

	P_op(v1,=(float),B,-,A);
	P_op(v2,=(float),C,-,A);
	pt=&trian[0];

	P.X=v1.Y*v2.Z-v1.Z*v2.Y;
	P.Y=v1.Z*v2.X-v1.X*v2.Z;
	P.Z=v1.X*v2.Y-v1.Y*v2.X;
	if(Vmm==1.0f){
		IO_SINGLE___float(pt,(float)P.X); pt++;	IO_SINGLE___float(pt,(float)P.Y);  pt++;	IO_SINGLE___float(pt,(float)P.Z);  pt++;
		IO_SINGLE___float(pt,(float)A.X); pt++;	IO_SINGLE___float(pt,(float)A.Y);  pt++;	IO_SINGLE___float(pt,(float)A.Z);  pt++;
		IO_SINGLE___float(pt,(float)B.X); pt++;	IO_SINGLE___float(pt,(float)B.Y);  pt++;	IO_SINGLE___float(pt,(float)B.Z);  pt++;
		IO_SINGLE___float(pt,(float)C.X); pt++;	IO_SINGLE___float(pt,(float)C.Y);  pt++;	IO_SINGLE___float(pt,(float)C.Z);  pt++;
	}else{
		IO_SINGLE___float(pt,(float)P.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)P.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)P.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)A.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)A.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)A.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)B.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)B.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)B.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)C.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)C.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)C.Z*Vmm);  pt++;
	}

	bowrite_SINGLEs(buf,trian,12);
}

//If Vmm is not 1.0, coordinates are multiplied by Vmm
//resto: primer u16 a escribir.
//color: último u16 a escribir
//Escribe 13 words
void write_triangle_dbl_resto(Buffer_bo *buf, PuntoXYZ_double A, PuntoXYZ_double B, PuntoXYZ_double C, float Vmm, u16int resto, u16int color){
	PuntoXYZ_float v1, v2, P;
	IO_SINGLE trian[4*3], *pt; //vector y los tres vértices

	P_op(v1,=(float),B,-,A);
	P_op(v2,=(float),C,-,A);
	pt=&trian[0];

	P.X=v1.Y*v2.Z-v1.Z*v2.Y;
	P.Y=v1.Z*v2.X-v1.X*v2.Z;
	P.Z=v1.X*v2.Y-v1.Y*v2.X;
	if(Vmm==1.0f){
		IO_SINGLE___float(pt,(float)P.X); pt++;	IO_SINGLE___float(pt,(float)P.Y);  pt++;	IO_SINGLE___float(pt,(float)P.Z);  pt++;
		IO_SINGLE___float(pt,(float)A.X); pt++;	IO_SINGLE___float(pt,(float)A.Y);  pt++;	IO_SINGLE___float(pt,(float)A.Z);  pt++;
		IO_SINGLE___float(pt,(float)B.X); pt++;	IO_SINGLE___float(pt,(float)B.Y);  pt++;	IO_SINGLE___float(pt,(float)B.Z);  pt++;
		IO_SINGLE___float(pt,(float)C.X); pt++;	IO_SINGLE___float(pt,(float)C.Y);  pt++;	IO_SINGLE___float(pt,(float)C.Z);  pt++;
	}else{
		IO_SINGLE___float(pt,(float)P.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)P.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)P.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)A.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)A.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)A.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)B.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)B.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)B.Z*Vmm);  pt++;
		IO_SINGLE___float(pt,(float)C.X*Vmm); pt++;	IO_SINGLE___float(pt,(float)C.Y*Vmm);  pt++;	IO_SINGLE___float(pt,(float)C.Z*Vmm);  pt++;
	}

	pt=&trian[0];
	dontimes(12,pt++){
		boput_1616(buf,*pt&0xFFFF,resto);
		resto=*pt>>16;
	}
	boput_1616(buf,color,resto); //attr. count (color) y último resto
}

int fstl___tinplano(const char8_t *fstl, const TINPlano *tin){
	int nret;
	Buffer_bo buf;
	uint cabecera[STL_CABECERA_UINTSIZEOF]={0};

	ifnzunlike(nret=boopen_utf8(&buf,fstl,ATBYTES_LITTLE_ENDIAN)) return nret;

	cabecera[0]=((uint)'O'<<24) | ((uint)'L'<<16) | ((uint)'O'<<8) | 'C'; //Assumes ASCII-compatible execution character encoding
	cabecera[1]=((uint)0x8080<<16) | ((uint)'='<<8) | 'R'; //G,B,'=','R'
	cabecera[2]=0x80; /*0,0,Alpha,R*/

	bowrite_uints(&buf,cabecera,STL_CABECERA_UINTSIZEOF);
	boput_32(&buf,tin->nt);

	if(TipoDatoTIN_is_integer(tin->tdato)){
		uint *p=tin->triángulos;
		uint *es=tin->estilos_t;
		dontimes(tin->nt>>1,){
			PuntoXYZ_ssint A,B,C;
			u16int resto, c;

			uint ip;
			ip=3**p++;		A.X=tin->puntos.in[ip];		A.Y=tin->puntos.in[ip+1];	A.Z=tin->puntos.in[ip+2];
			ip=3**p++;		B.X=tin->puntos.in[ip];		B.Y=tin->puntos.in[ip+1];	B.Z=tin->puntos.in[ip+2];
			ip=3**p++;		C.X=tin->puntos.in[ip];		C.Y=tin->puntos.in[ip+1];	C.Z=tin->puntos.in[ip+2];
			write_triangle(&buf,A,B,C,1.0f);

			//El color
			if(es==NULL) resto=0, c=0;
			else{
				uint d=*es++; resto=((d>>9)&0x7C00) | ((d>>6)&0x3E0) | ((d>>3)&0x1F);
				d=*es++;			c=((d>>9)&0x7C00) | ((d>>6)&0x3E0) | ((d>>3)&0x1F);
			}
			ip=3**p++;		A.X=tin->puntos.in[ip];		A.Y=tin->puntos.in[ip+1];	A.Z=tin->puntos.in[ip+2];
			ip=3**p++;		B.X=tin->puntos.in[ip];		B.Y=tin->puntos.in[ip+1];	B.Z=tin->puntos.in[ip+2];
			ip=3**p++;		C.X=tin->puntos.in[ip];		C.Y=tin->puntos.in[ip+1];	C.Z=tin->puntos.in[ip+2];
			write_triangle_resto(&buf,A,B,C,1.0f,resto,c);
		}
		if(tin->nt&1){ //Último triángulo suelto
			PuntoXYZ_ssint A,B,C;
			u16int resto;

			uint ip;
			ip=3**p++;		A.X=tin->puntos.in[ip];		A.Y=tin->puntos.in[ip+1];	A.Z=tin->puntos.in[ip+2];
			ip=3**p++;		B.X=tin->puntos.in[ip];		B.Y=tin->puntos.in[ip+1];	B.Z=tin->puntos.in[ip+2];
			ip=3**p++;		C.X=tin->puntos.in[ip];		C.Y=tin->puntos.in[ip+1];	C.Z=tin->puntos.in[ip+2];
			write_triangle(&buf,A,B,C,1.0);

			if(es==NULL) resto=0;
			else{uint d=*es; resto=((d>>9)&0x7C00) | ((d>>6)&0x3E0) | ((d>>3)&0x1F);}
			boput_1616(&buf,0,resto);
		}
	}else{
		uint *p=tin->triángulos;
		uint *es=tin->estilos_t;
		dontimes(tin->nt>>1,){
			PuntoXYZ_double A,B,C;
			u16int resto, c;

			uint ip;
			ip=3**p++;		A.X=tin->puntos.dbl[ip];	A.Y=tin->puntos.dbl[ip+1];		A.Z=tin->puntos.dbl[ip+2];
			ip=3**p++;		B.X=tin->puntos.dbl[ip];	B.Y=tin->puntos.dbl[ip+1];		B.Z=tin->puntos.dbl[ip+2];
			ip=3**p++;		C.X=tin->puntos.dbl[ip];	C.Y=tin->puntos.dbl[ip+1];		C.Z=tin->puntos.dbl[ip+2];
			write_triangle_dbl(&buf,A,B,C,1.0f);

			//El color
			if(es==NULL) resto=0, c=0;
			else{
				uint d=*es++; resto=((d>>9)&0x7C00) | ((d>>6)&0x3E0) | ((d>>3)&0x1F);
				d=*es++;			c=((d>>9)&0x7C00) | ((d>>6)&0x3E0) | ((d>>3)&0x1F);
			}
			ip=3**p++;		A.X=tin->puntos.dbl[ip];	A.Y=tin->puntos.dbl[ip+1];		A.Z=tin->puntos.dbl[ip+2];
			ip=3**p++;		B.X=tin->puntos.dbl[ip];	B.Y=tin->puntos.dbl[ip+1];		B.Z=tin->puntos.dbl[ip+2];
			ip=3**p++;		C.X=tin->puntos.dbl[ip];	C.Y=tin->puntos.dbl[ip+1];		C.Z=tin->puntos.dbl[ip+2];
			write_triangle_dbl_resto(&buf,A,B,C,1.0f,resto,c);
		}
		if(tin->nt&1){ //Último triángulo suelto
			PuntoXYZ_double A,B,C;
			u16int resto;

			uint ip;
			ip=3**p++;		A.X=tin->puntos.dbl[ip];	A.Y=tin->puntos.dbl[ip+1];		A.Z=tin->puntos.dbl[ip+2];
			ip=3**p++;		B.X=tin->puntos.dbl[ip];	B.Y=tin->puntos.dbl[ip+1];		B.Z=tin->puntos.dbl[ip+2];
			ip=3**p++;		C.X=tin->puntos.dbl[ip];	C.Y=tin->puntos.dbl[ip+1];		C.Z=tin->puntos.dbl[ip+2];
			write_triangle_dbl(&buf,A,B,C,1.0);

			if(es==NULL) resto=0;
			else{uint d=*es; resto=((d>>9)&0x7C00) | ((d>>6)&0x3E0) | ((d>>3)&0x1F);}
			boput_1616(&buf,0,resto);
		}
	}

	boclose(&buf);
	return buf.error_code;
}

int escribe_fichero_stl1(const char8_t *fstl, const TINMalla *tin, float funi, uint color_default, const uint16m *colores){
	int nret;
	Buffer_bo buf;
	uint cabecera[STL_CABECERA_UINTSIZEOF];

	ifnzunlike(nret=boopen_utf8(&buf,fstl,ATBYTES_LITTLE_ENDIAN)) return nret;

	{uint c=color_default;
	cabecera[0]=((uint)'O'<<24) | ((uint)'L'<<16) | ((uint)'O'<<8) | 'C'; //Assumes ASCII-compatible execution character encoding
	cabecera[1]=(((c>>8)&0xFF)<<16) | ((c&0xFF)<<8) | 'R'; //G,B,'=','R'
	cabecera[2]=(c>>16)&0xFF; /*0,0,Alpha,R*/}

	bowrite_uints(&buf,cabecera,STL_CABECERA_UINTSIZEOF);
	boput_32(&buf,tin->triangles.n);

	TinTriangle *p=tin->triangles.ppio;
	dontimes(tin->triangles.n>>1,p++){
		PuntoXYZ_ssint A,B,C;
		u16int resto, c;

		A=getP(tin,p->a);
		B=getP(tin,p->b);
		C=getP(tin,p->c);
		write_triangle(&buf,A,B,C,funi);

		//attr. count. Se guarda el color
		if(p->class==0 || colores==NULL) resto=0;
		else resto=0x8000|colores[p->class];

		p++;
		A=getP(tin,p->a);
		B=getP(tin,p->b);
		C=getP(tin,p->c);
		if(p->class==0 || colores==NULL) c=0;
		else c=0x8000|colores[p->class];
		write_triangle_resto(&buf,A,B,C,funi,resto,c);
	}
	if(tin->triangles.n&1){ //Último triángulo suelto
		PuntoXYZ_ssint A,B,C;
		u16int resto;

		A=getP(tin,p->a);
		B=getP(tin,p->b);
		C=getP(tin,p->c);
		write_triangle(&buf,A,B,C,funi);

		if(p->class==0 || colores==NULL) resto=0;
		else resto=0x8000|colores[p->class];
		boput_1616(&buf,0,resto);
	}

	boclose(&buf);
	return buf.error_code;
}

int escribe_fichero_stl2(const char8_t *fstl, const TINMalla *tin, float funi, uint color_default, const uint16m *colores){
	int nret;
	Buffer_bo buf;
	uint cabecera[STL_CABECERA_UINTSIZEOF];

	ifnzunlike(nret=boopen_utf8(&buf,fstl,ATBYTES_LITTLE_ENDIAN)) return nret;

	{uint c=color_default;
	cabecera[0]=((uint)'O'<<24) | ((uint)'L'<<16) | ((uint)'O'<<8) | 'C'; //Assumes ASCII-compatible execution character encoding
	cabecera[1]=(((c>>8)&0xFF)<<16) | ((c&0xFF)<<8) | 'R'; //G,B,'=','R'
	cabecera[2]=(c>>16)&0xFF; /*0,0,Alpha,R*/}

	bowrite_uints(&buf,cabecera,STL_CABECERA_UINTSIZEOF);
	boput_32(&buf,tin->triangles.n);

	durchVectorp(TinTriangle,tin->triangles){
		PuntoXYZ_ssint v1, v2;
		PuntoXYZ_ssint P,A,B,C;
		IO_SINGLE trian[4*3], *pt; //vector y los tres vértices

		A=getP(tin,p->a);
		B=getP(tin,p->b);
		C=getP(tin,p->c);
		P_op(v1,=,B,-,A);
		P_op(v2,=,C,-,A);

		pt=&trian[0];

		P.X=v1.Y*v2.Z-v1.Z*v2.Y;
		P.Y=v1.Z*v2.X-v1.X*v2.Z;
		P.Z=v1.X*v2.Y-v1.Y*v2.X;
		if(funi==1.0f){
			IO_SINGLE___float(pt,(float)P.X); pt++;	IO_SINGLE___float(pt,(float)P.Y);  pt++;	IO_SINGLE___float(pt,(float)P.Z);  pt++;
			IO_SINGLE___float(pt,(float)A.X); pt++;	IO_SINGLE___float(pt,(float)A.Y);  pt++;	IO_SINGLE___float(pt,(float)A.Z);  pt++;
			IO_SINGLE___float(pt,(float)B.X); pt++;	IO_SINGLE___float(pt,(float)B.Y);  pt++;	IO_SINGLE___float(pt,(float)B.Z);  pt++;
			IO_SINGLE___float(pt,(float)C.X); pt++;	IO_SINGLE___float(pt,(float)C.Y);  pt++;	IO_SINGLE___float(pt,(float)C.Z);  pt++;
		}else{
			IO_SINGLE___float(pt,(float)P.X*funi); pt++;	IO_SINGLE___float(pt,(float)P.Y*funi);  pt++;	IO_SINGLE___float(pt,(float)P.Z*funi);  pt++;
			IO_SINGLE___float(pt,(float)A.X*funi); pt++;	IO_SINGLE___float(pt,(float)A.Y*funi);  pt++;	IO_SINGLE___float(pt,(float)A.Z*funi);  pt++;
			IO_SINGLE___float(pt,(float)B.X*funi); pt++;	IO_SINGLE___float(pt,(float)B.Y*funi);  pt++;	IO_SINGLE___float(pt,(float)B.Z*funi);  pt++;
			IO_SINGLE___float(pt,(float)C.X*funi); pt++;	IO_SINGLE___float(pt,(float)C.Y*funi);  pt++;	IO_SINGLE___float(pt,(float)C.Z*funi);  pt++;
		}
		uint16m c; //Color
		if(p->class==0 || colores==NULL) c=0;
		else c=0x8000|colores[p->class];

		bowrite_SINGLEs(&buf,trian,12);
		boput_1616(&buf,c,2); //Color, attr. count
	}
	boclose(&buf);
	return buf.error_code;
}
