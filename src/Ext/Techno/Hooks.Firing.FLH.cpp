 #include "Body.h"

#include <Ext/TechnoType/Body.h>


DEFINE_HOOK(0x6F3AF9, TechnoClass_GetFLH_GetAlternateFLH, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET(int, weaponIdx, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	weaponIdx = -weaponIdx - 1;

	if(weaponIdx < static_cast<int>(pTypeExt->AlternateFLHs.size())) {
		const CoordStruct& flh = pTypeExt->AlternateFLHs[weaponIdx] ;

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

	std::pair<bool, CoordStruct> nResult = TechnoExt::GetBurstFLH(pThis, weaponIndex);

	if (!nResult.first && Is_Infantry(pThis)) {
		nResult = TechnoExt::GetInfantryFLH(reinterpret_cast<InfantryClass*>(pThis), weaponIndex);
	}

	if (nResult.first)
	{
		R->ECX(nResult.second.X);
		R->EBP(nResult.second.Y);
		R->EAX(nResult.second.Z);
		TechnoExt::ExtMap.Find(pThis)->FlhChanged = true;
	}

	return 0;
}

DEFINE_HOOK(0x6F3C88, TechnoClass_Transform_6F3AD0_BurstFLH_2, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	//GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));

	auto pExt = TechnoExt::ExtMap.Find(pThis);
		if (pExt->FlhChanged) {
			pExt->FlhChanged = false;
			R->EAX(0); //clear the angle ?
		}

	return 0;
}
