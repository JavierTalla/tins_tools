/*Crea una tabla de color con n elementos interpolando entre colores fijos dados.
Params:
	ncol: Número de colores fijos dados
	b: Los colores fijos
	pos: Las posiciones en un intervalo [0 - 1] de los colores fijos (v. infra)
	n: Número de colores de la tabla a generar
	tabla: La tabla donde se escribirán los colores calculados

    El número de colores fijos (ncol) ha de ser >=1. Si es 0 la función vuelve inmediatamente sin hacer nada.
Si es 1 la tabla se rellena con todos los valores iguales al único color fijo. Si es >=2 se llevará a cabo una
interpolación de ncol-1 tramos. El tramo i-ésimo es una progresión lineal entre los colores b[i-1] y b[i].
    Las posiciones de pos indican dónde, en el intervalo de n valores de la tabla a generar, se sitúan los
colores fijos. Un 0 indica el primer color; un 1, el último; un 0'5, a la mitad, etc. Los valores han de ir
en aumento y estar todos comprendidos en [0 - 1]. Si el primer valor no es 0 la función rellenará la tabla
con el color b[0] hasta la posición correspondiente a pos[0]. Si el último valor no es 1, la función rellenará
con b[ncol-1] desde la ultima posición hasta el final de la tabla.
*/
void interpola_tc32(uint8m ncol, const color b[/*static ncol*/], const float pos[/*static ncol*/], uint n, uint tabla[/*static n*/]){
	ifunlike(ncol==0) return;
	ifunlike(ncol==1){
		const color c=b[0];
		durchlaufep(uint,tabla,n) *p=c;
		return;
	}

	const float step=1.0f/(float)(n-1);
	color c2=ncol;
	float f2=0;
	uint pos2=0;
	uint *ptabla=tabla;
	Durchlaufe2(const color,b,ncol,const float,pos){
		color c1=c2;
		float f1=f2;
		c2=*ptr++;
		f2=*ptr_b++;
		ifunlike(f2==f1) continue;
		uint pos1=pos2;
		pos2=(uint)ceilf(f2*(n-1)); //pos2 no llega a entrar
		if(pos2==pos1) continue;

		if(c1==c2){
			dontimes(pos2-pos1,) *ptabla++=c1;
			continue;
		}

		//Componentes del color de pos1 e incrementos a las componentes
		struct{float x,y,z;} E, δE;
		{
		float _f2f1=1.0f/(f2-f1);
		float fd=pos1*step-f1; //Diferencia del punto pos1 respecto a f1
		fd*=_f2f1; //Pasado a la escala del intervalo [f1 - f2]
		float δf=step*_f2f1;
		float fc=1.0f-fd;
		float c,d;
		c=(float)(c1&0xFF);			d=(float)(c2&0xFF);			E.x=fc*c + fd*d + 0.5f;				δE.x=δf*(d-c);
		c=(float)(c1&0xFF00);		d=(float)(c2&0xFF00);		E.y=fc*c + fd*d + (float)0x80;		δE.y=δf*(d-c);
		c=(float)(c1&0xFF0000);	d=(float)(c2&0xFF0000);	E.z=fc*c + fd*d + (float)0x8000;	δE.z=δf*(d-c);
		}
		dontimes(pos2-pos1,ptabla++){
			*ptabla= (uint)E.z&0xFF0000 | (uint)E.y&0xFF00 | (uint)E.x;
			E.x+=δE.x;	E.y+=δE.y;	E.z+=δE.z;
		}
	}

	uint *pend=&tabla[n];
	if(ptabla!=pend){
		while(ptabla!=pend) *ptabla++=c2;
	}
}
