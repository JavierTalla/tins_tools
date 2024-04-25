//Devuelto por las funciones que tienen que devolver uno de los LAGOKIND si se quedan sin memoria
#define MARCA_NOMEM 0xFFFF

#define BORDE_IZDO	1U
#define BORDE_DCHO	2U
#define BORDE_SUPOR	4U
#define BORDE_INFOR	8U
#define BORDE_TODOS	15U //Para píxeles aislados
#define BORDE_SACO_IZDO 13U
#define BORDE_SACO_DCHO 14U
#define BORDE_DCHO_INFOR (BORDE_DCHO|BORDE_INFOR)
#define BORDE_IZDO_SUPOR (BORDE_IZDO|BORDE_SUPOR)
#define BORDE_has_TODOS(x) (((x)&BORDE_TODOS)==BORDE_TODOS)

#define LAGOKIND_NO (1U<<4)		//Flag de NO. Un polígono que se ha creado pero se vio que no es un lago
#define LAGOKIND_PEQ (2U<<4)		//Es pequeño (nº pix < mwater)
#define LAGOKIND_MEDIO (3U<<4)	//Es un cuerpo medio (mwater <= nº píx. < Mwater)
#define LAGOKIND_GRANDE (4U<<4) //Es un cuerpo principal de agua (Mwater <= nº pix)
#define LAGOKIND_TODOS (0x70U)
#define TABLE_SIGNALLED (8U<<4)	//Marca efímera para identificar el píxel de partida al ir recorriendo el borde de un polígono
#define TABLE_elimina_marcas(x) (x)&=TABLE_SIGNALLED|BORDE_TODOS|Special(BORDE_TODOS);
#define TSTATE_marca_AGUA(x)	((x)&LAGOKIND_TODOS)
#define TSTATE_is_GRANDE(x)	(TSTATE_marca_AGUA(x)==LAGOKIND_GRANDE)
#define TSTATE_is_NO(x)			(((x)&LAGOKIND_NO)==LAGOKIND_NO)
#define TSTATE_is_MED_OR_PEQ(x)	((TSTATE_marca_AGUA(x)&(2U<<4))!=0)
#define TSTATE_remove_WATER(x)	(x)&=~LAGOKIND_TODOS

//Para cuando en un mismo píxel es necesario distinguir el borde exterior
//del un borde interior del mismo lago
#define SPECIAL_IZDO		(1U<<8)
#define SPECIAL_DCHO		(2U<<8)
#define SPECIAL_SUPOR	(4U<<8)
#define SPECIAL_INFOR		(8U<<8)
#define Special(b) ((b)<<8)

/*Variables de estado para ir recorriendo el borde.

DirState: Una variable de este tipo, s, guardará la dirección desde la que hemos llegado al píxel en el que estamos. Por ejemplo,
	si hemos llegado desde abajo s valdrá UP. Mientras se está en un píxel indica la direccion hacia la que se mira (bien el siguiente
	píxel en esa dirección, bien el borde en esa posición), y va cambiando hasta que no cambia más, y entonces indica la dirección
	en la que se va a salir.
Δz: A recorrer un polígono hay que llevar la cuenta de la posición en vertical en relación al comienzo, para calcular el área. Para ello
	hay que tomar el borde superior del píxel de comienzo como z=1. Δz[s] es la variación en z correspondiente a recorrer un borde en
	la dirección s.
PixelOffset: El correspondiente a s. Los números 100 y -100 que aparecen en su inicialización habrán de reemplazarse por npx y -(int)npx.
		Por tanto NPO[s] es la primera opción para continuar; si no, NPO[s+1], NPO[s+2] y por último NPO[s+3], que consiste en volver
	por donde veníamos. El cero de NPO[7] está por si se pretende recorrer el borde de una región de un único píxel, para que el bucle
	termine. Terminará porque ptr[NPO[7]]==*ptr siempre es cierto.
Borde: El borde en la posición s: Si s=UP, Borde=BORDE_SUPOR, etc.
Multiplicador: Contribución al área del polígono del borde que acabamos de añadir al píxel en el que estamos (v. supra). Los movimientos
	verticales (UP y DOWN) no contribuyen; RIGHT contribuye +=z y LEFT contribuye -=z, porque recorremos el polígono en sentido
	horario. Este valor de z es el que se mantiene actualizado continuamente gracias a Δz.
NextEstado(s): Obtiene la dirección en la que hay que emplezar a mirar en función de la dirección en que se llegó al píxel en el que estamos.
	Es siempre hacia la izquierda respecto a la dirección de llegada.

	Cuando se lleva la cuenta de un valor de z, este es siempre el de una esquina concreta del píxel en el que se está. Por ejemplo, si se
mira hacia LEFT, se está en la esquina inferior izquierda del píxel. Si tras esta mirada se decide salir del píxel en esa dirección, la esquina
en que se está sigue siendo la misma, que ahora es la inferior derecha del píxel al que se ha llegado. Si por el contrario se pasa a mirar
UP (lo siguiente que toca), se interpreta que el borde izquierdo del píxel se ha recorrido de abajo hacia arriba, y se pasa a estar en la
esquina superior izquierda. En consonancia con esto Δz[LEFT]=+1. Un borde izquierdo se recorre de abajo hacia arriba; uno superior,
de izquierda a derecha, etc.
*/

