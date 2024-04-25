#include <ATcrt/ATmem.h>
#include <ATcrt/ATarrays.h>
#include <pmsqrt.h>

/*	Las funciones de esta librería tienen que decidir qué constituye una lámina de agua. Será un conjunto
de píxeles contiguos con la misma cota, sin ningún punto en su interior a una cota menor y con ningún
punto adyacente a una cota menor. Esto último se relajará para permitir algunos puntos a cota menor (los
lagos tienen desagües).
	Hay dos valores de corte para decidir qué es lámina de agua: M y m. El núcleo de una lámina de agua,
el bloque principal, ha de ser una superficie de píxeles contiguos de al menos M píxeles. Los bloques que
toquen al principal en una esquina (satélites) serán también lámina de agua, incondicionalmente.
Si un satélite tiene al menos m píxeles transmite la condición de lámina de agua a sus satélites, y así
sucesivamente; si es menor que m píxeles la lámina se acaba ahí.
	Estos valores dependen del tamaño del píxel efectivo de la talla en relación al tamaño de píxel de los
datos de origen. A menor píxel de talla, es decir, más detalle, se exige un mínimo de píxeles contiguos
mayor. Se tomarán los valores de MinWaterTablePixels[i], siendo i el primer valor para el cual step sea
>= MinWaterTablePixels[i].step10. Este valor de step es el cociente entre el píxel efectivo de la talla y
el de los datos de origen en dirección NS (para STRM30m, 1"). Pero si es <1 se tomarán los valores para 1,
divididos por f*f. */
static const struct{
	float step;
	uint M;
	uint m;
} MinWaterTablePixels[3]={
	{1.2f, 35, 5},
	{1.0f, 45, 6},
	{0, 600, 8},
};
#define MinWaterTablePixels_Index1 1 //Posición del elemento con step = 1.0
