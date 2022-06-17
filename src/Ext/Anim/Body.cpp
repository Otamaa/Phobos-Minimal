#include "Body.h"

#include <Ext/House/Body.h>

#include <ColorScheme.h>

std::vector<CellClass*> AnimExt::AnimCellUpdater::Marked;
template<> const DWORD TExtension<AnimExt::base_type>::Canary = 0xAAAAAAAA;
AnimExt::ExtContainer AnimExt::ExtMap;

// =============================
// load / save

template <typename T>
void AnimExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DeathUnitFacing)
		.Process(this->DeathUnitTurretFacing)
		.Process(this->FromDeathUnit)
		.Process(this->DeathUnitHasTurret)
		.Process(this->Invoker)
		;
}

//Modified from Ares
const bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
{
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);

	if (!pTypeExt || !pTypeExt->CreateUnit.Get())
	{
		if(!pAnim->Owner && pInvoker)
			pAnim->SetHouse(pInvoker);

		return false;
	}

	if (auto newOwner = HouseExt::GetHouseKind(pTypeExt->CreateUnit_Owner.Get(), true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim))
	{
		pAnim->SetHouse(newOwner);

		if (pTypeExt->CreateUnit_RemapAnim.Get() && !newOwner->Defeated)
			pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;

		return true;
	}

	return false;
}

TechnoClass* AnimExt::GetTechnoInvoker(AnimClass* pThis, bool DealthByOwner)
{
	if (!DealthByOwner)
		return nullptr;

	auto pExt = AnimExt::GetExtData(pThis);
	return pExt && pExt->Invoker ? pExt->Invoker : abstract_cast<TechnoClass*>(pThis->OwnerObject);
}

bool AnimExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(AnimCellUpdater::Marked)
		.Success();
}

bool AnimExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(AnimCellUpdater::Marked)
		.Success();
}

// =============================
// container

AnimExt::ExtContainer::ExtContainer() : TExtensionContainer("AnimClass") { }
AnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK_AGAIN(0x4226F0, AnimClass_AltExt_CTOR, 0xC)
DEFINE_HOOK_AGAIN(0x4228D2, AnimClass_AltExt_CTOR, 0x9)
DEFINE_HOOK(0x422126, AnimClass_AltExt_CTOR, 0x5)
{
	GET(AnimClass*, pItem, ESI);
	ExtensionWrapper::GetWrapper(pItem)->CreateExtensionObject<AnimExt::ExtData>(pItem);
	return 0;
}

/*
DEFINE_HOOK(0x422A18, AnimClass_AltExt_DTOR, 0x8)
{
	GET(AnimClass* const, pItem, ESI);
	ExtensionWrapper::GetWrapper(pItem)->DestoryExtensionObject();
	return 0;
}*/


DEFINE_HOOK(0x422967, AnimClass_AltExt_DTOR, 0x6)
{
	GET(AnimClass* const, pItem, ESI);
	ExtensionWrapper::GetWrapper(pItem)->DestoryExtensionObject();
	R->EAX(pItem->Type);
	return 0;
}

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

DEFINE_HOOK(0x425164, AnimClass_Detach, 0x8)
{
	GET(AnimClass* const, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	//when these 2 elements not present
	//the Ext data is already gone
	//so , we just skip these and dont take care any invalidation
	if (!pThis->Type && !pThis->OwnerObject)
		return 0x0;

	if (auto pAnimExt = AnimExt::GetExtData(pThis))
		pAnimExt->InvalidatePointer(target, all);

	return 0x0;
}
/*
DEFINE_HOOK(0x4251B1, AnimClass_Detach, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(void* , target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (auto pAnimExt = AnimExt::GetExtData(pThis))
		pAnimExt->InvalidatePointer(target,all);

	return pThis->AttachedBullet == target ? 0x4251B9 :0x4251C9;
}*/
