#pragma once
#include "str_dataset.h"

/*Leen el 'fichero' y con ello rellenan 'ds'.
Las claves que no reconozcan las ingoran.
Return:
	<0: El fichero no se pudo abrir. Es el código devuelto por la libreria ATcrt
	0: El fichero se pudo abrir y todo estaba bien
	1: El valor de alguna clave es erróneo o faltan claves.

Si se devuelve <0 ds no se modifica.
Si se devuelve 0, se asignará ds->md_state = METADATA_Right.
Si se devuelve 1, se asignará ds->md_state = METADATA_Wrong.
ds->folder y ds->cielo no se modifican.
*/
int lee_dsMETA_global(ZonalDataSet *ds, const char8_t *fichero);
int lee_dsMETA_local(LocalDataSet *ds, const char8_t *fichero);
int lee_dsMETA_puntos(PointsDataSet *ds, const char8_t *fichero);
