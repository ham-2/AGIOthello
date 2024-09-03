#include "misc.h"

#if defined _WIN64 && defined _MSC_VER 
#define _BITSCAN_
#include <intrin.h>
#define _POPCNT64_
#include <nmmintrin.h>
#elif defined __GNUC__
#include <x86intrin.h>
#endif

#if defined _BMI2_
#include <immintrin.h>
#endif