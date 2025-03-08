#include "GeneticMutator.h"

#include <Utilities/Helpers.h>

#include <Misc/DamageArea.h>

bool SW_GeneticMutator::HandleThisType(SuperWeaponType type) const
{
	return (type == SuperWeaponType::GeneticMutator);
}

bool SW_GeneticMutator::Activate(SuperClass* pThis, const CellStruct& Coords, bool IsPlayer)
{
	if (pThis->IsCharged)
	{
		SWTypeExtData* pData = SWTypeExtContainer::Instance.Find(pThis->Type);
		auto pFirer = this->GetFirer(pThis, Coords, false);
		const auto nDeferement = pData->SW_Deferment.Get(-1);

		if (nDeferement <= 0) {
			auto const damage = GetDamage(pData);
			auto range = GetRange(pData);
			CellClass* Cell = MapClass::Instance->GetCellAt(Coords);
			auto const pWarhead = GetWarhead(pData);
			auto cell_str = Cell->GetCoordsWithBridge();
			GeneticMutatorStateMachine::ApplyGeneticMutator(pFirer, pThis, pData, this, cell_str, Coords, pWarhead, range, damage);
		} else {
			this->newStateMachine(nDeferement, Coords, pThis, pFirer);
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
	if (!pData->SW_Range->empty())
	{
		return pData->SW_Range;
	}

	return RulesClass::Instance->MutateExplosion ? SWRange { 3, 3 } : SWRange { 5, -1 };
}

void GeneticMutatorStateMachine::Update()
{
	if (this->Finished())
	{
		auto pData = this->GetTypeExtData();

		pData->PrintMessage(pData->Message_Activate, this->Super->Owner);

		auto const sound = pData->SW_ActivationSound.Get(-1);
		if (sound != -1)
		{
			VocClass::PlayGlobal(sound, Panning::Center, 1.0);
		}

		auto range = this->Type->GetRange(pData);
		ApplyGeneticMutator(this->Firer, this->Super, pData, this->Type, this->CoordsWithBridge, this->Coords, this->Type->GetWarhead(pData), range, this->Type->GetDamage(pData));
	}
}

void GeneticMutatorStateMachine::ApplyGeneticMutator(TechnoClass* pFirer, SuperClass* pSuper, SWTypeExtData* pData, NewSWType* pNewType, CoordStruct& coord, const CellStruct& loc, WarheadTypeClass* pWarhead, SWRange& range, int damage)
{
	if (pData->Mutate_Explosion.Get(RulesClass::Instance->MutateExplosion))
	{
		// single shot using cellspread warhead
		DamageArea::Apply(&coord, damage, pFirer, pWarhead, pWarhead->Tiberium, pSuper->Owner);
	}
	else
	{
		// find everything in range and mutate it
		Helpers::Alex::DistinctCollector<InfantryClass*> items;
		Helpers::Alex::for_each_in_rect_or_range<InfantryClass>(loc, range.WidthOrRange, range.Height, items);
		// ranged approach
		items.apply_function_for_each([=](InfantryClass* pInf) -> bool
 {

			 if (!pInf->IsAlive || pInf->IsCrashing || pInf->IsSinking || pInf->InLimbo)
				 return true;

			 // is this thing affected at all?
			 if (!pData->IsHouseAffected(pSuper->Owner, pInf->Owner))
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
			 if (pType->Cyborg && pData->Mutate_IgnoreCyborg)
			 {
				 return true;
			 }

			 if (pType->NotHuman && pData->Mutate_IgnoreNotHuman)
			 {
				 return true;
			 }

			 // destroy or mutate
			 int damage = pType->Strength;
			 bool kill = (pType->Natural && pData->Mutate_KillNatural);
			 auto pWH = kill ? RulesClass::Instance->C4Warhead : pWarhead;

			 pInf->ReceiveDamage(&damage, 0, pWH, pFirer, true, false, pSuper->Owner);

			 return true;
		});
	}
}
