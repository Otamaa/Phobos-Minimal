#pragma once
#include <unknwn.h>
#include "IApplication.h"
#include <GeneralStructures.h>

DECLARE_INTERFACE_IID_(IHouse, IUnknown, "941582E0-86DA-11D1-B706-00A024DDAFD1")
{
	virtual LONG __stdcall				ID_Number() const PURE;
	virtual BSTR __stdcall				Name() const PURE;
	virtual IApplication* __stdcall		Get_Application() PURE;
	virtual LONG __stdcall				Available_Money() const PURE;
	virtual LONG __stdcall				Available_Storage() const PURE;
	virtual LONG __stdcall				Power_Output() const PURE;
	virtual LONG __stdcall				Power_Drain() const PURE;
	virtual LONG __stdcall				Category_Quantity(Category category) const PURE;
	virtual LONG __stdcall				Category_Power(Category category) const PURE;
	virtual CellStruct __stdcall		Base_Center() const PURE;
	virtual LONG __stdcall			Fire_Sale() const PURE;
	virtual LONG __stdcall			All_To_Hunt() PURE;
};