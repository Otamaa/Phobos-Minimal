#include "Body.h"

#include <Ext/WarheadType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Macro.h>

#include <InfantryClass.h>

WaveExtData::~WaveExtData() { };

void FakeWaveClass::_DamageCell(CoordStruct* pLoc){
	if(auto pOwner = this->Owner) {
		const auto cell = CellClass::Coord2Cell(pLoc);
		const auto pCell = MapClass::Instance->GetCellAt(cell);
		const auto pWpn = this->_GetExtData()->Weapon;
		const auto pWpnExt = WeaponTypeExtContainer::Instance.Find(pWpn);
		const bool isAlt = pCell->ContainsBridge()  && this->LimboCoords.Z >= Unsorted::LevelHeight * (pCell->Level + 4);
		const auto pWH = pWpnExt->AmbientDamage_Warhead.Get(pWpn->Warhead);

		for (auto Occupier =  pCell->Cell_Occupier(isAlt); Occupier; Occupier = Occupier->NextObject ) {
			if (Occupier == this->Target && pWpnExt->AmbientDamage_IgnoreTarget)
				continue;

            if ( Occupier != pOwner
              && Occupier->IsAlive
              && Occupier->IsOnMap
              && !Occupier->InLimbo
              && Occupier->Health > 0 )
            {
				if (const auto pTechnoVictim = flag_cast_to<TechnoClass* , false>(Occupier)){
					if (pTechnoVictim->IsSinking || pTechnoVictim->IsCrashing) {
						continue;
					}

					if (const auto pUnit = cast_to<UnitClass* , false>(Occupier)) {
						if (pUnit->DeathFrameCounter > 0) {
							continue;
						}
					}
				}

				WarheadTypeExtData::DetonateAt(pWH, Occupier, pCell->GetCoordsWithBridge(), pOwner, pWpn->AmbientDamage);
            }
        }

		if(pCell->OverlayTypeIndex != -1){
			auto pOverlay = OverlayTypeClass::Array->Items[pCell->OverlayTypeIndex];
			if(pOverlay->ChainReaction){
				pCell->ChainReaction();
			}

			if(pOverlay->Wall && pWH->Wall) {
				pCell->ReduceWall(pWpn->Damage);
			}
		}

		if(pCell->Tile_Is_DestroyableCliff()){
			if(ScenarioClass::Instance->Random.RandomRanged(0,99) < RulesClass::Instance->CollapseChance) {
				MapClass::Instance->DestroyCliff(pCell);
			}
		}

		if(pCell->OverlayTypeIndex == -1) {
			TechnoClass::ClearWhoTargetingThis(pCell);
		}
	}
}

DEFINE_FUNCTION_JUMP(LJMP , 0x75F330 ,FakeWaveClass::_DamageCell)

void WaveExtData::InitWeaponData()
{
	if (!this->Weapon)
		return;

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(this->Weapon);

	switch (this->This()->Target->WhatAmI())
	{
	case UnitClass::AbsID:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[0];
		break;
	case AircraftClass::AbsID:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[1];
		break;
	case BuildingClass::AbsID:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[2];
		break;
	case InfantryClass::AbsID:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[3];
		break;
	default:
		this->ReverseAgainstTarget = pWeaponExt->Wave_Reverse[4];
		break;
	}
}

void WaveExtData::SetWeaponType(WeaponTypeClass* pWeapon, int nIdx)
{
	this->Weapon = pWeapon;
	this->WeaponIdx = nIdx;
}

WaveClass* WaveExtData::Create(CoordStruct nFrom, CoordStruct nTo, TechnoClass* pOwner, WaveType nType, AbstractClass* pTarget,
	WeaponTypeClass* pWeapon, bool FromSourceCoord)
{
	auto const pWave = GameCreate<WaveClass>(nFrom, nTo, pOwner, nType, pTarget);
	const auto pExt = WaveExtContainer::Instance.Find(pWave);
	const auto nWeaponIdx = !FromSourceCoord ? TechnoExtContainer::Instance.Find(pOwner)->CurrentWeaponIdx : -1;
	pExt->SetWeaponType(pWeapon, nWeaponIdx);
	pExt->InitWeaponData();
	pExt->SourceCoord = nFrom;

	if (pExt->CanDoUpdate)
		pWave->WaveAI();

	return pWave;
}

