#define HUSO 30
#define strHUSO u8"30"

//Devuelve 0 si se puede rellenar con ceros, 1 en caso contrario.
bint zerofill(ssint x,ssint X,ssint y,ssint Y){
	if(x>=107900 && (Y<4132000 || y>=3977000)) return 0; //Sur: Atlántico y Medi. al N del estrecho
	if(x>=500000 && Y<4670000) return 0; //Bloque SE
	if(x>=1000000 && Y<4716500) return 0; //Esquina NE
	if(y>=4690000 && X<598000) return 0; //Cantábrico. Bloque NO
	if(X<30620 && y>=4665330) return 0; //Esquina SO de Galicia, 1.
	if(X<12730 && y>=4634000) return 0; //Esquina SO de Galicia, 2.
	return 1;
}
