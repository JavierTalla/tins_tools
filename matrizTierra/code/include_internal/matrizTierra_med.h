#define COMPILING_MATRIZTIERRA

#if defined(_WIN32)
#undef visibility_exposed
#define visibility_exposed //It is being exported via the def file
#endif
#if defined(__clang__) || defined ( __GNUC__)
#undef LARGE_STACK
#define LARGE_STACK PerTalla_scope
#endif
#include "../include/matrizTierra.h"
#include <ATcrt/ATcrt_utils.h>
#define NOMEM_RETURN_CODE AT_NOMEM

//PerTalla_scope no se define como _Tread_local porque una talla puede partirse en varios hilos
//Por tanto, si se desea emplear estas librerías para varias ejecuciones simultáneas ha de hacerse mediante llamadas desde procesos distintos,
//para que cada uno tenga su copia de los datos globales
#define N_THREADS 3
#define PerTalla_scope static
#define sinline static inline
#define elif else if

/**  Diversas constantes que definen el comportamiento del programa  **/

//Constantes para decidir qué proyección se aplica
#define MΦ 140 //Si se supera este valor de Δφ y (180+2*ESTδΛ) de Δλ se escojerá la proyección "Mundo"
#define ESδΛ 5 //Exceso permitido sobre una semiesfera para la estereográfica
#define PlCu 1.4F //Si no se alcanza este valor de cociente entre las escalas de los paralelos mín. y máx. se escogerá PlanoCuadra

//Máximo número de tiles de un dataset que pueden entrar en una talla con "sistema de coordenadas"
#define NTILES_MAX 5000 //SRTM900m/cerrados contiene 540 tiles

//Se emplea el radio 6372 Km en lugar de 6380 porque las deformaciones tienden por lo general a aumentar
//la escala, no a disminuirla, y este valor se emplea para pasar de grados sobre la Tierra a metros.
#define R_TIERRA 6372000

//Parámetro para enmascarar el entorno de una costura entre dos datasets distintos
//Este valor multiplicado por lo máximo que puede aparecer en la matriz tiene que caber en un ssint.
//De momento, lo máximo esperable en *matriz es más o menos 2*uint16_max. Si en el futuro esto
//fuere un problema, no hay más que cambiar el código de interpola_tile para que el promedio lo
//haga en float
#define DS_SEAM_MAX_SMOOTHNESS 24 //medios píxeles.

//Aplica a rect un giro de 90º en sentido horario
#define giraxy_proy___tierra(Type,x,y) {Type aux=x; x=y; y=-aux;}
#define giraxy_tierra___proy(Type,x,y) {Type aux=x; x=-y; y=aux;}
#define girapunto_proy___tierra(p) giraxy_proy___tierra(float,(p).x,(p).y)
#define girapunto_tierra___proy(p) giraxy_tierra___proy(double,(p).x,(p).y)

//DS_step en metros
//El array matriz->i.cotaslago se cierra con un MAX_UEARTHHIEGHT para encontrar rápidamente
//él número de láminas de agua sin tener que recorrer toda la matriz
int interpreta_matriz(MatrizTierra *matriz, float DS_step, const Opciones_Lagos *abst);