enum DirState{UP=0,RIGHT,DOWN,LEFT}; //Sea = s
static const struct{
	ssint ddz[6];	//Δz da error (error de la implementación del preprocesador)
	uint8m Borde[6];
	ssint Multiplicador[6];
} strBordear={
	{0,-1,0,1,0,-1}, //Δz al recorrer el borde Borde[s]
	{BORDE_SUPOR,BORDE_DCHO,BORDE_INFOR,BORDE_IZDO,
	  BORDE_SUPOR,BORDE_DCHO}, //Borde[s]
	{1,0,-1,0,1,0}, //Multiplicador correspondiente a Borde[s]; de la z para ir calculando el área
};
static int PixelOffset[8]={-100,1,100,-1,-100,1, 100, 0};

#define NextEstado(s) ((s+3)&3) //s-1 (mod 4). Se corresponde con PixelOffset
 #define npxi (int)(NPX)
#define Set_PixelOffset(NPX) PixelOffset[0]=-npxi, PixelOffset[2]=npxi, PixelOffset[4]=-npxi, PixelOffset[6]=npxi

#define Δz strBordear.ddz
#define Borde strBordear.Borde
#define Multiplicador strBordear.Multiplicador
//#define PixelOffset strBordear.PixelOffset

//Estructura empleada para guardar los lagos grandes
typedef struct strLago{
	uint ind; //Posición de un píxel del borde desde el principio de bwater/pmatriz
	uint internos; //Índice dentro del vector de regiones. Ahí empiezan y una con pb=NULL marca el final
	uLago_t klago; //Para las láminas grandes se dejará a 0. Se decide al final. Las medianas heredan el de su grande
} Lago;

//Definición del tipo "vector de Lagos" El tipo es Vector_Lago
defineVector(Lago)


/* Hay funciones que crean un borde de lago, un borde de isla y un borde de hueco. Todas ellas
van creando el borde desde dentro del elemento en cuestion (lago, isla o hueco). Las funciones
que crean los bordes especifican la condición que complen los puntos de la región respecto a
una z cen concreto: 'cota'. Así, _eq_ indica que el borde delimitará una región con z=cota
(lag); _gr_, que z> cota (isla), y _less_, que z< cota (aujero).
    Las que incluyen _izdo_ en su nombre empiezan en un píxel cuyo lado izquierdo es parte del borde
a crear; es decir, en un píxel cuyo píxel a la izquierda está fuera de la región. Comienzan a recorrer el
borde en la esquina superior izquierda de ese píxel y terminan en cuanto vuelven a dicho punto de inicio,
trass haber recorrido, de abajo a arriba, el lado izquierdo del píxel original.
    Las que incluyen _dcho_ en su nombre empiezan en un píxel cuyo lado derecho es parte del borde
a crear; es decir, en un píxel cuyo píxel a la derecha está fuera de la región. Comienzan a recorrer el
borde en la esquina inferior derecha de ese píxel y terminan en cuanto vuelven a dicho punto de inicio,
tras haber recorrido, de arriba a abajo, el lado derecho del píxel original.
    Todas reciben pb y ptr apuntando al píxel de comiendo. Así, en las de _izdo_ ptr-1 está fuera
mientras que en las de _dcho_ ptr+1 está fuera.
    Las que devuelven el área de la región devuelven >0 si trata de un borde exterior; <0 si de un borde interior.
    Las que reciben el parámetro MARCA aplican además esa marca a cada píxel con borde.*/


