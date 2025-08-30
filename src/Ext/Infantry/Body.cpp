#include "Body.h"

#include <Utilities/Macro.h>
#include <Ext/TechnoType/Body.h>

WeaponStruct* FakeInfantryClass::_GetDeployWeapon()
{
	int deployFireWeapon = this->Type->DeployFireWeapon;
	int weaponIndex = deployFireWeapon == -1 ? this->SelectWeapon(this->Target) : deployFireWeapon;
	return this->GetWeapon(weaponIndex);
}

int FakeInfantryClass::_SelectWeaponAgainst(AbstractClass* pTarget)
{
	auto pThisType = this->Type;
	int wp = -1;
	if (!pThisType->DeployFire || pThisType->DeployFireWeapon == -1)
	{
		return this->TechnoClass::SelectWeapon(pTarget);
	}

	if ((pThisType->IsGattling || TechnoTypeExtContainer::Instance.Find(pThisType)->MultiWeapon.Get()) && !this->IsDeployed())
	{
		return pThisType->DeployFireWeapon;
	}

	if (!this->InOpenToppedTransport || pThisType->OpenTransportWeapon == -1)
	{
		wp = 0;
	}
	else
	{
		wp = pThisType->OpenTransportWeapon;
	}

	return wp;
}

// =============================
// load / save

template <typename T>
void InfantryExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->IsUsingDeathSequence)
		.Process(this->CurrentDoType)
		.Process(this->SkipTargetChangeResetSequence)
		;
}

// =============================
// container

InfantryExtContainer InfantryExtContainer::Instance;
std::vector<InfantryExtData*> Container<InfantryExtData>::Array;

// =============================
// container hooks

//has NoInit constructor
ASMJIT_PATCH(0x517AEB, InfantryClass_CTOR, 0x5)
{
	GET(InfantryClass*, pItem, ESI);

	if(pItem->Type)
		InfantryExtContainer::Instance.Allocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x517D90, InfantryClass_DTOR, 0x5)
{
	GET(InfantryClass* const, pItem, ECX);
	InfantryExtContainer::Instance.Remove(pItem);
	return 0;
}

// ASMJIT_PATCH(0x51AA23, InfantryClass_Detach, 0x6)
// {
// 	GET(InfantryClass* const, pThis, ESI);
// 	GET(void*, target, EDI);
// 	GET_STACK(bool, all, STACK_OFFS(0x8, -0x8));
//
// 	InfantryExt::ExtMap.InvalidatePointerFor(pThis, target, all);
//
// 	return pThis->Type == target ? 0x51AA2B : 0x51AA35;
// }