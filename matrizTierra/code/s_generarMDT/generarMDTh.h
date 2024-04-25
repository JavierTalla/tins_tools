#include <ATcrt/ATcrt_utils.h>
#include <ATcrt/ATmem.h>
#include <ATcrt/ATfiles.h>
#include <ATcrt/ATarrays.h>
#include <ATcrt/file_fix.h>
#include <PI.h>
#include <pmsqrt.h>

typedef enum enumClipModo{ClipModo_nada, ClipModo_recortar, ClipModo_lago} ClipModo;

typedef struct{
	char8_t *path;
	char8_t *namebase, *name;
	char8_t *ext;
} Tilename;

typedef struct{
	char8_t *path;
	char8_t *namebase, *folder; //Raíz de todas las subcarpetas.
	char8_t *name;
	char8_t *ext;
} TilenameFolders;

typedef struct{
	umint *corte; //Distancia a la que se está en cada punto de _matriz de la línea de cambio de datasets de origen (v. infra)
	umint n_corte; //Lo que se va a emplear. Son medios-píxeles
	umint nmarco;
} Suavizado;

typedef struct{
	ssint *m;
	uint npx, npy;
	uint npixels;
} Matriz_ssint;

#define NSearchedFiles_suelo 320
#define NSearchedFiles_fondo 80
#define NSearchedFiles_edificios 800
PerTalla_scope SearchedFileInfo searchedfiles_suelo[NSearchedFiles_suelo];
PerTalla_scope SearchedFileInfo searchedfiles_fondo[NSearchedFiles_fondo];
PerTalla_scope SearchedFileInfo searchedfiles_edificios[NSearchedFiles_edificios];

#define LOCAL_PIXUNI_CM 100 //El tamaño de píxel de los DataSet locales se indica en cm
#define LOCAL_PIXm_CM 0.01F
#define LOCAL_PIXUNI_DTER 20 //O en décimas de tercero \approx= 5cm
#define LOCAL_PIXm_DTER 0.05F

typedef struct{
	const ZonalDataSet *DS;
	const LocalDataSet *DSL;
} DataSetPair;
static const int bad_ptr=0;
#define BAD_PTR ((void*)&bad_ptr) //Si lo devuelto en DS es esto es que no hay datasets con resolución suficiente.

//Se evalua a true si el rectángulo r1 está completamente dentro de r2
#define rect_in_rect2(r1,r2) ((r1).λmin>=(r2)->λmin && (r1).λmax<=(r2)->λmax && (r1).φmin>=(r2)->φmin && (r1).φmax<=(r2)->φmax)

//Se evalua a true si el rectángulo r1 corta al r2
#define rect_cut_rect2(r1,r2) ( ((r1).λmin<(r2)->λmax || (r1).λmax>(r2)->λmax) && ((r1).φmin<(r2)->φmin && (r1).φmax>(r2)->φmax) )

static const LagoZ LagoZVacío={.cota=MAX_UEARTHHEIGHT, .depth=0, .superficie=0, .supe_edif=0};
static const LagoZ LagoZNull={MAX_UEARTHHEIGHT,MAX_UEARTHHEIGHT,MAX_UEARTHHEIGHT,MAX_UEARTHHEIGHT};

//Macros para enmascarar el entorno de una costura entre dos datasets distintos
#define halfside___ncorte(n) ((n)>>1) //Obtiene el medio-lado de la máscara a partir del valor 'n' de ancho de la franja a enmascarar.
											 //Hace falta esta función porque 'n' no son píxeles, sino un parámetro con una cierta interpretación
#define MASK_MAX_NHALF halfside___ncorte(DS_SEAM_MAX_SMOOTHNESS)
#define MASK_MAX_N (1+(MASK_MAX_NHALF<<1)) //Lado de la máscara máxima
#include "matriz_mask.h"

/*En algunas ocasiones es necesario conocer el paso de la matriz en metros. Se extrae a esta definición para que
su cálculo sea siempre igual. No importa mucho una fórmula u otra, es un concepto aproximado, pero sí que
se haga siempre igual.
Se ginora pix_λ porque requeriría emplear cos(φ) y las transformaciones se determinan de manera que el píxel
sea cuadrado, o cuando menos que si alguna vez es más ancho en un sentido otras lo sea en otro, pudiendo
para zonas muy amplias de la Tierra variar mucho en λ, manteniéndose por lo general constante en φ. Se emplea
el radio 6372 Km en lugar de 6380 porque las deformaciones tienden por lo general a aumentar la escala, no a
disminuirla.
pix_φ tiene que venir en segundos de arco */
#define PIXEL_METRO___Φ(pix_φ) (fabsf(pix_φ)*(float)PI_180*((float)R_TIERRA/3600.0F)) //=6372000/3600

#define NODATA_ssint INT32_MIN
#define NODATA_uEarth MAX_UEARTHHEIGHT

//Parámetros para eliminar un lago con mucho edificio
#define ELIMINA_LAGO_PIXMAX 100 //metros
//pix en metros, <=255
sinline uint8m factorK(uint pix){
	if(pix>200) return 2;
	if(pix<6) return 7;
	return 2+30/(uint8m)pix;
}
