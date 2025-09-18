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

void Container<InfantryExtData>::Clear()
{
	Array.clear();
}

bool InfantryExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool InfantryExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);
}

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

ASMJIT_PATCH(0x517F83, InfantryClass_DTOR, 0x6)
{
	GET(InfantryClass* const, pItem, ESI);
	InfantryExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeInfantryClass::_Detach(AbstractClass* target, bool all)
{
	InfantryExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->InfantryClass::PointerExpired(target, all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB080, FakeInfantryClass::_Detach)

HRESULT __stdcall FakeInfantryClass::_Load(IStream* pStm)
{
	HRESULT hr = this->InfantryClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = InfantryExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeInfantryClass::_Save(IStream* pStm, BOOL clearDirty)
{
	HRESULT hr = this->InfantryClass::Save(pStm, clearDirty);
	if (SUCCEEDED(hr))
		hr = InfantryExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB06C, FakeInfantryClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB070, FakeInfantryClass::_Save)