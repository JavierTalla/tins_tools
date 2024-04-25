#define STL_CABECERA_UINTSIZEOF 20

typedef struct{
	PuntoXYZ_float vector,
						  p1, p2, p3;
	uint16_t attr_count;	//Siempre es cero
} Stl_triangle;
#define FILESIZEOF_STL_TRIANGLE 50
#define SIZEOF_STL_TRIANGLE (uoffsetof(Stl_triangle,attr_count)+usizeof(uint16_t))

typedef struct{
	PuntoXYZ_float vector,
						  v1, v2, v3;
	uint attrs; //El u16 más bajo es attr_count y vale 2. El alto, 0 o se rellena con información.
} Stl_triangle2;

/* IMPORTANTE: Los listos de StL van y definen la etructura de triángulo con todo floats
y 2 bytes al final, por lo que cualquier compilador añadirá 2 bytes de alineación al final. Por
tanto no se puede crear un archivo StL en memoria situando estructuras StL_triangle una
tras otra. Muchas máquinas ni siquiera permitirán tener floats con alineación de dos bytes.
    La mejor solución es establecer un valor de attr_count de 2, pero la especificación del
formato dice que ha de valer cero.
    Por ello existen dos funciones. escribe_fichero_stl1, con attr_count=0 (estándar) y
    escribe_fichero_stl2, con attr_count=2 y dos bytes de relleno para alinear los triángulos.
*/
typedef struct str_Stl_fichero_puro{
	uint cabecera[20];	//El fichero empieza aquí y ocupa byte_count bytes
	uint ntriángulos;
	Stl_triangle triángulos;	//Primer triángulo
} Stl_fichero_puro;

#define malloc_para_ntrián(n) (Stl_fichero_puro*)malloc(offsetof(Stl_fichero_puro,triángulos)+(n)*usizeof(Stl_triangle))
#define realloc_para_ntrián(stl,n) (Stl_fichero_puro*)realloc((stl)->stl_in_mem,offsetof(Stl_fichero_puro,triángulos)+(n)*usizeof(Stl_triangle))
#define stl_bytecount_ntrián(n) offsetof(Stl_fichero_puro,triángulos)*(CHAR_BIT/8)+FILESIZEOF_STL_TRIANGLE*(n)
#define redirige_ptri(stl,nuevo,ptri) ptri=(Stl_triangle*)((char*)nuevo+((char*)ptri-(char*)(stl)->stl_in_mem))

//<= para que también funcione con f=g=0
#define flt_equal(f,g) (fabsf((f)-(g))*262144.0F<=max(fabsf(f),fabsf(g)))
