#include <ATcrt/ATcrt_types.h>
#include <ATcrt/ATvisibility.h>
#define true 1

#ifndef COMPILING_TINS
#define tins_api set_visibility(imported)
#define TINS_API VISIBILITY_BLOCK(imported)
#else
#define tins_api set_visibility(exposed)
#define TINS_API VISIBILITY_BLOCK(exposed)
#endif

TINS_API
#include "str_tin.h"

//Libera el contenido de una estructura TIN y deja los punteros a NULL y los tamaños a 0
void free_TIN(TIN *tin);

//Libera el contenido de un TINPlano.
void free_TINplano(TINPlano* tin);

/*Lectura de un fichero de formato tin con datos sin32, etc.

ftin: El nombre del fichero a leer
tin: Estructura que se rellenará

Return:
	0: Todo bien
	AT_NOMEM: No hay memoria para escribir el fichero
	Otro <0: Error al abrir el fichero*/
	#define FILEREAD_BADFORMAT 1

/*Si el valor devuelto es !=0, en la estructura tin los punteros no apuntan a nada. Cualquier
memoria que se hubiera podido reservar se habrá liberado.*/
int lee_fichero_tin(const char8_t *ftin, TIN *tin);

//Escribe un fichero de formato tin
int escribe_tin(const char8_t *ftin, const TIN *tin);
int escribe_tinplano(const char8_t *ftin, const TINPlano *tin);

/*Escribe un fichero de formato tin a partir de unos puntos y triángulos (tin) y cierta información adicional

ftin: El nombre del fichero a generar
tin: La información geométrica, así como la clase a la que pertenece cada triángulo. También incluye las unidades.
	Estas se emplean simplemente para escribir en la cabecera del fichero.
estilos: Para los triángulos. Puede ser NULL
nestilos: Cuantos elementos del array colores han de escribirse. Puede ser 0.

Return:
	0: Todo bien
	AT_NOMEM: No hay memoria para escribir el fichero
	1-4: Otro error al intentar crear el fichero
	>4: Se produjo algún error durante la escritura del fichero
*/
int escribe_fichero_tin(const char8_t *ftin, const TINMalla *tin, const EstiloTriángulos1 *estilos, uint nestilos);

/*Escribe un fichero de formato tin a partir de unos puntos y triángulos (tin)

ftin: El nombre del fichero a generar
tin: La información geométrica, así como la clase a la que pertenece cada triángulo

Return:
	0: Todo bien
	AT_NOMEM: No hay memoria para escribir el fichero
	1-4: Otro error al intentar crear el fichero
	>4: Se produjo algún error durante la escritura del fichero
*/
static inline int escribe_fichero_tin_simple(const char8_t *ftin, const TINMalla *tin){
	return escribe_fichero_tin(ftin,tin,NULL,0);
}

//Aplana un tin
int tinplano___tin(TINPlano *plano, const TIN *tin);

//Escribe un fichero stl a partir de un tin plano
int fstl___tinplano(const char8_t *fstl, const TINPlano *tin);

int lee_fichero_stl(const char8_t *ftin, TINPlano *tin);
/*Escribir un fichero stl
fstl:	Nombre del fichero a escribir
tin:	tin. Sus unidades no se emplean. En su lugar se emplea el parámetro funi.
funi:	Valor por el que multiplicar todas las coordenadas al pasarlas al fichero. Si no se quiere, pásese 1.0f.
color_default:	El que se escribira en la cabecera del stl. En la forma (R<<16) | (G<<8) | B.
colores:			Puede ser NULL. Si no es NULL ha de tener (al menos) tantos elementos como el
	máximo elemento de 'class' en los triángulos del tin. Los colores han de estar en la forma de los stl:
	0R(5)G(5)B(5); e.d., el bit alto a 0 y siguen 5 bits por color.

Return:
	0: Todo bien
	AT_NOMEM: No hay memoria para escribir el fichero
	1-4: Otro error al intentar crear el fichero
	>4: Se produjo algún error durante la escritura del fichero
*/
int escribe_fichero_stl1(const char8_t *fstl, const TINMalla *tin, float funi, uint color_default, const uint16m *colores);
int escribe_fichero_stl2(const char8_t *fstl, const TINMalla *tin, float funi, uint color_default, const uint16m *colores);


