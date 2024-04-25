#pragma once
#include "str_clips.h"

/* Lee el archivo de área de clip 'file'. Este puede a su vez requerir la apertura de otros archivos.
El área se compone de uno o varios polígonos que se guardan en vec.

file: El archivo a leer
vec: Donde se guardarán los polígonos leídos
log: Puede ser NULL. Fichero a donde escribir el log.

Return:
	0: Todo bien
	AT_NOMEM: Eso. Toda la memoria que se hubiera reservado se libera
	<0 (i. a. AT_NOMEM): Error al intentar abrir algún fichero.

Si devuelve <0 no ha reservado ninguna memoria (o la que hubiera reservado la ha liberado).
Los polígonos siempre está cerrados, e. d., el último punto es igual que el primero.

La función no devuelve ningún polígono vacío: Todos los elementos de vec->ppio a
vec->next-1 son polígonos de al menos tres vértices, con .next - .ppio >=4.
*/
int lee_clip(const char8_t *file, Growing_Polígono_xy *vec, Bufferto8 *log);

/* Lee el contenido de s como si fuera el contenido de un archivo. V lee_clip() para más
información.

s: Cadena de texto a leer
vec: Donde se guardarán los polígonos leídos
log: Puede ser NULL. Fichero a donde escribir el log.

Return:
	0: Todo bien
	AT_NOMEM: Eso. Toda la memoria que se hubiera reservado se libera

Si la cadena s quiere incluir otro fichero y la ruta es relativa esta se tomará respecto
al "current path" del proceso, que será el del archivo ejecutable. Probablemente no es
lo que se desee. Esta función no está pensada para ello, pero se puede incluir.
*/
int lee_clip_string(const char8_t *s, Growing_Polígono_xy *vec, Bufferto8 *log);

/* Aplica un clip a una matriz rectangular.

state: La martriz
npx, pny: Dimensiones de la matriz
clip: Apunta a un array de poligonos (o a uno solo)
npols: Número de polígonos del array 'clip'

Pone a 1 los elementos de state que estén dentro de algún polígono. Los elementos a 1
que ya existieran no los modifica.
*/
void aplica_clip(umint *state, uint npx, uint npy, Polígono_xy *clip, u8int npols);
