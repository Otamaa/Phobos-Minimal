#pragma once

#include "BlitterPack.byte.scalar.h"
#include "BlitterPack.byte.sse2.h"
#include "BlitterPack.byte.avx2.h"
#include "BlitterPack.byte.avx512.h"
#include "BlitterPack.word.scalar.h"
#include "BlitterPack.word.sse2.h"
#include "BlitterPack.word.avx2.h"
#include "BlitterPack.word.avx512.h"

static_assert(sizeof(BlitterPack8SSE2) == sizeof(BlitterPack8Scalar));
static_assert(sizeof(BlitterPack8AVX2) == sizeof(BlitterPack8Scalar));

#if defined(YR_SIMD_COMPILE_AVX512)
static_assert(sizeof(BlitterPack8AVX512) == sizeof(BlitterPack8Scalar));
#endif

static_assert(sizeof(BlitterPack16SSE2) == sizeof(BlitterPack16Scalar));
static_assert(sizeof(BlitterPack16AVX2) == sizeof(BlitterPack16Scalar));
static_assert(sizeof(BlitterPack16AVX512) == sizeof(BlitterPack16Scalar));
