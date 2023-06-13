#include "GeneticMutator.h"

bool SW_GeneticMutator::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::GeneticMutator);
}

bool SW_GeneticMutator::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	return true;
}

void SW_GeneticMutator::Initialize(SWTypeExt::ExtData* pData)
{ }

void SW_GeneticMutator::LoadFromINI(SWTypeExt::ExtData* pData, CCINIClass* pINI)
{ }

WarheadTypeClass* SW_GeneticMutator::GetWarhead(const SWTypeExt::ExtData* pData) const
{
	return RulesClass::Instance->MutateExplosion ? RulesClass::Instance->MutateExplosionWarhead : RulesClass::Instance->MutateWarhead;
}

AnimTypeClass* SW_GeneticMutator::GetAnim(const SWTypeExt::ExtData* pData) const
{
	return RulesClass::Instance->IonBlast;
}

int SW_GeneticMutator::GetSound(const SWTypeExt::ExtData* pData) const
{
	return RulesClass::Instance->GeneticMutatorActivateSound;
}

int SW_GeneticMutator::GetDamage(const SWTypeExt::ExtData* pData) const
{
	return pData->SW_Damage.Get(10000);
}

SWRange SW_GeneticMutator::GetRange(const SWTypeExt::ExtData* pData) const
{
	if (!pData->SW_Range->empty()) {
		return pData->SW_Range;
	}
	else if (RulesClass::Instance->MutateExplosion) {
		return { 5, -1 };
	}

	return { 3, 3 };
}