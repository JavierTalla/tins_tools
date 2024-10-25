//Datos para la cabecera
typedef enum{
	TIN_tdato_unset=0,
	TIN_sint16='s', TIN_uint16='S', TIN_sint32='i',
	TIN_half='h', TIN_float='f', TIN_double='d'
} TipoDatoTIN;

#define TipoDatoTIN_is_integer(t) ((t)==TIN_sint16 || (t)==TIN_uint16 || (t)==TIN_sint32)
#define TipoDatoTIN_is_floating(t) ((t)==TIN_half || (t)==TIN_float || (t)==TIN_double)

typedef enum{
	UniTin_m_float,	//metros, un dato float. Todos los que siguen son uints.
	UniTin_μm, UniTin_mm, UniTin_m, //μm, mm, m.
	UniTin_MM, UniTin_M, UniTin_KM //Fracción de mm, fracción de m, fracción de km.
} TipoUnidadesTin;

typedef struct strCabeceraTin{
	TipoUnidadesTin tipo;
	union{
		uint u; float f;
	} valor;
} TinUnidad;

typedef struct{
	uint X,Y,Z;
} PuntoXYZ_uint;

typedef struct{
	ssint X,Y,Z;
} PuntoXYZ_ssint;

typedef struct{
	uint16m X,Y,Z;
} PuntoXYZ_uint16m;

typedef struct{
	sint16m X,Y,Z;
} PuntoXYZ_sint16m;

#ifndef PuntoXYZ
typedef struct strPuntoXYZ_double{
	double X,Y,Z;
} PuntoXYZ_double;

typedef struct strPuntoXYZ_float{
	float X,Y,Z;
} PuntoXYZ_float;
#endif

typedef struct{
	FloatHalf X,Y,Z;
} PuntoXYZ_half;

typedef struct{
	uint a,b,c; //Puntos; counter-clockwise sense.
	u16int class;
} TinTriangle;

typedef struct{
	uint color; //ARGB
} EstiloTriángulos1;

typedef struct{
	uint material;
} EstiloTriángulos2;

typedef struct{
	uint material;
	uint color; //ARGB
} EstiloTriángulos3;


/** Estructura TIN **/

typedef struct{
	uint p_retícula;
	uint p_increm;
	uint p_indiv;
	uint t_malla;
	uint t_increm;
	uint t_indiv;
} TIN_enes;

//Si Δu=0 desplaz es NULL. Si Δu!=0 desplaz apunta a un vector de m x n datos.
//Si bhalf es false el puntero a emplear en desplaz es pfull; si es true, es phalf.
typedef struct{
	uint m,n;
	bint bhalf;
	PuntoXYZ_ssint P0;
	PuntoXYZ_ssint Δ1;
	PuntoXYZ_ssint Δn;
	PuntoXYZ_ssint Δu;
	union{
		sint16m *phalf;
		ssint *pfull;
	} desplaz; //Múltiplos de Δu.
} PuntosRetícula_s32;

//Si Δu=0 desplaz es NULL. Si Δu!=0 desplaz apunta a un vector de m x n datos.
//Si bhalf es false el puntero a emplear en desplaz es pfull; si es true, es phalf.
typedef struct{
	uint m,n;
	bint bhalf;
	PuntoXYZ_uint16m P0;
	PuntoXYZ_sint16m Δ1;
	PuntoXYZ_sint16m Δn;
	PuntoXYZ_uint16m Δu;
	union{
		uint16m *phalf;
		uint16m *pfull;
	} desplaz; //Múltiplos de Δu.
} PuntosRetícula_u16;

//Si Δu=0 desplaz es NULL. Si Δu!=0 desplaz apunta a un vector de m x n datos.
//Si bhalf es false el puntero a emplear en desplaz es pfull; si es true, es phalf.
typedef struct{
	uint m,n;
	bint bhalf;
	PuntoXYZ_sint16m P0;
	PuntoXYZ_sint16m Δ1;
	PuntoXYZ_sint16m Δn;
	PuntoXYZ_sint16m Δu;
	union{
		sint16m *phalf;
		sint16m *pfull;
	} desplaz; //Múltiplos de Δu.
} PuntosRetícula_s16;

//Si Δu=0 desplaz es NULL. Si Δu!=0 desplaz apunta a un vector de m x n datos.
//Si bhalf es false el puntero a emplear en desplaz es pfull; si es true, es phalf.
typedef struct{
	uint m,n;
	bint bhalf;
	PuntoXYZ_double P0;
	PuntoXYZ_double Δ1;
	PuntoXYZ_double Δn;
	PuntoXYZ_double Δu;
	union{
		float *phalf;
		double *pfull;
	} desplaz; //Múltiplos de Δu.
} PuntosRetícula_dbl;

