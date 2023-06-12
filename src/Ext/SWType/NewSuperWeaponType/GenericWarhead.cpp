#include "GenericWarhead.h"

std::vector<const char*> SW_GenericWarhead::GetTypeString() const
{
	return { "GenericWarhead" };
}

bool SW_GenericWarhead::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_GenericWarhead::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("GenericWarhead[%s] init\n", pData->Get()->ID);
}