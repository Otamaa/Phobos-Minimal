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

#ifndef _useSTD
#define MinImpl(a,b) (((a) < (b)) ? (a) : (b))
#define MaxImpl(a,b) (((a) > (b)) ? (a) : (b))
#else
#define MinImp std::min
#define MaxImpl std::max
#endif

#define LessOrEqualTo(a, b) (a <= b) ? (a) : (b);
#define MoreOrEqualTo(a, b) (a >= b) ? (a) : (b);

#define PRAGMA(X)					 __pragma(#X)
#define PRAGMA_DISABLEWARNING()      PRAGMA(warning(push))
#define PRAGMA_DISABLEWARNING_S(x)   PRAGMA(warning(disable : x))
#define PRAGMA_DISABLEWARNING_POP()	 PRAGMA(warning(pop))

#define R1 {return 1;}
#define R0 {return 0;}
#define RX {}
#define RT(type) {return type();}

#define NOVTABLE __declspec(novtable)
#define NOINLINE __declspec(noinline)
#define UNUSED  PRAGMA(warning(suppress : 4100 4101))
#define NORETURN __declspec(noreturn)
#define NOTHROW __declspec(nothrow)
#define SELECTANY __declspec(selectany)
#define NAKED __declspec(naked)
#define NAKEDNOINLINE __declspec(noinline) __declspec(naked)

#ifdef _DEBUG
#define FORCEDINLINE inline
#else
#define FORCEDINLINE __forceinline
#endif

#define OPTIONALINLINE inline
#define COMPILETIMEEVAL constexpr

#define ALIGN(val) __declspec(align(val))
#define ALIGNOF(type) (sizeof(type) - sizeof(type) + __alignof(type))
#define ALIGNAS(byte_alignment) __declspec(align(byte_alignment))

#define SAFE_RELEASE(ptr) {if(ptr) delete[] ptr;}

#define STRING2(x) #x
#define STRING(x) STRING2(x)

#define DEFINE_CLSID(_addrs) __declspec(uuid(_addrs))

#define COMPILE_TIME_SIZEOF(t) \
template<int s> struct SIZEOF_ ## t ## _IS; \
struct foo { int a,b; }; \
SIZEOF_ ## t ## _IS<sizeof(t)> SIZEOF_ ## t ## _IS;

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

/*
 *	This set of macros is necessary because a type argument can contain commas, which would split it into multiple arguments.
 *	To circumvent this, we can enclose the argument in parentheses.
 *	However, parentheses are significant, so we then need a macro to remove them so as
 *	to not affect our final result.
 *	Source: https://groups.google.com/a/isocpp.org/g/std-proposals/c/Ngl_vTAdddA
 */
#define UNPAREN( ... ) UNPAREN __VA_ARGS__
#define DONE_UNPAREN

#define CAT_LIT(A, ...) A ## __VA_ARGS__
#define CAT(A, ...) CAT_LIT(A, __VA_ARGS__)
#define UIP(...) CAT( DONE_, UNPAREN __VA_ARGS__  )

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

#define BYTE0(x) ((unsigned char)((x) >>  0))
#define BYTE1(x) ((unsigned char)((x) >>  8))
#define BYTE2(x) ((unsigned char)((x) >> 16))
#define BYTE3(x) ((unsigned char)((x) >> 24))