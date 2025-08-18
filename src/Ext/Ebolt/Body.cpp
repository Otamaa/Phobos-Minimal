#include "Body.h"

#include <Ext/WeaponType/Body.h>

PhobosMap<EBolt*, EboltExtData> EboltExtData::Container;

bool EboltExtData::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(EboltExtData::Container)
		.Success();
}

bool EboltExtData::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(EboltExtData::Container)
		.Success();
}

void EboltExtData::Clear()
{
	EboltExtData::Container.clear();
}

void EboltExtData::GetColors(int(&color)[3] , EBolt* pBolt, Nullable<ColorStruct>& clr1, Nullable<ColorStruct>& clr2, Nullable<ColorStruct>& clr3)
{
	const int colrIdx = pBolt->AlternateColor != 0 ? 5 : 10;
	const auto nFirst = FileSystem::PALETTE_PAL()->inline_02(colrIdx);
	const auto nSec = FileSystem::PALETTE_PAL()->inline_02(15);

	color[0] = color[1] = nFirst;
	color[2] = nSec;

	if (clr1.isset()) { color[0] = Drawing::RGB_To_Int(clr1.Get()); }
	if (clr2.isset()) { color[1] = Drawing::RGB_To_Int(clr2.Get()); }
	if (clr3.isset()) { color[2] = Drawing::RGB_To_Int(clr3.Get()); }
}

EBolt* EboltExtData::_CreateOneOf(WeaponTypeClass * pWeapon, TechnoClass * pFirer) {

	auto pWpExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	auto ret = GameCreate<EBolt>();

	ret->AlternateColor = pWeapon->IsAlternateColor;
	ret->Lifetime = 1 << (std::clamp(pWpExt->Bolt_Duration.Get(), 1, 31) - 1);

	auto map = &EboltExtData::Container[ret];

	if(pFirer)
		map->BurstIndex = pFirer->CurrentBurstIndex;

	map->ParticleSysEnabled = !pWpExt->Bolt_ParticleSys.isset() || pWpExt->Bolt_ParticleSys ;
	map->pSys = pWpExt->Bolt_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem);

	map->Disable[0] = pWpExt->Bolt_Disables[0];
	map->Disable[1] = pWpExt->Bolt_Disables[1];
	map->Disable[2] = pWpExt->Bolt_Disables[2];
	map->Arcs = pWpExt->Bolt_Arcs;

	EboltExtData::GetColors(map->Color , ret, pWpExt->Bolt_Colors[0], pWpExt->Bolt_Colors[1], pWpExt->Bolt_Colors[2]);

	return ret;
}

EBolt* EboltExtData::_CreateOneOf(bool disable1 , bool disable2 , bool dosable3 , bool alternateColor, int arch , int lifetime, Nullable<ColorStruct>& clr1, Nullable<ColorStruct>& clr2, Nullable<ColorStruct>& clr3) {

	auto ret = GameCreate<EBolt>();

	ret->AlternateColor = alternateColor;
	ret->Lifetime = 1 << (std::clamp(lifetime, 1, 31) - 1);

	auto map = &EboltExtData::Container[ret];

	map->pSys = RulesClass::Instance->DefaultSparkSystem;
	map->Arcs = arch;

	EboltExtData::GetColors(map->Color , ret, clr1, clr2, clr3);

	return ret;
}
