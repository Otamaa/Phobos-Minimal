#include "Body.h"
#include <EBolt.h>
#include <map>

DEFINE_HOOK(0x6FD5FC, TechnoClass_CreateEbolt_UnnessesaryData, 0xA)
{
	GET(UnitClass*, pThis, ESI);
	GET(int, nWeaponIdx, EBX);
	GET(EBolt*, pBolt, EDI);

	pThis->ElectricBolt = pBolt;
	pBolt->WeaponSlot = nWeaponIdx;

	return 0x6FD60B;
}

namespace BoltTemp
{
	std::map<EBolt*, bool[3]> bolt_disabled {};
	bool Disable[3] { false };
}

DEFINE_HOOK(0x6FD494, TechnoClass_FireEBolt_SetExtMap_AfterAres, 0x7)
{
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x30, -0x8));
	GET(EBolt*, pBolt, EAX);

	if (pWeapon){
		auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		auto & nDisable = BoltTemp::bolt_disabled[pBolt];
		nDisable[0] = pExt->Bolt_Disable1;
		nDisable[1] = pExt->Bolt_Disable2;
		nDisable[2] = pExt->Bolt_Disable3;
	}

	return 0;
}

DEFINE_HOOK(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt*, pBolt, ECX);

	BoltTemp::bolt_disabled.erase(pBolt);
	BoltTemp::Disable[0] = false;
	BoltTemp::Disable[1] = false;
	BoltTemp::Disable[2] = false;

	return 0;
}

DEFINE_HOOK(0x4C24E4, Ebolt_DrawFist_Disable, 0x8)
{
	GET_STACK(EBolt*, pBolt, 0x40);

	if (BoltTemp::bolt_disabled.contains(pBolt))
	{
		auto const& nData = BoltTemp::bolt_disabled.at(pBolt);
		BoltTemp::Disable[0] = nData[0];
		BoltTemp::Disable[1] = nData[1];
		BoltTemp::Disable[2] = nData[2];
	}

	return BoltTemp::Disable[0] ? 0x4C2515 :0;
}

DEFINE_HOOK(0x4C25FD, Ebolt_DrawSecond_Disable, 0xA) {
	return BoltTemp::Disable[1] ? 0x4C262A : 0;
}

DEFINE_HOOK(0x4C26EE, Ebolt_DrawThird_Disable, 0x8) {
	return BoltTemp::Disable[2] ? 0x4C2710 : 0;
}
