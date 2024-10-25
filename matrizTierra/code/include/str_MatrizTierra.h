#pragma once
/***     Una matriz npx x npy con datos de elevaciones     ***/

//Tipo de dato para los valores de altura de los puntos sobre la Tierra.
//El máximo valor que puede aparecer en los datos es 32766.
typedef sint16m EarthHeight;
typedef uint16m uEarthHeight;
#define UEARTHHEIGHT_BITS 16
#define MAX_UEARTHHEIGHT 0xFFFF //65535 = 8192*8-1
#define MAX_EARTHHEIGHT 0x7FFF

/* Estructura que define el sistema de Z de los elementos almacenados en una MatrizTierra
Los valores almacenados estarán en el intervalo [escala*(zmin-offset), escala*(zmax-offset)].
offset<=zmin. Es decir, ningún valor de z en el terreno es menor que offset.*/
typedef struct{
	//Los dos valores necesarios para la transformación
	EarthHeight offset;	//Los puntos que se almacenan con z cero tienen este valor de z en la realidad, en metros.
	u8int escala;		//1= metros. >1, el multiplicador. E.g., 4=cuartos de metro.
	//Estadísticas.
	//Todos los valores son en metros en el sistema local (esto es, ignorando el Z0 de MatrizTierra).
	EarthHeight zmin, zmax;	//z mín y z máx encontradas so far.
	//Máximos y mínimos del terreno, sin fondos de mar y lagos ni edificios, pero sí con las láminas de agua.
	struct{EarthHeight zmin, zmax;} tierra;
	uEarthHeight maxdp; //Máxima profundidad de un mar o lago.
} ZBounds;

//Flags con info. sobre cada punto de la matriz
#define MATRIZ_MAX_NLAGO ((1U<<14)-2)
typedef uint16m uLago_t; //Un tipo de dato suficiente para MATRIZ_MAX_NLAGO+1

typedef struct{
	unsigned lago: 14; //Número de lago
	unsigned edif: 1; //1 si es un punto de edificio. Equivale a cielo!=suelo
	unsigned: 1;
	unsigned: 15;
	unsigned fuera: 1; //El punto está fuera del mapa. El valor de z almacenado no tiene sentido.
} flagMDT;

#define flag_is_water_table(flags) ((flags).lago!=0)

/*En la estructura InterpretaciónMatriz:
flags: Cada píxel de la matriz se corresponde con un valor de flags. Esta flag almacena, entre otra
		información, el número de lago. Si es =0 significa que no es punto de lago. Este valor sirve
		de índice para los arrays gruposlago y cotaslagos.
gruposlago, cotaslagos: Estos arrays tienen datos hasta el elemento nláminas = máx{flags[i].lago}.
		El valor de gruposlago[flag.lago] sirve de índice para lagos. Con el algoritmo actual se
		cumple gruposlago[i]>=i.
lagos: Tiene datos hasta el elemento nláminas. Puede tener elementos vacíos por el medio. Se
		cumple lagos[gruposlago[i].cota]=cotaslago[i].  0<=i<npuntos.

Puede eliminarse algún lago tras haber dejado todos listos. En ese caso los elementos correspondientes
en gruposlago y cotaslagos no se cambian. Se pueden detectar porque gruposlago apupnta a un lago
(en 'lagos') vacío.
*/
typedef struct strLagoZ{
	uEarthHeight cota; //de la superficie, en el sistema de Z de la matriz.
	uEarthHeight depth; //En las unidades de los valores de Z de la matriz.
	uint superficie; //En píxeles
	uint supe_edif; //Superficie "edificada": Barcos, edificios, pantalanes...
} LagoZ;

typedef struct{
	flagMDT *flags;  //Si el valor del uint es cero el píxel no tiene nada de especial.
	uLago_t *gruposlago; //[nláminas+2]. El primer elemento está vacío. Un valor 0 indica el final
	uEarthHeight *cotaslagos; //[nláminas+2]. El primer elemento está vacío. Un valor MAX_UEARTHHEIGHT indica el final
	LagoZ *lagos; //[nláminas+2]. El primer elemento está a {0,0}. Un valor con cota MAX_UEARTHHEIGHT y depth =0
						//indica un elemento vacío. Un valor con cota y depth MAX_UEARTHHEIGHT indica el final.
} InterpretaciónMatriz;

/*Macros para los elementos del array LagoZ de InterpretaciónMatriz*/

#define GRUPOSLAGO_CIERRE 0
#define COTASLAGO_CIERRE MAX_UEARTHHEIGHT
//Mirar si es el elemento que cierra el array
#define LAGOZ_end(l) ((l).depth==MAX_UEARTHHEIGHT)
//Se entiende que cuando se emplea esta macro ya se sabe que l no es el lago que cierra el array
#define LAGOZ_is_empty(l) ((l).cota==MAX_UEARTHHEIGHT)

