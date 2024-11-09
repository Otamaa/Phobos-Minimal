#pragma once

#include <inttypes.h>
#include <windows.h>

#define NAME_MAX FILENAME_MAX

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

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
#define ALIGN(val) __declspec(align(val))

#define SAFE_RELEASE(ptr) {if(ptr) delete[] ptr;}

#define STRING2(x) #x
#define STRING(x) STRING2(x)

#define DEFINE_CLSID(_addrs) __declspec(uuid(_addrs))

#define COMPILE_TIME_SIZEOF(t) \
template<int s> struct SIZEOF_ ## t ## _IS; \
struct foo { int a,b; }; \
SIZEOF_ ## t ## _IS<sizeof(t)> SIZEOF_ ## t ## _IS;

#define CLOSE_ENOUGH(x, y) \
	(fabs(x - y) < 0.001)

#define LESS_EQUAL(x, y) \
	((x - y) <= 0.001)

#define EPILOG_STDCALL \
	_asm{mov esp, ebp} \
	_asm{pop ebp}

#define JMP(address) \
	_asm{mov eax, address} \
	_asm{jmp eax}

#define JMP_STD(address) { EPILOG_STDCALL JMP(address); }