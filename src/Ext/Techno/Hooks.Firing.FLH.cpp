 #include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <InfantryClass.h>

ASMJIT_PATCH(0x6F3AEB, TechnoClass_GetFLH, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET(TechnoTypeClass*, pType, EAX);
	GET(int, weaponIndex, ESI);
	GET_STACK(CoordStruct*, pCoords, STACK_OFFSET(0xD8, 0x4));

	auto const pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);
	bool allowOnTurret = true;
	bool useBurstMirroring = true;
	CoordStruct flh = CoordStruct::Empty;

	if (weaponIndex >= 0)
	{
		auto[found , _flh] =
		!pExt->CustomFiringOffset.has_value() ?
	 	TechnoExtData::GetBurstFLH(pThis, weaponIndex) :
	 	std::make_pair(true , pExt->CustomFiringOffset.value());

		if (!found) {

			if (pThis->WhatAmI() == InfantryClass::AbsID) {
				auto res = TechnoExtData::GetInfantryFLH(reinterpret_cast<InfantryClass*>(pThis), weaponIndex);
				found = res.first;
				_flh = res.second;
			}
				
			if (!found) {
				_flh = pThis->GetWeapon(weaponIndex)->FLH;
			}

			flh = _flh;
		} else {
			useBurstMirroring = false;
		}
	}
	else
	{
		int index = -weaponIndex - 1;
		useBurstMirroring = false;

		if ((size_t)index < pTypeExt->AlternateFLHs.size())
			flh = pTypeExt->AlternateFLHs[index];

		if (!pTypeExt->AlternateFLH_OnTurret)
			allowOnTurret = false;
	}

	if (useBurstMirroring && pThis->CurrentBurstIndex % 2 != 0)
		flh.Y = -flh.Y;

	*pCoords = TechnoExtData::GetFLHAbsoluteCoords(pThis, flh, allowOnTurret);
	R->EAX(pCoords);

	return 0x6F3D50;
}
