#include "HunterSeeker.h"

const char* SW_HunterSeeker::GetTypeString() const
{
	return "HunterSeeker";
}

bool SW_HunterSeeker::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_HunterSeeker::Initialize(SWTypeExt::ExtData* pData)
{

}

void SW_HunterSeeker::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}
