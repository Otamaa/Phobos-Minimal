#include "EMPField.h"

std::vector<const char*> SW_EMPField::GetTypeString() const
{
	return { "EMPField" };
}

bool SW_EMPField::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	auto pType = pThis->Type;
	auto pData = SWTypeExt::ExtMap.Find(pType);

	if (pData->EMPField_Duration == 0)
		return false;

	auto pFirer = this->GetFirer(pThis,Coords , false);

	//does not work ?
	return GameCreate<EMPulseClass>(Coords, this->GetRange(pData).width() , pData->EMPField_Duration.Get(), pFirer);
}

void SW_EMPField::Initialize(SWTypeExt::ExtData* pData)
{
	pData->OwnerObject()->Action = Action(AresNewActionType::SuperWeaponAllowed);
	pData->SW_RadarEvent = false;
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::IronCurtain;
	pData->CursorType = int(MouseCursorType::Attack);
	pData->NoCursorType = int(MouseCursorType::AttackOutOfRange);
}

void SW_EMPField::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->Get()->ID;

	INI_EX exINI(pINI);
	pData->EMPField_Duration.Read(exINI, section, "EMPField.Duration");
}

bool SW_EMPField::IsLaunchSite(const SWTypeExt::ExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}