/** Funciones de creación/modificación de bordes **/

ssint crea_borde_eq_izdo(uint16m *pb, const uEarthHeight *ptr){
	const uEarthHeight cota=*ptr;

	*pb|=TABLE_SIGNALLED;
	ssint área=0;		//Nos situamos en la esquina superior izquierda del píxel de comienzo
	ssint z=1;			//como si hubiésemos llegado desde la izquierda
	uint8m s=RIGHT;
	ptr--, pb--; //La primera iteración del do{} los llevará a su sitio
	do{
		ptr+=PixelOffset[s]; //Siguiente píxel
		pb+=PixelOffset[s];
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]!=cota){
			*pb|=Borde[s];
			área+=Multiplicador[s]*z;
			z+=Δz[s++];
		}											//Se termina cuando se escribe el borde IZDO del píxel de comienzo
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || !(*pb&BORDE_IZDO));
	*pb&=~TABLE_SIGNALLED;
	return área-((*pb&BORDE_SUPOR)!=0); //Si el píxel de partida tiene borde superior este se recorre dos veces
}

//Como la anterior, pero crea un borde "especial". Destruye el primitivo, si es que ya existía.
void hacer_especial_izdo(uint16m *pb, const uEarthHeight *ptr){
	const uEarthHeight cota=*ptr;

	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	ptr--, pb--; //La primera iteración del do{} los llevará a su sitio
	do{
		ptr+=PixelOffset[s]; //Siguiente píxel
		pb+=PixelOffset[s];
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]!=cota){
			uint16m b=Borde[s++];
			*pb&=~b; *pb|=Special(b); //Quitar el normal si lo había y crear el especial
		}											//Se termina cuando se escribe el borde IZDO del píxel de comienzo
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || !(*pb&SPECIAL_IZDO));
	*pb&=~TABLE_SIGNALLED;
}

//La inversa de la anterior
void hacer_normal_izdo(uint16m *pb, const uEarthHeight *ptr){
	const uEarthHeight cota=*ptr;

	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	ptr--, pb--; //La primera iteración del do{} los llevará a su sitio
	do{
		ptr+=PixelOffset[s]; //Siguiente píxel
		pb+=PixelOffset[s];
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]!=cota){
			uint16m b=Borde[s++];
			*pb&=~Special(b); *pb|=b; //Quitar el especial si lo había y crear el normal
		}											//Se termina cuando se escribe el borde IZDO del píxel de comienzo
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || !(*pb&BORDE_IZDO));
	*pb&=~TABLE_SIGNALLED;
}

ssint crea_borde_gr_izdo(uint16m *pb, const uEarthHeight *ptr, const uEarthHeight cota){
	uint16m * const ppio=pb;
	ssint área=0;
	ssint z=1;
	uint8m s=RIGHT;
	ptr--, pb--;
	do{
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]<=cota){
			*pb|=Borde[s];
			área+=Multiplicador[s]*z;
			z+=Δz[s++];
		}											//Se termina cuando se escribe el borde IZDO del píxel de comienzo
	}while(_likely(pb!=ppio) || !(*pb&BORDE_IZDO));
	return área-((*pb&BORDE_SUPOR)!=0);
}

ssint crea_borde_less_izdo(uint16m *pb, const uEarthHeight *ptr, const uEarthHeight cota){
	uint16m * const ppio=pb;
	ssint área=0;
	ssint z=1;
	uint8m s=RIGHT;
	ptr--, pb--;
	do{
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]>=cota){
			*pb|=Borde[s];
			área+=Multiplicador[s]*z;
			z+=Δz[s++];
		}											//Se termina cuando se escribe el borde IZDO del píxel de comienzo
	}while(_likely(pb!=ppio) || !(*pb&BORDE_IZDO));
	return área-((*pb&BORDE_SUPOR)!=0);
}


