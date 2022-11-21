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
#define ILINE __forceinline

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
