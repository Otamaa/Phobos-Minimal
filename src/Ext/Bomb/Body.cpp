#include "Body.h"

#include <Ext/Anim/Body.h>

template<> const DWORD Extension<BombClass>::Canary = 0x87659781;
BombExt::ExtContainer BombExt::ExtMap;
BombClass* BombExt::BombTemp = nullptr;

HouseClass* __fastcall BombExt::GetOwningHouse(BombClass* pThis, void* _) { return pThis->OwnerHouse; }

DamageAreaResult __fastcall BombExt::DamageArea(CoordStruct* pCoord, int Damage, TechnoClass* Source, WarheadTypeClass* Warhead, bool AffectTiberium, HouseClass* SourceHouse)
{
	const auto pBomb = BombExt::BombTemp;
	const auto OwningHouse = pBomb->GetOwningHouse();
	const auto nCoord = *pCoord;
	const auto nResult = Map.DamageArea(nCoord, Damage, Source, Warhead, Warhead->Tiberium, OwningHouse);
						 Map.FlashbangWarheadAt(Damage, Warhead, nCoord);
	const auto pCell = Map.TryGetCellAt(nCoord);

	if (auto pAnimType = Map.SelectDamageAnimation(Damage, Warhead, pCell ? pCell->LandType : LandType::Clear, nCoord)) {
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, 0x2600, -15, false)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, OwningHouse, pBomb->Target ? pBomb->Target->GetOwningHouse() : nullptr, pBomb->Owner, false);
		}
	}

	BombExt::BombTemp = nullptr;
	return nResult;
}

// =============================
// load / save

template <typename T>
void BombExt::ExtData::Serialize(T& Stm) { }

void BombExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BombClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void BombExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BombClass>::Serialize(Stm);
	this->Serialize(Stm);
}

bool BombExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm >> BombExt::BombTemp;
}

bool BombExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm << BombExt::BombTemp;
}

// =============================
// container

void BombExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {
	AnnounceInvalidPointer(BombExt::BombTemp, ptr);
}

BombExt::ExtContainer::ExtContainer() : Container("BombClass") { };
BombExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

// BombListClass::Plant()
DEFINE_HOOK_AGAIN(0x438EE9, BombClass_CTOR , 0x6)
DEFINE_HOOK(0x4385FC, BombClass_CTOR, 0x6) // is this inline ?
{
	GET(BombClass*, pItem, ESI);
	BombExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x4393F2, BombClass_SDDTOR, 0x5)
{
	GET(BombClass *, pItem, ECX);
	BombExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x438B40, BombClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x438BD0, BombClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(BombClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	BombExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x438BBD, BombClass_Load_Suffix, 0xA)
{
	BombExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x438BE4, BombClass_Save_Suffix, 0x5)
{
	BombExt::ExtMap.SaveStatic();
	return 0;
}