#pragma once

typedef struct{
	float λ, φ;
	float Λ, Φ;
} RECT_ΛΦ;

typedef struct{
	float λ1, φ1;
	float λ2, φ2;
	float λ3, φ3;
	float λ4, φ4;
} RECT_ΛΦ_girado;

#include "Matriz___Tierra.h"
#include "str_MatrizTierra.h"
#include "../../extern_libs/include/datasets/str_dataset.h"
#include "../../extern_libs/include/datasets/str_clips.h"
#include "MatrizMarcadaCall.h"
