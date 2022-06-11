#include "Body.h"
#include <Ext/WeaponType/Body.h>

template<> const DWORD Extension<AircraftClass>::Canary = 0x55555765;
AircraftExt::ExtContainer AircraftExt::ExtMap;

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber)
{
	int weaponIndex = pThis->SelectWeapon(pTarget);
	auto weaponType = pThis->GetWeapon(weaponIndex)->WeaponType;
	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(weaponType);

	if (weaponType->Burst > 0)
	{
		for (int i = 0; i < weaponType->Burst; i++)
		{
			if (weaponType->Burst < 2 && pWeaponTypeExt->Strafing_SimulateBurst)
				pThis->CurrentBurstIndex = (int)shotNumber;

			pThis->Fire(pThis->Target, weaponIndex);
		}
	}
}

// =============================
// load / save
template <typename T>
void AircraftExt::ExtData::Serialize(T& Stm)
{
	Stm
		;
}

void AircraftExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AircraftClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AircraftExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AircraftClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void AircraftExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

bool AircraftExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool AircraftExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

AircraftExt::ExtContainer::ExtContainer() : Container("AircraftClass") { }
AircraftExt::ExtContainer::~ExtContainer() = default;

