#pragma once

#include <unknwn.h>
#include <comdef.h>

DECLARE_INTERFACE_IID_(ISwizzle, IUnknown, "5FF0CA70-8B12-11D1-B708-00A024DDAFD1")
{
	STDMETHOD_(LONG, Reset)() PURE;
	STDMETHOD_(LONG, Swizzle)(void** pointer)  PURE;
	STDMETHOD_(LONG, Fetch_Swizzle_ID)(void* pointer, LONG * id) PURE;
	STDMETHOD_(LONG, Here_I_Am)(LONG id, void* pointer)  PURE;
	STDMETHOD(Save_Interface)(IStream * stream, IUnknown * pointer) PURE;
	STDMETHOD(Load_Interface)(IStream * stream, CLSID * riid, void** pointer) PURE;
	STDMETHOD_(LONG, Get_Save_Size)(LONG * size) PURE;
};

_COM_SMARTPTR_TYPEDEF(ISwizzle, __uuidof(ISwizzle));
