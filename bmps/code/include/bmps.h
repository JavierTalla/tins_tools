#pragma once
#include <ATcrt/ATsystem.h>
#include "str_raster.h"

/* Estructuras para la escritura de bmps */

//Por culpa de los dos bytes del principio va todo decalado.
//El matriz_offset es desde el principio del fichero
typedef struct{
	uint16_t BM;
	uint filesize;
	uint unused;
	uint matrix_offset; //0x42 (0x44) si no hay tabla de color
} BmpFileHeader;

typedef struct{
	uint headersize; //0x28
	uint wd;
	uint ht; //Si es >=0, van de abajo hacia arriba. si es <0, van de arriba hacia abajo
	uint planes_bpp; //(nbits<<16)|1
	uint compression; //BI_BITFIELDS (3) Indica que siguen RGB bit masks
	uint imagesize; //En bytes
	uint ppmeter_x;
	uint ppmeter_y;
	uint ncolors; //En la paleta. 0 si no hay
	uint ignored; //nº de colores importantes (0)
} BmpInfoHeader;

typedef struct{
	uint R, G, B;
} BmpMasks;

//Estas dos no sirven para leer porque en el fichero, el tamaño
//de BmpFileHeader no es múltiplo de 4 bytes.
typedef struct{
	BmpFileHeader f;
	BmpInfoHeader i;
	uint table[256]; //Color table
} Bmp8FullHeader;

typedef struct{
	BmpFileHeader f;
	BmpInfoHeader i;
	BmpMasks m;
} Bmp16FullHeader;

/*i0, i1: Índices dentro de table de donde a donde se quiere que se escriban los colores calculados.
Si queremos aprovechar toda la tabla se pasará i0=0, i1=255.*/
typedef struct strTablaOpts{
	color fondo;
	color frente;
	uint8m i0, i1;
	bint nolineal; //True si se quiere no_lineal
	color *specials_first; //Si !=NULL, lista de colores para el principio, cerrada con Яcolor
	color *specials_last; //Si !=NULL, lista de colores para el final, cerrada con Яcolor
} TablaOpts;

#define COL_NEGRO ((color)0)
#define COL_BLANCO ((color)0xFFFFFF)
#define TablaOptsNB (TablaOpts){COL_NEGRO,COL_BLANCO,0,255, 0,NULL,NULL}
#define TablaOptsBN (TablaOpts){COL_BLANCO,COL_NEGRO,0,255, 0,NULL,NULL}

/*Array de pares (posición/color), entre los cuales se interpolarán linealmente colores
Para crear la tabla de color para el bitmap. Además, puede haber una serie de colores inciales,
specials_first, para aplicar a los valores 0,1,2... y una de colores finales, para aplicar a los
valores 255, 254, 253...

specials_first y specials_last: Pueden ser NULL. Si no lo es, es una lista de colores cerrada
	por Яcolor.
ncols: Número de pares color/posición.
pcols: Lista de ncols colores.
ppos: Lista de de ncols posiciones, de 0.0 a 1.0.

Los colores se interpolarán para el rango de valores que quedan una vez extraídos los valores
correspondientes a los colores de specials_first y specials_last. Por ejemplo, si el primero de
estos especifica 3 colores y el segundo 2, el intervalo [0, 1] de las ppos se aplica al intervalo
[3, 253] de posiciones de la tabla de color.
*/
typedef struct strTabladeColor{
	const color *specials_first;
	uint8m ncols;
	const color *pcols;
	const float *ppos;
	const color *specials_last;
} TabladeColor;

/* Estructuras para la lectura de bmps */

//Estructura para leer un bitmap sin tabla de color
typedef struct{
	bint btop; //0: almacenado de abajo a arriba. 1: de arriba a abajo
	uint16m planes;
	uint16m bpp; //bits per pixel
	uint npx, npy;
	uint n4fila, resto;
	uint nuints;
	uint *puntos; //Apunta a los datos.
} BmpRead;


/* Funciones de escritura */

