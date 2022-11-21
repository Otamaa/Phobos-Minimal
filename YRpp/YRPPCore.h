#pragma once

//Syringe interaction header - also includes <windows.h>
#include <Syringe.h>

#include <Fundamentals.h>


//Assembly macros
#include <ASMMacros.h>

#include <wchar.h>
#include <cstdio>

//control key flags
typedef DWORD eControlKeyFlags;

//Avoid default CTOR trick
#define DECLARE_PROPERTY(type,name)\
union{\
	type name; \
	char __##name[sizeof(type)]; \
}

#define DECLARE_PROPERTY_ARRAY(type,name,cnt)\
union{\
	type name[cnt]; \
	char __##name[sizeof(type) * cnt]; \
}

//Not gettable/settable members
#define PROTECTED_PROPERTY(type,name)\
	protected:\
		type name; \
	public:

#define PRIVATE_PROPERTY(type,name)\
	private:\
		type name; \
	public:
/*
Operation: The Cleansing

These two replace a function's implementation.

R0 is used for functions which return a numeric value or a pointer.
RX is for functions without a return type.
Functions that return struct instances will have to be written manually.

I chose short names to keep things clean.

Example usage:
virtual int foo(int bar) R0;
virtual void alla(double malla) RX;
*/

#define R1 {return 1;}
#define R0 {return 0;}
#define RX {}
#define RT(type) {return type();}

#define NOVTABLE __declspec(novtable)
#define NOINLINE __declspec(noinline)
#define UNUSED __pragma(warning(suppress : 4100 4101))
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

#define VTABLE_SET(item, addr) ((int*)item)[0] = addr
#define VTABLE_SET_AT(item, pos, addr) ((int*)item)[pos] = addr
#define VTABLE_GET(item) (((int*)item)[0])

struct noinit_t final {};
