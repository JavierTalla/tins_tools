#include <ATcrt/ATcrt.h>
#include <float.h>

static uint hash_puntofl(PuntoXYZ_float P, uint umbral){
	uint *p;
	uint h=0;

	if(umbral<0x10000){
		if(umbral<0x1000){
			p=(uint*)&P.X;	h=(*p>>20) & 0x03F;
			p=(uint*)&P.Y;	h|=(*p>>16) & 0xFC0;
			p=(uint*)&P.Z;	h^=(*p>>18) & 0x1F8;
			return h%umbral;
		}
		p=(uint*)&P.X;	h=(*p>>19) & 0x00FF;
		p=(uint*)&P.Y;	h|=(*p>>11) & 0xFF00;
		p=(uint*)&P.Z;	h^=(*p>>15) & 0x0FF0;
		return h%umbral;
	}
	if(umbral<0x100000){
		p=(uint*)&P.X; h=*p>>6;
		p=(uint*)&P.Y; h^=*p>>6;
		p=(uint*)&P.Z; h^=*p>>6;
		return h%umbral;
	}
	p=(uint*)&P.X; h=*p>>4;
	p=(uint*)&P.Y; h^=*p>>4;
	p=(uint*)&P.Z; h^=*p>>4;
	return h%umbral;
}

#define keytype PuntoXYZ_float
#define datatype uint
#define Punto_EMPTY (PuntoXYZ_float){NOTFINITE_F,NOTFINITE_F,NOTFINITE_F}
#define field_value_EMPTY Punto_EMPTY
#define funchash hash_puntofl
#if 0 //COMPILER_ID!=MSC_ID //MS cl has a bug here. But we prefer the uint version anyway
#define Hash_isfield_equal(field1,field2,h) ((field1).X==(field2).X && (field1).Y==(field2).Y && (field1).Z==(field2).Z)
#define Hash_isfield_not_equal(field1,field2,h) ((field1).X!=(field2).X || (field1).Y!=(field2).Y || (field1).Z!=(field2).Z)
#else
#define Hash_isfield_equal(field1,field2,h) (*(uint*)&(field1).X==*(uint*)&(field2).X && *(uint*)&(field1).Y==*(uint*)&(field2).Y && *(uint*)&(field1).Z==*(uint*)&(field2).Z)
#define Hash_isfield_not_equal(field1,field2,h) (*(uint*)&(field1).X!=*(uint*)&(field2).X || *(uint*)&(field1).Y!=*(uint*)&(field2).Y || *(uint*)&(field1).Z!=*(uint*)&(field2).Z)
#endif
#define PuntoXYZ_float_is_empty(x) (!isfinite((x).X))
#define Hash_isfield_empty(field) PuntoXYZ_float_is_empty(field)
#include <ATcrt/hash_keydata.cod>
#undef Hash_isfield_empty
#undef Hash_isfield_equal
#undef Hash_isfield_not_equal

#include "file_stl.h"
