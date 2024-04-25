#include "bmps.h"
#include "../../extern_libs/include/matrizTierra/Matriz___Tierra.h"
#include "../../extern_libs/include/matrizTierra/str_MatrizTierra.h"

/*Escribe en buf un fichero bmp de 8 bits, con tabla de color.
Si transf y zbounds no son NULL, se escribe el fichero de georreferenciación, de
nombre como filename pero cambiando los tres últimos caracteres por "jgw".
Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero bmp para escribirlo
	Un código de error en la escritura

El posible error en la escritura del fichero de georreferenciación se ignora.
*/
int escribe_bmp8_geo(const char8_t *filename, const Bmp8FullHeader *head, Bitmap8 bm, const Matriz___Tierra *transf, const ZBounds *zbounds);

/* Escritura de un bitmap pasando el bitmap, opciones para el color y, opcionalmente, información para georreferenciación */

/*Escribe el fichero bmp con una tabla de color que es un gradiente desde un color
de 'fondo' a un color de 'frente'. v. escribe_bn8 para más detalles y el valor devuelto.

transf y zbounds: Si no son NULL escribe un archivo de georreferenciación.
El posible error en la escritura del fichero de georreferenciación se ignora.
*/
static inline int escribe_bn8_geo(const char8_t *filename, Bitmap8 bm, const Matriz___Tierra *transf, const ZBounds *zbounds, const TablaOpts *tcolor){
	Bmp8FullHeader head;
	headerb8_colores(&head,bm.npx,bm.npy,tcolor);
	return escribe_bmp8_geo(filename,&head,bm,transf,zbounds);
}

/*Escribe el fichero bmp con una tabla de color.

tabla: La tabla de color
ncolors: Número de elmentos en 'tabla'
transf y zbounds: Si no son NULL escribe un archivo de georreferenciación

Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero bmp para escribirlo
	Un código de error en la escritura

El posible error en la escritura del fichero de georreferenciación se ignora.
*/
static inline int escribe_bn8_tabla_geo(const char8_t *filename, Bitmap8 bm, const Matriz___Tierra *transf, const ZBounds *zbounds, const uint* tabla, uint ncolors){
	Bmp8FullHeader head;
	headerb8_tabla(&head,bm.npx,bm.npy,tabla,ncolors);
	return escribe_bmp8_geo(filename,&head,bm,transf,zbounds);
}

/*Escribe el fichero bmp con una tabla de color predefinida

tb_color: El número de la tabla de color, del array TabladeColor Tablas[NTABLAS]
transf y zbounds: Si no son NULL escribe un archivo de georreferenciación

Return:
	0: todo bien
	BMP_WRITE_NOOPEN: No se pudo abrir el fichero bmp para escribirlo
	Un código de error en la escritura

El posible error en la escritura del fichero de georreferenciación se ignora.
*/
static inline int escribe_c8_geo(const char8_t *filename, Bitmap8 bm, const Matriz___Tierra *transf, const ZBounds *zbounds, const TabladeColor *tabla){
	Bmp8FullHeader head;
	headerc8(&head,bm.npx,bm.npy,tabla);
	return escribe_bmp8_geo(filename,&head,bm,transf,zbounds);
}


int escribe_bmp16_geo(const char8_t *filename, const Bmp16FullHeader *head, Bitmap16 bm, const Matriz___Tierra *transf, const ZBounds *zbounds);

/* Escritura de un bitmap pasando el bitmap, opciones para el color y, opcionalmente, información para georreferenciación */

//Escribe el fichero bmp en color de 16bits
//transf y zbounds: Si no son NULL escribe un archivo de georreferenciación
static inline int escribe16_geo(const char8_t *filename, Bitmap16 bm, const Matriz___Tierra *transf, const ZBounds *zbounds, const BmpMasks *col){
	Bmp16FullHeader head;
	headerc16(&head,bm.npx,bm.npy,col);
	return escribe_bmp16_geo(filename,&head,bm,transf,zbounds);
}