typedef struct{
	uint m,n;
	bint bhalf;
	PuntoXYZ_float P0;
	PuntoXYZ_float Δ1;
	PuntoXYZ_float Δn;
	PuntoXYZ_float Δu;
	union{
		FloatHalf *phalf;
		float *pfull;
	} desplaz; //Múltiplos de Δu.
} PuntosRetícula_float;

typedef struct{
	uint m,n;
	bint bhalf;
	PuntoXYZ_half P0;
	PuntoXYZ_half Δ1;
	PuntoXYZ_half Δn;
	PuntoXYZ_half Δu;
	union{
		FloatHalf *phalf;
		FloatHalf *pfull;
	} desplaz; //Múltiplos de Δu.
} PuntosRetícula_half;

//Si n<=1 increm es NULL. Si n>=2 increm apunta a un vector de 3*(n-1) datos.
typedef struct{
	uint n;
	PuntoXYZ_ssint P0;
	sint16m *increm;
} PuntosIncrementos_s32;

//Si n<=1 increm es NULL. Si n>=2 increm apunta a un vector de 3*(n-1) datos.
typedef struct{
	uint n;
	PuntoXYZ_uint16m P0;
	sint16m *increm;
} PuntosIncrementos_u16;

//Si n<=1 increm es NULL. Si n>=2 increm apunta a un vector de 3*(n-1) datos.
typedef struct{
	uint n;
	PuntoXYZ_sint16m P0;
	sint16m *increm;
} PuntosIncrementos_s16;

//Si n<=1 increm es NULL. Si n>=2 increm apunta a un vector de 3*(n-1) datos.
typedef struct{
	uint n;
	PuntoXYZ_double P0;
	float *increm;
} PuntosIncrementos_dbl;

//Si n<=1 increm es NULL. Si n>=2 increm apunta a un vector de 3*(n-1) datos.
typedef struct{
	uint n;
	PuntoXYZ_float P0;
	FloatHalf *increm;
} PuntosIncrementos_float;

//Si n<=1 increm es NULL. Si n>=2 increm apunta a un vector de 3*(n-1) datos.
typedef struct{
	uint n;
	PuntoXYZ_half P0;
	FloatHalf *increm;
} PuntosIncrementos_half;

//Si Δu=0 desplaz es NULL. Si Δu!=0 desplaz apunta a un vector de m x n datos.
typedef struct{
	uint m,n;
	uint a0, b0, c0;
	ssint Δa1, Δb1, Δc1;
	ssint Δan, Δbn, Δcn;
} TriángulosMalla;

//increm apunta a un vector de 3*n-1 datos.
typedef struct{
	uint n;
	uint a0;
	sint16m *increm;
} TriángulosIncrementos;

defineGrowing(PuntosRetícula_s32)
defineGrowing(PuntosRetícula_u16)
defineGrowing(PuntosRetícula_s16)
defineGrowing(PuntosRetícula_dbl)
defineGrowing(PuntosRetícula_float)
defineGrowing(PuntosRetícula_half)
defineGrowing(PuntosIncrementos_s32)
defineGrowing(PuntosIncrementos_u16)
defineGrowing(PuntosIncrementos_s16)
defineGrowing(PuntosIncrementos_dbl)
defineGrowing(PuntosIncrementos_float)
defineGrowing(PuntosIncrementos_half)
defineGrowing(TriángulosMalla)
defineGrowing(TriángulosIncrementos)

typedef Growing_PuntosRetícula_s32 PRetículas_s32;
typedef Growing_PuntosRetícula_u16 PRetículas_u16;
typedef Growing_PuntosRetícula_s16 PRetículas_s16;
typedef Growing_PuntosRetícula_dbl PRetículas_dbl;
typedef Growing_PuntosRetícula_float PRetículas_fl;
typedef Growing_PuntosRetícula_half PRetículas_half;
typedef Growing_PuntosIncrementos_s32 PIncrem_s32;
typedef Growing_PuntosIncrementos_u16 PIncrem_u16;
typedef Growing_PuntosIncrementos_s16 PIncrem_s16;
typedef Growing_PuntosIncrementos_dbl PIncrem_dbl;
typedef Growing_PuntosIncrementos_float PIncrem_fl;
typedef Growing_PuntosIncrementos_half PIncrem_half;
typedef Growing_TriángulosMalla TMallas;
typedef Growing_TriángulosIncrementos TIncrem;

