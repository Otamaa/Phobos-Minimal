#pragma once

#include <unknwn.h>
#include "ILocomotion.h"

DEFINE_GUID(IPiggyback_CLSID, 0x92FEA800, 0xA184, 0x11D1, 0xB7, 0x0A, 0x00, 0xA0, 0x24, 0xDD, 0xAF, 0xD1);
DECLARE_INTERFACE_IID_(IPiggyback, IUnknown, "92FEA800-A184-11D1-B70A-00A024DDAFD1")
//'Piggyback' one locomotor onto another.
{
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) PURE;	//Piggybacks a locomotor onto this one.
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) PURE;	//End piggyback process and restore locomotor interface pointer.
	virtual bool __stdcall Is_Ok_To_End() PURE;	//Is it ok to end the piggyback process?
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) PURE;	//Fetches piggybacked locomotor class ID.
	virtual bool __stdcall Is_Piggybacking() PURE;	//Is it currently piggy backing another locomotor?
};

_COM_SMARTPTR_TYPEDEF(IPiggyback, __uuidof(IPiggyback));
