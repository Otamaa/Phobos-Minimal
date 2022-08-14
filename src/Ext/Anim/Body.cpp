#include "Body.h"

#include <Ext/House/Body.h>

#include <ColorScheme.h>

std::vector<CellClass*> AnimExt::AnimCellUpdater::Marked;
AnimExt::ExtContainer AnimExt::ExtMap;

void AnimExt::ExtData::InitializeConstants() {
}

AnimExt::ExtData* AnimExt::GetExtData(AnimExt::base_type* pThis)
{
	return ExtMap.Find(pThis);
}

void AnimExt::ExtData::InvalidatePointer(void* const ptr, bool bRemoved)
{
	if(Invoker)
	  AnnounceInvalidPointer(Invoker, ptr);

}

// =============================
// load / save

template <typename T>
void AnimExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DeathUnitFacing)
		.Process(this->DeathUnitTurretFacing)
		.Process(this->Invoker)
		;
}

//Modified from Ares
const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
{
	if (auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type))
	{
		if (!pTypeExt->NoOwner)
		{
			if (!pTypeExt->CreateUnit.Get())
			{
				if (!pAnim->Owner && pInvoker)
					pAnim->SetHouse(pInvoker);

				return true; //yes return true
			}
			else
			{
				if (auto newOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
				{
					pAnim->SetHouse(newOwner);

					if (pTypeExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
						pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;

					return true;//yes
				}
			}
		}
	}

	// no we dont set the owner
	// this return also will prevent Techno `Invoker` to be set !
	return false;
}

const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim,TechnoClass* pTechnoInvoker, bool defaultToVictimOwner)
{
	if (auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type))
	{
		if (!pTypeExt->NoOwner)
		{
			if (const auto pExt = AnimExt::ExtMap.Find(pAnim))
				pExt->Invoker = pTechnoInvoker;

			if (!pTypeExt->CreateUnit.Get())
			{
				if (!pAnim->Owner && pInvoker)
					pAnim->SetHouse(pInvoker);

				return true; //yes return true
			}
			else
			{
				if (auto newOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
				{
					pAnim->SetHouse(newOwner);

					if (pTypeExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
						pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;

					return true;//yes
				}
			}
		}
	}

	// no we dont set the owner
	// this return also will prevent Techno `Invoker` to be set !
	return false;
}

//Modified from Ares
const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, AnimTypeExt::ExtData* pExt, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
{
	if (!pExt->NoOwner)
	{
		if (!pExt->CreateUnit.Get())
		{
			if (!pAnim->Owner && pInvoker)
				pAnim->SetHouse(pInvoker);

			return true; //yes return true
		}
		else
		{
			if (auto const newOwner = HouseExt::GetHouseKind(pExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
			{
				pAnim->SetHouse(newOwner);

				if (pExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
					pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;

				return true;//yes
			}
		}
	}

	// no we dont set the owner
	// this return also will prevent Techno `Invoker` to be set !
	return false;
}

TechnoClass* AnimExt::GetTechnoInvoker(AnimClass* pThis, bool DealthByOwner)
{
	if (!DealthByOwner)
		return nullptr;

	auto const pExt = AnimExt::GetExtData(pThis);
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

	return nullptr;
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
	Extension<AnimClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void AnimExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AnimClass>::Serialize(Stm);
	this->Serialize(Stm);
}

AnimExt::ExtContainer::ExtContainer() : Container("AnimClass") { }
AnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK_AGAIN(0x4226F0, AnimClass_AltExt_CTOR, 0xC)
DEFINE_HOOK_AGAIN(0x4228D2, AnimClass_AltExt_CTOR, 0x9)
DEFINE_HOOK(0x422126, AnimClass_AltExt_CTOR, 0x5)
{
	GET(AnimClass*, pItem, ESI);
#ifdef ENABLE_NEWHOOKS
	AnimExt::ExtMap.JustAllocate(pItem,pItem->Type,"Creating an animation with null Type !");
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
DEFINE_HOOK(0x422967, AnimClass_AltExt_DTOR, 0x6)
{
	GET(AnimClass* const, pItem, ESI);
	AnimExt::ExtMap.Remove(pItem);
	R->EAX(pItem->Type);
	return 0;
}
#endif

/*
DEFINE_HOOK(0x422A18, AnimClass_AltExt_DTOR, 0x8)
{
	GET(AnimClass* const, pItem, ESI);
	ExtensionWrapper::GetWrapper(pItem)->DestoryExtensionObject();
	return 0;
}
*/

DEFINE_HOOK_AGAIN(0x425280, AnimClass_AltExt_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4253B0, AnimClass_AltExt_SaveLoad_Prefix, 0x5)
{
	GET_STACK(AnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	AnimExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x425391, AnimClass_AltExt_Load_Suffix, 0x7)
DEFINE_HOOK_AGAIN(0x4253A2, AnimClass_AltExt_Load_Suffix, 0x7)
DEFINE_HOOK(0x425358, AnimClass_AltExt_Load_Suffix, 0x7)
{
	AnimExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x4253FF, AnimClass_AltExt_Save_Suffix, 0x5)
{
	AnimExt::ExtMap.SaveStatic();
	return 0;
}

/*
DEFINE_HOOK(0x425164, AnimClass_Detach, 0x8)
{
	GET(AnimClass* const, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	//if the type is gone , it is mostly already detached
	if(pThis->Type) {
		if (auto pAnimExt = AnimExt::GetExtData(pThis)) {
			pAnimExt->InvalidatePointer(target, all);
		}
	}

	R->EBX(0);
	return target && pThis->OwnerObject == target ? 0x425174:0x4251A3;
}


DEFINE_HOOK(0x4251B1, AnimClass_Detach, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(void* , target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (auto pAnimExt = AnimExt::ExtMap.Find(pThis))
		pAnimExt->InvalidatePointer(target,all);

	return pThis->AttachedBullet == target ? 0x4251B9 :0x4251C9;
}*/