#define BMP_WRITE_NOOPEN 1 //No se puedo abrir el archivo para escribir

//Rellena el objeto Bmp8FullHeader a partir de npx, npy y la tabla de color.
void headerb8_colores(Bmp8FullHeader *head, uint npx, uint npy, const TablaOpts *colores);

//Rellena el objeto Bmp8FullHeader a partir de npx, npy y un array de colores.
//ncolors ha de ser <=256. Si no se cumple esto el programa toma los 256 primeros colores de tabla.
void headerb8_tabla(Bmp8FullHeader *head, uint npx, uint npy, const uint* tabla, uint ncolors);

//Rellena el objeto Bmp8FullHeader a partir de npx, npy y la tabla de color.
void headerc8(Bmp8FullHeader *head, uint npx, uint npy, const TabladeColor* tabla);

/*Escribe en buf un fichero bmp de 8 bits, con tabla de color.
Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero para escribirlo
	Un código de error en la escritura
*/
int escribe_bmp8(const char8_t *filename, const Bmp8FullHeader *head, Bitmap8 bm);


/* Escritura de un bitmap pasando el bitmap y opciones para el color */

/*Escribe el fichero bmp con una tabla de color que es un gradiente desde un color
de 'fondo' a un color de 'frente', pudiendo a mayores asignar un color particular a
un valor 'especial'.

tcolor: Incluye los colores de fondo, frente y el valor/color especial. Esta última estructura
	no es posible omitirla; si no se quiere ninguno especial pásese {0,fondo} ahí.
tcolor->blineal: true si se quiere que la escala del color no sea lineal. Va cambiando
	más rápido en valores oscuros y más lentamente en valores claros.

Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero bmp para escribirlo
	Un código de error en la escritura
*/
static inline int escribe_bn8(const char8_t *filename, Bitmap8 bm, const TablaOpts *tcolor){
	Bmp8FullHeader head;
	headerb8_colores(&head,bm.npx,bm.npy,tcolor);
	return escribe_bmp8(filename,&head,bm);
}

/*Escribe el fichero bmp con una tabla de color.

tabla: La tabla de color
ncolors: Número de elmentos en 'tabla'

Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero bmp para escribirlo
	Un código de error en la escritura
*/
static inline int escribe_bn8_tabla(const char8_t *filename, Bitmap8 bm, const uint* tabla, uint ncolors){
	Bmp8FullHeader head;
	headerb8_tabla(&head,bm.npx,bm.npy,tabla,ncolors);
	return escribe_bmp8(filename,&head,bm);
}

/*Escribe el fichero bmp con una tabla de color predefinida

tb_color: El número de la tabla de color, del array TabladeColor Tablas[NTABLAS], comenzando en 1.

Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero bmp para escribirlo
	Un código de error en la escritura
*/
static inline int escribe_c8(const char8_t *filename, Bitmap8 bm, const TabladeColor *tabla){
	Bmp8FullHeader head;
	headerc8(&head,bm.npx,bm.npy,tabla);
	return escribe_bmp8(filename,&head,bm);
}


//Rellena el objeto Bmp16FullHeader a partir de npx, npy y la máscara para los colores.
void headerc16(Bmp16FullHeader *head, uint npx, uint npy, const BmpMasks *col);

/*Escribe en buf un fichero bmp de 16 bits
Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero para escribirlo
	Un código de error en la escritura
*/
int escribe_bmp16(const char8_t *filename, const Bmp16FullHeader *head, Bitmap16 bm);

/* Escritura de un bitmap pasando el bitmap y opciones para el color */

/*Escribe el fichero bmp en color de 16bits

Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero bmp para escribirlo
	Un código de error en la escritura
*/
static inline int escribe16(const char8_t *filename, Bitmap16 bm, const BmpMasks *col){
	Bmp16FullHeader head;
	headerc16(&head,bm.npx,bm.npy,col);
	return escribe_bmp16(filename,&head,bm);
}


/* Funciones de lectura */

BmpRead read_bmphead(const uint *buf);
