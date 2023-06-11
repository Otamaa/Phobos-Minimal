#pragma once

#include <unknwn.h>

DECLARE_INTERFACE_IID_(IFlyControl, IUnknown, "820F501C-4F39-11D2-9B70-00104B972FE8")
{
	virtual LONG __stdcall Landing_Altitude() PURE;
	virtual LONG __stdcall Landing_Direction() PURE;
	virtual BOOL __stdcall Is_Loaded() PURE;
	virtual LONG __stdcall Is_Strafe() PURE;
	virtual LONG __stdcall Is_Fighter() PURE;
	virtual LONG __stdcall Is_Locked() PURE;
};

_COM_SMARTPTR_TYPEDEF(IFlyControl, __uuidof(IFlyControl));