bool WaveExtData::ModifyWaveColor(
WORD const src, WORD& dest, int const intensity, WaveClass* const pWave, WaveColorData const* colorDatas)
{
	if (!colorDatas->Color && !colorDatas->Intent_Color.IsValid())
		return false;

	ColorStruct modified {};
	Drawing::WordToColorStruct(src, modified);

	// ugly hack to fix byte wraparound problems
	auto const upcolor = [&modified, &colorDatas , &intensity]
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

Point3D WaveExtData::GetIntent(WeaponTypeClass* pWeapon)
{
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

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

ColorStruct WaveExtData::GetColor(WeaponTypeClass* pWeapon, WaveClass* pWave)
{
	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

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

WaveColorData WaveExtData::GetWaveColor(WaveClass* pWave)
{
	const auto pData = WaveExtContainer::Instance.Find(pWave);

	if (!pData->Weapon)
		return { Point3D::Empty , ColorStruct::Empty };

	const Point3D Intent = GetIntent(pData->Weapon);
	const ColorStruct Color = GetColor(pData->Weapon, pWave);

	return { Intent , Color };
}

// =============================
// load / save
template <typename T>
void WaveExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Weapon)
		.Process(this->WeaponIdx)
		.Process(this->ReverseAgainstTarget)
		.Process(this->SourceCoord)
		.Process(this->CanDoUpdate)
		;
}

// =============================
// container
WaveExtContainer WaveExtContainer::Instance;
std::vector<WaveExtData*> Container<WaveExtData>::Array;

 void Container<WaveExtData>::Clear()
{
	Array.clear();
}

bool WaveExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool WaveExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

// =============================
// container hooks
//

ASMJIT_PATCH(0x75EA66, WaveClass_CTOR, 0x5)
{
	GET(WaveClass*, pItem, ESI);
	WaveExtContainer::Instance.FindOrAllocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x763226, WaveClass_DTOR, 0x6)
{
	GET(WaveClass*, pItem, EDI);
	WaveExtContainer::Instance.Remove(pItem);
	return 0;
}ASMJIT_PATCH_AGAIN(0x75ED57, WaveClass_DTOR, 0x6)

// broke atm , it keep shooting even the owner aleady gone
// not sure what causing it atm
//ASMJIT_PATCH(0x75F610, WaveClass_Detach, 0x5)
//{
//	GET(WaveClass*, pItem, ECX);
//	GET_STACK(AbstractClass*, pTarget, 0x4);
//	GET_STACK(bool, bRemove, 0x8);
//
//	pItem->ObjectClass::PointerExpired(pTarget, bRemove);
//
//	//WaveExtContainer::Instance.InvalidatePointerFor(pItem, pTarget, bRemove);
//
//	if (bRemove && pItem->Owner == pTarget)
//		pItem->Owner = nullptr;
//
//	if (bRemove && pItem->Target == pTarget)
//		pItem->Target = nullptr;
//
//	return 0x75F645;
//}

#include <Misc/Hooks.Otamaa.h>

void FakeWaveClass::_Detach(AbstractClass* target , bool all)\
{
	//WaveExtContainer::Instance.InvalidatePointerFor(pThis , target , all);
	this->WaveClass::PointerExpired(target , all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6C1C, FakeWaveClass::_Detach)

HRESULT __stdcall FakeWaveClass::_Load(IStream* pStm)
{
	HRESULT hr = this->WaveClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = WaveExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeWaveClass::_Save(IStream* pStm, BOOL clearDirty)
{
	HRESULT hr = this->WaveClass::Save(pStm, clearDirty);
	if (SUCCEEDED(hr))
		hr = WaveExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6C08, FakeWaveClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F6C0C, FakeWaveClass::_Save)