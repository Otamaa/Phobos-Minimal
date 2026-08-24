#pragma once

#include "Intrinsics.h"
#include "Macros.h"
#include <inttypes.h>

#include <windows.h>
#define NAME_MAX FILENAME_MAX

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

/**
 *  Enable inline recursion.
 */
#pragma inline_recursion(on)
#pragma inline_depth(255) // Allow lots of inlining.

 /**
  *  Alias the ICU unicode functions when not building against it.
  */
#define u_strlen wcslen
#define u_strcpy wcscpy
#define u_strcat wcscat
#define u_vsnprintf_u vswprintf
#define u_strcmp wcscmp
#define u_strcasecmp(x, y, z) _wcsicmp(x, y)
#define u_isspace iswspace
#define u_tolower towlower
#define U_COMPARE_CODE_POINT_ORDER 0x8000

  /**
   *  Define some stuff here for cross platform consistency.
   */
#define strcasecmp _stricmp
#define strncasecmp _strnicmp

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

static_assert(sizeof(char) == 1);
static_assert(sizeof(float) == 4);
static_assert(sizeof(int) >= 4);

typedef unsigned char  uchar;
typedef signed   char  schar;

typedef unsigned short ushort;
typedef signed   short sshort;

typedef unsigned int   uint;
typedef signed   int   sint;

typedef unsigned long  ulong;
typedef signed   long  slong;

typedef unsigned long long ulonglong;
typedef signed   long long slonglong;

static_assert(sizeof(uchar) == sizeof(schar));
static_assert(sizeof(ushort) == sizeof(sshort));
static_assert(sizeof(uint) == sizeof(sint));
static_assert(sizeof(ulong) == sizeof(slong));
static_assert(sizeof(ulonglong) == sizeof(slonglong));

static_assert(sizeof(uchar) <= sizeof(ushort));
static_assert(sizeof(ushort) <= sizeof(uint));
static_assert(sizeof(uint) <= sizeof(ulong));
static_assert(sizeof(ulong) <= sizeof(ulonglong));

typedef schar int8;
typedef schar sint8;
typedef uchar uint8;
static_assert(sizeof(uint8) == 1);
static_assert(sizeof(sint8) == 1);

typedef sshort int16;
typedef sshort sint16;
typedef ushort uint16;
static_assert(sizeof(uint16) == 2);
static_assert(sizeof(sint16) == 2);

typedef sint int32;
typedef sint sint32;
typedef uint uint32;
static_assert(sizeof(uint32) == 4);
static_assert(sizeof(sint32) == 4);

typedef slonglong int64;

typedef float  f32;
typedef double f64;
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

typedef union {
	struct {
		uint16_t high;
		uint16_t low;
	} pieces;
	uint32_t all;
} splitint_t_big;

typedef union {
	struct {
		uint16_t low;
		uint16_t high;
	} pieces;
	uint32_t all;
} splitint_t_little;

// fuck these , dont use it , it causing code to avoid compiler optimization most of the time
// also defining those address on the dll massively polluting the segment
// nor worth the shit
 /*
  *	Use these macros to define a reference to an address in the game's memory.
  */
  //#define DEFINE_NONSTATIC_REFERENCE(type, name, address) UIP(type) (&name) = *reinterpret_cast<UIP(type)*>(address);
  //#define DEFINE_REFERENCE(type, name, address) static inline DEFINE_NONSTATIC_REFERENCE(type, name, address);
  //#define DEFINE_NONSTATIC_ARRAY_REFERENCE(type, dimensions, name, address) UIP(type) (&name)dimensions = *reinterpret_cast<UIP(type) (*)dimensions>(address);
  //#define DEFINE_ARRAY_REFERENCE(type, dimensions, name, address) static inline DEFINE_NONSTATIC_ARRAY_REFERENCE(type, dimensions, name, address);

	/*
	 *	Use these macros to define a pointer to an address in the game's memory.
	 *	Pretty much only useful for strings that exist in the executable,
	 *	for everything else, prefer references.
	 */
	 //#define DEFINE_NONSTATIC_POINTER(type, name, address) UIP(type)* const (name) = reinterpret_cast<UIP(type)*>(address);
	 //#define DEFINE_POINTER(type, name, address) static inline DEFINE_NONSTATIC_POINTER(type, name, address);


	 /**
	 * Use when some function argument is unneeded.
	 * Currently that happens when faking __thiscall functions
	 * via __fastcall ones (fastcall function accepts args via
	 * ECX, EDX, then stack, thiscall via ECX for this and stack
	 * for rest, so second arg in fastcall-faked function would need to be discarded).
	 */
typedef size_t discard_t;

#define SET_TO_ALL_BITS_ONE(x) (x = static_cast<std::make_unsigned_t<decltype(x)>>(-1))
#define SET_MINUS_ONE(x) (x = ~decltype(x)(0))
#define SET_UNSIGNED_MINUS_ONE(x) (x = (unsigned)-1)