#pragma once

#include "Macros.h"
#include <cstdint>
#include <intrin.h>

// GCC and MSVC use the same name but have different signatures so we use a new common name.
#define __cpuidc __cpuid

// Rotate instructions
#define __rotl8 _rotl8
#define __rotl16 _rotl16
#define __rotr8 _rotr8
#define __rotr16 _rotr16
#define __rotl32 _rotl
#define __rotr32 _rotr
#define __rotl64 _rotr64
#define __rotr64 _rotr64
