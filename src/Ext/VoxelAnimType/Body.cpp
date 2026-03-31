#include "Body.h"
#include <Utilities/Macro.h>

void VoxelAnimTypeExtData::Initialize(){
	LaserTrail_Types.reserve(1);
}

bool VoxelAnimTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	if (!this->ObjectTypeExtData::LoadFromINI(pINI, parseFailAddr))
		return false;

	const char* pID = this->This()->ID;
	INI_EX exINI(pINI);

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
	this->TrailerAnim_SpawnDelay.Read(exINI, pID, "Trailer.SpawnDelay");
#pragma endregion

	return true;
}

// =============================
// load / save
template <typename T>
void VoxelAnimTypeExtData::Serialize(T& Stm)
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
		.Process(TrailerAnim_SpawnDelay)
	;
}

// =============================
// container
VoxelAnimTypeExtContainer VoxelAnimTypeExtContainer::Instance;

void VoxelAnimTypeExtContainer::LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr)
{
	if (auto ptr = this->Find(key))
	{
		if (!pINI)
		{
			return;
		}

		//load anywhere other than rules
		ptr->LoadFromINI(pINI, parseFailAddr);
		//this function can be called again multiple time but without need to re-init the data
		ptr->SetInitState(InitState::Ruled);
	}

}

void VoxelAnimTypeExtContainer::WriteToINI(ext_t::base_type* key, CCINIClass* pINI)
{

	if (auto ptr = this->TryFind(key))
	{
		if (!pINI)
		{
			return;
		}

		ptr->WriteToINI(pINI);
	}
}

ASMJIT_PATCH(0x74AF5C, VoxelAnimTypeClass_CTOR, 0x7)
{
	GET(VoxelAnimTypeClass*, pItem, ESI);
	VoxelAnimTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x74BA66, VoxelAnimTypeClass_DTOR, 0x7)
{
	GET(VoxelAnimTypeClass*, pItem, ESI);

	VoxelAnimTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

bool FakeVoxelAnimTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	VoxelAnimTypeExtContainer::Instance.Find(this)->SplashList.reserve(RulesClass::Instance->SplashList.Count);
	bool status = this->VoxelAnimTypeClass::LoadFromINI(pINI);
	VoxelAnimTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F65AC, FakeVoxelAnimTypeClass::_ReadFromINI)
