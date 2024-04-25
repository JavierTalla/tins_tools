void headerc16(Bmp16FullHeader *head, uint npx, uint npy, const BmpMasks *col){
	uint bm_uints=((npx+1)>>1)*npy;

	head->f.BM='B'|('M'<<8);
	head->f.matrix_offset=0x44;
	head->f.filesize=head->f.matrix_offset+4*bm_uints;
	head->f.unused=0;
	//
	head->i.headersize=0x28;
	head->i.wd=npx;
	head->i.ht=~npy+1;
	head->i.planes_bpp=(16<<16)|1;
	head->i.compression=3;
	head->i.imagesize=4*bm_uints;
	head->i.ppmeter_x=0x0EC4;
	head->i.ppmeter_y=head->i.ppmeter_x;
	head->i.ncolors=0;
	head->i.ignored=0;
	//
	head->m=*col;
}

int escribe_bmp16(const char8_t *filename, const Bmp16FullHeader *head, Bitmap16 bm){
	Buffer_bo buf;
	uint16m c;

	ifnzunlike(boopen_utf8(&buf,filename,ATBYTES_LITTLE_ENDIAN)) return BMP_WRITE_NOOPEN;

	//Como va todo decalado hay que escribirlo así;
	bowrite_headerf(&buf,&head->f);
	c=(uint16m)(head->f.matrix_offset>>16);
	{durchlaufep(uint,(uint*)&head->i,uintsizeof(head->i)){boput_1616(&buf,*p&0xFFFF,c); c=(uint16m)(*p>>16);}}
	{durchlaufep(uint,(uint*)&head->m,uintsizeof(head->m)){boput_1616(&buf,*p&0xFFFF,c); c=(uint16m)(*p>>16);}}
	boput_1616(&buf,0,c); //El alto es de relleno. A partir de aquí ya está alineado.
	bowrite_uints(&buf,bm.values,bm.nuints);
	boclose(&buf);
	return buf.error_code;
}

//Escribe en buf un fichero bmp de 16 bits.
//Si transf y zbounds no son NULL, se escribe el fichero de georreferenciación, de nombre como filename pero cambiando los tres últimos caracteres por "jgw".
int escribe_bmp16_geo(const char8_t *filename, const Bmp16FullHeader *head, Bitmap16 bm, const Matriz___Tierra *transf, const ZBounds *zbounds){
	int nret=escribe_bmp16(filename,head,bm);
	ifunlike(nret==BMP_WRITE_NOOPEN) return nret;

	//El fichero de texto con la georreferenciación
	if(transf!=NULL && zbounds!=NULL){
		char8_t filegeo[SHRT_PATH];
		char8_t *ext=strpcpy8(filegeo,filename);
		*--ext='w'; *--ext='m'; *--ext='p';
		escribe_georref(filegeo,zbounds,transf,16);
	}
	return nret;
}
