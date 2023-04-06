#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>

BombExt::ExtContainer BombExt::ExtMap;
BombClass* BombExt::BombTemp = nullptr;

HouseClass* __fastcall BombExt::GetOwningHouse(BombClass* pThis, void* _) { return pThis->OwnerHouse; }

DamageAreaResult __fastcall BombExt::DamageArea(CoordStruct* pCoord, int Damage, TechnoClass* Source, WarheadTypeClass* Warhead, bool AffectTiberium, HouseClass* SourceHouse)
{
	const auto pBomb = BombExt::BombTemp;
	auto nCoord = *pCoord;
	const auto OwningHouse = pBomb->GetOwningHouse();
	const auto nResult = MapClass::Instance->DamageArea(nCoord, Damage, Source, Warhead, Warhead->Tiberium, OwningHouse);
						 MapClass::Instance->FlashbangWarheadAt(Damage, Warhead, nCoord);
	const auto pCell = MapClass::Instance->GetCellAt(nCoord);

	if (auto pAnimType = MapClass::Instance->SelectDamageAnimation(Damage, Warhead, pCell->LandType, nCoord)) {
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -15, false)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, OwningHouse, pBomb->Target ? pBomb->Target->GetOwningHouse() : nullptr, pBomb->Owner, false);
		}
	}

	return nResult;
}

// =============================
// load / save

template <typename T>
void BombExt::ExtData::Serialize(T& Stm) { 

	Stm
		.Process(this->Weapon)

		;
}

void BombExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BombClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BombExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BombClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BombExt::ExtContainer::InvalidateExtDataIgnorable(void* const ptr) const
{
	switch ((((DWORD*)ptr)[0]))
	{
	case BombClass::vtable:
		return false;
	}

	return true;
}

void BombExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved)
{
	AnnounceInvalidPointer(BombExt::BombTemp, ptr);
}

bool BombExt::LoadGlobals(PhobosStreamReader& Stm)
{
	Stm.Process(BombExt::BombTemp);
	return true;
}

bool BombExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	 Stm.Process(BombExt::BombTemp);
	 return true;
}

// =============================
// container

BombExt::ExtContainer::ExtContainer() : Container("BombClass") { };
BombExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//
DEFINE_HOOK_AGAIN(0x438EE9, BombClass_CTOR , 0x6)
DEFINE_HOOK(0x4385FC, BombClass_CTOR, 0x6)
{
	GET(BombClass*, pItem, ESI);
#
	BombExt::ExtMap.Allocate(pItem);

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

DEFINE_HOOK(0x438BAD, BombClass_Load_Suffix, 0x9)
{
	GET(BombClass*, pThis, ESI);
	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->Target);
	BombExt::ExtMap.LoadStatic();
	return 0x438BBB;
}

DEFINE_HOOK(0x438BE4, BombClass_Save_Suffix, 0x5)
{
	GET(HRESULT, nRes, EAX);

	if(SUCCEEDED(nRes))
		BombExt::ExtMap.SaveStatic();

	return 0;
}