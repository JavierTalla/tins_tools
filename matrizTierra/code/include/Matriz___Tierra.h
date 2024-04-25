#pragma once
struct strMatriz___Tierra;
typedef struct strMatriz___Tierra Matriz___Tierra;

#include <ATsistemas/ClaseSistema.h>
/*Estructura para una transformación "PlanoCuadra": Un escalado de (λ,φ) (distintos para cada
una) y, opcionalamente, un giro de 90º.
λmin ... φmax: Rectángulo de coordenadas geográficas en el que encaja el área de la Tierra pedida.
*/
typedef struct{
	float λmin, φmin; //Bordes del rectángulo
	float λmax, φmax; //Tienen que encajar exactamente con pix_λ,npλ y pix_φ,npφ.
	float pix_λ, pix_φ;	//Tamaño del píxel en longitud y en latitud, sobre la Tierra. En grados.
	uint npλ, npφ; //Si b90 es false son npx, npy; si b90 es true son npy, npx.
	float cosφ0; //Factor de escala que se aplica a los valores de λ
	float _cosφ0; //Su inverso
	struct{
		float λmin, φmin;
		float λmax, φmax;
		float pix_λ, pix_φ;
	} radianes;
} PlanoCuadra;

/* Una estructura con información que no afecta al cálculo de la transformación */
//Si la proyección es PlanoCuadra x,y son valores de λ,φ
//En caso contrario son coordenadas (x,y) en el sistema sis correspondiente
typedef struct{
	float x0,y0; //Coordenadas del centro del píxel superior izquierdo. Si es PlanoCuadra son (λ,φ)
} PíxelesMatriz_Info;

/* Estructura que indica cómo pasar planimétricamente de la Tierra al plano.
npx, npy:		Rectángulo en el que encaja el área de la tierra transformada.
b90:			Si al paso de la Tierra a la matriz se añade, al final un giro de 90º. Este giro
				se entiende como parte de la proyección.
				Para proyecciones con el N arriba, si es true el W se muestra arriba y el N a la derecha.
rectProy		Coordenadas de las esquinas en la proyección (por tanto, tras b90), en metros de los puntos...
					mx: x del borde izquierdo de la matriz
					MX: x del borde derecho de la matriz
					my: y del borde inferior de la matriz
					MY: y del borde superior de la matriz
				Son float porque la proyección siempre se elige centrada en el rectángulo.
				En el caso PlanoCuadra son los grados pasados a metros

pix_x, pix_y:	Tamaño de pixel en metros. Se ha de cumplir pix_x*npx=rectProy.MX - rectProy.my; análo. para y.
tipo:			Tipo de transformacion. 0: plano-cuadra. 1: sis.
pixel_m:		Píxel nominal en metros. Para que todas la funciones empleen el mismo valor

info: Los elementos de la estructura info no influyen en la transformación.
	x0, y0: Valores precalculados para un remuestreo. Son medio píxel a la dcha. y hacia abajo
			del primer dato que se almacenará en la matriz. Para PlanoCuadra están en grados,
			para Sistema, en metros, en la proyección.
*/
#define TIPO_PROY_PlanoCuadra 0
#define TIPO_PROY_Sistema 1
struct strMatriz___Tierra{
	uint npx,npy;
	Extremos2D_fl rectProy;
	float pix_x, pix_y; //En la proyección.
	bint b90;
	umint tipo;
	union{
		PlanoCuadra cc;
		Sistema sis;
	};
	float pixel_m; //Píxel nominal en metros. Para que todas la funciones empleen el mismo valor
	PíxelesMatriz_Info info;
};