#define FLAG_MATRIZ_HAYTIERRA 1
#define FLAG_MATRIZ_HAYAGUA 2
#define FLAG_MATRIZ_HAYFONDOMAR 4 //Implica HAYAGUA. Equivale a zbounds.maxdp!=0 y a algún dplago>0.
#define FLAG_MATRIZ_HAYFUERA 8 //Fuera de la Tierra o de la zona pedida

typedef struct{
	uint nlagos; //Cada lago puede componerse de varias láminas que se tocan
	uint nláminas; //Cada lago puede componerse de varias láminas que se tocan
	uint ncomponentes; //Cada lámina está formada por un núcleo que puede crecer incorporando otras componentes
	uint nislas; //Número total de islas en los lagos
	umint flags; //Una combinación de las tres flags definidas arriba
} EstadísticasMatriz;

/* MatrizTierra: matriz de una cierta área de la tierra.
	pixel, px: El paso de la matriz es px. Pero para las decisiones que dependen del tamaño de píxel se emplea 'píxel'.
		Por ejemplo, para seleccionar los datasets o decidir qué es lago.
	cielo y suelo: Valores de Z en un sistema definido por un ZBounds.
*/
typedef struct{
	float pixel; //Tamaño de píxel efectivo de la talla sobre el terreno, en metros.
	float px; //Paso de la matriz en metros. Si píxel > px significa que la matriz está sobrepixelada en relación a su precisión
	uint npx, npy;
	uint npuntos; //npx*npy
	float Z0;	//Un punto con un valor de z en la matriz tiene Z=Z0+(zbounds.offset+z/zbounds.esc) en metros, en el sistema global de alturas (GRSxx).
	ZBounds zbounds;
	uEarthHeight *cielo;
	uEarthHeight *suelo;
	InterpretaciónMatriz i;
	EstadísticasMatriz esta;
	uEarthHeight zmax_aguaborde; //Cota máxima de las láminas de agua que llegan hasta el borde. 0 si no hay ninguna.
} MatrizTierra;

#define MatrizTierra_superf(m) ((m)->cielo==NULL? (m)->suelo : (m)->cielo)

#define MatrizTierra_setNULL(m) do{\
	(m).cielo=(m).suelo=NULL;\
	(m).i.flags=NULL;\
	(m).i.cotaslagos=NULL;\
	(m).i.gruposlago=NULL;\
	(m).i.lagos=NULL;}while(0)

//Si freeif no está definido habrá de definirse como algo así:
//#define freeif(x) if((x)!=NULL) free(x)
//o simplemente como free(x)
#define freeifMatrizTierra(m) do{\
	freeif((m).suelo);\
	freeif((m).cielo);\
	freeif((m).i.flags);\
	freeif((m).i.cotaslagos);\
	freeif((m).i.gruposlago);\
	freeif((m).i.lagos);\
}while(0)

#define free_null_if_MatrizTierra(m) do{\
	free_null_if((m).suelo);\
	free_null_if((m).cielo);\
	free_null_if((m).i.flags);\
	free_null_if((m).i.cotaslagos);\
	free_null_if((m).i.gruposlago);\
	free_null_if((m).i.lagos);\
}while(0)

/*Macros para la estrcutura ZBounds de MatrizTierra*/

//Pasa de un valor en el terreno en metros al valor almacenado en la matriz
#define MATRIZ_STORED___GROUND(z,zb) (uEarthHeight)((uint8m)((zb).escala)*(uEarthHeight)((z)-((zb).offset)))
//zb es zbounds. Devuelve el valor almacenado en matriz para un valor z=0 en el terreno (el nivel del mar)
#define MATRIZ_STORED___0(zb) MATRIZ_STORED___GROUND(0,zb)
//zb es zbounds. Los valores devueltos por estas macros abarcan todo lo almacenado en matriz.
//Si se quiere exactamente los mínimos y máximos almacenados, recórrase la matriz . La diferencia, si existe, se debe
//solamente al redondeo al dividir: los valores de zb están en metros, los de matriz pueden estar con más resolución.
#define MATRIZ_MIN_STORED(zb) MATRIZ_STORED___GROUND((zb).zmin,zb)
#define MATRIZ_MAX_STORED(zb) MATRIZ_STORED___GROUND((zb).zmax,zb)
//Sin contar los fondos de lagos y mares, pero sí su superficie
#define MATRIZ_MIN_STORED_NOFONDO(zb) MATRIZ_STORED___GROUND((zb).tierra.zmin,zb)
//El terreno sin fondos ni edificios
#define MATRIZ_MAX_STORED_TIERRA(zb) MATRIZ_STORED___GROUND((zb).tierra.zmax,zb)

//Pasan de un valor almacenado en la matriz al valor en el terreno en metros
#define MATRIZ_GROUND___STORED_ROUNDUP(z,zb) ((EarthHeight)(((z)+(zb).escala-1)/(zb).escala)+(zb).offset)
#define MATRIZ_GROUND___STORED_ROUNDDOWN(z,zb) ((EarthHeight)((z)/(zb).escala)+(zb).offset)
