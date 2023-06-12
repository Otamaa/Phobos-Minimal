#include "MultiLauncher.h"

const char* SW_MultiLauncher::GetTypeString() const
{
	return "MultiLauncher";
}

void SW_MultiLauncher::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("MultiLauncher[%s] init\n", pData->Get()->ID);
}