#include "DropPod.h"

std::vector<const char*> SW_DropPod::GetTypeString() const
{
	return { "DropPod" , "DropPodReinforcement" };
}

bool SW_DropPod::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_DropPod::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("DropPod[%s] init\n", pData->Get()->ID);
}

void SW_DropPod::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}
