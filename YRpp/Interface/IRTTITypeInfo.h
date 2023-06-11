#pragma once

#include <unknwn.h>
#include <GeneralDefinitions.h>
#include <comdef.h>

DECLARE_INTERFACE_IID_(IRTTITypeInfo, IUnknown, "170DAC82-12E4-11D2-8175-006008055BB5")
{
	virtual AbstractType __stdcall What_Am_I() const PURE;
	virtual int __stdcall Fetch_ID() const PURE;
	virtual void __stdcall Create_ID() PURE;
};

_COM_SMARTPTR_TYPEDEF(IRTTITypeInfo, __uuidof(IRTTITypeInfo));
