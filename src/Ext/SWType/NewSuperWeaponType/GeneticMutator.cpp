#include "GeneticMutator.h"

#include <Utilities/Helpers.h>

bool SW_GeneticMutator::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::GeneticMutator);
}

bool SW_GeneticMutator::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged)
	{
		SuperWeaponTypeClass* pSW = pThis->Type;
		SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pSW);

		CellClass* Cell = MapClass::Instance->GetCellAt(Coords);
		CoordStruct coords = Cell->GetCoordsWithBridge();

		auto pFirer = this->GetFirer(pThis, Coords, false);

		if (pData->Mutate_Explosion.Get(RulesClass::Instance->MutateExplosion))
		{
			auto const pExtWh = GetWarhead(pData);
			// single shot using cellspread warhead
			MapClass::DamageArea(coords, GetDamage(pData), pFirer, pExtWh, pExtWh->Tiberium, pThis->Owner);
		}
		else
		{
			// ranged approach
			auto Mutate = [=](InfantryClass* pInf) -> bool
			{
				if(!pInf->IsAlive || pInf->IsCrashing || pInf->IsSinking || pInf->InLimbo)
					return true;

				// is this thing affected at all?
				if (!pData->IsHouseAffected(pThis->Owner, pInf->Owner))
				{
					return true;
				}

				if (!pData->IsTechnoAffected(pInf))
				{
					// even if it makes little sense, we do this.
					// infantry handling is hardcoded and thus
					// this checks water and land cells.
					return true;
				}

				InfantryTypeClass* pType = pInf->Type;

				// quick ways out
				if (pType->Cyborg && pData->Mutate_IgnoreCyborg) {
					return true;
				}

				if (pType->NotHuman && pData->Mutate_IgnoreNotHuman) {
					return true;
				}

				// destroy or mutate
				int damage = pType->Strength;
				bool kill = (pType->Natural && pData->Mutate_KillNatural);
				auto pWH = kill ? RulesClass::Instance->C4Warhead : GetWarhead(pData);

				pInf->ReceiveDamage(&damage, 0, pWH, pFirer, true, false, pThis->Owner);

				return true;
			};

			// find everything in range and mutate it
			auto range = GetRange(pData);
			Helpers::Alex::DistinctCollector<InfantryClass*> items;
			Helpers::Alex::for_each_in_rect_or_range<InfantryClass>(Coords, range.WidthOrRange, range.Height, items);
			items.apply_function_for_each(Mutate);
		}
	}

	return true;
}

void SW_GeneticMutator::Initialize(SWTypeExtData* pData)
{
	pData->AttachedToObject->Action = Action::GeneticConverter;
	// Defaults to Genetic Mutator values
	pData->SW_AnimHeight = 5;

	// defaults depend on MutateExplosion property
	pData->Mutate_KillNatural = true;
	pData->Mutate_IgnoreCyborg = false;
	pData->Mutate_IgnoreNotHuman = false;

	pData->EVA_Detected = VoxClass::FindIndexById(GameStrings::EVA_GeneticMutatorDetected);
	pData->EVA_Ready = VoxClass::FindIndexById(GameStrings::EVA_GeneticMutatorReady);
	pData->EVA_Activated = VoxClass::FindIndexById(GameStrings::EVA_GeneticMutatorActivated);

	pData->SW_AITargetingMode = SuperWeaponAITargetingMode::GeneticMutator;
	pData->CursorType = int(MouseCursorType::GeneticMutator);
}

void SW_GeneticMutator::LoadFromINI(SWTypeExtData* pData, CCINIClass* pINI)
{
	const char* section = pData->get_ID();


	INI_EX exINI(pINI);
	pData->Mutate_Explosion.Read(exINI, section, "Mutate.Explosion");
	pData->Mutate_IgnoreCyborg.Read(exINI, section, "Mutate.IgnoreCyborg");
	pData->Mutate_IgnoreNotHuman.Read(exINI, section, "Mutate.IgnoreNotHuman");
	pData->Mutate_KillNatural.Read(exINI, section, "Mutate.KillNatural");

	// whatever happens, always target everything
	pData->SW_AffectsTarget = pData->SW_AffectsTarget | SuperWeaponTarget::AllTechnos;
}

WarheadTypeClass* SW_GeneticMutator::GetWarhead(const SWTypeExtData* pData) const
{
	// is set to non-null?
	if (pData->SW_Warhead.Get(nullptr))
	{
		return pData->SW_Warhead;
	}
	else if (pData->Mutate_Explosion.Get(RulesClass::Instance->MutateExplosion))
	{
		return RulesClass::Instance->MutateExplosionWarhead;
	}
	else
	{
		return RulesClass::Instance->MutateWarhead;
	}
}

AnimTypeClass* SW_GeneticMutator::GetAnim(const SWTypeExtData* pData) const
{
	return pData->SW_Anim.Get(RulesClass::Instance->IonBlast);
}

int SW_GeneticMutator::GetSound(const SWTypeExtData* pData) const
{
	return pData->SW_Sound.Get(RulesClass::Instance->GeneticMutatorActivateSound);
}

int SW_GeneticMutator::GetDamage(const SWTypeExtData* pData) const
{
	return pData->SW_Damage.Get(10000);
}

SWRange SW_GeneticMutator::GetRange(const SWTypeExtData* pData) const
{
	if (!pData->SW_Range->empty()) {
		return pData->SW_Range;
	}
	else if (RulesClass::Instance->MutateExplosion) {
		return { 5, -1 };
	}

	return { 3, 3 };
}