//Multiplica todo el tin por esc
void reescala_tin(TINMalla* tin, float esc);
//Escala los valores de Z
void reescalaZ_tin(TINMalla* tin, float esc);
//Desplaza en Z
void desplazaZ_tin(TINMalla* tin, ssint Δz);

//Cómo asignar la clase de un triángulo según la de sus vértices
typedef enum{Tclass___Vclass_min, Tclass___Vclass_max, Tclass___Vclass_2min, Tclass___Vclass_2max,
					Tclass___Vclass_3func, Tclass___Vclass_func} Tclass___Vclass;

typedef uint16m (*tclass_from_vclasses)(umint f1, umint f2, umint f3);

/*Genera una estructura tin en memoria a partir de una matriz de datos sint16m

	tin: Apunta a una estuctura tin vacía. Se rellenará
	matriz: La matriz con datos de Z a partir de la cual generar el tin.
	npx,npy: Dimensiones de la matriz
	pix: Paso de píxel en la matriz, en las unidades del tin. Por ejemplo un 6 indica '6 unidades'.
			Si no se sabe qué pasar, pásese 1.
	bcompacto: Si se generará un fichero más compacto. La diferencia está en las diagonales de los
			cuadrados de la malla que se escogen para la triangulación. Si bcompacto es true se tomará
			siempre la misma, lo que permite reducir el tamaño del fichero. Si es false, el programa
			tomará para cada cuadrado la diagonal que considere mejor.
	clases: La clase a la que pertenece cada punto. El programa lo emplea para asignar un número de
			clase a cada triángulo. Si se pasa NULL se asigna la clase 0 a todos los triángulos.
	kak_tclass: Si clases no es NULL, indica cómo asignar la clase del tríangulo según la de sus vértices:
		Tclass___Vclass_min: El mínimo de los tres vértices
		Tclass___Vclass_max: El máximo
		Tclass___Vclass_2min: Si hay dos iguales, esa; si no, el mínimo
		Tclass___Vclass_2max: Si hay dos iguales, esa; si no, el máximo
		Tclass___Vclass_3func: Llamando a la función func si no son las tres iguales
		Tclass___Vclass_func: Llamando a la función func siempre
	func: Si kak_tclass es Tclass___Vclass_2max u Tclass___Vclass_func, la clase de cada triángulo se determina
		llamando a la función func.

Return values:
	0: Todo bien
	AT_NOMEM		//No hay memoria para generar el tin

	La estructura TINMalla incluye un campo TinUnidad por si se quiere asignar luego. La función pone
que las undades son metros.
	Si npx<2 o npy<2 establece los punteros de tin a NULL y devuelve sin hacer nada. Si la función devuelve
AT_NOMEM se libera lo que se hubiera reservado (e.d., como si no hubiera hecho nada). En caso contrario
reserva memoria que habrá que liberar, p.e. con TINMalla_free_null(*tin).
*/
int tin___matriz16(TINMalla *tin, const sint16m *matriz, uint pix, uint npx, uint npy, bint bcompacto, const uint8m *clases, Tclass___Vclass kak_tclass, tclass_from_vclasses func);

/*Genera una estructura tin en memoria a partir de una matriz de datos sint16m

	tin: El tin, que la función rellenará.
	matriz: La matriz, con npx x npy valores de Z.
	pix: Paso de píxel en la matriz, en las unidades del tin. Por ejemplo un 6 indica '6 unidades'.
			Si no se sabe qué pasar, pásese 1.
	npx,npy: Dimensiones de la matriz

Return values:
	0: Todo bien
	AT_NOMEM		//No hay memoria para generar el tin

	La estructura TINMalla incluye un campo TinUnidad por si se quiere asignar luego. La función pone
que las undades son metros.
	Si npx<2 o npy<2 establece los punteros de tin a NULL y devuelve sin hacer nada. Si la función devuelve
AT_NOMEM se libera lo que se hubiera reservado (e.d., como si no hubiera hecho nada). En caso contrario
reserva memoria que habrá que liberar, p.e. con TINMalla_free_null(*tin).
*/
static inline int tin___matriz16_plain(TINMalla *tin, const sint16m *matriz, uint pix, uint npx, uint npy){
	return tin___matriz16(tin,matriz,pix,npx,npy,true,NULL,0,NULL);
}

