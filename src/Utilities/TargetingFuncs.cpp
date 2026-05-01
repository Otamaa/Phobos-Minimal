#include "TargetingFuncs.h"

#include <Ext/Building/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/SWTypeHandler.h>
#include <Ext/House/Body.h>
#include <Ext/Infantry/Body.h>

#include <Ext/TechnoType/Body.h>

#include <New/Entity/TargetingData.h>

bool TargetingFuncs::IsTargetAllowed(TechnoClass* pTechno)
{
	return !pTechno->InLimbo && pTechno->IsAlive;
}

bool TargetingFuncs::IgnoreThis(TechnoClass* pTechno)
{
	if (const auto pBld = cast_to<BuildingClass*>(pTechno))
	{
		if (BuildingExtContainer::Instance.Find(pBld)->LimboID >= 0)
			return true;
	}

	return false;
}

int TargetingFuncs::GetIonCannonValue(TechnoClass* pTechno, TechnoTypeExtData* pTypeExt, const TargetingData* pTargeting)
{
	if (pTypeExt->AIIonCannonValue.isset() && pTargeting->Owner)
	{
		const auto diffIndex = pTargeting->Owner->GetAIDifficultyIndex();

		// PartialVector3D: X=Hard(2), Y=Normal(1), Z=Easy(0)
		if (diffIndex < pTypeExt->AIIonCannonValue->ValueCount)
		{
			if (diffIndex == 0) // Easy
				return pTypeExt->AIIonCannonValue->Z;
			else if (diffIndex == 1) // Normal
				return pTypeExt->AIIonCannonValue->Y;
			else // diffIndex == 2 (Hard)
				return pTypeExt->AIIonCannonValue->X;
		}
	}

	return pTechno->GetIonCannonValue(pTargeting->Owner->AIDifficulty);
}

int NOINLINE ProcessIonCannonTargetAnyMax(TechnoClass* pTechno, int curMax, SWTypeHandler* pNewType, const TargetingData* pTargeting, HouseClass* pEnemy, CloakHandling cloak)
{
	// original game code only compares owner and doesn't support nullptr
	auto const passedFilter = (!pEnemy || pTechno->Owner == pEnemy);

	if (passedFilter && !TargetingFuncs::IgnoreThis(pTechno) && ((FakeHouseClass*)pTargeting->Owner)->_IsIonCannonEligibleTarget(pTechno))
	{
		auto const cell = CellClass::Coord2Cell(pTechno->GetCoords());

		if (!MapClass::Instance->IsWithinUsableArea(cell, true)) { return -1; }

		const auto pTypeExt = GET_TECHNOTYPEEXT(pTechno);

		int value = 0;
		bool RandomiedCloaked = false; // avoid the early AIIonCannnonValue

		// cloak options
		if (cloak != CloakHandling::AgnosticToCloak)
		{
			const bool cloaked = pTechno->IsCloaked();

			// original behavior
			if (cloak == CloakHandling::RandomizeCloaked && cloaked)
			{
				RandomiedCloaked = true;
				value = ScenarioClass::Instance->Random.RandomFromMax(curMax + 10);
			} // this prevents the 'targeting cloaked units bug'
			else if (cloak == CloakHandling::IgnoreCloaked && cloaked)
			{
				return -1;
			}
			else if (cloak == CloakHandling::RequireCloaked && !cloaked)
			{
				return -1;
			}
		}

		if (!RandomiedCloaked)
		{
			value = TargetingFuncs::GetIonCannonValue(pTechno, pTypeExt, pTargeting);
		}

		// do not do heavy lifting on objects that
		// would not be chosen anyhow
		if (value >= curMax)
		{
			if (pNewType->CanTargetingFireAt(pTargeting, cell, false))
			{
				return value;
			}
		}
	}

	return -1;
}

