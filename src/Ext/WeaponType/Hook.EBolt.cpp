#include "Body.h"
#include <EBolt.h>
#include <map>

#include <Misc/AresData.h>

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
	const WeaponTypeExt::ExtData* pType = nullptr;
}

//DEFINE_HOOK(0x6FD494, TechnoClass_FireEBolt_SetExtMap_AfterAres, 0x7)
//{
//	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFS(0x30, -0x8));
//	GET(EBolt* const, pBolt, EAX);
//
//	if (pWeapon) {
//		WeaponTypeExt::boltWeaponTypeExt[pBolt] =  WeaponTypeExt::ExtMap.Find(pWeapon);
//	}
//
//	return 0;
//}

static std::vector<BYTE> dump;

inline unsigned inline_02(ConvertClass* pConvert , int idx)
{
	switch (pConvert->BytesPerPixel)
	{
	default:
	case ConvertClass::BytesPerPixel::One:
		return reinterpret_cast<uint8_t*>(pConvert->BufferMid)[idx];
	case ConvertClass::BytesPerPixel::Two:
		return reinterpret_cast<uint16_t*>(pConvert->BufferMid)[idx];
	};
}

DEFINE_OVERRIDE_HOOK(0x4C1F33, EBolt_Draw_Colors, 7)
{
	GET(EBolt*, pThis, ECX);
	GET_BASE(int, nColorIdx, 0x20);

	auto& data1 = Ares_EboltColors1;
	auto& data2 = Ares_EboltColors2;
	auto& data3 = Ares_EboltColors3;
	auto const& nMap = Ares_EboltMap;

	const auto nFirst = FileSystem::PALETTE_PAL()->inline_02(nColorIdx);
	const auto nSec = FileSystem::PALETTE_PAL()->inline_02(15);
	data1 = data2 = nFirst;
	data3 = nSec;


	if (auto pAresExt = nMap.get_or_default(pThis))
	{
		const auto pData = WeaponTypeExt::ExtMap.Find(pAresExt->AttachedToObject);
		BoltTemp::pType = pData;

		auto& clr1 = pData->Bolt_Color1;
		if (clr1.isset()) { data1 = Drawing::ColorStructToWord(clr1.Get()); }

		auto& clr2 = pData->Bolt_Color2;
		if (clr2.isset()) { data2 = Drawing::ColorStructToWord(clr2.Get()); }

		auto& clr3 = pData->Bolt_Color3;
		if (clr3.isset()) { data3 = Drawing::ColorStructToWord(clr3.Get()); }
	}

	return 0x4C1F66;
}

//DEFINE_HOOK(0x4C2951, EBolt_DTOR, 0x5)
//{
//	GET(EBolt* const, pBolt, ECX);
//
//	WeaponTypeExt::boltWeaponTypeExt.erase(pBolt);
//	
//	return 0;
//}

DEFINE_HOOK(0x4C24E4, Ebolt_DrawFist_Disable, 0x8)
{
	//GET_STACK(EBolt* const, pBolt, 0x40);

	if (BoltTemp::pType && BoltTemp::pType->Bolt_Disable1){

		if (!BoltTemp::pType->Bolt_Disable2 && !BoltTemp::pType->Bolt_Disable3)
			BoltTemp::pType = nullptr;

		return 0x4C2515;
	}

	return 0;
}

DEFINE_HOOK(0x4C20BC, EBolt_DrawArcs, 0xB)
{
	enum { DoLoop = 0x4C20C7, Break = 0x4C2400 };

	//GET_STACK(EBolt*, pBolt, 0x40);
	GET_STACK(int, plotIndex, STACK_OFFSET(0x408, -0x3E0))
	
	if(BoltTemp::pType){
		return plotIndex < BoltTemp::pType->Bolt_Arcs
		? DoLoop : Break;
	}

	return 0x0;
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