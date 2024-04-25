/*Calcula la transformación y asigna rect->npx, rect->npy, rect->wd y rect->ht,
en base a rect->pix_x y rect->pix_y, que no son modificables, respetando al menos
una de las parejas {wd,npx}, {ht,npy}.
bgirar indica si se permite girar 90º el área de la Tierra para que ajuste mejor a la tabla*/
void calcula_matriz___tierra(const uint npx, const uint npy, const RECT_ΛΦ *geográficas, Matriz___Tierra *transf, bint bgirar);

/*Calcula una transformacion basándose simplemente en las dimensiones de la talla, ignorando toda opción de la talla.
No se trata de una función aproximada sino que es un resultado defiitivo, que se aplicará cuando no
se va a tallar, sino simplemente generar un MDT.

Parameters:

geográficas: Zona de la Tierra que se desea
rem: Rectángulo con npx y npy.
bgirar: Si se permite un giro de 90º para ajustar mejor a las proporciones de la 'rem'.
bajustar: Si se ampliará la matriz para que se ajuste a las dimensiones de 'rem'.

Return:
	0: Todo bien
	MATRIZ_TOO_SMALL
	MATRIZ_TOO_BIG

Si bajustar es false, uno de transf->npx o transf->npy será menor que rem.npx y rem.npy (salvo que coincida exactamente).
transf->cc.npφ y transf->cc.npλ tendrán los valores ajustados al rectángulo 'geográficas' +- 1píxel.
*/
#define MATRIZ_TOO_SMALL 2		//npx o npy <2
#define MATRIZ_TOO_BIG 3			//npx*npy excede MATRIZ_MAX_PIXCOUNT

int calcula_transf_plani_bb(const uint npx, const uint npy, const RECT_ΛΦ *geográficas, Matriz___Tierra *transf, bint bgirar, bint bajustar);

static inline int calcula_transf_plani(const uint npx, const uint npy, const RECT_ΛΦ *geográficas, Matriz___Tierra *transf, bint bgirar){
	return calcula_transf_plani_bb(npx,npy,geográficas,transf,bgirar,false);
}
static inline int calcula_y_ajusta_transf_plani(const uint npx, const uint npy, const RECT_ΛΦ *geográficas, Matriz___Tierra *transf, bint bgirar){
	return calcula_transf_plani_bb(npx,npy,geográficas,transf,bgirar,true);
}
static inline int ajusta_dimensiones_Tierra(uint wd, uint ht, const RECT_ΛΦ *geográficas, Matriz___Tierra *transf){
	return calcula_transf_plani_bb(wd,ht,geográficas,transf,true,true);
}

//Expande ex píxeles por cada lado de x y ey por cada lado de y
void expande_MatrizTierra(Matriz___Tierra *transf, uint ex, uint ey);
//iz, de: El número de píxeles a añadir por la izquierda y por la derecha
void expande_MatrizTierra_x(Matriz___Tierra *transf, uint iz, uint de);
//ab, ar: El número de píxeles a añadir por la abajo y por arriba
void expande_MatrizTierra_y(Matriz___Tierra *transf, uint ab, uint ar);

/* Aplicar el giro de 90º a un rectángulos */

//Aplica a rect un giro de 90º en sentido horario. Es el que aplica
//una Matriz___Tierra si b90 es true al pasar de la Tierra a la proyección
static inline void girarect_proy___tierra(Extremos2D_fl *rect){
	float aux=rect->mx;
	rect->mx=rect->my;
	rect->my=-rect->MX;
	rect->MX=rect->MY;
	rect->MY=-aux;
}

//Aplica a rect un giro de 90º en sentido antihorario. Es el que aplica
//una Matriz___Tierra si b90 es true al pasar de la proyección a la Tierra
static inline void girarect_tierra___proy(Extremos2D_fl *rect){
	float aux=rect->mx;
	rect->mx=-rect->MY;
	rect->MY=rect->MX;
	rect->MX=-rect->my;
	rect->my=aux;
}

Puntoxy_float matriz___tierra_planocuadra1(Puntoxy_float p, const Matriz___Tierra *transf, bint besquina);
Puntoxy_float matriz___tierra_sistema1(Puntoxy_float p, const Matriz___Tierra *transf, bint besquina);
static inline Puntoxy_float matriz___tierra1(Puntoxy_float p, const Matriz___Tierra *transf, bint besquina){
	if(transf->tipo==TIPO_PROY_PlanoCuadra) return matriz___tierra_planocuadra1(p,transf, besquina);
	return matriz___tierra_sistema1(p,transf, besquina);
}

//Return 0, AT_NOMEM
void matriz___tierra_planocuadra(Puntoxy_float *p, uint n, const Matriz___Tierra *transf, bint besquina);
int matriz___tierra_sistema(Puntoxy_float *p, uint n, const Matriz___Tierra *transf, bint besquina);
static inline int matriz___tierra(Puntoxy_float *p, uint n, const Matriz___Tierra *transf, bint besquina){
	if(transf->tipo==TIPO_PROY_PlanoCuadra){matriz___tierra_planocuadra(p,n,transf, besquina); return 0;}
	return matriz___tierra_sistema(p,n,transf, besquina);
}
