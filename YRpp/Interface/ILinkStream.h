#pragma once

#include <unknwn.h>

DECLARE_INTERFACE_IID_(ILinkStream, IUnknown, "0D5CD78E-6470-11D2-9B74-00104B972FE8")
{
	virtual HRESULT __stdcall Link_Stream(IUnknown* stream) PURE;
	virtual HRESULT __stdcall Unlink_Stream(IUnknown** stream) PURE;
};