/*Las funciones de borde derecho pueden estarse ejecutando dentro de un lago a la vez que en él
se está recorriendo el borde, así que no empleamos SIGNALLED. Serviría un SIGANLLED_2,
pero así es más limpio*/
void crea_borde_eq_dcho(uint16m *pb, const uEarthHeight *ptr, uint16m MARCA){
	const uEarthHeight cota=*ptr;

	uint16m * const ppio=pb; //Nos situamos en la esquina inferior derecha
	uint8m s=LEFT;			//del píxel, como si hubiésemos llegado desde la derecha
	ptr++, pb++; //La primera iteración del do{} los llevará a su sitio
	do{
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
		*pb|=MARCA;
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]!=cota) *pb|=Borde[s++];		//Se termina cuando se escribe el borde DCHO del píxel de comienzo
	}while(_likely(pb!=ppio) || !(*pb&BORDE_DCHO));
}

ssint crea_borde_eq_dcho_área(uint16m *pb, const uEarthHeight *ptr){
	const uEarthHeight cota=*ptr;

	uint16m * const ppio=pb;
	ssint área=0;
	ssint z=0;			//Nos situamos en la esquina inferior derecha
	uint8m s=LEFT;	//del píxel, como si hubiésemos llegado desde la derecha
	ptr++, pb++; //La primera iteración del do{} los llevará a su sitio
	do{
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]!=cota){
			*pb|=Borde[s];
			área+=Multiplicador[s]*z;
			z+=Δz[s++];
		}													//Se termina cuando se escribe el borde DCHO del píxel de comienzo
	}while(_likely(pb!=ppio) || !(*pb&BORDE_DCHO));
	return área;
}

void crea_borde_leq_dcho(uint16m *pb, const uEarthHeight *ptr, const uEarthHeight cota, uint16m MARCA){
	uint16m * const ppio=pb;
	uint8m s=LEFT;
	ptr++, pb++;
	do{
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
		*pb|=MARCA;
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]>cota) *pb|=Borde[s++];		//Se termina cuando se escribe el borde DCHO del píxel de comienzo
	}while(_likely(pb!=ppio) || !(*pb&BORDE_DCHO));
}


/** Funciones de destrucción de bordes y marcas **/

//Elimina solamente las marcas de borde. Emplear esta función solamente si sabemos
//que los píxeles no tienen ninguna marca a mayores
void destruye_borde_izdo(uint16m *pb){
	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	pb--;
	do{pb+=PixelOffset[s];
		s=NextEstado(s);
		while(*pb&Borde[s]) *pb&=~Borde[s++];		//Se termina cuando se elimina el borde IZDO del píxel de comienzo
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || (*pb&BORDE_IZDO));
	*pb&=~TABLE_SIGNALLED;
}

void destruye_bordemarca_izdo(uint16m *pb){
	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	pb--;
	do{pb+=PixelOffset[s];
		s=NextEstado(s);
		while(*pb&Borde[s]) *pb&=~Borde[s++];		//Eliminar las marcas solo tras haber eliminado todos los bordes, porque
		if((*pb&BORDE_TODOS)==0) TABLE_elimina_marcas(*pb); //el píxel puede ser común a otro borde del mismo lago
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || (*pb&BORDE_IZDO)); //Se termina cuando se elimina el borde IZDO del píxel de comienzo
	*pb&=~TABLE_SIGNALLED;
}

//Elimina solamente las marcas de borde. Emplear esta función solamente si sabemos
//que los píxeles no tienen ninguna marca a mayores
void destruye_borde_dcho(uint16m *pb){
	uint16m * const ppio=pb;
	uint8m s=LEFT;
	pb++;
	do{pb+=PixelOffset[s];
		s=NextEstado(s);
		while(*pb&Borde[s]) *pb&=~Borde[s++];		//Se termina cuando se elimina el borde DCHO del píxel de comienzo
	}while(_likely(pb!=ppio) || (*pb&BORDE_DCHO));
}

