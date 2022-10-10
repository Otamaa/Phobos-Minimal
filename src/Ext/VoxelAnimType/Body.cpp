#include "Body.h"

VoxelAnimTypeExt::ExtContainer VoxelAnimTypeExt::ExtMap;

void VoxelAnimTypeExt::ExtData::Initialize(){
	LaserTrail_Types.reserve(1);
}

void VoxelAnimTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->Get()->ID;
	INI_EX exINI(pINI);

	if (!pINI->GetSection(pID))
		return;

	this->LaserTrail_Types.Read(exINI, pID, "LaserTrail.Types");
	this->Warhead_Detonate.Read(exINI, pID, "Warhead.Detonate");

#pragma region Otamaa
	this->SplashList.Read(exINI, pID, "SplashAnims");
	this->SplashList_Pickrandom.Read(exINI, pID, "SplashAnims.PickRandom");
	this->WakeAnim.Read(exINI, pID, "WakeAnim");
	this->ExplodeOnWater.Read(exINI, pID, "ExplodeOnWater");
	this->Damage_DealtByOwner.Read(exINI, pID, "Damage.DealtByOwner");
	this->ExpireDamage_ConsiderInvokerVet.Read(exINI, pID, "ExpireDamage.ConsiderInvokerVeterancy");
	this->Weapon.Read(exINI, pID, "Weapon", true);
#ifdef COMPILE_PORTED_DP_FEATURES
	this->Trails.Read(exINI, pID, true);
#endif
#pragma endregion
}

// =============================
// load / save
template <typename T>
void VoxelAnimTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(LaserTrail_Types)

		.Process(SplashList)
		.Process(SplashList_Pickrandom)
		.Process(Warhead_Detonate)
		.Process(WakeAnim)
		.Process(ExplodeOnWater)
		.Process(Damage_DealtByOwner)
		.Process(Weapon)
		.Process(ExpireDamage_ConsiderInvokerVet)
		;
#ifdef COMPILE_PORTED_DP_FEATURES
	this->Trails.Serialize(Stm);
#endif
}

void VoxelAnimTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TExtension<VoxelAnimTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void VoxelAnimTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TExtension<VoxelAnimTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void VoxelAnimTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {}

bool VoxelAnimTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool VoxelAnimTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

VoxelAnimTypeExt::ExtContainer::ExtContainer() : TExtensionContainer("VoxelVoxelAnimTypeClass") {}
VoxelAnimTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x74AF5C, VoxelAnimTypeClass_CTOR, 0x7)
{
	GET(VoxelAnimTypeClass*, pItem, ESI);
#ifndef ENABLE_NEWHOOKS
	VoxelAnimTypeExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	VoxelAnimTypeExt::ExtMap.FindOrAllocate(pItem);
#endif
	return 0;
}

DEFINE_HOOK(0x74BA66, VoxelAnimTypeClass_DTOR, 0x7)
{
	GET(VoxelAnimTypeClass*, pItem, ESI);

	VoxelAnimTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x74B810, VoxelAnimTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x74B8D0, VoxelAnimTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(VoxelAnimTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	VoxelAnimTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x74B8C2, VoxelAnimTypeClass_Load_Suffix, 0x7)
{
	VoxelAnimTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x74B8EA, VoxelAnimTypeClass_Save_Suffix, 0x5)
{
	VoxelAnimTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x74B607, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B561, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B54A, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B51B, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x74B4F0, VoxelAnimTypeClass_LoadFromINI, 0x5)
{
	GET(VoxelAnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x4);

	VoxelAnimTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}