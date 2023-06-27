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

	BuildingClass* pFirer = nullptr;

	for (auto const& pBld : pThis->Owner->Buildings) {	
		if (pData->IsLaunchSiteEligible(Coords, pBld, false)) {
			pFirer = pBld;
			break;
		}
	}

	//does not work ?
	return GameCreate<EMPulseClass>(Coords, this->GetRange(pData).width() , pData->EMPField_Duration.Get(), pFirer);
}

void SW_EMPField::Initialize(SWTypeExt::ExtData* pData)
{
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