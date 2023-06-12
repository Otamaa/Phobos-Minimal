#include "HunterSeeker.h"

std::vector<const char*> SW_HunterSeeker::GetTypeString() const
{
	return { "HunterSeeker" };
}

bool SW_HunterSeeker::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_HunterSeeker::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("HunterSeeker[%s] init\n", pData->Get()->ID);
}

void SW_HunterSeeker::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}
