#pragma once

#include <unknwn.h>
#include <GeneralStructures.h>

DECLARE_INTERFACE_IID_(IPublicHouse, IUnknown, "CAACF210-86E3-11D1-B706-00A024DDAFD1")
{
	virtual long __stdcall			ID_Number() const PURE;
	virtual BSTR __stdcall			Name() const PURE;
	virtual long __stdcall			Apparent_Category_Quantity(Category category) const PURE;
	virtual long __stdcall			Apparent_Category_Power(Category category) const PURE;
	virtual CellStruct __stdcall	Apparent_Base_Center() const PURE;
	virtual bool __stdcall			Is_Powered() const PURE;
};

_COM_SMARTPTR_TYPEDEF(IPublicHouse, __uuidof(IPublicHouse));