//No señala pb
void destruye_bordemarca_dcho(uint16m *pb){
	uint16m * const ppio=pb;
	uint8m s=LEFT;
	pb++;
	do{pb+=PixelOffset[s];
		s=NextEstado(s);
		while(*pb&Borde[s]) *pb&=~Borde[s++];		//Se termina cuando se elimina el borde DCHO del píxel de comienzo
		if((*pb&BORDE_TODOS)==0) TABLE_elimina_marcas(*pb); //el píxel puede ser común a otro borde del mismo lago
	}while(_likely(pb!=ppio) || (*pb&BORDE_DCHO));
}


/** Funciones que marcan **/

//Indica si, al llegar a un píxel con borde izquierdo, vamos a salir sin siquiera mirarlo.
//s (mod 4) puede ser DOWN, LEFT o UP.
#define PIXEL_IZDO_Continuar(pb,s) (  ((s&=3) && 0)\
		|| (s==DOWN && (*(pb)&BORDE_DCHO_INFOR)!=BORDE_DCHO_INFOR)\
		|| (s==LEFT && (*(pb)&BORDE_INFOR)!=BORDE_INFOR)\
	)

//Indica si,al llegar a un píxel con borde derecho, vamos a salir sin siquiera mirarlo.
//s (mod 4) puede ser UP, RIGHT o BOTTOM.
#define PIXEL_DCHO_Continuar(pb,s) (  ((s&=3) && 0)\
		|| (s==UP && (*(pb)&BORDE_IZDO_SUPOR)!=BORDE_IZDO_SUPOR)\
		|| (s==RIGHT && (*(pb)&BORDE_SUPOR)!=BORDE_SUPOR)\
	)

#define PIXEL_IZDO_Continuar_Cota(ptr,s,cota) (  ((s&=3) && 0)\
		|| (s==DOWN && (ptr[PixelOffset[DOWN]]==cota || ptr[PixelOffset[RIGHT]]==cota))\
		|| (s==LEFT && (ptr[PixelOffset[DOWN]]==cota))\
	)

#define PIXEL_DCHO_Continuar_Cota(ptr,s,cota) (  ((s&=3) && 0)\
		|| (s==UP && (ptr[PixelOffset[UP]]==cota || ptr[PixelOffset[LEFT]]==cota))\
		|| (s==RIGHT && (ptr[PixelOffset[UP]]==cota))\
 )

void remarca_izdo(uint16m *pb, uint16m MARCA){
	if(BORDE_has_TODOS(*pb)){
		TSTATE_remove_WATER(*pb); *pb|=MARCA;
		return;
	}

	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	do{TSTATE_remove_WATER(*pb); *pb|=MARCA;
		s=NextEstado(s);
		while(*pb & Borde[s]) s++;
		pb+=PixelOffset[s];
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || PIXEL_IZDO_Continuar(pb,s));
	*pb&=~TABLE_SIGNALLED;
}

//Añade la marca a lo que hubiera.
//MARCA no puede emplear los bits de borde
void marca_izdo(uint16m *pb, uint16m MARCA){
	if(BORDE_has_TODOS(*pb)){*pb|=MARCA; return;}

	uint16m * const ppio=pb;
	uint8m s=RIGHT;
	do{*pb|=MARCA;
		s=NextEstado(s);
		while(*pb & Borde[s]) s++;
		pb+=PixelOffset[s];
	}while(_likely(pb!=ppio) || PIXEL_IZDO_Continuar(pb,s));
}

//Esta no se emplea procesando el lago exterior
void remarca_dcho(uint16m *pb, uint16m MARCA){
	if(BORDE_has_TODOS(*pb)){
		TSTATE_remove_WATER(*pb); *pb|=MARCA;
		return;
	}

	*pb|=TABLE_SIGNALLED;
	uint8m s=LEFT;
	do{TSTATE_remove_WATER(*pb); *pb|=MARCA;
		s=NextEstado(s);
		while(*pb & Borde[s]) s++;
		pb+=PixelOffset[s];
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || PIXEL_DCHO_Continuar(pb,s));
	*pb&=~TABLE_SIGNALLED;
}

/** Funciones que se mueven dentro del lago buscando un borde izquierdo **/

