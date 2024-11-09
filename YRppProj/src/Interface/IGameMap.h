#pragma once

#include <unknwn.h>
#include <comdef.h>
#include <GeneralStructures.h>

DECLARE_INTERFACE_IID_(IGameMap, IUnknown, "96F02EC7-6FE8-11D1-B6FD-00A024DDAFD1")
{
	virtual BOOL __stdcall Is_Visible(CellStruct cell) PURE;
};

_COM_SMARTPTR_TYPEDEF(IGameMap, __uuidof(IGameMap));
