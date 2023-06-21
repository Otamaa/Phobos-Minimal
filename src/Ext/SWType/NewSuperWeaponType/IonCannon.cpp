#include "IonCannon.h"

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

	//the anim is weird ,....
	if(auto const pBlast = GameCreate<IonBlastClass>(CellClass::Cell2Coord(Coords)))
		pBlast->DisableIonBeam = false;

	return true;
}

void SW_IonCannon::Initialize(SWTypeExt::ExtData* pData)
{ 
	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::Nuke;
}

void SW_IonCannon::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{ }
