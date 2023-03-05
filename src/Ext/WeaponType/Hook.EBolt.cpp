#include "Body.h"
#include <EBolt.h>
#include <map>

DEFINE_HOOK(0x6FD5FC, TechnoClass_CreateEbolt_UnnessesaryData, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	GET(int, nWeaponIdx, EBX);
	GET(EBolt*, pBolt, EDI);

	pThis->ElectricBolt = pBolt;
	pBolt->Owner = pThis;
	pBolt->WeaponSlot = nWeaponIdx;

	return 0x6FD60B;
}

namespace BoltTemp
{
	std::unordered_map<EBolt*, const WeaponTypeExt::ExtData*> boltWeaponTypeExt;
	const WeaponTypeExt::ExtData* pType = nullptr;
}

DEFINE_HOOK(0x6FD494, TechnoClass_FireEBolt_SetExtMap_AfterAres, 0x7)
{
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x30, -0x8));
	GET(EBolt* const, pBolt, EAX);

	if (pWeapon) {
		auto const pWpTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		if(pWpTypeExt->Bolt_Disable1 || pWpTypeExt->Bolt_Disable2 || pWpTypeExt->Bolt_Disable3){
			BoltTemp::boltWeaponTypeExt.emplace(pBolt, pWpTypeExt);
		}
	}

	return 0;
}

DEFINE_HOOK(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt* const, pBolt, ECX);

	BoltTemp::boltWeaponTypeExt.erase(pBolt);
	
	return 0;
}

DEFINE_HOOK(0x4C24E4, Ebolt_DrawFist_Disable, 0x8)
{
	GET_STACK(EBolt* const, pBolt, 0x40);

	auto const& nMap = BoltTemp::boltWeaponTypeExt;

	if (nMap.contains(pBolt)) {

		auto const pWeaponLinked = nMap.at(pBolt);

		if (pWeaponLinked)
		{

			if (pWeaponLinked->Bolt_Disable3 || pWeaponLinked->Bolt_Disable2)
				BoltTemp::pType = pWeaponLinked;

			if (pWeaponLinked->Bolt_Disable1)
				return 0x4C2515;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4C25FD, Ebolt_DrawSecond_Disable, 0xA)
{
	if (BoltTemp::pType && BoltTemp::pType->Bolt_Disable2) {
		if (!BoltTemp::pType->Bolt_Disable3)
			BoltTemp::pType = nullptr;	

		return 0x4C262A;
	}

	return 0;
}

DEFINE_HOOK(0x4C26EE, Ebolt_DrawThird_Disable, 0x8)
{
	if (BoltTemp::pType && BoltTemp::pType->Bolt_Disable3) {
		BoltTemp::pType = nullptr;
		return 0x4C2710;
	} 	

	return  0;
}