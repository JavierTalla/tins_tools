/*****         Estructuras necesarias para la llamada a la función matrizmarcada___userspecs         *****/

/* Opciones del MDT que se va a generar */

// Posibles valores para tipo_producto:
#define EARTH_TODO 0 //Representamos la tierra firme y el fondo del mar
#define EARTH_SOLO_TIERRA 1 //Representamos la tierra firme y el mar queda horizontal
#define EARTH_SOLO_FONDOMAR 2 //Representamos el fondo del mar y la tierra queda plana

typedef struct{
	u8int base; //Mínima superficie exigida para cualquier lago
	u8int planos;  //Factor por el que se multiplica 'base' para el mínimo de lagos planos
	uint pequeños; //Superficie mínima de un lagos para píxel de 1m.
	uint8m resolución;
	uint grandes; //A partir de este tamaño no se mira el desagüe
	uint8m exig_desagües; //Nivel de exigencia para los desagües. Cuanto más alto, más exigente
	uint8m expansión; //1: Las láminas pequeñas transmiten el lago igual que las medianas
} Opciones_Lagos;

typedef struct{
	umint tipo_producto;
	struct{
		uEarthHeight píxel;	//Píxel máximo para buscar edificios, en metros
		uEarthHeight htlim_grupo; //Si un edificio es más alto que esto se considera un error.  Ej: 300 m
		uEarthHeight htlim_aislado;  //Si un píxel aislado es más alto que esto se considera un error. Ej: 50m.
	} edificios;
	Opciones_Lagos lagos;
	bint fuera_is_agua;
	EarthHeight h_fuera; //MAX_UEARTHHEIGHT indica que el programa escoja.
} OpcionesMDTierra;

/* DataSets */

typedef struct{
	ZonalDataSet Gsuelos[N_ZONALDATASETS+1]; //El primero con folder[0]='\0' marca el cierre
	ZonalDataSet Gfondos[N_ZONALDATASETS+1];
	LocalDataSet Lsuelos[N_LOCALDATASETS+1];
	LocalDataSet Lfondos[N_LOCALDATASETS+1];
	PointsDataSet Puntos[N_LOCALDATASETS+1]; //Siempre son cielos locales
} DataSetsSets;

typedef struct{
	float pixeljump;	//pixeljump * Tierra[i].tamaño_de_píxel es un tamaño de píxel terreno aceptable para emplear el DataSet Tierra[i]
	float lastpixeljump;
	s8int wd_merge;
} DSopts;

typedef struct{
	char8_t DSraster[PathLengthRoot_Max]; //A título informativo.
	char8_t DSvector[PathLengthRoot_Max]; //A título informativo.
	DataSetsSets sets;
	DSopts opts;
} DataSets;


/*****         Estructura devuelta por matrizmarcada___userspecs         *****/

//Códigos de error
#define MTIERRA_DSempty 1				//El array DataSets no contiene ningún Data Set.
#define MTIERRA_DSlowres 2				//No existe ningún Data Set con resolución suficiente.
#define MTIERRA_DSpartiallowres 3		//El dataset local no cubre toda la zona y no existe ninguno global con resolución suficiente.
//#define MTIERRA_DSnometadata 4		//No se encontró el archivo de metadata del dataset (cuando en su momento se buscó, no ahora)
//#define MTIERRA_DSmetadatawrong 5	//El archivo de metadata no contiene todos los campos necesarios o alguno de los valores es erróneo (cuando en su momento se buscó, no ahora)
#define MTIERRA_DSfolder_notfound 6 //No existe la carpeta del dataset
#define MTIERRA_DSnooverlap 7			//El dataset que se quiere emplear no solapa con la zona a generar
#define TILE_NoUnzipCommand 10		//No se encuentra en el sistema el comando necesario para la descompresión de los tiles
#define TILE_BadDecompression 11		//La descompresión de un tile no se produjo correctamente
#define TILE_OpenFailure 12				//Error al intentar abrir un tile
#define TILE_BadSize 13						//El fichero del tile no tiene el tamaño esperado.
#define MTIERRA_DSnotsearched 19		//No se llegó a buscar, bien porque no hace falta para el producto pedido, bien debido a un error anterior.
#define MTIERRA_DSnoextradata 20		//El dataset no modificó nada (por ejemplo, porque no hay ningún edificio en la zona o ningún punto por debajo del agua)
#define MTIERRA_MuchoDesnivel 21		//El área pedida tiene un desnivel de más de 64 Km (e.d., el DataSet está mal).
#define MTIERRA_TooManyLakes 22		//Lo puede devolver matrizmarcada___userspecs

//Estructuras de tracing de los ficheros leídos y datos empleados

typedef struct{
	uint kind; //Tipo de fichero. Los posibles valores los define cada función que rellene una estructura de este tipo
	char8_t file[100]; //Solamente se debe guardar la parte variable, desde la raíz del dataset
	bint found;
} SearchedFileInfo;

#define MatrizSuelo_filekind_Global 0 //Tipos de ficheros buscados. Valores posibles del
#define MatrizSuelo_filekind_Local 0  //parámetro 'kind' en Debug_matrizsuelo.
typedef struct{
	const ZonalDataSet *dsGlobal; //DataSet global empleado. NULL si ninguno
	const LocalDataSet *dsLocal; //DataSet local empleado. NULL si ninguno
	SearchedFileInfo *files; //Un elemento con file[0]='\0' marca el final del array
	uint nfiles; //Número de elementos disponibles en el array files
} Debug_matrizsuelo;

//Estructura de debug de matriz_edificios. El valor de kind de los files se ignora
typedef struct{
	const PointsDataSet *dset; //DataSet local empleado. NULL si ninguno
	SearchedFileInfo *files; //Un elemento con file[0]='\0' marca el final del array
	uint nfiles; //Número de elementos disponibles en el array files
} Debug_matrizedificios;

typedef struct{
	int nret; //nret global
	int nret_suelo, nret_fondo, nret_edif;
	Debug_matrizsuelo suelo;
	Debug_matrizsuelo fondo;
	Debug_matrizedificios edificios;
	u8int bflags; //Copia del de matriz.esta
} MatrizMarcadaReturn;