TargetResult TargetingFuncs::GetIonCannonTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting, HouseClass* pEnemy, CloakHandling cloak)
{
	std::vector<TechnoClass*> s_targets;
	const auto it = pTargeting->TypeExt->GetPotentialAITargets(pEnemy, s_targets);
	const auto pResult = GetTargetAnyMax(it,
		[=](TechnoClass* pTechno, int curMax) {
			return ProcessIonCannonTargetAnyMax(pTechno, curMax, pNewType, pTargeting, pEnemy, cloak);
		});

	return pResult ?
		TargetResult { CellClass::Coord2Cell(pResult->GetCoords()) , SWTargetFlags::AllowEmpty } :
		TargetResult { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::PickByHouseType(HouseClass* pThis, QuarryType type)
{
	const auto nTarget = pThis->PickTargetByType(type);
	const SWTargetFlags flag = nTarget.IsValid() ? SWTargetFlags::AllowEmpty : SWTargetFlags::DisallowEmpty;
	return { nTarget , flag };
}

bool TargetingFuncs::IsEligibleDominatorTarget(const TargetingData* pTargeting, FootClass* pTarget)
{
	auto const pTechnoType = pTarget->GetTechnoType();

	// Always ignore Insignificant or civilian-house owned targets - change from Ares and vanilla behaviour.
	if (pTechnoType->Insignificant || pTarget->Owner->Type->MultiplayPassive)
		return false;

	{
		auto const pTypeExt = pTargeting->TypeExt;
		auto const pOwner = pTargeting->Owner;
		auto const pTargetHouse = pTarget->Owner;

		// If SW.AIRequiresHouse is explicitly set use that instead of restricting to enemies only.
		if (pTypeExt->SW_AIRequiresHouse.isset() && !EnumFunctions::CanTargetHouse(pTypeExt->SW_AIRequiresHouse, pOwner, pTargetHouse))
		{
			return false;
		}
		else if (pOwner->IsAlliedWith(pTargetHouse))
		{
			return false;
		}

		// If SW.AIRequiresTarget is explicitly set check that here as well.
		if (pTypeExt->SW_AIRequiresTarget.isset() && !pTypeExt->IsTechnoEligible(pTarget, pTypeExt->SW_AIRequiresTarget))
			return false;

		if (pTypeExt->SW_AITargeting_PsyDom_AllowTypes.size() > 0 && !pTypeExt->SW_AITargeting_PsyDom_AllowTypes.Contains(pTechnoType))
			return false;

		if (pTypeExt->SW_AITargeting_PsyDom_DisallowTypes.size() > 0 && pTypeExt->SW_AITargeting_PsyDom_DisallowTypes.Contains(pTechnoType))
			return false;

		// Skip normal MC immunity etc. checks and only check air & invulnerability separately with toggles to turn them off.
		if (pTypeExt->SW_AITargeting_PsyDom_SkipChecks)
		{
			if (pTarget->IsInAir() && !pTypeExt->SW_AITargeting_PsyDom_AllowAir)
				return false;

			if (pTarget->IsIronCurtained() && !pTypeExt->SW_AITargeting_PsyDom_AllowInvulnerable)
				return false;

			return true;
		}

		// original game does not consider cloak
		if (pTarget->CloakState == CloakState::Cloaked && !pTypeExt->SW_AITargeting_PsyDom_AllowCloak)
			return false;
	}

	return pTarget->CanBePermaMindControlled();
}

TargetResult TargetingFuncs::GetDominatorTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	std::vector<TechnoClass*> s_targets;
	const auto it = pTargeting->TypeExt->GetPotentialAITargets(nullptr, s_targets);
	const auto pTarget = GetTargetFirstMax(it, [pTargeting, pNewType](TechnoClass* pTechno, int curMax)
{

	if (!TargetingFuncs::IsTargetAllowed(pTechno) || TargetingFuncs::IgnoreThis(pTechno))
	{
		return -1;
	}

	const auto cell = pTechno->GetCell()->MapCoords;
	int value = 0;

	for (size_t i = 0; i < CellSpread::NumCells(3); ++i)
	{
		auto pCell = MapClass::Instance->GetCellAt(cell + CellSpread::GetCell(i));
		for (NextObject j(pCell->FirstObject); flag_cast_to<FootClass*>(*j); ++j)
		{
			const auto pFoot = static_cast<FootClass*>(*j);

			if (pFoot->IsAlive && IsEligibleDominatorTarget(pTargeting, pFoot))
			{

				++value;
			}
		}
	}

	// new check
	return (value <= curMax || !pNewType->CanTargetingFireAt(pTargeting, cell, false)) ? -1 : value;
	});

	return pTarget ?
		TargetResult { CellClass::Coord2Cell(pTarget->GetCoords())  , SWTargetFlags::AllowEmpty } :
		TargetResult { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::GetParadropTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	static const int SpaceSize = 5;
	auto target = CellStruct::Empty;

	if (pTargeting->Owner->PreferredTargetType == QuarryType::Anything)
	{
		// if no enemy yet, reinforce own base
		const auto pTargetPlayer = HouseClass::Array->get_or_default(pTargeting->Owner->EnemyHouseIndex, pTargeting->Owner);

		target = MapClass::Instance->NearByLocation(
			pTargetPlayer->GetBaseCenter(), SpeedType::Foot, ZoneType::None,
			MovementZone::Normal, false, SpaceSize, SpaceSize, false,
			false, false, true, CellStruct::Empty, false, false);

		if (target != CellStruct::Empty)
		{
			target += CellStruct { 2 , 2 };
		}
	}
	else
	{
		target = pTargeting->Owner->PickTargetByType(pTargeting->Owner->PreferredTargetType);
	}

	return  (!target.IsValid() || !pNewType->CanTargetingFireAt(pTargeting, target, false)) ?
		TargetResult { CellStruct::Empty, SWTargetFlags::DisallowEmpty } :
		TargetResult { target , SWTargetFlags::AllowEmpty };
}

TargetResult TargetingFuncs::GetMutatorTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	std::vector<TechnoClass*> s_targets;
	const auto it = pTargeting->TypeExt->GetPotentialAITargets(nullptr, s_targets);
	const auto pResult = GetTargetFirstMax(it, [pTargeting, pNewType](TechnoClass* pTechno, int curMax)
{

	if (!TargetingFuncs::IsTargetAllowed(pTechno) || TargetingFuncs::IgnoreThis(pTechno))
	{
		return -1;
	}

	if (pTargeting->TypeExt->This()->Type == SuperWeaponType::GeneticMutator
		&& pTechno->WhatAmI() == AbstractType::Infantry)
	{
		const auto pInfantryType = static_cast<InfantryClass*>(pTechno)->Type;

		if (pInfantryType->Cyborg && pTargeting->TypeExt->Mutate_IgnoreCyborg)
			return -1;

		if (pInfantryType->NotHuman && pTargeting->TypeExt->Mutate_IgnoreNotHuman)
			return -1;
	}

	auto cell = pTechno->GetCell()->MapCoords;
	int value = 0;

	// Count eligible infantry across all cells in spread range
	for (size_t i = 0; i < CellSpread::NumCells(1); ++i)
	{
		const auto pCell = MapClass::Instance->GetCellAt(cell + CellSpread::GetCell(i));

		for (NextObject j(pCell->GetInfantry(pTechno->OnBridge)); cast_to<InfantryClass*>(*j); ++j)
		{
			const auto pInf = static_cast<InfantryClass*>(*j);

			if (pInf->IsAlive && !pTargeting->Owner->IsAlliedWith(pInf) && !pInf->IsInAir())
			{
				if (pInf->CloakState != CloakState::Cloaked)
				{
					++value;
				}
			}
		}
	}

	// Check AFTER totaling all cells, not inside the loop
	if (value <= curMax || !pNewType->CanTargetingFireAt(pTargeting, cell, false))
		return -1;

	return value;
	});

	return pResult ?
		TargetResult { CellClass::Coord2Cell(pResult->GetCoords()), SWTargetFlags::AllowEmpty } :
		TargetResult { CellStruct::Empty, SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::GetForceShieldTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	if (pTargeting->Owner->PreferredDefensiveCell.IsValid()
		&& (RulesClass::Instance->AISuperDefenseFrames + pTargeting->Owner->PreferredDefensiveCellStartTime) > Unsorted::CurrentFrame.get()
		&& pNewType->CanTargetingFireAt(pTargeting, pTargeting->Owner->PreferredDefensiveCell, false))
	{
		return { pTargeting->Owner->PreferredDefensiveCell , SWTargetFlags::AllowEmpty };
	}

	return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::GetOffensiveTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	return GetIonCannonTarget(pNewType,
		pTargeting,
		HouseClass::Array->get_or_default(pTargeting->Owner->EnemyHouseIndex),
		CloakHandling::IgnoreCloaked);
}

TargetResult TargetingFuncs::GetNukeAndLighningTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	if (pTargeting->Owner->PreferredTargetType == QuarryType::Anything)
	{
		return TargetingFuncs::GetIonCannonTarget(pNewType, pTargeting,
			HouseClass::Array->get_or_default(pTargeting->Owner->EnemyHouseIndex),
			CloakHandling::IgnoreCloaked);
	}

	return TargetingFuncs::PickByHouseType(pTargeting->Owner, pTargeting->Owner->PreferredTargetType);
}

TargetResult TargetingFuncs::GetStealthTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	return TargetingFuncs::GetIonCannonTarget(pNewType, pTargeting, nullptr, CloakHandling::RequireCloaked);
}

