BmpRead read_bmphead(const uint *buf){
	BmpRead bm;
	BmpFileHeader *fhead;
	BmpInfoHeader *ihead;

	fhead=(BmpFileHeader*)buf;
	ihead=(BmpInfoHeader*)((char*)buf+3*usizeof(uint)+usizeof(uint16_t));
	bm.planes=get_bytes01(&ihead->planes_bpp);
	bm.bpp=get_bytes23(&ihead->planes_bpp);
	bm.npx=ihead->wd;
	bm.btop=isneg(ihead->ht);
	if(bm.btop) bm.npy=-ihead->ht;
	else bm.npy=ihead->ht;
	if(bm.bpp==8){bmnuints8___npxy(bm,bm.n4fila,bm.resto);}
	elif(bm.bpp==16){bmnuints16___npxy(bm,bm.n4fila,bm.resto);}
	else{bm.resto=bm.n4fila=0;}
	bm.puntos=(uint*)((char*)buf+*(uint*)((char*)fhead+10));

	return bm;
}
