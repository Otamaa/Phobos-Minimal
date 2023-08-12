#include "Body.h"


#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

void  WaveExt::ExtData::InitWeaponData()
{
	if (!this->Weapon)
		return;

	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(this->Weapon);

	switch (GetVtableAddr(this->Get()->Target))
	{
	case UnitClass::vtable:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[0];
		break;
	case AircraftClass::vtable:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[1];
		break;
	case BuildingClass::vtable:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[2];
		break;
	case InfantryClass::vtable:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[3];
		break;
	default:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[4];
		break;
	}
}

void WaveExt::ExtData::SetWeaponType(WeaponTypeClass* pWeapon, int nIdx)
{
	this->Weapon = pWeapon;
	this->WeaponIdx = nIdx;
}

WaveClass* WaveExt::Create(CoordStruct nFrom, CoordStruct nTo, TechnoClass* pOwner, WaveType nType, AbstractClass* pTarget,
	WeaponTypeClass* pWeapon, bool FromSourceCoord)
{
	if (auto const pWave = GameCreate<WaveClass>(nFrom, nTo, pOwner, nType, pTarget))
	{
		const auto pExt = WaveExt::ExtMap.Find(pWave);
		const auto nWeaponIdx = !FromSourceCoord ? TechnoExt::ExtMap.Find(pOwner)->CurrentWeaponIdx : -1;
		pExt->SetWeaponType(pWeapon, nWeaponIdx);
		pExt->InitWeaponData();
		pExt->SourceCoord = nFrom;

		if (pExt->CanDoUpdate)
			pWave->WaveAI();

		return pWave;
	}

	return nullptr;
}

bool WaveExt::ModifyWaveColor(
WORD const src, WORD& dest, int const intensity, WaveClass* const pWave, WaveColorData const* colorDatas)
{
	if (!colorDatas->Color && !colorDatas->Intent_Color.IsValid())
		return false;

	ColorStruct modified;
	Drawing::WordToColorStruct(src, modified);

	// ugly hack to fix byte wraparound problems
	auto const upcolor = [=, &modified, &colorDatas]
	(int Point3D::* intentmember, BYTE ColorStruct::* member)
	{
		auto const component = std::clamp(modified.*member
			+ ((intensity * colorDatas->Intent_Color.*intentmember * modified.*member) >> 16)
			+ ((intensity * colorDatas->Color.*member) >> 8), 0, 255);

		modified.*member = static_cast<BYTE>(component);
	};

	upcolor(&Point3D::X, &ColorStruct::R);
	upcolor(&Point3D::Y, &ColorStruct::G);
	upcolor(&Point3D::Z, &ColorStruct::B);

	dest = Drawing::ColorStructToWord(modified);

	return true;
}

Point3D WaveExt::GetIntent(WeaponTypeClass* pWeapon)
{
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt->Wave_Intent.isset())
		return pWeaponExt->Wave_Intent.Get();

	if (pWeapon->IsMagBeam)
	{
		return WaveClass::DefaultMag.Intent_Color;
	}
	else if (pWeapon->IsSonic)
	{
		return WaveClass::DefaultSonic.Intent_Color;
	}

	return WaveClass::DefaultLaser.Intent_Color;
}

ColorStruct WaveExt::GetColor(WeaponTypeClass* pWeapon, WaveClass* pWave)
{
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt->Wave_IsHouseColor && pWave->Owner && pWave->Owner->Owner)
		return pWave->Owner->Owner->Color;

	if (pWeaponExt->Wave_Color.isset())
		return pWeaponExt->Wave_Color.Get();

	if (pWeapon->IsMagBeam)
	{
		return WaveClass::DefaultWaveColorMagBeam //WaveClass::DefaultMag.Color
			;
	}
	else if (pWeapon->IsSonic)
	{
		return  WaveClass::DefaultWaveColorSonic//WaveClass::DefaultSonic.Color
			;
	}

	return WaveClass::DefaultLaser.Color;
}

WaveColorData WaveExt::GetWaveColor(WaveClass* pWave)
{
	const auto pData = WaveExt::ExtMap.Find(pWave);

	if (!pData->Weapon)
		return { Point3D::Empty , ColorStruct::Empty };

	const Point3D Intent = GetIntent(pData->Weapon);
	const ColorStruct Color = GetColor(pData->Weapon, pWave);

	return { Intent , Color };
}

// =============================
// load / save
template <typename T>
void WaveExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->Weapon)
		.Process(this->WeaponIdx)
		.Process(this->ReverseAgainstTarget)
		.Process(this->SourceCoord)
		.Process(this->CanDoUpdate)
		;
}

// =============================
// container
WaveExt::ExtContainer WaveExt::ExtMap;

WaveExt::ExtContainer::ExtContainer() : Container("WaveClass") {}
WaveExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
//

DEFINE_HOOK_AGAIN(0x75ED27, WaveClass_CTOR, 0x5)
DEFINE_HOOK(0x75EA59, WaveClass_CTOR, 0x5)
{
	GET(WaveClass*, pItem, ESI);
	WaveExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x75F7D0, WaveClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x75F650, WaveClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(WaveClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	WaveExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

//we load it before DVC<CellStruct> get loaded
DEFINE_HOOK(0x75F704, WaveClass_Load_Suffix, 0x7)
{
	WaveExt::ExtMap.LoadStatic();
	return 0;
}

//write it before DVC<CellStruct>
DEFINE_HOOK(0x75F7E7, WaveClass_Save_Suffix, 0x6)
{
	GET(HRESULT, nRes, EAX);

	WaveExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x75ED57 , WaveClass_DTOR, 0x6)
DEFINE_HOOK(0x763226, WaveClass_DTOR, 0x6)
{
	GET(WaveClass*, pItem, EDI);
	WaveExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK(0x75F610, WaveClass_Detach, 0x5)
{
	GET(WaveClass*, pItem, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(bool, bRemove, 0x8);

	pItem->ObjectClass::PointerExpired(pTarget, bRemove);

	//WaveExt::ExtMap.InvalidatePointerFor(pItem, pTarget, bRemove);

	if (bRemove && pItem->Owner == pTarget)
		pItem->Owner = nullptr;

	if (bRemove && pItem->Target == pTarget)
		pItem->Target = nullptr;

	return 0x75F645;
}
