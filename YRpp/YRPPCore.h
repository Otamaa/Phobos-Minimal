#pragma once

#include <Base/Always.h>

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


struct noinit_t final {};
struct noprapere_tag final { };
