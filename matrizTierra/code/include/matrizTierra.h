#pragma once

#include <ATcrt/ATvisibility.h>

#ifndef COMPILING_MATRIZTIERRA
#define matriztierra_api set_visibility(imported)
#define MATRIZTIERRA_API VISIBILITY_BLOCK(imported)
#else
#define matriztierra_api set_visibility(exposed)
#define MATRIZTIERRA_API VISIBILITY_BLOCK(exposed)
#endif

/** Some structures and macros from the ATcrt library **/
#include <ATcrt/ATcrt_types.h>
#include "matrizTierra_structs.h"
#include <stdbool.h>

MATRIZTIERRA_API
#include "transformación.h"
#include "generarMDT.h"
VISIBILITY_BLOCK_END
