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

Point3D EboltExtData::GetColors(EBolt* pBolt, Nullable<ColorStruct>& clr1, Nullable<ColorStruct>& clr2, Nullable<ColorStruct>& clr3)
{
	Point3D color { };

	const int colrIdx = pBolt->AlternateColor != 0 ? 5 : 10;

	const auto nFirst = FileSystem::PALETTE_PAL()->inline_02(colrIdx);
	const auto nSec = FileSystem::PALETTE_PAL()->inline_02(15);

	color.X = color.Y = nFirst;
	color.Z = nSec;

	if (clr1.isset()) { color.X = Drawing::ColorStructToWord(clr1.Get()); }
	if (clr2.isset()) { color.Y = Drawing::ColorStructToWord(clr2.Get()); }
	if (clr3.isset()) { color.Z = Drawing::ColorStructToWord(clr3.Get()); }

	return color;
}

EBolt* EboltExtData::_CreateOneOf(WeaponTypeClass * pWeapon, TechnoClass * pFirer) {

	auto pWpExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	auto ret = GameCreate<EBolt>();

	ret->AlternateColor = pWeapon->IsAlternateColor;
	ret->Lifetime = 1 << (std::clamp(pWpExt->Bolt_Duration.Get(), 1, 31) - 1);

	auto map = &EboltExtData::Container[ret];

	if(pFirer)
		map->BurstIndex = pFirer->CurrentBurstIndex;


	map->ParticleSysEnabled = pWpExt->Bolt_ParticleSys_Enabled;
	map->pSys = pWpExt->Bolt_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem);

	map->Disable[0] = pWpExt->Bolt_Disable1;
	map->Disable[1] = pWpExt->Bolt_Disable2;
	map->Disable[2] = pWpExt->Bolt_Disable3;

	map->Arcs = pWpExt->Bolt_Arcs;

	auto[col1, col2, col3] = GetColors(ret, pWpExt->Bolt_Color1, pWpExt->Bolt_Color2, pWpExt->Bolt_Color3);
	map->Color[0] = col1;
	map->Color[0] = col2;
	map->Color[0] = col3;

	return ret;
}

EBolt* EboltExtData::_CreateOneOf(bool disable1 , bool disable2 , bool dosable3 , bool alternateColor, int arch , int lifetime, Nullable<ColorStruct>& clr1, Nullable<ColorStruct>& clr2, Nullable<ColorStruct>& clr3) {

	auto ret = GameCreate<EBolt>();
	ret->AlternateColor = alternateColor;
	ret->Lifetime = 1 << (std::clamp(lifetime, 1, 31) - 1);
	auto map = &EboltExtData::Container[ret];
	map->pSys = RulesClass::Instance->DefaultSparkSystem;
	auto[col1, col2, col3] = GetColors(ret, clr1, clr2, clr3);
	map->Color[0] = col1;
	map->Color[0] = col2;
	map->Color[0] = col3;
	map->Arcs = arch;
	return ret;
}
