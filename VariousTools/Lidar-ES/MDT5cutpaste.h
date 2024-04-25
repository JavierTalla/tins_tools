#include <ATcrt/ATcrt.h>
#include <stdio.h>
#include <ATcrt/file_fix.h>
//#include <ATcrt/definesBufferto8.h>

#define isWORD(s) (strcmp8(ptr,s)==0)
#define ifWORD(s) if(isWORD(s))
#define elifWORD(s) else ifWORD(s)

typedef struct{
	//ssint xmin, xmax, ymin, ymax;
	char8_t fnombre[SHRT_PATH];
	char8_t *varnombre; //Puntero a la parte variable del nombre.
	uint16_t *T;
	uint16_t *pos0, *pfinal;
			//pos0: Posición del primer elemento de la fila que se está rellenando. Puede ser el primer de la fila
				//o uno intermedio si el fichero que estamos leyendo no llega hasta el extremo izquierdo del tile.
			//pfinal: Bien el final de la fila del tile, bien (para el último tile) la posición que corresponde al final
} Tile;			//de la fila en el fichero que se está leyendo

#define NODATA 0x8000 //El de los tiles generados

//Devuelve 0 si se puede rellenar con ceros, 1 en caso contrario.
bint zerofill(ssint x,ssint X,ssint y,ssint Y);
