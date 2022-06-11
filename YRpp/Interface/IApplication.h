#pragma once

#include <unknwn.h>
#include "iswizzle.h"

DECLARE_INTERFACE_IID_(IApplication, IUnknown, "96F02EC3-6FE8-11D1-B6FD-00A024DDAFD1")
{
	virtual LONG __stdcall FullName(BSTR* retval) PURE;
	virtual LONG __stdcall Name(BSTR* retval) PURE;
	virtual LONG __stdcall Quit() PURE;
	virtual LONG __stdcall ScenarioName(BSTR* retval) PURE;
	virtual LONG __stdcall FrameCount(long* retval) PURE;
	virtual LONG __stdcall Swizzle_Interface(ISwizzle** pVal) PURE;
};