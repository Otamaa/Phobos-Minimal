#pragma once

#include <unknwn.h>

DECLARE_INTERFACE_IID_(ISwizzle, IUnknown, "5FF0CA70-8B12-11D1-B708-00A024DDAFD1")
{
	virtual HRESULT __stdcall Reset() PURE;
	virtual HRESULT __stdcall Swizzle(void** pointer) PURE;
	virtual HRESULT __stdcall Fetch_Swizzle_ID(void* pointer, long* id) const PURE;
	virtual HRESULT __stdcall Here_I_Am(long id, void* pointer) PURE;
	virtual HRESULT __stdcall Save_Interface(IStream* stream, IUnknown* pointer) PURE;
	virtual HRESULT __stdcall Load_Interface(IStream* stream, GUID* riid, void** pointer) PURE;
	virtual HRESULT __stdcall Get_Save_Size(int* size) const PURE;
};