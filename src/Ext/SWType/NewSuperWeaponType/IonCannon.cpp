#include "IonCannon.h"
#include <Misc/AresData.h>

#include <Ext/WarheadType/Body.h>
//TODO : rethink the implementaion of IonBlastClass
std::vector<const char*> SW_IonCannon::GetTypeString() const
{
	return { "IonCannon" };
}

SuperWeaponFlags SW_IonCannon::Flags() const
{
	return SuperWeaponFlags::None;
}

bool SW_IonCannon::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (!Coords.IsValid())
		return false;

	auto const pCell = MapClass::Instance->GetCellAt(Coords);
	auto coords = pCell->GetCoordsWithBridge();
	const auto pWarhead = GetWarhead(SWTypeExt::ExtMap.Find(pThis->Type));

	if (!pWarhead) {
		//the anim is weird ,....
		if (auto const pBlast = GameCreate<IonBlastClass>(coords))
			pBlast->DisableIonBeam = false;
	} else {
		WarheadTypeExt::CreateIonBlast(pWarhead, coords);
	}

	return true;
}

void SW_IonCannon::Initialize(SWTypeExt::ExtData* pData)
{ 
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Nuke;
}

void SW_IonCannon::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{ }