//El b23 alto del último uint no lo escribe
static void bowrite_headerf(Buffer_bo *buf, const BmpFileHeader *hf){
	boput_1616(buf,hf->filesize&0xFFFF,hf->BM);
	boput_1616(buf,hf->unused&0xFFFF,hf->filesize>>16);
	boput_1616(buf,hf->matrix_offset&0xFFFF,hf->unused>>16);
}

static void table_fondo_frente(uint table[256], const TablaOpts *colores){
	ifunlike(colores->i1<colores->i0) goto especiales;
	ifunlike(colores->i1==colores->i0){table[colores->i0]=colores->fondo; goto especiales;}
	ifunlike(colores->i1==colores->i0+1){table[colores->i0]=colores->fondo; table[colores->i0+1]=colores->frente; goto especiales;}

	uint8m Δ=colores->i1-colores->i0;
	if(!colores->nolineal){ //Lineal
		if(Δ==255 && colores->fondo==COL_NEGRO && colores->frente==COL_BLANCO){
			uint c=0, s=0x010101;
			for(uint *ptr=table;(c&0x1000000)==0;c+=s) *ptr++=c;
		}else if(Δ==255 && colores->fondo==COL_BLANCO && colores->frente==COL_NEGRO){
			uint s=0x010101, c=0xFFffFF+s; s=-s;
			uint *ptr=table; do c+=s, *ptr++=c; while(c);
		}else{
			uint C1, C2, C3;
			float F1,F2,F3, f1,f2,f3;

			C1=colores->fondo&0xFF0000;	C2=colores->fondo&0xFF00;		C3=colores->fondo&0xFF;
			{const float f0=1.0F/(float)Δ;
			ssint d1, d2, d3;
			d1=(ssint)(colores->frente&0xFF0000);
			d2=(ssint)(colores->frente&0xFF00);
			d3=(ssint)(colores->frente&0xFF);
			F1=(d1-(ssint)C1)*f0;
			F2=(d2-(ssint)C2)*f0;
			F3=(d3-(ssint)C3)*f0;}

			uint *ptr=table+colores->i0;
			f3=f2=f1=0;
			dontimes(Δ,(f1+=F1,f2+=F2,f3+=F3, ptr++)){
				uint c1,c2,c3;
				c1=(C1+(uint)(ssint)f1)&0xFF0000;
				c2=(C2+(uint)(ssint)f2)&0xFF00;
				c3=(C3+(uint)(ssint)f3)&0xFF;
				*ptr=c1|c2|c3;
			}
			*ptr=colores->frente&0xFFffFF; //Escrita al final para evitar errores de redondeo
		}
	}else{ //No lineal
		uint C1,C2,C3;
		float Fb1,Fb2,Fb3, fm, F1,F2,F3, f1,f2,f3;
		const float f0=1.0F/(float)Δ;
		uint *ptr;
		uint8m count;

		C1=colores->fondo&0xFF0000;	C2=colores->fondo&0xFF00;		C3=colores->fondo&0xFF;
		{ssint d1, d2, d3;
		d1=(ssint)(colores->frente&0xFF0000);
		d2=(ssint)(colores->frente&0xFF00);
		d3=(ssint)(colores->frente&0xFF);
		Fb1=(float)(d1-(ssint)C1);
		Fb2=(float)(d2-(ssint)C2);
		Fb3=(float)(d3-(ssint)C3);}

		ptr=table+colores->i0;
		f3=f2=f1=0;

		if(Fb1+Fb2+Fb3 >= 0) fm=f0*3.F/2.F,		count=(uint8m)(0x55/255.0F*Δ); //Pendiente 3/2
		else fm=f0*3.F/2.F,							count=(uint8m)(0x55/255.0F*Δ); //Pendiente 3/2 (f0*4/3, count=0x60/...)
		F1=Fb1*fm; F2=Fb2*fm; F3=Fb3*fm;

		dontimes(count,(f1+=F1,f2+=F2,f3+=F3, ptr++)){
			uint c1,c2,c3;
			c1=(C1+(uint)(ssint)f1)&0xFF0000;
			c2=(C2+(uint)(ssint)f2)&0xFF00;
			c3=(C3+(uint)(ssint)f3)&0xFF;
			*ptr=c1|c2|c3;
		}

		fm=1.F-count*fm; //Lo que queda (hasta 1.0).
		count=Δ-count; //Los pasos que quedan
		fm/=count;
		F1=Fb1*fm; F2=Fb2*fm; F3=Fb3*fm;

		dontimes(count,(f1+=F1,f2+=F2,f3+=F3, ptr++)){
			uint c1,c2,c3;
			c1=(C1+(uint)(ssint)f1)&0xFF0000;
			c2=(C2+(uint)(ssint)f2)&0xFF00;
			c3=(C3+(uint)(ssint)f3)&0xFF;
			*ptr=c1|c2|c3;
		}
		*ptr=colores->frente&0xFFffFF; //Escrita al final para evitar errores de redondeo
	}

	uint *ptr;
	const color *p;
especiales:
	if((p=colores->specials_first)!=NULL){ptr=table; while(*p!=Яcolor) *ptr++=*p++;}
	if((p=colores->specials_last)!=NULL){ptr=table+256; while(*p!=Яcolor) *--ptr=*p++;}
}

