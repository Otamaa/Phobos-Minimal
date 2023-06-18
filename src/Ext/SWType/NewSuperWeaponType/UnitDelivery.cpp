#include "UnitDelivery.h"

std::vector<const char*> SW_UnitDelivery::GetTypeString() const
{
	return { "UnitDelivery" };
}

bool SW_UnitDelivery::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	SuperWeaponTypeClass* pSW = pThis->Type;
	SWTypeExt::ExtData* pData = SWTypeExt::ExtMap.Find(pSW);

	int deferment = pData->SW_Deferment.Get(-1);
	if (deferment < 0)
	{
		deferment = 20;
	}

	this->newStateMachine(deferment, Coords, pThis);

	return true;
}

void SW_UnitDelivery::Initialize(SWTypeExt::ExtData* pData)
{ 
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::ParaDrop;
}

void SW_UnitDelivery::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();

	INI_EX exINI(pINI);
	pData->SW_Deliverables.Read(exINI, section, "Deliver.Types");
	pData->SW_DeliverBuildups.Read(exINI, section, "Deliver.Buildups");
	pData->SW_OwnerHouse.Read(exINI, section, "Deliver.Owner");
}