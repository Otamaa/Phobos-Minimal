#pragma once

#include <YRPPCore.h>
#include <ArrayClasses.h>
#include <Interface/ISwizzle.h>

#include <Helpers/CompileTime.h>

class SwizzlePointerClass
{
	DWORD unknown_0; //no idea, only found it being zero
	void* pAnything; //the pointer, to literally any object type

public:
	bool operator==(const SwizzlePointerClass& tOther) const
		{
		return
			(unknown_0 == tOther.unknown_0) &&
			(pAnything == tOther.pAnything);
	}

	bool operator!=(const SwizzlePointerClass& that) const { return unknown_0 != that.unknown_0; }
	bool operator<(const SwizzlePointerClass& that) const { return unknown_0 < that.unknown_0; }
	bool operator>(const SwizzlePointerClass& that) const { return unknown_0 > that.unknown_0; }
};

class NOVTABLE SwizzleManagerClass : public ISwizzle
{
public:
	static COMPILETIMEEVAL reference<SwizzleManagerClass, 0xB0C110u> const Instance{};

	//IUnknown
	IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);
	IFACEMETHOD_(ULONG, AddRef)();
	IFACEMETHOD_(ULONG, Release)();

	//ISwizzle
	STDMETHOD_(LONG, Reset)()override R0;
	STDMETHOD_(LONG, Swizzle)(void** pointer) override R0;
	STDMETHOD_(LONG, Fetch_Swizzle_ID)(void* pointer, LONG* id) override R0;
	STDMETHOD_(LONG, Here_I_Am)(LONG id, void* pointer)  override R0;
	STDMETHOD(Save_Interface)(IStream* stream, IUnknown* pointer) override R0;
	STDMETHOD(Load_Interface)(IStream* stream, CLSID* riid, void** pointer)override R0;
	STDMETHOD_(LONG, Get_Save_Size)(LONG* size) override R0;

	//DTOR
	virtual ~SwizzleManagerClass() RX;

	void ConvertNodes()
	{ JMP_THIS(0x6CF350); }

	//CTOR
	SwizzleManagerClass()
		: SwizzleManagerClass(noinit_t())
	{ JMP_THIS(0x6CF180); }

protected:
	explicit __forceinline SwizzleManagerClass(noinit_t)
	{ }

public:

	DECLARE_PROPERTY(DynamicVectorClass<SwizzlePointerClass>, Swizzles_Old);
	DECLARE_PROPERTY(DynamicVectorClass<SwizzlePointerClass>, Swizzles_New);

};

#define SWIZZLE_RESET(func) SwizzleManagerClass::Instance().Reset();

#define SWIZZLE_REQUEST_POINTER_REMAP(pointer) \
            { \
                SwizzleManagerClass::Instance().Swizzle((void **)&pointer); \
            }

#define SWIZZLE_REQUEST_POINTER_REMAP_LIST(pointer, var, vector) \
            { \
                for (int i = 0; i < vector.Length(); ++i) { \
                   SwizzleManagerClass::Instance().Swizzle((void **)&vector[i]); \
                } \
            }

#define SWIZZLE_FETCH_POINTER_ID(pointer, id) \
            { \
                SwizzleManagerClass::Instance().Fetch_Swizzle_ID((void *)pointer, id); \
            }

#define SWIZZLE_HERE_I_AM(id, pointer) \
            { \
                SwizzleManagerClass::Instance().Here_I_Am(id, (void *)pointer); \
            }