void headerb8_colores(Bmp8FullHeader *head, uint npx, uint npy, const TablaOpts *colores){
	uint bm_uints=((npx+3)>>2)*npy;

	head->f.BM='B'|('M'<<8);
	head->f.matrix_offset=0x38+0x400; //0x400 es lo que ocupa la tabla de color
	head->f.filesize=head->f.matrix_offset+4*bm_uints;
	head->f.unused=0;
	//
	head->i.headersize=0x28;
	head->i.wd=npx;
	head->i.ht=~npy+1;
	head->i.planes_bpp=(8<<16)|1;
	head->i.compression=0;
	head->i.imagesize=4*bm_uints;
	head->i.ppmeter_x=0x0EC4;
	head->i.ppmeter_y=head->i.ppmeter_x;
	head->i.ncolors=256;
	head->i.ignored=0;

	table_fondo_frente(head->table,colores);
}

void headerb8_tabla(Bmp8FullHeader *head, uint npx, uint npy, const uint* tabla, uint ncolors){
	uint bm_uints=((npx+3)>>2)*npy;
	mineq(ncolors,256);

	head->f.BM='B'|('M'<<8);
	head->f.matrix_offset=0x38+usizeof(uint)*ncolors;
	head->f.filesize=head->f.matrix_offset+4*bm_uints;
	head->f.unused=0;
	//
	head->i.headersize=0x28;
	head->i.wd=npx;
	head->i.ht=~npy+1;
	head->i.planes_bpp=(8<<16)|1;
	head->i.compression=0;
	head->i.imagesize=4*bm_uints;
	head->i.ppmeter_x=0x0EC4;
	head->i.ppmeter_y=head->i.ppmeter_x;
	head->i.ncolors=ncolors;
	head->i.ignored=0;

	memcpy_uint(&head->table[0], tabla,ncolors);
}

void headerc8(Bmp8FullHeader *head, uint npx, uint npy, const TabladeColor *table){
	uint bm_uints=((npx+3)>>2)*npy;

	head->f.BM='B'|('M'<<8);
	head->f.matrix_offset=0x38+0x400; //0x400 es lo que ocupa la tabla de color
	head->f.filesize=head->f.matrix_offset+4*bm_uints;
	head->f.unused=0;
	//
	head->i.headersize=0x28;
	head->i.wd=npx;
	head->i.ht=~npy+1;
	head->i.planes_bpp=(8<<16)|1;
	head->i.compression=0;
	head->i.imagesize=4*bm_uints;
	head->i.ppmeter_x=0x0EC4;
	head->i.ppmeter_y=head->i.ppmeter_x;
	head->i.ncolors=256;
	head->i.ignored=0;

	uint *colores=head->table,
		*back=colores+256;
	const color *p;	if((p=table->specials_first)!=NULL){while(*p!=Яcolor) *colores++=*p++;}
						if((p=table->specials_last)!=NULL){while(*p!=Яcolor) *--back=*p++;}
	interpola_tc32(table->ncols,table->pcols,table->ppos,(pdif)(back-colores),colores);
}

/*Escribe en buf un fichero bmp de 8 bits, con tabla de color.
Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero para escribirlo
	Un código de error en la escritura
*/
int escribe_bmp8(const char8_t *filename, const Bmp8FullHeader *head, Bitmap8 bm){
	Buffer_bo buf;
	uint16m c;
	uint ncolors=head->i.ncolors;
	if(ncolors==0) ncolors=256;

	ifnzunlike(boopen_utf8(&buf,filename,ATBYTES_LITTLE_ENDIAN)) return BMP_WRITE_NOOPEN;

	//Como va todo decalado hay que escribirlo así;
	bowrite_headerf(&buf,&head->f);
	c=(uint16m)(head->f.matrix_offset>>16);
	{durchlaufep(uint,(uint*)&head->i,uintsizeof(head->i)){boput_1616(&buf,*p&0xFFFF,c); c=(uint16m)(*p>>16);}}
	{durchlaufep(uint,(uint*)&head->table,ncolors){boput_1616(&buf,*p&0xFFFF,c); c=(uint16m)(*p>>16);}}
	boput_1616(&buf,0,c); //El alto es de relleno. A partir de aquí ya está alineado.
	bowrite_uints(&buf,bm.values,bm.nuints);
	boclose(&buf);
	return buf.error_code;
}

int escribe_bmp8_geo(const char8_t *filename, const Bmp8FullHeader *head, Bitmap8 bm, const Matriz___Tierra *transf, const ZBounds *zbounds){
	int nret=escribe_bmp8(filename,head,bm);
	ifunlike(nret==BMP_WRITE_NOOPEN) return nret;

	//El fichero de texto con la georreferenciación
	if(transf!=NULL && zbounds!=NULL){
		char8_t filegeo[SHRT_PATH];
		char8_t *ext=strpcpy8(filegeo,filename);
		*--ext='w'; *--ext='g'; *--ext='j';
		escribe_georref(filegeo,zbounds,transf,8);
	}
	return nret;
}
