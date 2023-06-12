#include "IonCannon.h"

std::vector<const char*> SW_IonCannon::GetTypeString() const
{
	return { "IonCannon" };
}

bool SW_IonCannon::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (Coords != CellStruct::Empty)
		return false;

	if(auto const pBlast = GameCreate<IonBlastClass>(CellClass::Cell2Coord(Coords)))
		pBlast->DisableIonBeam = false;

	return true;
}

void SW_IonCannon::Initialize(SWTypeExt::ExtData* pData)
{
	Debug::Log("IonCannon[%s] init\n", pData->Get()->ID);
}

void SW_IonCannon::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{

}

bool SW_IonCannon::IsLaunchSite(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	return false;
}

std::pair<double, double> SW_IonCannon::GetLaunchSiteRange(SWTypeExt::ExtData* pSWType, BuildingClass* pBuilding) const
{
	return {};
}

SWRange SW_IonCannon::GetRange(const SWTypeExt::ExtData* pData) const
{
	return {};
}
