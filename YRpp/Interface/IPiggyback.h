#pragma once

#include <unknwn.h>
#include "ILocomotion.h"

DECLARE_INTERFACE_IID_(IPiggyback, IUnknown, "92FEA800-A184-11D1-B70A-00A024DDAFD1")
//'Piggyback' one locomotor onto another.
{
public:
	static GUID _CLSID;

	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) PURE;	//Piggybacks a locomotor onto this one.
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) PURE;	//End piggyback process and restore locomotor interface pointer.
	virtual bool __stdcall Is_Ok_To_End() PURE;	//Is it ok to end the piggyback process?
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) PURE;	//Fetches piggybacked locomotor class ID.
	virtual bool __stdcall Is_Piggybacking() PURE;	//Is it currently piggy backing another locomotor?
};

_COM_SMARTPTR_TYPEDEF(IPiggyback, __uuidof(IPiggyback));