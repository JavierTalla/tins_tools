#pragma once
#include <ATcrt/ATsystem.h>

/***     Generación de ficheros raster de bits     ***/

/* Estas estrcuturas emplean, además de uint, los tipos IO2char16 e IO2char8 porque están
pensadas para escribir a disco. Los datos alamacenados en los arrays con esos tipos son enteros
sin signo de 16-bits u 8-bits, y pueden recorrerse con un puntero de tipo uint16_t o uint8_t
en cualquier ordenador moderno que disponga de esos tipos.*/

/*El formato bmp exige tamaño de 4 bytes para cada fila. Por ello, en las matrices de tipo
IO4char8, que almacenan datos de 8-bit, podrá haber un hueco de 1 a 3 bytes al final de cada
fila cuando npx sea impar. Para las matrices de tipo IO2char16 el hueco será de 1 char16.

nuints: número de IO4char8/IO2char16 reservados*/

typedef struct{
	uint npx, npy;
	uint nuints;
	IO4char8 *values;
} Bitmap8;

typedef struct{
	uint npx, npy;
	uint nuints;
	IO2char16 *values;
} Bitmap16;

/*Calcula bm.nuints a partir de bm.npx y bm.npy, que ya tienen que estar asignados
Además, para cada fila del mapa de bits guarda
en 'n4fila':	El número de uints de cada fila
en 'resto':	El número de bytes (8-bits) con datos que incluye el byte medio-lleno del final de cada fila.

Si el valor guardado en resto es 0, no hay último byte medio-lleno, y cada fila consta de n4fila bytes.
Si resto!=0, cada fila consta de n4fila bytes, el último de los cuales tiene 'resto' bytes con datos y
4-'resto' de relleno.
*/
#define bmnuints8___npxy(bm,n4fila,resto) \
	n4fila=NUINTS___NCHARS8((bm).npx); resto=(bm).npx&3;\
	(bm).nuints=n4fila*(bm).npy;

#define bmnuints16___npxy(bm,n4fila,resto) \
	n4fila=NUINTS___NCHARS16((bm).npx); resto=(bm).npx&1;\
	(bm).nuints=n4fila*(bm).npy;

#define getpos_uint(b,c,n4fila) (b)*n4fila+((c)>>2)
#define getpos_byte(b,c) ((c)&3)

typedef uint color;
#define Яcolor Я
