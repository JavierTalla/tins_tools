#pragma once
#include <ATcrt/ATcrt_types.h>

#ifndef PathLengthFull_Max
	#define PathLengthFull_Max 280
	#define PathLengthDerived_Max 200
	#define PathLengthRoot_Max 80
#endif
#define N_ZONALDATASETS 8 //Máximo nº de datasets globales (7)
#define N_LOCALDATASETS 20 //Máximo nº de datasets locales (19)

typedef enum{METADATA_Notfound=0, METADATA_Right, METADATA_Wrong} MetadataState;

//These would be enums, where they allowed to be of uchar type
//Los datos signed se entiende que están representados mod. 2^n.
#define INTDATA_TYPE_8U 2
#define INTDATA_TYPE_8S 3
#define INTDATA_TYPE_16U 4
#pragma once

#define INTDATA_TYPE_16S 5
#define INTDATA_TYPE_32U 8
#define INTDATA_TYPE_32S 9

/*DataSets que abarcan toda la Tierra o una amplia zona latitudinal. Los datos están
en una malla de coordenadas geográficas. Se almacenan por filas, comenzando en la
fila más al Norte y teminando en la fila más al Sur.*/

//Estructura que indica cómo interpretar los bits que representan cada dato de Z en un tile
typedef struct{
	umint endianness; //ATBYTES_BIG_ENDIAN or _LITTLE (_UNSET is not permitted)
	umint type;	//Uno de los valores INTDATA_TYPE_ de arriba
	uint8m uniZ;	//Unidades de los valores (de Z) almacenados en el fichero. Indica la fracción
						//de metro. Por ejemplo, un 10 indicaría decímetros; un 4, cuartos de metro.
	uint16m offset;	//En metros. A los valores almacenados en el fichero hay que restarles este offset
						//para obtener el valor de altura.
	uint nodata;		//Valor que, una vez truncado al tamaño del dato del fichero, coindice bit a bit
						//con el valor que indica que no hay dato. Pero si es cero indica que no existe ninguna
						//serie de bits que indique nodata. Por tanto, el 0 no se puede emplear como valor
						//que indique ausencia de datos.

} DatoTile;

typedef struct{
	char8_t folder[PathLengthFull_Max];	//Nombre de la carpeta que los contiene
	MetadataState md_state;
	DatoTile Dato;
	float φmin, φmax;		//Si son -90, 90 el dataset es global.
	uint16m px;				//Tamaño de píxel en latitud, en metros (informativo)
	uint16m SN, WE;		//Lo que va del borde de un tile al borde del siguiente, en grados. Un valor 255 en WE significa 360
	sint16m φstep, λstep;	//En segundos.
	uint16m nrows, ncols;	//Número de filas y de columnas. Idelamente es SN/φstep +1. Puede ser mayor, pero no menor que SN/φstep.
	bool8 cielo;		//true si los datos son de "arriba" (incl. edificios y vegetación), false si son del suelo.
} GlobalDataSet, ZonalDataSet;


/*DataSets locales.*/

#define UNIDADES_XY_UNSET Я8
#define UNIDADES_XY_MIN_DTER 0
#define UNIDADES_XY_KM_CM 1
typedef struct{
	char8_t folder[PathLengthFull_Max];	//Nombre de la carpeta que los contiene
	u8int md_state;
	DatoTile Dato;
	//umint unidades_xy;	//Uno de los dos valores de arriba
	float φmin, φmax;
	float λmin, λmax;
	uint16m px;				//Tamaño de píxel nominal, en centímetros.
	uint8m SN, WE;		//Lo que va del borde de un tile al borde del siguiente, en minutos o en km
	sint16m φstep, λstep;	//En décimas de tercero o en centímetros. (1"=60'")
	uint16m nrows, ncols;	//Número de filas y de columnas
	float Z0;			//Los puntos cuya Z=0 en este dataset tienen un cota de Z0 en el sistema global,
						//en metros. Por tanto, para los dataset globales tiene que ser cero.
	bool8 cielo;		//true si los datos son de "arriba" (incl. edificios y vegetación), false si son del suelo.
} LocalDataSet;

/*Datasets en los cuales los datos no siguen una malla regular sino que pueden estar en cualquier
punto dentro del rectángulo abarcado por el fichero. Por ello cada entrada contará con X,Y,Z.
El número de puntos por fichero es variable.*/
typedef struct{
	char8_t folder[PathLengthFull_Max];	//Nombre de la carpeta que los contiene
	u8int md_state;
	DatoTile Dato;
	float φmin, φmax;
	float λmin, λmax;
	uint8m SN, WE;		//Lo que abarca cada tile, en minutos o en km
	double uniXY;	//unidades de los datos X e Y, en segundos de arco. (Normalmente bastante <1).
	float Z0;			//Los puntos cuya Z=0 en este dataset tienen un cota de Z0 en el sistema global.
} PointsDataSet;
