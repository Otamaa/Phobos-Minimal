#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/AnimType/Body.h>

#include <Utilities/Macro.h>

#include <ParticleSystemClass.h>
#include <ColorScheme.h>

//std::vector<CellClass*> AnimExt::AnimCellUpdater::Marked;
AnimExt::ExtContainer AnimExt::ExtMap;

void AnimExt::ExtData::InitializeConstants()
{
	CreateAttachedSystem();
}

AbstractClass* AnimExt::GetTarget(const AnimClass* const pThis)
{
	auto const pType = pThis->Type;
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pType);

	switch (pTypeExt->Damage_TargetFlag.Get())
	{
	case DamageDelayTargetFlag::Cell:
		return  pThis->GetCell();
	case DamageDelayTargetFlag::AttachedObject:
	{
		if (pThis->AttachedBullet)
		{
			return pThis->AttachedBullet->Owner;
		}
		else
		{
			if (auto const pBullet = specific_cast<BulletClass*>(pThis->OwnerObject))
				return pBullet->Owner;
			else
				return pThis->OwnerObject;
		}
	}
	case DamageDelayTargetFlag::Invoker:
	{
		return AnimExt::ExtMap.Find(pThis)->Invoker;
	}
	}

	return nullptr;
}

bool AnimExt::ExtData::InvalidateIgnorable(void* const ptr) const
{
	auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
	switch (abs)
	{
	case AbstractType::Building:
	case AbstractType::Infantry:
	case AbstractType::Unit:
	case AbstractType::Aircraft:
	case AbstractType::ParticleSystem:
		return false;
	}

	return true;
}

void AnimExt::ExtData::InvalidatePointer(void* const ptr, bool bRemoved)
{
	if (this->InvalidateIgnorable(ptr))
		return;

	AnnounceInvalidPointer(this->Invoker, ptr);
	AnnounceInvalidPointer(this->ParentBuilding, ptr);
	AnnounceInvalidPointer(this->AttachedSystem, ptr);
}

// =============================
// load / save

template <typename T>
void AnimExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Something)
		.Process(this->DeathUnitFacing)
		.Process(this->DeathUnitTurretFacing)
		.Process(this->Invoker)
		.Process(this->AttachedSystem)
		.Process(this->OwnerSet)
		//.Process(this->SpawnData)
		.Process(this->ParentBuilding)
		;
}

void AnimExt::ExtData::CreateAttachedSystem()
{
	const auto pThis = this->Get();
	const auto pData = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (!pData || !pData->AttachedSystem || this->AttachedSystem)
		return;
	auto nLoc =  pThis->Location;

	if(pData->AttachedSystem->BehavesLike == BehavesLike::Smoke)
		nLoc.Z += 100;

	if (auto const pSystem = GameCreate<ParticleSystemClass>(pData->AttachedSystem.Get(), nLoc, pThis->GetCell(), pThis, CoordStruct::Empty, pThis->GetOwningHouse()))
		this->AttachedSystem = pSystem;
}

void AnimExt::ExtData::DeleteAttachedSystem()
{
	if (!this->AttachedSystem)
		return;

	this->AttachedSystem->Owner = nullptr;
	this->AttachedSystem->UnInit();
	this->AttachedSystem = nullptr;
}

