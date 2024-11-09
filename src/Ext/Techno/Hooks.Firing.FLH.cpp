 #include "Body.h"

#include <Ext/TechnoType/Body.h>

#include <InfantryClass.h>

DEFINE_HOOK(0x6F3AF9, TechnoClass_GetFLH_GetAlternateFLH, 0x5)
{
	GET(TechnoClass*, pThis, EBX);
	GET(int, weaponIdx, ESI);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
	const auto conpy_weaponIdx = (-weaponIdx);

	if(conpy_weaponIdx <= 5 || pTypeExt->AlternateFLHs.empty())
		return 0x0;

	const auto conpy_weaponIdx_B = Math::abs(5 + weaponIdx);
	Debug::Log("[%s] Trying to get Additional AlternateFLH at [original %d vs changed %d] !\n", pTypeExt->AttachedToObject->ID, conpy_weaponIdx , conpy_weaponIdx_B);

	if((size_t)conpy_weaponIdx_B < pTypeExt->AlternateFLHs.size()) {
		const CoordStruct& flh = pTypeExt->AlternateFLHs[conpy_weaponIdx_B] ;

		R->ECX(flh.X);
		R->EBP(flh.Y);
		R->EAX(flh.Z);

		return 0x6F3B37;
	}

	return 0x0;
}

DEFINE_HOOK(0x6F3B37, TechnoClass_Transform_6F3AD0_BurstFLH_1, 0x7)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));

	std::pair<bool, CoordStruct> nResult = TechnoExtData::GetBurstFLH(pThis, weaponIndex);

	if (!nResult.first && pThis->WhatAmI() == InfantryClass::AbsID) {
		nResult = TechnoExtData::GetInfantryFLH(reinterpret_cast<InfantryClass*>(pThis), weaponIndex);
	}

	if (nResult.first)
	{
		R->ECX(nResult.second.X);
		R->EBP(nResult.second.Y);
		R->EAX(nResult.second.Z);
		TechnoExtContainer::Instance.Find(pThis)->FlhChanged = true;
	}

	return 0;
}

DEFINE_HOOK(0x6F3C88, TechnoClass_Transform_6F3AD0_BurstFLH_2, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	//GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));

	auto pExt = TechnoExtContainer::Instance.Find(pThis);
		if (pExt->FlhChanged) {
			pExt->FlhChanged = false;
			R->EAX(0); //clear the angle ?
		}

	return 0;
}