//pb apunta a un píxel con borde izdo. Devuelve un borde derecho a la misma altura que pb
//y más a la izquierda que este, o pb si estamos dentro de un borde exterior (área>0) y ya no
//hay borde en la misma línea más a la izquierda
uint16m* get_borde_dcho(uint16m *pb){
	if(BORDE_has_TODOS(*pb)) return pb;
	const uint NPX=PixelOffset[DOWN];

	uint16m *ppio=pb;
	uint8m s=RIGHT;
	do{ s=NextEstado(s);
		while(*pb & Borde[s]) s++;
		pb+=PixelOffset[s];
		if(pb<ppio && (pdif)(ppio-pb)<NPX){ //Estamos en la misma fila. Mirar que el borde izquierdo
			if((*pb&BORDE_DCHO) && !PIXEL_DCHO_Continuar(pb,s)) break; //sea del borde que estamos recorriendo
		}
	}while(_likely(pb!=ppio) || PIXEL_IZDO_Continuar(pb,s));
	return pb;
}

//Obtiene un píxel con borde izquierdo del borde exterior de la región
//pb puede venir apuntando a cualquier punto del interior de la región, no necesariamente con borde.
uint16m* get_borde_leftmost(uint16m *pb){
	if(BORDE_has_TODOS(*pb)) return pb;
	uint16m *pold;
	do{while(!(*pb&BORDE_IZDO)) pb--; //Ir al primer borde que nos encontremos a la altura de donde estemos
		pold=pb;
		pb=get_borde_dcho(pb);
	}while(pb!=pold); //Aún podemos seguir más hacia la izquierda
	return pold;
}

//pb apunta a un píxel con borde izquierdo. Va recorriendo el borde buscando un pixel a la
//izquierda del inicial y a su misma altura con borde derecho. Si no lo hay devuelve 1 (bad);
//si lo hay devuele is_bad_dcho() de ese píxel.
bint is_bad_izdo(uint16m *pb, const uEarthHeight *ptr);

/*pb apunta, desde dentro del lago, a un píxel con borde derecho, de un supuesto borde interior.
El borde exterior de lago tiene que estar con marca; todos los interiores, sin marca (salvo en
los píxeles en los que coincidan con un borde exterior, en cuyo caso tendrá la marca debida a
este último.
    Va hacia la izquierda desde pb hasta encontrar un píxel bien a *ptr!=cota bien con
borde izquierdo. En el primer caso devuelve 1 (bad). En el segundo, si el borde tiene
marca, y por tanto es el exterior del lago, devuelve 0. Si no tiene marca se trata de otro
borde interior y se devuelve el carácter bad o good de ese borde.*/
bint is_bad_dcho(uint16m *pb, const uEarthHeight *ptr){
	const uEarthHeight cota=*ptr;
	while(*ptr==cota && !(*pb&BORDE_IZDO)) ptr--, pb--;
	if(*ptr!=cota) return 1;
	ifnz(*pb&LAGOKIND_TODOS) return 0;
	return is_bad_izdo(pb,ptr);
}
bint is_bad_izdo(uint16m *pb, const uEarthHeight *ptr){
	uint16m *pold=pb;
	if((pb=get_borde_dcho(pb))==pold) return 1; //bad. Estamos encerrados dentro del borde de pb.
	return is_bad_dcho(pb,ptr-(pold-pb));
}


/** Funciones que destruyen lo que hubiera dentro **/

//El borde exterior tiene que estar sin marca. Los interiores tienen que tener todos marca.
//y no pueden tener píxeles en común con el exterior. Por tanto, no puede emplearse esta
//función para destruir los bordes interiores de un lago. Sirve para destruir lagos, huecos
//o islas interiores al propio lago, pues esos bordes estarán a otra cota o a la misma del lago
//pero en ese caso aisladas por regiones a otra cota.
void destruye_interior(uint16m *pb, const uEarthHeight *ptr){
	if(BORDE_has_TODOS(*pb)) return;

	//Buscar lo que hay dentro. Eliminarlo (de momento)
	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	do{
		if(*pb & BORDE_IZDO){
			const uEarthHeight *ptr2=ptr;
			uint16m *pb2=pb; //Este mismo punto no hace falta comprobarlo
			while(!((*pb2 & BORDE_DCHO) && (*pb2&LAGOKIND_TODOS)==0)){
				ptr2++, pb2++;
				if((*pb2&LAGOKIND_TODOS) && (*pb2&BORDE_IZDO)){
					destruye_bordemarca_izdo(pb2);
				}
		}	}
		s=NextEstado(s);
		while(*pb & Borde[s]) s++;
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || PIXEL_IZDO_Continuar(pb,s));
	*pb&=~TABLE_SIGNALLED;
}

