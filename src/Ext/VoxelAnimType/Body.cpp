#include "Body.h"

void VoxelAnimTypeExt::ExtData::Initialize(){
	LaserTrail_Types.reserve(1);
	SplashList.reserve(RulesClass::Instance->SplashList.Count);
}

void VoxelAnimTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
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
	this->Weapon.Read(exINI, pID, "Weapon" , true);

	this->Trails.Read(exINI, pID, false);
#pragma endregion
}

// =============================
// load / save
template <typename T>
void VoxelAnimTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
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
		this->Trails.Serialize(Stm);
}

// =============================
// container
VoxelAnimTypeExt::ExtContainer VoxelAnimTypeExt::ExtMap;

VoxelAnimTypeExt::ExtContainer::ExtContainer() : Container("VoxelVoxelAnimTypeClass") {}
VoxelAnimTypeExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x74AF5C, VoxelAnimTypeClass_CTOR, 0x7)
{
	GET(VoxelAnimTypeClass*, pItem, ESI);
	VoxelAnimTypeExt::ExtMap.Allocate(pItem);
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

// Before :  DEFINE_HOOK(0x74B8C2, VoxelAnimTypeClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x74B8C0 , VoxelAnimTypeClass_Load_Suffix, 0x6)
{
	VoxelAnimTypeExt::ExtMap.LoadStatic();
	return 0;
}

// Before :  DEFINE_HOOK(0x74B8EA, VoxelAnimTypeClass_Save_Suffix, 0x5)
DEFINE_HOOK(0x74B8E8, VoxelAnimTypeClass_Save_Suffix, 0x5)
{
	VoxelAnimTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x74B612, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B607, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B561, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B54A, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x74B51B, VoxelAnimTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x74B4F0, VoxelAnimTypeClass_LoadFromINI, 0x5)
{
	GET(VoxelAnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x4);

	VoxelAnimTypeExt::ExtMap.LoadFromINI(pItem, pINI , R->Origin() == 0x74B612);

	return 0;
}