//Según el valor de tipo, el puntero de p a emplear será uno u otro
typedef struct{
	uint8m tipo;
	uint nestilos;
	union{
		EstiloTriángulos1 *pest1;
		EstiloTriángulos2 *pest2;
		EstiloTriángulos3 *pest3;
	} p;
} EstilosTriángulos;

//El puntero, si no es NULL, apunta a un array de t_malla + t_incem + t_inidv elementos.
typedef struct{
	u8int nclass; //Números de clases. <=2^16. Si <=256, el puntero es p8.
	union{	//Pero si es 0 o 1 los punteros estarán a NULL.
		uint8m *p8;
		uint16m *p16;
	} p;
} ClaseTriángulos;

typedef struct strPuntos_s32{
	PRetículas_s32 p_retículas;
	PIncrem_s32 p_increm;
	ssint* p_indiv;
} Puntos_s32;

typedef struct strPuntos_u16{
	PRetículas_u16 p_retículas;
	PIncrem_u16 p_increm;
	uint16m* p_indiv;
} Puntos_u16;

typedef struct strPuntos_s16{
	PRetículas_s16 p_retículas;
	PIncrem_s16 p_increm;
	sint16m* p_indiv;
} Puntos_s16;

typedef struct strPuntos_dbl{
	PRetículas_dbl p_retículas;
	PIncrem_dbl p_increm;
	double* p_indiv;
} Puntos_dbl;

typedef struct strPuntos_fl{
	PRetículas_fl p_retículas;
	PIncrem_fl p_increm;
	float* p_indiv;
} Puntos_fl;

typedef struct strPuntos_half{
	PRetículas_half p_retículas;
	PIncrem_half p_increm;
	FloatHalf* p_indiv;
} Puntos_half;

//El elemento de puntos que hay que usar es el que corresponda con tdato
typedef struct strTIN{
	TipoDatoTIN tdato;
	TinUnidad uni;
	TIN_enes ns;
	union{
		Puntos_s32 s32;
		Puntos_u16 u16;
		Puntos_s16 s16;
		Puntos_dbl dbl;
		Puntos_fl fl;
		Puntos_half half;
	} puntos;
	TMallas t_malla;
	TIncrem t_increm;
	uint* t_indiv;
	EstilosTriángulos estilos_t;
	ClaseTriángulos clases_t;
} TIN;


/* TIN plano */

typedef struct strTINPlano{
	TipoDatoTIN tdato; //En el que cabrían las coordenadas de los puntos
	TinUnidad uni;
	uint np, nt;
	union{
		ssint *in;
		double *dbl;
	} puntos;
	uint *triángulos; //3*nt
	uint *estilos_t;
} TINPlano;


/* TINMalla */

//Rectángulo con datos ssint
typedef struct{
	ssint mx, my;
	ssint MX, MY;
} RECT_ssint;

typedef struct{
	ssint x,y;
} Puntoxy_ssint;

defineVector(sint16m)
defineVector(PuntoXYZ_ssint)
defineVector(TinTriangle)

/* Tin con triángulos en malla y otros individuales.
bdiag=0:
	No se presupone nada respecto a los triángulos. Pueden llenar la malla o no.
bdiag=1:
	La malla da lugar a ntm=2*(nx-1)*(ny-1) triángulos, que están al principio del vector triangles.
bdiag=2:
	La malla da lugar a ntm=2*(nx-1)*(ny-1) triángulos, que están al principio del vector triangles.
	Todos los triángulos toman para cada cuadradito de la malla la misma diagonal.

Aunque sólo existan los triángulos de la malla, no se requiere p_indiv.n=0. Simplemente,
estos puntos no se emplearían.*/
typedef struct strTINMalla{
	TinUnidad uni;
	RECT_ssint minmax;
	struct{
		ssint x0,y0;
		ssint Δx,Δy;
		uint nx,ny; //tiene que cumplirse z.n=nx*ny
		Vector_sint16m z;
	} malla;
	Vector_PuntoXYZ_ssint p_indiv;
	Vector_TinTriangle triangles;
	umint bdiag;
} TINMalla;

#define TINMalla_free_null(tin) do{\
	free_null((tin).malla.z.ppio); free_null((tin).p_indiv.ppio); free_null((tin).triangles.ppio);\
}while(0)
