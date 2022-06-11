#include "Body.h"

#include <Ext/Techno/Body.h>

template<> const DWORD TExtension<AnimExtAlt::base_type>::Canary = 0xAAAAAAAA;
AnimExtAlt::ExtContainer AnimExtAlt::ExtMap;

// =============================
// load / save

template <typename T>
void AnimExtAlt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DeathUnitFacing)
		.Process(this->DeathUnitTurretFacing)
		.Process(this->FromDeathUnit)
		.Process(this->DeathUnitHasTurret)
		.Process(this->Invoker)
		;
}

void AnimExtAlt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void AnimExtAlt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

bool AnimExtAlt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool AnimExtAlt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

AnimExtAlt::ExtContainer::ExtContainer() : TExtensionContainer("AnimClass") { }
AnimExtAlt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK_AGAIN(0x4226F0, AnimClass_AltExt_CTOR, 0xC)
DEFINE_HOOK_AGAIN(0x4228D2, AnimClass_AltExt_CTOR, 0x9)
DEFINE_HOOK(0x422126, AnimClass_AltExt_CTOR, 0x5)
{
	GET(AnimClass*, pItem, ESI);
	ExtensionWrapper::GetWrapper(pItem)->CreateExtensionObject<AnimExtAlt::ExtData>(pItem);
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
	AnimExtAlt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK_AGAIN(0x425391, AnimClass_AltExt_Load_Suffix, 0x7)
DEFINE_HOOK_AGAIN(0x4253A2, AnimClass_AltExt_Load_Suffix, 0x7)
DEFINE_HOOK(0x425358, AnimClass_AltExt_Load_Suffix, 0x7)
{
	AnimExtAlt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x4253FF, AnimClass_AltExt_Save_Suffix, 0x5)
{
	AnimExtAlt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x4251B1, AnimClass_Detach, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	GET(void* , target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if (auto pAnimExt = AnimExtAlt::GetExtData(pThis))
		pAnimExt->InvalidatePointer(target,all);

	return pThis->AttachedBullet == target ? 0x4251B9 :0x4251C9;
}
