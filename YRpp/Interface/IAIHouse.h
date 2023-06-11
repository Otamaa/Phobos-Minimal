#pragma once

#include <unknwn.h>
#include <comdef.h>

DECLARE_INTERFACE_IID_(IAIHouse, IUnknown, "96F02EC4-6FE8-11D1-B6FD-00A024DDAFD1")
{
	virtual LONG __stdcall Link_House(void* unknown) const PURE;
	virtual LONG __stdcall AI(int* unknown) PURE;
};

_COM_SMARTPTR_TYPEDEF(IAIHouse, __uuidof(IAIHouse));