/*Genera una estructura tin en memoria a partir de una matriz de datos sint16m
Como tin___matriz16 pero incluye el parámetro mask. V. descripción de tin___matriz16.

	mask: Los puntos con mask=0 se excluyen del tin generado.

Return values:
	0: Todo bien
	AT_NOMEM		//No hay memoria para generar el tin
*/
int tin___matriz16_mask(TINMalla *tin, const sint16m *matriz, uint pix, uint npx, uint npy, const bool8 *mask, const uint8m *clases, Tclass___Vclass kak_tclass, tclass_from_vclasses func);

/* Genera una estructura tin en memoria a partir de una matriz de datos bien sint16m.

	Como tin___matriz16 (vedi) pero con el parámetro ngrueso a mayores.
	ngrueso: Indica cuántas posiciones de la matriz así como del tablero hay que avanzar
			     para generar cada triángulo del tin. Es decir, es un factor de zoom-.

Return values:
	0: Todo bien
	AT_NOMEM		//No hay memoria para generar el tin

	La estructura TINMalla no almacena unidades de ningún tipo. Es adimensional, en sentido literal.
	Si npx<ngrueso+1 o npy<ngrueso+1 la funcion establece los punteros de tin a NULL y devuelve sin hacer
nada. En caso contrario reserva memoria para tin->p_indiv.ppio y tin->triángulos.ppio. Si el valor devuelo
es 0 la memoria habrá que liberarla. Si se devuelve !=0, los punteros tin->p_indiv.ppio y tin->triángulos.ppio
quedarán a NULL.
	El tin resultante tendrá ntx=[(npx-1)/ngrueso] filas y nty=[(npy-1)/ngrueso] columnas. Es decir, ntx xnty
cuadraditos, formado cada uno por dos triángulos. El píxel de ese tin será pix x ngrueso.
*/
int tingrueso___matriz16_size(TINMalla *tin, uint8m ngrueso, const sint16m *matriz, uint8m size, uint pix, uint npx, uint npy, bint bcompacto, const uint8m *clases, Tclass___Vclass kak_tclass, tclass_from_vclasses func);
static inline int tingrueso___matriz16(TINMalla *tin, uint8m ngrueso, const sint16m *matriz, uint pix, uint npx, uint npy, bint bcompacto, const uint8m *clases, Tclass___Vclass kak_tclass, tclass_from_vclasses func){
	return tingrueso___matriz16_size(tin,ngrueso,matriz,1,pix,npx,npy,bcompacto,clases,kak_tclass,func);
}

/*Genera una estructura tin en memoria a partir de una matriz de datos sint16m

	tin: El tin, que la función rellenará.
	ngrueso: La función creará el tin tomano uno de cada 'ngrueso' puntos de la matriz.
	matriz: La matriz, con npx x npy datos.
	pix: Paso de píxel en la matriz, en las unidades del tin. Por ejemplo un 6 indica '6 unidades'.
	npx,npy: Dimensiones de la matriz

Return values:
	0: Todo bien
	AT_NOMEM		//No hay memoria para generar el tin

	La estructura TINMalla no almacena unidades de ningún tipo. Es adimensional, en sentido literal.
	Si npx<ngrueso+1 o npy<ngrueso+1 la funcion establece los punteros de tin a NULL y devuelve sin hacer
nada. En caso contrario reserva memoria para tin->p_indiv.ppio y tin->triángulos.ppio. Si el valor devuelo
es 0 la memoria habrá que liberarla. Si se devuelve !=0, los punteros tin->p_indiv.ppio y tin->triángulos.ppio
quedarán a NULL.
	El tin resultante tendrá ntx=[(npx-1)/ngrueso] filas y nty=[(npy-1)/ngrueso] columnas. Es decir, ntx xnty
cuadraditos, formado cada uno por dos triángulos. El píxel de ese tin será pix x ngrueso.
*/
static inline int tingrueso___matriz16_plain(TINMalla *tin, uint8m ngrueso, const sint16m *matriz, uint pix, uint npx, uint npy){
	return tingrueso___matriz16_size(tin,ngrueso,matriz,1,pix,npx,npy,true,NULL,0,NULL);
}

VISIBILITY_BLOCK_END
