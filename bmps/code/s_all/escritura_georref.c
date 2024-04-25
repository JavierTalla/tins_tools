int escribe_georref(const char8_t *filename, const ZBounds *zbounds, const Matriz___Tierra *transf, uint8m bpp){
	Bufferto8 buf;
	int nret;
	uEarthHeight Δh;
	float pixwd;

	if(transf->tipo!=TIPO_PROY_PlanoCuadra) return 0;
	ifnzunlike(nret=toopen_utf8(&buf,filename)) return nret;
	Δh=MATRIZ_MAX_STORED(*zbounds);
	Δh-=MATRIZ_MIN_STORED(*zbounds)-1;
	pixwd=(float)(1U<<bpp);

	setbuf_absolute(&buf);
	buf.prec.absol=7;
	if(!transf->b90) towritef(&buf,"%f\n0\n0\n%f\n",transf->cc.pix_λ,-transf->cc.pix_φ);
	else towritef(&buf,"0\n%f\n%f0\n",transf->cc.pix_φ,transf->cc.pix_λ);
	buf.prec.absol=5;
	towritef(&buf,"%f\n%f\n",transf->info.x0,transf->info.y0);
	buf.prec.absol=2;
	towrite_float(&buf,zbounds->zmin/*+Z0*/); toput_char(&buf,'\n'); //Por lo general se quieren las alturas en el sistema "local".
	towrite_float(&buf,(float)Δh/pixwd); toput_char(&buf,'\n');
	toclose(&buf);

	return buf.error_code;
}
