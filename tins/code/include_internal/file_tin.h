//Tipo de datos que se emplea para las coordenadas de los ficheros tin que genera esta librería
#define Coord_t ssint

#define TIN_CABECERA_UINTSIZEOF 40

typedef struct{
	uint W0;
	uint tipos_dato;
	uint unidades; //Puede ser un IO_SINGLE
	uint W3, W4, W5, W6, W7;
	uint p_retic_pos,		p_retic_n;
	uint p_inc_pos,			p_inc_n;
	uint p_indiv_pos,		p_indiv_n;
	uint t_malla_pos,		t_malla_n;
	uint t_inc_pos,			t_inc_n;
	uint t_indiv_pos,		t_indiv_n;
	uint estilos_p_pos;
	uint estilos_t_pos;
	uint clases_p_pos;
	uint estilo_pindiv_pos;
	uint clases_t_pos;
	uint estilo_tindiv_pos;
} TinCabecera_Data;

typedef union{
	TinCabecera_Data datos;
	uint uints[TIN_CABECERA_UINTSIZEOF];
} TinCabecera;

typedef struct{
	uint m,n;
	uint siguiente;
	uint dato; //0: igual que fich., 1: la mitad
	Coord_t X0,Y0,Z0;
	Coord_t ΔX1, ΔY1, ΔZ1;
	Coord_t ΔXn, ΔYn, ΔZn;
	Coord_t ΔX, ΔY, ΔZ; //(0,0,1)
} TinPuntosRetícula;