//Modified from Ares
const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
{
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
	if (pTypeExt->NoOwner)
		// no we dont set the owner
	// this return also will prevent Techno `Invoker` to be set !
		return false;

	if (pTypeExt->CreateUnit.Get())
	{
		if (const auto newOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
		{
			pAnim->SetHouse(newOwner);

			if (pTypeExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
				pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;

			return true;//yes
		}
	}

	if (!pAnim->Owner && pInvoker)
		pAnim->SetHouse(pInvoker);

	return true; //yes return true
}

const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, TechnoClass* pTechnoInvoker, bool defaultToVictimOwner)
{
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
	if (pTypeExt->NoOwner)
		// no we dont set the owner
		// this return also will prevent Techno `Invoker` to be set !
		return false;

	AnimExt::ExtMap.Find(pAnim)->Invoker = pTechnoInvoker;

	if (pTypeExt->CreateUnit.Get())
	{
		if (const auto newOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
		{
			pAnim->SetHouse(newOwner);

			if (pTypeExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
				pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;

			return true;//yes
		}
	}

	if (!pAnim->Owner && pInvoker)
		pAnim->SetHouse(pInvoker);

	return true; //yes return true
}

//Modified from Ares
//const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
//{
//	auto const pExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
//
//	if (!pExt->NoOwner)
//	{
//		if (!pExt->CreateUnit.Get())
//		{
//			if (!pAnim->Owner && pInvoker)
//				pAnim->SetHouse(pInvoker);
//
//			return true; //yes return true
//		}
//		else
//		{
//			if (auto const newOwner = HouseExt::GetHouseKind(pExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
//			{
//
//				pAnim->SetHouse(newOwner);
//				AnimExt::ExtMap.Find(pAnim)->OwnerSet = true;
//
//				if (pExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
//					pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;
//
//				return true;//yes
//			}
//		}
//	}
//
//	// no we dont set the owner
//	// this return also will prevent Techno `Invoker` to be set !
//	return false;
//}

TechnoClass* AnimExt::GetTechnoInvoker(const AnimClass* const pThis, bool DealthByOwner)
{
	if (!DealthByOwner)
		return nullptr;

	auto const pExt = AnimExt::ExtMap.Find(pThis);
	if (pExt && pExt->Invoker)
		return pExt->Invoker;

	if (pThis->OwnerObject)
	{
		switch (pThis->OwnerObject->WhatAmI())
		{
		case AbstractType::Building:
		case AbstractType::Unit:
		case AbstractType::Infantry:
		case AbstractType::Aircraft:
			return static_cast<TechnoClass*>(pThis->OwnerObject);
		case AbstractType::Bullet:
			return static_cast<BulletClass*>(pThis->OwnerObject)->Owner;
		}
	}

	if (auto const pBullet = pThis->AttachedBullet)
		return pBullet->Owner;

	return nullptr;
}

Layer __fastcall AnimExt::GetLayer_patch(AnimClass* pThis, void* _)
{
	if (!pThis->OwnerObject)
		return pThis->Type ? pThis->Type->Layer : Layer::Air;

	const auto pExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (!pExt || !pExt->Layer_UseObjectLayer.isset())
			return Layer::Ground;

	if (pExt->Layer_UseObjectLayer.Get())
	{
		if (auto const pFoot = generic_cast<FootClass*>(pThis->OwnerObject))
		{
			if (auto const pLocomotor = pFoot->Locomotor.get())
				return pLocomotor->In_Which_Layer();
		}
		else if (auto const pBullet = specific_cast<BulletClass*>(pThis->OwnerObject))
			return pBullet->InWhichLayer();

		return pThis->OwnerObject->ObjectClass::InWhichLayer();
	}

	return pThis->Type ? pThis->Type->Layer : Layer::Air;
}

bool AnimExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		//.Process(AnimCellUpdater::Marked)
		.Success();
}

bool AnimExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		//.Process(AnimCellUpdater::Marked)
		.Success();
}

// =============================
// container
void AnimExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AnimClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AnimExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AnimClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

AnimExt::ExtContainer::ExtContainer() : Container("AnimClass") { }
AnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//DEFINE_HOOK_AGAIN(0x4226F0, AnimClass_AltExt_CTOR, 0x6)
//DEFINE_HOOK_AGAIN(0x4228D2, AnimClass_AltExt_CTOR, 0x5)

//Only Extend Anim that Has "Type" Pointer
DEFINE_HOOK(0x422131, AnimClass_CTOR, 0x6)
{
	GET(AnimClass*, pItem, ESI);
#ifndef ENABLE_NEWEXT
	AnimExt::ExtMap.JustAllocate(pItem, pItem->Fetch_ID() != -2, "Creating an animation with invalid ID !");
#else
	AnimExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

#ifdef ENABLE_NEWHOOKS
DEFINE_HOOK(0x426590, AnimClass_SDDTOR, 0x8)
{
	GET(AnimClass* const, pItem, ECX);
	GET_STACK(char, nFlag, 0x4);

	pItem->DestroyPointer();
	if ((nFlag & 1) != 0)
		YRMemory::Deallocate(pItem);

	AnimExt::ExtMap.Remove(pItem);

	R->EAX(pItem);
	return 0x4265AB;
}
#else
DEFINE_HOOK(0x422A59, AnimClass_DTOR, 0x6)
{
	GET(AnimClass* const, pItem, ESI);
	AnimExt::ExtMap.Remove(pItem);
	return 0;
}
#endif

DEFINE_HOOK_AGAIN(0x425280, AnimClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4253B0, AnimClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(AnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	AnimExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x425391, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK_AGAIN(0x4253A2, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x425358, AnimClass_Load_Suffix, 0x7)
{
	AnimExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x4253FF, AnimClass_Save_Suffix, 0x5)
{
	AnimExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x425164, AnimClass_Detach, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (auto pAnimExt = AnimExt::ExtMap.Find(pThis))
	{
		pAnimExt->InvalidatePointer(target, all);
	}

	R->EBX(0);
	return pThis->OwnerObject == target && target ? 0x425174 : 0x4251A3;
}

//remove from CRC
DEFINE_JUMP(LJMP, 0x42543A, 0x425448)

DEFINE_HOOK_AGAIN(0x421EF4, AnimClass_CTOR_setD0, 0x6)
DEFINE_HOOK(0x42276D, AnimClass_CTOR_setD0, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	pThis->unknown_D0 = 0;
	return R->Origin() + 0x6;
}