TargetResult TargetingFuncs::GetDroppodTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	const auto nRandom = ScenarioClass::Instance->Random.RandomRangedSpecific(ZoneType::North, ZoneType::West);
	const auto nCell = pTargeting->Owner->RandomCellInZone(nRandom);
	const auto nNearby = MapClass::Instance->NearByLocation(nCell,
		SpeedType::Foot, ZoneType::None, MovementZone::Normal, false, 1, 1, false,
		false, false, true, CellStruct::Empty, false, false);

	return (nNearby.IsValid() && pNewType->CanTargetingFireAt(pTargeting, nNearby, false)) ?
		TargetResult { nNearby, SWTargetFlags::AllowEmpty } :
		TargetResult { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::GetLighningRandomTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	CellStruct nBuffer {};
	for (int i = 0; i < 5; ++i)
	{
		auto& nRand = ScenarioClass::Instance->Random;
		if (!MapClass::Instance->CoordinatesLegal(nBuffer))
		{
			do
			{
				nBuffer.X = (short)nRand.RandomFromMax(MapClass::MapCellDimension->Width);
				nBuffer.Y = (short)nRand.RandomFromMax(MapClass::MapCellDimension->Height);

			}
			while (!MapClass::Instance->CoordinatesLegal(nBuffer));
		}

		if (pNewType->CanTargetingFireAt(pTargeting, nBuffer, false))
		{
			return { nBuffer , SWTargetFlags::AllowEmpty };
		}
	}

	return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::GetOwnerBuildingAsTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting, bool checkLauchsite)
{
	// find the first building providing super
	auto index = pTargeting->TypeExt->This()->ArrayIndex;
	const auto& buildings = pTargeting->Owner->Buildings;
	// Ares < 0.9 didn't check power
	const auto it = buildings.find_if([index, pTargeting, checkLauchsite, pNewType](BuildingClass* pBld)
		{
		auto const pExt = BuildingExtContainer::Instance.Find(pBld);
		const bool IsEligibleBuilding = !checkLauchsite ?
			pNewType->IsLaunchSite(pTargeting->TypeExt, pBld) :
			pExt->HasSuperWeapon(index, true);

		if (IsEligibleBuilding && pBld->IsPowerOnline())
		{
			auto cell = CellClass::Coord2Cell(pBld->GetCoords());

			if (pNewType->CanTargetingFireAt(pTargeting, cell, false))
			{
				return true;
			}
		}

		return false;
		});

	return (it != buildings.end()) ?
		TargetResult { CellClass::Coord2Cell((*it)->GetCoords()), SWTargetFlags::AllowEmpty } :
		TargetResult { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::GetBaseTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	// fire at the SW's owner's base cell
	CellStruct cell = pTargeting->Owner->GetBaseCenter();

	return cell.IsValid() && pNewType->CanTargetingFireAt(pTargeting, cell, false) ?
		TargetResult { cell, SWTargetFlags::AllowEmpty } :
		TargetResult { CellStruct::Empty, SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::GetMultiMissileTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	std::vector<TechnoClass*> s_targets;
	const auto it = pTargeting->TypeExt->GetPotentialAITargets(HouseClass::Array->get_or_default(pTargeting->Owner->EnemyHouseIndex), s_targets);
	const auto pResult = GetTargetFirstMax(it, [pTargeting, pNewType](TechnoClass* pTechno, int curMax)
	{
	if (!TargetingFuncs::IsTargetAllowed(pTechno) || TargetingFuncs::IgnoreThis(pTechno))
	{
		return -1;
	}

	auto cell = CellClass::Coord2Cell(pTechno->GetCoords());

	auto const value = pTechno->IsCloaked()
		? ScenarioClass::Instance->Random.RandomFromMax(100)
		: MapClass::Instance->GetThreatPosed(cell, pTargeting->Owner);

	if (value <= curMax || !pNewType->CanTargetingFireAt(pTargeting, cell, false)) { return -1; }

	return value;
	});

	return pResult ?
		TargetResult { CellClass::Coord2Cell(pResult->GetCoords()), SWTargetFlags::AllowEmpty } :
		TargetResult { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::GetEnemyBaseTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	if (auto pEnemy = HouseClass::Array->get_or_default(pTargeting->Owner->EnemyHouseIndex))
	{
		CellStruct cell = pEnemy->GetBaseCenter();

		if (pNewType->CanTargetingFireAt(pTargeting, cell, false))
		{
			return { cell , SWTargetFlags::AllowEmpty };
		}
	}

	return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
}

TargetResult TargetingFuncs::GetAuxTechnoTarget(SWTypeHandler* pNewType, const TargetingData* pTargeting)
{
	if (!pTargeting->TypeExt->Aux_Techno.empty())
	{
		//const auto TargetOwner = info.TypeExt->GetAIRequiredHouse();
		//HouseClass* TargetHouse = nullptr;
		//if ((TargetOwner & AffectedHouse::Owner) != AffectedHouse::None)
		//	TargetHouse = info.Owner;
		//else if ((TargetOwner & AffectedHouse::Enemies) != AffectedHouse::None)
		//	TargetHouse = HouseClass::Array->get_or_default(info.Owner->EnemyHouseIndex);
		//else if ((TargetOwner & AffectedHouse::Allies) != AffectedHouse::None){
		//	const auto It = std::find_if(HouseClass::Array->begin(), HouseClass::Array->end(),
		//		[&](HouseClass* pHouse) {
		//			 if (pHouse->Defeated || pHouse->IsObserver())
		//				 return false;
		//
		//			return info.Owner->Allies.Contains(pHouse);
		//		});
		//
		//	TargetHouse = It != HouseClass::Array->end() ? *It : nullptr;
		//}
		//
		//if (!TargetHouse || TargetHouse->Defeated || TargetHouse->IsObserver())
		//	return { CellStruct::Empty ,SWTargetFlags::DisallowEmpty };
		std::vector<TechnoClass*> s_targets;
		for (auto pTech : pTargeting->TypeExt->GetPotentialAITargets(nullptr, s_targets))
		{
			if (TechnoExtData::IsAlive(pTech, false, false, false) && !TargetingFuncs::IgnoreThis(pTech))
			{

				auto nLoc = pTech->GetCoords();
				auto nLocCell = CellClass::Coord2Cell(nLoc);

				if (nLoc == CoordStruct::Empty || nLocCell == CellStruct::Empty)
					continue;

				if (pTargeting->TypeExt->Aux_Techno.Contains(GET_TECHNOTYPE(pTech)))
				{
					if (pNewType->CanTargetingFireAt(pTargeting, nLocCell, false))
					{
						return { nLocCell , SWTargetFlags::AllowEmpty };
					}
				}
			}
		}
	}
	else
	{
		Debug::LogInfo("Uneable to fire SW [{} - {}] , AuxTechno is empty!", pTargeting->TypeExt->Name.data(), pTargeting->Owner->Type->ID);
	}

	return { CellStruct::Empty , SWTargetFlags::DisallowEmpty };
}
