#include "EMPField.h"

#include <Misc/Ares/Hooks/Header.h>
#include <Ext/SWType/Body.h>

std::vector<const char*> SW_EMPField::GetTypeString() const
{
	return { "EMPField" };
}

bool SW_EMPField::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	auto pType = pThis->Type;
	auto pData = SWTypeExtContainer::Instance.Find(pType);

	if (pData->EMPField_Duration <= 0)
		return false;

	auto pFirer = this->GetFirer(pThis,Coords , false);

	// TODO : creating version that not require WH to be function !
	//
	//AresEMPulse::CreateEMPulse()
	//does not work ?
	return GameCreate<EMPulseClass>(Coords, this->GetRange(pData).width() , pData->EMPField_Duration.Get(), pFirer);
}

void SW_EMPField::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action(AresNewActionType::SuperWeaponAllowed);
	pData->SW_RadarEvent = false;
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::IronCurtain;
	pData->CursorType = int(MouseCursorType::Attack);
	pData->NoCursorType = int(MouseCursorType::AttackOutOfRange);
}

void SW_EMPField::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->AttachedToObject->ID;

	INI_EX exINI(pINI);
	pData->EMPField_Duration.Read(exINI, section, "EMPField.Duration");
}

bool SW_EMPField::IsLaunchSite(const SWTypeExtData* pData, BuildingClass* pBuilding) const
{
	if (!this->IsLaunchsiteAlive(pBuilding))
		return false;

	if (!pData->SW_Lauchsites.empty() && pData->SW_Lauchsites.Contains(pBuilding->Type))
		return true;

	return this->IsSWTypeAttachedToThis(pData, pBuilding);
}
