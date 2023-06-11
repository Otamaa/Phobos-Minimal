#include "DropPod.h"

const char* SW_DropPod::GetTypeString() const
{
	return "DropPod";
}

bool SW_DropPod::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_DropPod::Initialize(SWTypeExt::ExtData* pData)
{

}

void SW_DropPod::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}
