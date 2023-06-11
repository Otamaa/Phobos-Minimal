#include "GenericWarhead.h"

const char* SW_GenericWarhead::GetTypeString() const
{
	return "GenericWarhead";
}

bool SW_GenericWarhead::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_GenericWarhead::Initialize(SWTypeExt::ExtData* pData)
{

}