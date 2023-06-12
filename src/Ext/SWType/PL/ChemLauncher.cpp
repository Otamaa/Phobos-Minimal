#include "ChemLauncher.h"

const char* SW_ChemLauncher::GetTypeString() const
{
	return "ChemLauncher";
}

void SW_ChemLauncher::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("ChemLauncher[%s] init\n", pData->Get()->ID);
}