//Cambiar esta función para que destruya sólo los bordes interiores del lago en el que se está
void destruye_bordes_interiores(uint16m *pb, const uEarthHeight *ptr){
	if(BORDE_has_TODOS(*pb)) return;

	const uEarthHeight cota=*ptr;
	hacer_especial_izdo(pb,ptr); //Convierte los bordes en especiales

	//Buscar lo que hay dentro. Eliminarlo (de momento)
	*pb|=TABLE_SIGNALLED;
	uint8m s=RIGHT;
	do{
		if(*pb & SPECIAL_IZDO){
			const uEarthHeight *ptr2=ptr;
			uint16m *pb2=pb; //Este mismo punto no hace falta comprobarlo
			while(!(*pb2&SPECIAL_DCHO)){
				ptr2++, pb2++;
				//if(*pb2&BORDE_IZDO){ //Acabamos de entrar en un lago en uno de los huecos del lago que estamos procesando
					//uint16m *pb3=rodear_izdo(pb2);
					//ptr2+=(pb3-pb2); pb2=pb3; // ++ en la siguiente iteración del bucle while
				//}else
				if(*pb2&BORDE_DCHO) destruye_bordemarca_dcho(pb2);
		}	}
		s=NextEstado(s);
		while(*pb & Special(Borde[s])) s++;
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
	}while(_likely(!(*pb&TABLE_SIGNALLED)) || PIXEL_IZDO_Continuar_Cota(ptr,s,cota));
	*pb&=~TABLE_SIGNALLED;

	hacer_normal_izdo(pb,ptr); //Convierte los bordes en normales
}


/** Funciones que remarcan el borde con un número de lago **/
//Destruyen las marcas que hubiera. Se distinguen de otro píxeles por tener el píxel alto a 1: macro TSTATE_has_LAGO

#define max_nlago 0x7FFE //Número máximo de lago. 0xFFFF (= 0x8000|0x7FFF) se emplear para detectar el final de fila
#if max_nlago > MATRIZ_MAX_NLAGO
	#undef max_nlago
	#define max_nlago MATRIZ_MAX_NLAGO
#endif

//Mientras se procesan los lagos para expandirlos es cuando más bits hacen falta
//Los ocho más altos no se han empleado para nada. De esos, los 4 bajos se emplearán
//Para señalar el borde de los satélites mientras se están procesando.
#define LAGO_get_n(x)			((x)&0x7FFF)
#define TSTATE_has_LAGO(x)	(((x)&0x8000)!=0) //Si un píxel está marcado con número de lago (y entonces, además, sus bits no se pueden interpretar como flags)
#define Num_LAGO_BORDE(n) (0x8000|(n))

//Marca los píxeles con Num_LAGO_BORDE(klago)
void crea_marcalago_izdo(uint16m *pb, const uEarthHeight *ptr, uint16m klago){
	const uEarthHeight cota=*ptr;
	klago=Num_LAGO_BORDE(klago);

	uint16m * const ppio=pb;
	uint8m s=RIGHT;
	do{ *pb=klago;
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]!=cota) s++;
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
	}while(_likely(pb!=ppio) || PIXEL_IZDO_Continuar_Cota(ptr,s,cota));
}

//Marca los píxeles con Num_LAGO_BORDE(klago)
void crea_marcalago_dcho(uint16m *pb, const uEarthHeight *ptr, uint16m klago){
	const uEarthHeight cota=*ptr;
	klago=Num_LAGO_BORDE(klago);

	uint16m * const ppio=pb;
	uint8m s=LEFT;
	do{ *pb=klago;
		s=NextEstado(s);
		while(ptr[PixelOffset[s]]!=cota) s++;
		ptr+=PixelOffset[s];
		pb+=PixelOffset[s];
	}while(_likely(pb!=ppio) || PIXEL_DCHO_Continuar_Cota(ptr,s,cota));
}
