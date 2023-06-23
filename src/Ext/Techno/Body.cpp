#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>
#include <InfantryClass.h>
#include <Unsorted.h>

#include <BitFont.h>

#include <New/Entity/FlyingStrings.h>

#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>

#include <Locomotor/Cast.h>

#include <Misc/AresData.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Cast.h>
#include <Utilities/Macro.h>
#include <Utilities/LocomotionCast.h>
#include <Phobos_ECS.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Techno/GiftBox/GiftBoxFunctional.h>


#include <memory>

bool TechnoExt::IsCullingImmune(TechnoClass* pThis)
{
	return HasAbility(pThis, PhobosAbilityType::CullingImmune);
}

bool TechnoExt::IsEMPImmune(TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToEMP)
		return true;

	return HasAbility(pThis, PhobosAbilityType::EmpImmune);
}

bool TechnoExt::IsPsionicsImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (pType->ImmuneToPsionics)
		return true;

	return HasAbility(pThis, PhobosAbilityType::PsionicsImmune);
}

bool TechnoExt::IsCritImmune(TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToCrit)
		return true;

	return HasAbility(pThis, PhobosAbilityType::CritImmune);
}

bool TechnoExt::IsChronoDelayDamageImmune(FootClass* pThis)
{
	if (!pThis)
		return false;

	auto const pLoco = pThis->Locomotor.GetInterfacePtr();

	if (!pLoco)
		return false;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (VTable::Get(pLoco) != TeleportLocomotionClass::ILoco_vtable)
		return false;

	if (!pThis->IsWarpingIn())
		return false;

	if (pTypeExt->ChronoDelay_Immune.Get())
		return true;

	return HasAbility(pThis, PhobosAbilityType::ChronoDelayDamageImmune);
}

bool TechnoExt::IsRadImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToRadiation)
		return true;

	return HasAbility(pThis, PhobosAbilityType::RadImmune);
}

bool TechnoExt::IsPsionicsWeaponImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPsionicWeapons)
		return true;

	return HasAbility(pThis, PhobosAbilityType::PsionicsWeaponImmune);
}

bool TechnoExt::IsPoisonImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPoison)
		return true;

	return HasAbility(pThis, PhobosAbilityType::PoisonImmune);
}

bool TechnoExt::IsBerserkImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->ImmuneToBerserk.Get())
		return true;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pShield = pExt->GetShield();

	if (pShield && pShield->IsActive() && pExt->CurrentShieldType->ImmuneToPsychedelic)
		return true;

	return HasAbility(pThis , PhobosAbilityType::BerzerkImmune);
}

bool TechnoExt::IsAbductorImmune(TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->ImmuneToAbduction)
		return true;

	return HasAbility(pThis, PhobosAbilityType::AbductorImmune);
}

bool TechnoExt::IsAssaulter(InfantryClass* pThis)
{
	if (pThis->Type->Assaulter)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Assaulter);
}

bool TechnoExt::IsParasiteImmune(TechnoClass* pThis)
{
	if (pThis->GetTechnoType()->Parasiteable)
		return false;

	return HasAbility(pThis, PhobosAbilityType::ParasiteImmune);
}

bool TechnoExt::IsUnwarpable(TechnoClass* pThis)
{
	if (!pThis->GetTechnoType()->Warpable)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Unwarpable);
}

bool TechnoExt::ExtData::IsInterceptor()
{
	auto const pThis = this->Get();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(Type);

	if (pTypeExt->Interceptor)
		return true;

	return HasAbility(pThis, PhobosAbilityType::Interceptor);
}

void TechnoExt::ExtData::CreateInitialPayload()
{
	if (this->PayloadCreated) {
		return;
	}

	this->PayloadCreated = true;

	auto const pThis = this->Get();
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	auto const pBld = specific_cast<BuildingClass*>(pThis);

	auto freeSlots = (pBld && pBld->Type->CanBeOccupied)
		? pBld->Type->MaxNumberOccupants - pBld->GetOccupantCount()
		: pType->Passengers - pThis->Passengers.NumPassengers;

	auto const sizePayloadNum = pTypeExt->InitialPayload_Nums.size();
	auto const sizePayloadRank = pTypeExt->InitialPayload_Vet.size();
	auto const sizePyloadAddTeam = pTypeExt->InitialPayload_AddToTransportTeam.size();

	for (auto i = 0u; i < pTypeExt->InitialPayload_Types.size(); ++i)
	{
		auto const pPayloadType = pTypeExt->InitialPayload_Types[i];

		if (!pPayloadType)
		{
			continue;
		}

		// buildings and aircraft aren't valid payload, and building payload
		// can only be infantry
		auto const absPayload = pPayloadType->WhatAmI();
		if (absPayload == AbstractType::BuildingType
			|| absPayload == AbstractType::AircraftType
			|| (pBld && absPayload != AbstractType::InfantryType))
		{
			continue;
		}

		// if there are no nums, index gets huge and invalid, which means 1
		auto const idxPayloadNum = std::min(i + 1, sizePayloadNum) - 1;
		auto const payloadNum = (idxPayloadNum < sizePayloadNum)
			? pTypeExt->InitialPayload_Nums[idxPayloadNum] : 1;

		auto const rank = idxPayloadNum < sizePayloadRank ?
			pTypeExt->InitialPayload_Vet[idxPayloadNum] : Rank::Invalid;

		auto const addtoteam = idxPayloadNum < sizePyloadAddTeam ?
			pTypeExt->InitialPayload_AddToTransportTeam[idxPayloadNum] : false;

		// never fill in more than allowed
		auto const count = std::min(payloadNum, freeSlots);
		freeSlots -= count;

		for (auto j = 0; j < count; ++j)
		{
			auto const pObject = (TechnoClass*)pPayloadType->CreateObject(pThis->Owner);

			if (!pObject)
				continue;

			if (rank == Rank::Veteran)
				pObject->Veterancy.SetVeteran();
			else if (rank == Rank::Elite)
				pObject->Veterancy.SetElite();

			if (pBld)
			{
				// buildings only allow infantry payload, so this in infantry
				auto const pPayload = static_cast<InfantryClass*>(pObject);

				if (pBld->Type->CanBeOccupied)
				{
					pBld->Occupants.AddItem(pPayload);
					auto const pCell = pThis->GetCell();
					//TODO:
					AresGarrisonedIn(pPayload) = pBld;
					pThis->UpdateThreatInCell(pCell);
				}
				else
				{
					pPayload->Limbo();

					if (pBld->Type->InfantryAbsorb)
					{
						pPayload->Absorbed = true;

						if (pPayload->CountedAsOwnedSpecial)
						{
							--pPayload->Owner->OwnedInfantry;
							pPayload->CountedAsOwnedSpecial = false;
						}

						if (pBld->Type->ExtraPowerBonus > 0)
						{
							pBld->Owner->RecheckPower = true;
						}
					}
					else
					{
						pPayload->SendCommand(RadioCommand::RequestLink, pBld);
					}

					pBld->Passengers.AddPassenger(pPayload);
					pPayload->AbortMotion();
				}

			}
			else
			{
				auto const pPayload = static_cast<FootClass*>(pObject);
				pPayload->SetLocation(pThis->Location);
				pPayload->Limbo();

				if (pType->OpenTopped)
				{
					pThis->EnteredOpenTopped(pPayload);
				}

				pPayload->Transporter = pThis;

				auto const old = std::exchange(VocClass::VoicesEnabled(), false);
				pThis->AddPassenger(pPayload);
				VocClass::VoicesEnabled = old;

				if (addtoteam)
				{
					if (auto pTeam = ((FootClass*)pThis)->Team)
						pTeam->AddMember(pPayload, true);
				}
			}
		}
	}
}

bool TechnoExt::HasAbility(TechnoClass* pThis, PhobosAbilityType nType)
{
	const bool IsVet = pThis->Veterancy.IsVeteran();
	const bool IsElite = pThis->Veterancy.IsElite();

	if (!IsVet && !IsElite){
		return false;
	}

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (IsVet)
	{
		return pTypeExt->Phobos_VeteranAbilities.at((int)nType);
	}
	else if (IsElite)
	{
		return  pTypeExt->Phobos_VeteranAbilities.at((int)nType) || pTypeExt->Phobos_EliteAbilities.at((int)nType);
	}

	return false;
}

bool TechnoExt::HasImmunity(TechnoClass* pThis, int nType)
{
	const bool IsVet = pThis->Veterancy.IsVeteran();
	const bool IsElite = pThis->Veterancy.IsElite();

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (IsVet)
	{
		return pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains(nType);
	}
	else if (IsElite)
	{
		return  pTypeExt->R_ImmuneToType.Contains(nType) ||
			pTypeExt->V_ImmuneToType.Contains((int)nType) ||
			pTypeExt->E_ImmuneToType.Contains((int)nType);
	}

	return pTypeExt->R_ImmuneToType.Contains(nType);
}

bool TechnoExt::IsCullingImmune(Rank vet, TechnoClass* pThis)
{
	return HasAbility(vet, pThis, PhobosAbilityType::CullingImmune);
}

bool TechnoExt::IsEMPImmune(Rank vet, TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToEMP)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::EmpImmune);
}

bool TechnoExt::IsPsionicsImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();

	if (pType->ImmuneToPsionics)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::PsionicsImmune);
}

bool TechnoExt::IsCritImmune(Rank vet, TechnoClass* pThis)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->ImmuneToCrit)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::CritImmune);
}

bool TechnoExt::IsChronoDelayDamageImmune(Rank vet, FootClass* pThis)
{
	if (!pThis)
		return false;

	auto const pLoco = pThis->Locomotor.GetInterfacePtr();

	if (!pLoco)
		return false;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (VTable::Get(pLoco) != TeleportLocomotionClass::ILoco_vtable)
		return false;

	if (!pThis->IsWarpingIn())
		return false;

	if (pTypeExt->ChronoDelay_Immune.Get())
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::ChronoDelayDamageImmune);
}

bool TechnoExt::IsRadImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToRadiation)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::RadImmune);
}

bool TechnoExt::IsPsionicsWeaponImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPsionicWeapons)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::PsionicsWeaponImmune);
}

bool TechnoExt::IsPoisonImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	if (pType->ImmuneToPoison)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::PoisonImmune);
}

bool TechnoExt::IsBerserkImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->ImmuneToBerserk.Get())
		return true;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pShield = pExt->GetShield();

	if (pShield && pShield->IsActive() && pExt->CurrentShieldType->ImmuneToPsychedelic)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::BerzerkImmune);
}

bool TechnoExt::IsAbductorImmune(Rank vet, TechnoClass* pThis)
{
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->ImmuneToAbduction)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::AbductorImmune);
}

bool TechnoExt::IsAssaulter(Rank vet, InfantryClass* pThis)
{
	if (pThis->Type->Assaulter)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::Assaulter);
}

bool TechnoExt::IsParasiteImmune(Rank vet, TechnoClass* pThis)
{
	if (pThis->GetTechnoType()->Parasiteable)
		return false;

	return HasAbility(vet , pThis, PhobosAbilityType::ParasiteImmune);
}

bool TechnoExt::IsUnwarpable(Rank vet, TechnoClass* pThis)
{
	if (!pThis->GetTechnoType()->Warpable)
		return true;

	return HasAbility(vet, pThis, PhobosAbilityType::Unwarpable);
}


bool TechnoExt::HasAbility(Rank vet, TechnoClass* pThis, PhobosAbilityType nType)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (vet == Rank::Veteran)
	{
		return pTypeExt->Phobos_VeteranAbilities.at((int)nType);
	}
	else if (vet == Rank::Elite)
	{
		return  pTypeExt->Phobos_VeteranAbilities.at((int)nType) || pTypeExt->Phobos_EliteAbilities.at((int)nType);
	}

	return false;
}

bool TechnoExt::HasImmunity(Rank vet, TechnoClass* pThis, int nType)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (vet == Rank::Veteran)
	{
		return pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains(nType);
	}
	else if (vet == Rank::Elite)
	{
		return  pTypeExt->R_ImmuneToType.Contains(nType) || pTypeExt->V_ImmuneToType.Contains((int)nType) || pTypeExt->E_ImmuneToType.Contains((int)nType);
	}

	return pTypeExt->R_ImmuneToType.Contains(nType);
}


#include <Ext/TerrainType/Body.h>

bool TechnoExt::IsCrushable(ObjectClass* pVictim, TechnoClass* pAttacker)
{
	if (!pVictim || !pAttacker || pVictim->IsBeingWarpedOut())
		return false;

	if (pVictim->IsIronCurtained())
		return false;

	if (pAttacker->Owner && pAttacker->Owner->IsAlliedWith(pVictim))
		return false;

	auto const pAttackerType = pAttacker->GetTechnoType();
	auto const pVictimTechno = abstract_cast<TechnoClass*>(pVictim);
	auto const pAttackerTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pAttackerType);

	if (!pVictimTechno)
	{
		if(auto const pTerrain = specific_cast<TerrainClass*>(pVictim))
		{
			if(pTerrain->Type->Immune || pTerrain->Type->SpawnsTiberium || !pTerrain->Type->Crushable)
				return false;
		
			const auto pTerrainExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);
			if(pTerrainExt->IsPassable)
				return false;
		
			return pAttackerTechnoTypeExt->CrushLevel.Get(pAttacker) > pTerrainExt->CrushableLevel;
		}

		return false;
	}

	auto const pWhatVictim = GetVtableAddr(pVictim);
	auto const pVictimType = pVictim->GetTechnoType();

	if (pAttackerType->OmniCrusher)
	{
		if (pWhatVictim == BuildingClass::vtable || pVictimType->OmniCrushResistant)
			return false;
	}
	else
	{
		if (pVictimTechno->Uncrushable || !pVictimType->Crushable)
			return false;
	}

	auto const pVictimTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pVictimType);

	if (pWhatVictim == InfantryClass::vtable)
	{
		const auto& crushableLevel = static_cast<InfantryClass*>(pVictim)->IsDeployed() ?
			pVictimTechnoTypeExt->DeployCrushableLevel :
			pVictimTechnoTypeExt->CrushableLevel;

		if (pAttackerTechnoTypeExt->CrushLevel.Get(pAttacker) < crushableLevel.Get(pVictimTechno))
			return false;
	}

	if (TechnoExt::IsChronoDelayDamageImmune(abstract_cast<FootClass*>(pVictim)))
	{
		return false;
	}

	//auto const pExt = TechnoExt::ExtMap.Find(pVictimTechno);
	//if (auto const pShieldData = pExt->Shield.get()) {
	//	auto const pWeaponIDx = pAttacker->SelectWeapon(pVictim);
	//	auto const pWeapon = pAttacker->GetWeapon(pWeaponIDx);

	//	if (pWeapon && pWeapon->WeaponType &&
	//		pShieldData->IsActive() && !pShieldData->CanBeTargeted(pWeapon->WeaponType)) {
	//		return false;
	//	}
	//}

	return true;
}

AreaFireReturnFlag TechnoExt::ApplyAreaFire(TechnoClass* pThis, CellClass*& pTargetCell, WeaponTypeClass* pWeapon)
{
	const auto pExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	switch (pExt->AreaFire_Target.Get())
	{
	case AreaFireTarget::Random:
	{
		std::vector<CellStruct> adjacentCells;
		GeneralUtils::AdjacentCellsInRange(adjacentCells, static_cast<size_t>(pWeapon->Range.ToDouble() + 0.99));
		size_t const size = adjacentCells.size();

		for (int i = 0; i < (int)size; i++)
		{
			const int rand = ScenarioClass::Instance->Random.RandomFromMax(size - 1);
			CellStruct const tgtPos = pTargetCell->MapCoords + adjacentCells.at((i + rand) % size);
			CellClass* const tgtCell = MapClass::Instance->GetCellAt(tgtPos);

			if (EnumFunctions::AreCellAndObjectsEligible(tgtCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), pThis->Owner, true))
			{
				pTargetCell = tgtCell;
				return AreaFireReturnFlag::Continue;
			}
		}

		return AreaFireReturnFlag::DoNotFire;
	}
	case AreaFireTarget::Self:
	{
		if (!EnumFunctions::AreCellAndObjectsEligible(pThis->GetCell(), pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), nullptr, false))
			return AreaFireReturnFlag::DoNotFire;

		return AreaFireReturnFlag::SkipSetTarget;
	}
	default:
	{
		if (!EnumFunctions::AreCellAndObjectsEligible(pTargetCell, pExt->CanTarget.Get(), pExt->CanTargetHouses.Get(), nullptr, false))
			return AreaFireReturnFlag::DoNotFire;
	}
	}

	return AreaFireReturnFlag::ContinueAndReturn;
}

int TechnoExt::GetThreadPosed(TechnoClass* pThis)
{
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (const auto pShieldData = pExt->GetShield()) {
		if (pShieldData->IsActive()) {
			auto const pShiedType = pShieldData->GetType();
			if (pShiedType->ThreadPosed.isset())
				return pExt->Type->ThreatPosed + pShiedType->ThreadPosed.Get();
		}
	}

	return pExt->Type->ThreatPosed;
}

bool TechnoExt::IsReallyTechno(TechnoClass* pThis)
{
	const auto pAddr = (((DWORD*)pThis)[0]);
	if (pAddr != UnitClass::vtable
		&& pAddr != AircraftClass::vtable
		&& pAddr != InfantryClass::vtable
		&& pAddr != BuildingClass::vtable)
	{
		return false;
	}

	return true;
}

int TechnoExt::GetDeployFireWeapon(UnitClass* pThis)
{
	if (pThis->Type->DeployFireWeapon == -1)
		return pThis->TechnoClass::SelectWeapon(pThis->Target ? pThis->Target : pThis->GetCell());

	return pThis->Type->DeployFireWeapon;
}

void TechnoExt::SetMissionAfterBerzerk(TechnoClass* pThis, bool Immediete)
{
	auto const pType = pThis->GetTechnoType();

	const Mission nEndMission = !pType->ResourceGatherer ?
		pThis->IsArmed() ?
		pThis->Owner && pThis->Owner->IsHumanPlayer ? Mission::Guard : Mission::Area_Guard
		: Mission::Sleep
		: Mission::Harvest;

	pThis->QueueMission(nEndMission, Immediete);
}

std::pair<TechnoClass*, CellClass*> TechnoExt::GetTargets(ObjectClass* pObjTarget, AbstractClass* pTarget)
{
	TechnoClass* pTargetTechno = nullptr;
	CellClass* targetCell = nullptr;

	//pTarget nullptr check already done above this hook
	if (Is_Cell(pTarget))
	{
		return { nullptr , targetCell = static_cast<CellClass*>(pTarget) };
	}
	else if (pObjTarget)
	{
		// it is an techno target
		if (((pObjTarget->AbstractFlags & AbstractFlags::Techno) != AbstractFlags::None))
		{
			pTargetTechno = static_cast<TechnoClass*>(pObjTarget);
			if (!pTargetTechno->IsInAir())	// Ignore target cell for airborne technos.
				targetCell = pTargetTechno->GetCell();
		}
		else // non techno target , but still an object
		{
			targetCell = pObjTarget->GetCell();
		}
	}

	return { pTargetTechno , targetCell };
}

bool TechnoExt::ObjectHealthAllowFiring(ObjectClass* pTargetObj, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pTargetObj && pWeaponExt->Targeting_Health_Percent.isset())
	{
		auto const pHP = pTargetObj->GetHealthPercentage();

		if (!pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP <= pWeaponExt->Targeting_Health_Percent.Get())
			return false;
		else if (pWeaponExt->Targeting_Health_Percent_Below.Get() && pHP >= pWeaponExt->Targeting_Health_Percent.Get())
			return false;
	}

	return true;
}

bool TechnoExt::CheckCellAllowFiring(CellClass* pCell, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pCell && !EnumFunctions::IsCellEligible(pCell, pWeaponExt->CanTarget, true))
	{
		return false;
	}

	return true;
}

bool TechnoExt::TechnoTargetAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (!EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget) ||
		!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTarget->Owner))
	{
		return false;
	}

	return true;
}

bool TechnoExt::FireOnceAllowFiring(TechnoClass* pThis, WeaponTypeClass* pWeapon, AbstractClass* pTarget)
{
	const auto pTechnoExt = TechnoExt::ExtMap.Find(pThis);

	if (auto pUnit = specific_cast<UnitClass*>(pThis))
	{
		if (!pUnit->Type->IsSimpleDeployer && !pUnit->Deployed && pTarget)
		{
			if (pUnit->Type->DeployFire && pWeapon->FireOnce)
			{
				if (pTechnoExt->DeployFireTimer.GetTimeLeft() > 0)
					return false;
			}
		}
	}

	return true;
}

bool TechnoExt::CheckFundsAllowFiring(TechnoClass* pThis, WarheadTypeClass* pWH)
{
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	const int nMoney = pWHExt->TransactMoney;
	if (nMoney != 0 && !pThis->Owner->CanTransactMoney(nMoney))
		return false;

	return true;
}

bool TechnoExt::InterceptorAllowFiring(TechnoClass* pThis, ObjectClass* pTarget)
{
	//this code is used to remove Techno as auto target consideration , so interceptor can find target faster
	const auto pTechnoExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTechnoExt->IsInterceptor() && pTechnoTypeExt->Interceptor_OnlyTargetBullet.Get())
	{
		if (!pTarget || !Is_Bullet(pTarget))
		{
			return false;
		}
	}

	return true;
}

bool TechnoExt::TargetTechnoShieldAllowFiring(TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	const auto pTargetTechnoExt = TechnoExt::ExtMap.Find(pTarget);
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	if (const auto pShieldData = pTargetTechnoExt->Shield.get()) {
		if (pShieldData->IsActive()) {
			if (!pShieldData->CanBePenetrated(pWeapon->Warhead))
			{
				if (pWHExt->GetVerses(pShieldData->GetType()->Armor).Verses < 0.0 && pShieldData->GetType()->CanBeHealed)
				{
					const bool IsFullHP = pShieldData->GetHealthRatio() >= RulesClass::Instance->ConditionGreen;
					if (!IsFullHP)
						return true;
					else
					{
						if(pShieldData->GetType()->PassthruNegativeDamage) {
							return !(pShieldData->GetHealthRatio() >= RulesClass::Instance->ConditionGreen);
						}				
					}
				}

				return false;
			}
		}
	}

	return true;
}

bool TechnoExt::IsAbductable(TechnoClass* pThis, WeaponTypeClass* pWeapon, FootClass* pFoot)
{

	if (pFoot->InLimbo ||
	pFoot->IsIronCurtained() ||
	pFoot->IsSinking ||
	!pFoot->IsAlive ||
	Is_DriverKilled(pFoot)) {
		return false;
	}

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

		//Don't abduct the target if it has more life then the abducting percent
	if (pWeaponExt->Abductor_AbductBelowPercent < pFoot->GetHealthPercentage()) {
		return false;
	}

	if (pWeaponExt->Abductor_MaxHealth > 0 && pWeaponExt->Abductor_MaxHealth < pFoot->Health) {
		return false;
	}

	if (TechnoExt::IsAbductorImmune(pFoot))
		return false;

	if (!TechnoExt::IsEligibleSize(pThis, pFoot))
		return false;

	return true;
}

void TechnoExt::SendPlane(size_t Aircraft, size_t Amount, HouseClass* pOwner, Rank SendRank, Mission SendMission, AbstractClass* pTarget, AbstractClass* pDest)
{
	if (!pOwner || Amount <= 0)
		return;

	const auto pAirCraft = AircraftTypeClass::Array->GetItemOrDefault(Aircraft);

	if (!pAirCraft)
		return;

	//safeguard
	Mission result = Mission::None;
	switch (SendMission)
	{
	case Mission::Move: {
		if (!pDest)
			pDest = pTarget;

		result = SendMission;
	}
	break;
	case Mission::ParadropApproach:
	case Mission::Attack:
	case Mission::SpyplaneApproach:
		result = SendMission;
		break;
	default:
		result = Mission::SpyplaneApproach;
		break;
	}

	for (size_t i = 0; i < Amount; ++i)
	{
		++Unsorted::ScenarioInit;
		auto const pPlane = static_cast<AircraftClass*>(pAirCraft->CreateObject(pOwner));
		--Unsorted::ScenarioInit;

		if (!pPlane)
			continue;

		pPlane->Spawned = true;
		Edge ed = pOwner->StartingEdge;

		if (ed < Edge::North || ed > Edge::West)
			ed = pOwner->GetCurrentEdge();

		const auto nCell = MapClass::Instance->PickCellOnEdge(ed, CellStruct::Empty, CellStruct::Empty, SpeedType::Winged, true, MovementZone::Normal);
		pPlane->QueueMission(result, false);

		if (SendRank != Rank::Rookie && SendRank != Rank::Invalid && pPlane->CurrentRanking < SendRank)
			pPlane->Veterancy.SetRank(SendRank);

		if (pDest)
			pPlane->SetDestination(pDest, true);

		if (pTarget)
			pPlane->SetTarget(pTarget);

		bool UnLimboSucceeded = false;
		++Unsorted::ScenarioInit;
		UnLimboSucceeded = pPlane->Unlimbo(CellClass::Cell2Coord(nCell), DirType::North);
		--Unsorted::ScenarioInit;

		if (!UnLimboSucceeded) {
			pPlane->UnInit();
		} else {

			// we cant create InitialPayload when mutex atives 
			// so here we handle the InitialPayload Creation !
			// this way we can make opentopped airstrike happen !
			TechnoExt::ExtMap.Find(pPlane)->CreateInitialPayload();
			pPlane->NextMission();
		}
	}
}

/*
 * Object should NOT be placed on the map (->Limbo() it or don't Put in the first place)
 * otherwise Bad Things (TM) will happen. Again.
 */
bool TechnoExt::CreateWithDroppod(FootClass* Object, const CoordStruct& XYZ) {
	auto MyCell = MapClass::Instance->GetCellAt(XYZ);
	if (Object->IsCellOccupied(MyCell, -1, -1, nullptr, false) != Move::OK) {
		return false;
	}
	else {
		LocomotionClass::ChangeLocomotorTo(Object, CLSIDs::Droppod);
		CoordStruct xyz = XYZ;
		xyz.Z = 0;

		Object->SetLocation(xyz);
		Object->SetDestination(MyCell, 1);
		Object->Locomotor->Move_To(XYZ);

		//Object->PrimaryFacing.set(DirStruct()); // TODO : random or let loco set the facing

		if (!Object->InLimbo) {
			Object->See(0, 0);
			Object->QueueMission(Mission::Guard, 0);
			Object->NextMission();
			return true;
		}

		return false;
	}
}

bool TechnoExt::TargetFootAllowFiring(TechnoClass* pThis, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	if ((pTarget->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
	{
		const auto pFoot = static_cast<FootClass*>(pTarget);
		const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

		if(pWeaponExt->Abductor.Get() && !TechnoExt::IsAbductable(pThis,pWeapon, pFoot))
			return false;

		if (auto const pUnit = specific_cast<UnitClass*>(pTarget)) {
			if (pUnit->DeathFrameCounter > 0)
				return false;
		}

		if (TechnoExt::IsChronoDelayDamageImmune(pFoot))
			return false;
	}

	return true;
}

ObjectTypeClass* TechnoExt::SetInfDefaultDisguise(TechnoClass* const pThis, TechnoTypeClass* const pType)
{
	auto const pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (auto pDisguise = pExt->DefaultDisguise.Get(nullptr))
	{
		pThis->Disguised = true;
		pThis->DisguisedAsHouse = pThis->Owner;
		return pDisguise;
	}

	return nullptr;
}

void TechnoExt::UpdateMCOverloadDamage(TechnoClass* pOwner)
{
	auto pThis = pOwner->CaptureManager;

	if(!pThis)
		return;

	if (!pThis->InfiniteMindControl)
		return;

	const auto pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

	if (pThis->OverloadPipState > 0)
		--pThis->OverloadPipState;

	if (pThis->OverloadDamageDelay <= 0)
	{

		const auto OverloadCount = pOwnerTypeExt->Overload_Count.GetElements(RulesClass::Instance->OverloadCount);

		if (OverloadCount.empty())
			return;

		int nCurIdx = 0;
		const int nNodeCount = pThis->ControlNodes.Count;

		for (int i = 0; i < (int)(OverloadCount.size()); ++i)
		{
			if (nNodeCount > OverloadCount[i])
			{
				nCurIdx = i + 1;
			}
		}

		const auto nOverloadfr = pOwnerTypeExt->Overload_Frames.GetElements(RulesClass::Instance->OverloadFrames);
		pThis->OverloadDamageDelay = nOverloadfr.GetItemAtOrMax(nCurIdx);

		const auto nOverloadDmg = pOwnerTypeExt->Overload_Damage.GetElements(RulesClass::Instance->OverloadDamage);
		auto nDamage = nOverloadDmg.GetItemAtOrMax(nCurIdx);

		if (nDamage <= 0)
		{
			pThis->OverloadDeathSoundPlayed = false;
		}
		else
		{
			pThis->OverloadPipState = 10;
			auto const pWarhead = pOwnerTypeExt->Overload_Warhead.Get(RulesClass::Instance->C4Warhead);
			pOwner->ReceiveDamage(&nDamage, 0, pWarhead, 0, 0, 0, 0);

			if (!pThis->OverloadDeathSoundPlayed)
			{
				VocClass::PlayIndexAtPos(pOwnerTypeExt->Overload_DeathSound.Get(RulesClass::Instance->MasterMindOverloadDeathSound), pOwner->Location, 0);
				pThis->OverloadDeathSoundPlayed = true;
			}

			if (auto const pParticle = pOwnerTypeExt->Overload_ParticleSys.Get(RulesClass::Instance->DefaultSparkSystem))
			{
				for (int i = pOwnerTypeExt->Overload_ParticleSysCount.Get(5); i > 0; --i)
				{
					auto const nRandomY = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					auto const nRamdomX = ScenarioClass::Instance->Random.RandomRanged(-200, 200);
					auto nLoc = pOwner->Location;

					if (pParticle->BehavesLike == BehavesLike::Smoke)
						nLoc.Z += 100;

					CoordStruct nParticleCoord { pOwner->Location.X + nRamdomX, nRandomY + pOwner->Location.Y, nLoc };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, pOwner->GetCell(), pOwner, CoordStruct::Empty, pOwner->Owner);
				}
			}

			if (nCurIdx > 0 && pOwner->IsAlive)
			{
				double const nBase = (nCurIdx != 1) ? 0.015 : 0.029999999;
				double const nCopied_base = (ScenarioClass::Instance->Random.RandomFromMax(100) < 50) ? -nBase : nBase;
				pOwner->RockingSidewaysPerFrame = static_cast<float>(nCopied_base);
			}
		}

	}
	else
	{
		--pThis->OverloadDamageDelay;
	}
}

bool TechnoExt::AllowedTargetByZone(TechnoClass* pThis, TechnoClass* pTarget, const TargetZoneScanType& zoneScanType, WeaponTypeClass* pWeapon, std::optional<std::reference_wrapper<const ZoneType>> zone)
{
	if (!pThis || !pTarget)
		return false;

	if (Is_Aircraft(pThis))
		return true;

	const auto pThisType = pThis->GetTechnoType();
	const MovementZone mZone = pThisType->MovementZone;
	const ZoneType currentZone = zone ? zone.value() : 
		MapClass::Instance->GetMovementZoneType(pThis->InlineMapCoords(), mZone, pThis->IsOnBridge());

	if (currentZone != ZoneType::None)
	{
		if (zoneScanType == TargetZoneScanType::Any)
			return true;

		const ZoneType targetZone = 
			MapClass::Instance->GetMovementZoneType(pTarget->InlineMapCoords(), mZone, pTarget->IsOnBridge());

		if (zoneScanType == TargetZoneScanType::Same)
		{
			if (currentZone != targetZone)
				return false;
		}
		else
		{
			if (!pWeapon)
			{
				const int weaponIndex = pThis->SelectWeapon(pTarget);

				if (weaponIndex < 0)
					return false;

				if (const auto pWpStruct = pThis->GetWeapon(weaponIndex))
					pWeapon = pWpStruct->WeaponType;
				else
					return false;
			}

			auto const speedType = pThisType->SpeedType;
			const auto cellStruct = MapClass::Instance->NearByLocation(pTarget->InlineMapCoords(),
				speedType, -1, mZone, false, 1, 1, true,
				false, false, speedType != SpeedType::Float, CellStruct::Empty, false, false);

			auto const pCell = MapClass::Instance->GetCellAt(cellStruct);

			if (!pCell)
				return false;

			const double distance = pCell->GetCoordsWithBridge().DistanceFrom(pTarget->GetCenterCoords());

			if (distance > pWeapon->Range)
				return false;
		}
	}

	return true;
}

//ToDo : Auto regenerate and transferable passengers (Problem : Driver killed and operator stuffs )
void TechnoExt::PutPassengersInCoords(TechnoClass* pTransporter, const CoordStruct& nCoord, AnimTypeClass* pAnimToPlay, int nSound, bool bForce)
{
	if (!pTransporter || !pTransporter->Passengers.NumPassengers || !MapClass::Instance->IsWithinUsableArea(nCoord))
		return;

	//TODO : check if passenger is actually allowed to go outside 
	auto pPassenger = pTransporter->Passengers.RemoveFirstPassenger();
	CoordStruct nDest = nCoord;

	//if (bForce)
	{
		//TechnoTypeClass* pPassengerType = pPassenger->GetTechnoType();
		//auto const pPassengerMZone = pPassengerType->MovementZone;
		//auto const pPassengerSpeedType = pPassengerType->SpeedType;


		//auto const pCellFrom = Map.GetCellAt(nCoord);
		//auto const nZone = Map.Zone_56D230(&pCellFrom->MapCoords, pPassengerMZone, pCellFrom->ContainsBridgeEx());

		//if (!Map[nCoord]->IsClearToMove(pPassengerSpeedType, false, false, nZone, pPassengerMZone, -1, 1))
		{
			nDest = MapClass::Instance->GetRandomCoordsNear(nCoord, ScenarioClass::Instance->Random.RandomFromMax(2000), ScenarioClass::Instance->Random.RandomFromMax(1));
		}
	}

	MapClass::Instance->GetCellAt(nCoord)->ScatterContent(pTransporter->GetCoords(), true, true, false);

	bool Placed = false;
	if (bForce)
	{
		++Unsorted::ScenarioInit;
		Placed = pPassenger->Unlimbo(nDest, DirType::North);
		--Unsorted::ScenarioInit;
	}
	else
	{
		Placed = pPassenger->Unlimbo(nDest, DirType::North);
		//Placed = CreateWithDroppod(pPassenger, nDest , LocomotionClass::CLSIDs::Teleport);
	}

	//Only remove passengers from the Transporter if it succeeded
	if (Placed)
	{
		pPassenger->OnBridge = MapClass::Instance->GetCellAt(nCoord)->ContainsBridgeEx();
		pPassenger->StopMoving();
		pPassenger->SetDestination(nullptr, true);
		pPassenger->SetTarget(nullptr);
		pPassenger->CurrentTargets.Clear();
		pPassenger->SetFocus(nullptr);
		pPassenger->unknown_C4 = 0; // don't ask
		pPassenger->unknown_5A0 = 0;
		pPassenger->CurrentGattlingStage = 0;
		pPassenger->SetCurrentWeaponStage(0);
		pPassenger->SetLocation(nDest);
		pPassenger->LiberateMember();

		if (pPassenger->SpawnManager)
		{
			pPassenger->SpawnManager->ResetTarget();
		}

		pPassenger->ClearPlanningTokens(nullptr);

		pPassenger->DiscoveredBy(pTransporter->GetOwningHouse());

		if (auto pFoot = generic_cast<FootClass*>(pTransporter))
		{
			if (pTransporter->GetTechnoType()->Gunner)
			{
				pFoot->RemoveGunner(pPassenger);
			}

			if (pTransporter->GetTechnoType()->OpenTopped)
			{
				pFoot->ExitedOpenTopped(pPassenger);
			}
		}
		else if (auto pBuilding = specific_cast<BuildingClass*>(pTransporter))
		{
			if (pBuilding->Absorber())
			{
				pPassenger->Absorbed = false;
				if (pBuilding->Type->ExtraPowerBonus > 0)
				{
					pBuilding->Owner->RecheckPower = true;
				}
			}
		}

		VocClass::PlayIndexAtPos(nSound, nDest);

		if (pAnimToPlay)
		{
			if (auto pAnim = GameCreate<AnimClass>(pAnimToPlay, nDest))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pTransporter->GetOwningHouse(), nullptr, pTransporter, false);
			}
		}

		if (pPassenger->CurrentMission != Mission::Guard)
			pPassenger->Override_Mission(Mission::Area_Guard);
	}
	else
	{
		pTransporter->AddPassenger(pPassenger);
	}
}

void TechnoExt::SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo)
{
	if (pFrom->IsIronCurtained() && pFrom->ProtectType == ProtectTypes::IronCurtain)
	{
		const auto bSyncIC = TechnoTypeExt::ExtMap.Find(pFrom->GetTechnoType())->IronCurtain_SyncDeploysInto
			.Get(RulesExt::Global()->IronCurtain_SyncDeploysInto);

		if (bSyncIC)
		{
			pTo->IronCurtain(pFrom->IronCurtainTimer.GetTimeLeft(), pFrom->Owner, false);
			pTo->IronTintStage = pFrom->IronTintStage;
		}
	}
}

void TechnoExt::PlayAnim(AnimTypeClass* const pAnim, TechnoClass* pInvoker)
{
	if (pAnim && pInvoker) {
		if (auto pCreated = GameCreate<AnimClass>(pAnim, pInvoker->Location)) {
			AnimExt::SetAnimOwnerHouseKind(pCreated, pInvoker->GetOwningHouse(), nullptr, pInvoker, false);
		}
	}
}

double TechnoExt::GetDamageMult(TechnoClass* pSouce, bool ForceDisable)
{
	if (!pSouce || !pSouce->IsAlive || ForceDisable)
		return 1.0;

	const auto pType = pSouce->GetTechnoType();

	if (!pType)
		return 1.0;

	const bool firepower = pSouce->HasAbility(AbilityType::Firepower);

	const auto nFirstMult = !firepower ? 1.0 : RulesClass::Instance->VeteranCombat;
	const auto nSecondMult = (!pSouce->Owner || !pSouce->Owner->Type) ? 1.0 : pSouce->Owner->Type->FirepowerMult;
	return nFirstMult * pSouce->FirepowerMultiplier * nSecondMult; ;
}

const std::vector<std::vector<CoordStruct>>* TechnoExt::PickFLHs(TechnoClass* pThis)
{
	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const auto pInf = specific_cast<InfantryClass*>(pThis);
	//const auto pUnit = specific_cast<UnitClass*>(pThis);

	if (pThis->Veterancy.IsElite())
	{
		if (pInf)
		{
			if (pInf->IsDeployed() && !pExt->EliteDeployedWeaponBurstFLHs.empty())
				return &pExt->EliteDeployedWeaponBurstFLHs;
			else if (pInf->Crawling && !pExt->EliteCrouchedWeaponBurstFLHs.empty())
				return &pExt->EliteCrouchedWeaponBurstFLHs;
		}

		return &pExt->EliteWeaponBurstFLHs;
	}
	else
	{
		if (pInf)
		{
			if (pInf->IsDeployed() && !pExt->DeployedWeaponBurstFLHs.empty())
				return &pExt->DeployedWeaponBurstFLHs;
			else if (pInf->Crawling && !pExt->CrouchedWeaponBurstFLHs.empty())
				return &pExt->CrouchedWeaponBurstFLHs;
		}


		return &pExt->WeaponBurstFLHs;
	}
}

std::pair<bool, CoordStruct> TechnoExt::GetBurstFLH(TechnoClass* pThis, int weaponIndex)
{
	bool FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	if (!pThis || weaponIndex < 0)
		return { FLHFound , FLH };

	const auto pickedFLHs = PickFLHs(pThis);

	if (!pickedFLHs->empty())
	{
		if ((int)(*pickedFLHs)[weaponIndex].size() > pThis->CurrentBurstIndex)
		{
			FLHFound = true;
			FLH = (*pickedFLHs)[weaponIndex][pThis->CurrentBurstIndex];
		}
	}

	return { FLHFound , FLH };
}

const Nullable<CoordStruct>* TechnoExt::GetInfrantyCrawlFLH(InfantryClass* pThis, int weaponIndex)
{
	const auto pTechnoType = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pThis->IsDeployed())
	{
		if (weaponIndex == 0)
		{
			return pThis->Veterancy.IsElite() ?
				&pTechnoType->E_DeployedPrimaryFireFLH :
				&pTechnoType->DeployedPrimaryFireFLH;
		}
		else if (weaponIndex == 1)
		{
			return pThis->Veterancy.IsElite() ?
				&pTechnoType->E_DeployedSecondaryFireFLH :
				&pTechnoType->DeployedSecondaryFireFLH;
		}
	}
	else
	{
		if (pThis->Crawling)
		{
			if (weaponIndex == 0)
			{
				return pThis->Veterancy.IsElite() ?

					pTechnoType->E_PronePrimaryFireFLH.isset() ?
					&pTechnoType->E_PronePrimaryFireFLH :
					&pTechnoType->Elite_PrimaryCrawlFLH
					:

				pTechnoType->PronePrimaryFireFLH.isset() ?
					&pTechnoType->PronePrimaryFireFLH :
					&pTechnoType->PrimaryCrawlFLH
					;
			}
			else if (weaponIndex == 1)
			{
				return pThis->Veterancy.IsElite() ?
					pTechnoType->E_ProneSecondaryFireFLH.isset() ?
					&pTechnoType->E_ProneSecondaryFireFLH :
					&pTechnoType->E_ProneSecondaryFireFLH
					:

				pTechnoType->ProneSecondaryFireFLH.isset() ?
					&pTechnoType->ProneSecondaryFireFLH :
					&pTechnoType->SecondaryCrawlFLH
					;
			}
		}
	}

	return nullptr;
}

const Armor TechnoExt::GetTechnoArmor(TechnoClass* pThis , WarheadTypeClass* pWarhead)
{
	auto const pTargetTechnoExt = TechnoExt::ExtMap.Find(pThis);
	auto nArmor = pThis->GetTechnoType()->Armor;

	if (!pTargetTechnoExt)
		return nArmor;

	auto const pShieldData = pTargetTechnoExt->Shield.get();

	if (!pShieldData)
		return nArmor;

	if (pShieldData->IsActive())
	{
		if (pShieldData->CanBePenetrated(pWarhead))
			return nArmor;

		return pShieldData->GetType()->Armor.Get();
	}

	return nArmor;
}

std::pair<bool, CoordStruct> TechnoExt::GetInfantryFLH(InfantryClass* pThis, int weaponIndex)
{
	if (!pThis || weaponIndex < 0)
		return { false , CoordStruct::Empty };

	const auto pickedFLH = TechnoExt::GetInfrantyCrawlFLH(pThis, weaponIndex);

	if (pickedFLH && pickedFLH->isset() && pickedFLH->Get()) {
		return { true , pickedFLH->Get() };
	}

	return{ false , CoordStruct::Empty };
}

CoordStruct TechnoExt::PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger, int maxAttempts)
{
	if (!pThis || !pPassenger)
		return CoordStruct::Empty;

	//guarantee
	if (maxAttempts < 1)
		maxAttempts = 1;

	CellClass* pCell = pThis->GetCell();
	CellStruct placeCoords = CellStruct::Empty;
	auto pTypePassenger = pPassenger->GetTechnoType();
	SpeedType speedType = SpeedType::Track;
	MovementZone movementZone = MovementZone::Normal;

	if (!Is_AircraftType(pTypePassenger)) {
		speedType = pTypePassenger->SpeedType;
		movementZone = pTypePassenger->MovementZone;
	}

	for (Point2D ExtDistance = { 0,0 }; ExtDistance.X < maxAttempts; ++ExtDistance)
	{
		placeCoords = pCell->MapCoords - CellStruct { (short)(ExtDistance.X / 2), (short)(ExtDistance.Y / 2) };
		placeCoords = MapClass::Instance->NearByLocation(placeCoords, speedType, -1, movementZone, false, ExtDistance.X, ExtDistance.Y, true, false, false, false, CellStruct::Empty, false, false);
		
		pCell = MapClass::Instance->GetCellAt(placeCoords);

		if ((pThis->IsCellOccupied(pCell, -1, -1, nullptr, false) == Move::OK) || pCell->MapCoords == CellStruct::Empty)
		{
			pPassenger->OnBridge = pCell->ContainsBridge();
			return pCell->GetCoordsWithBridge();
		}
	}

	return CoordStruct::Empty;
}

void TechnoExt::DrawSelectBrd(const TechnoClass* pThis, TechnoTypeClass* pType, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry, bool sIsDisguised)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (!pTypeExt->UseCustomSelectBrd.Get(RulesExt::Global()->UseSelectBrd.Get(Phobos::Config::EnableSelectBrd)))
		return;

	SHPStruct* SelectBrdSHP = pTypeExt->SHP_SelectBrdSHP.isset() ? 
		pTypeExt->SHP_SelectBrdSHP.Get() : 
		(isInfantry ? RulesExt::Global()->SHP_SelectBrdSHP_INF : RulesExt::Global()->SHP_SelectBrdSHP_UNIT).Get();

	if (!SelectBrdSHP)
		return;

	ConvertClass* SelectBrdPAL = (pTypeExt->SHP_SelectBrdPAL ?
		pTypeExt->SHP_SelectBrdPAL :
		(isInfantry ? RulesExt::Global()->SHP_SelectBrdPAL_INF : RulesExt::Global()->SHP_SelectBrdPAL_UNIT))
		->GetConvert<PaletteManager::Mode::Temperate>();

	if (!SelectBrdPAL)
		return;

	Point2D vPos = { 0, 0 };
	Point2D vLoc = *pLocation;
	int frame, XOffset, YOffset;

	const Point3D selectbrdFrame = (pTypeExt->SelectBrd_Frame.isset() ? pTypeExt->SelectBrd_Frame
		: (isInfantry ? RulesExt::Global()->SelectBrd_Frame_Infantry : RulesExt::Global()->SelectBrd_Frame_Unit)).Get();

	const auto nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->SelectBrd_TranslucentLevel.Get(RulesExt::Global()->SelectBrd_DefaultTranslucentLevel.Get()));
	const auto canSee = sIsDisguised && pThis->DisguisedAsHouse ? pThis->DisguisedAsHouse->IsAlliedWith(HouseClass::CurrentPlayer) :
		pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer)
		|| HouseExt::IsObserverPlayer()
		|| pTypeExt->SelectBrd_ShowEnemy.Get(RulesExt::Global()->SelectBrd_DefaultShowEnemy.Get());

	const Point2D offs = (pTypeExt->SelectBrd_DrawOffset.isset() ? pTypeExt->SelectBrd_DrawOffset : (isInfantry ?
		RulesExt::Global()->SelectBrd_DrawOffset_Infantry : RulesExt::Global()->SelectBrd_DrawOffset_Unit)).Get();

	XOffset = offs.X;
	YOffset = pTypeExt->Get()->PixelSelectionBracketDelta + offs.Y;
	vLoc.Y -= 5;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 1 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}
	else
	{
		vPos.X = vLoc.X + 2 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}

	if (pThis->IsSelected && canSee)
	{
		if (pThis->IsGreenHP())
			frame = selectbrdFrame.X;
		else if (pThis->IsYellowHP())
			frame = selectbrdFrame.Y;
		else
			frame = selectbrdFrame.Z;

		DSurface::Temp->DrawSHP(SelectBrdPAL, SelectBrdSHP,
			frame, &vPos, pBound, nFlag, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

std::pair<TechnoTypeClass*, HouseClass*> TechnoExt::GetDisguiseType(TechnoClass* pTarget, bool CheckHouse, bool CheckVisibility, bool bVisibleResult)
{
	HouseClass* pHouseOut = pTarget->GetOwningHouse();
	TechnoTypeClass* pTypeOut = pTarget->GetTechnoType();
	const bool bIsVisible = !CheckVisibility ? bVisibleResult : (pTarget->IsClearlyVisibleTo(HouseClass::CurrentPlayer));

	if (pTarget->IsDisguised() && !bIsVisible)
	{
		if (CheckHouse) {
			if (const auto pDisguiseHouse = pTarget->GetDisguiseHouse(true)) {
				pHouseOut = pDisguiseHouse;
			}
		}

		if (pTarget->Disguise != pTypeOut) {
			if (const auto pDisguiseType = type_cast<TechnoTypeClass*, true>(pTarget->Disguise)) {
				return { pDisguiseType, pHouseOut };
			}
		}
	}

	return { pTypeOut, pHouseOut };
}

std::tuple<CoordStruct , SHPStruct* , int> GetInsigniaDatas(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	bool isCustomInsignia = false;
	SHPStruct* pShapeFile = FileSystem::PIPS_SHP;
	const auto nCurRank = pThis->CurrentRanking;
	int frameIndex = pTypeExt->InsigniaFrame.GetFromSpecificRank(nCurRank);
	Point3D insigniaFrames = pTypeExt->InsigniaFrames.Get();
	CoordStruct drawOffs = pTypeExt->InsigniaDrawOffset.Get();
	int DefaultFrameIdx = -1;

	if (SHPStruct* pShapeFileHere = pTypeExt->Insignia.GetFromSpecificRank(nCurRank))
	{
		pShapeFile = pShapeFileHere;
		isCustomInsignia = true;
		DefaultFrameIdx = 0;
	}

	if (pTypeExt->OwnerObject()->Gunner && !pTypeExt->Insignia_Weapon.empty())
	{
		const int weaponIndex = pThis->CurrentWeaponNumber;
		auto const& data = pTypeExt->Insignia_Weapon[weaponIndex];

		if (auto const pCustomShapeFile = data.Shapes.GetFromSpecificRank(nCurRank)) {
			pShapeFile = pCustomShapeFile;
			isCustomInsignia = true;
			DefaultFrameIdx = 0;
		}

		const int frame = data.Frame.GetFromSpecificRank(nCurRank);

		if (frame != -1)
			frameIndex = frame;

		const auto frames = data.Frames.GetEx();

		if (frames->X != -1 && frames->Y != -1 && frames->Z != -1)
			insigniaFrames = *frames;
	}

	int nRankPerFrames = insigniaFrames.X;
	switch (nCurRank)
	{
	case Rank::Elite:
		DefaultFrameIdx = !isCustomInsignia ? 15 : DefaultFrameIdx;
		nRankPerFrames = insigniaFrames.Z;
		break;
	case Rank::Veteran:
		DefaultFrameIdx = !isCustomInsignia ? 14 : DefaultFrameIdx;
		nRankPerFrames = insigniaFrames.Y;
		break;
	default:
		break;
	}

	const int FrameCalc = (frameIndex == -1 ? nRankPerFrames : frameIndex);
	const int frameIndexRet = FrameCalc == -1 ? DefaultFrameIdx : FrameCalc;

	return { drawOffs, pShapeFile  , frameIndexRet };
}

// Based on Ares source.
void TechnoExt::DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	if (pThis->CurrentRanking == Rank::Invalid)
		return;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const bool IsObserverPlayer = HouseExt::IsObserverPlayer();
	Point2D offset = *pLocation;
	TechnoTypeClass* pTechnoType = nullptr;
	auto const& [pTechnoTyper, pOwner] = TechnoExt::GetDisguiseType(pThis, true, true, false);
	const bool isDisguised = pTechnoTyper != pExt->Type;

	if (isDisguised && IsObserverPlayer) {
		pTechnoType = pExt->Type;
	} else {
		pTechnoType = pTechnoTyper;
	}

	if (!pTechnoType)
		return;

	TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	if (!pTypeExt->DrawInsignia)
		return;

	const bool isVisibleToPlayer = (pOwner && pOwner->IsAlliedWith(HouseClass::CurrentPlayer))
		|| IsObserverPlayer
		|| pTypeExt->Insignia_ShowEnemy.Get(RulesExt::Global()->EnemyInsignia);

	if (!isVisibleToPlayer)
		return;

	auto const&[drawOffs, pShapeFile, frameIndex] = 
				GetInsigniaDatas(pThis, pTypeExt);

	if (frameIndex != -1 &&  pShapeFile)
	{
		offset.X += (5 + drawOffs.X);
		offset.Y += (!Is_Infantry(pThis) ? 4 : 2 + drawOffs.Y);

		DSurface::Temp->DrawSHP(
			FileSystem::PALETTE_PAL, pShapeFile, frameIndex, &offset, pBounds, BlitterFlags(0xE00), 
			0, -2 + drawOffs.Z, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	return;
}

bool TechnoExt::CheckIfCanFireAt(TechnoClass* pThis, AbstractClass* pTarget)
{
	const int wpnIdx = pThis->SelectWeapon(pTarget);
	const FireError fErr = pThis->GetFireError(pTarget, wpnIdx, true);
	if (fErr != FireError::ILLEGAL
		&& fErr != FireError::CANT
		&& fErr != FireError::MOVING
		&& fErr != FireError::RANGE)

	{
		return pThis->IsCloseEnough(pTarget, wpnIdx);
	}
	else
		return false;
}

void TechnoExt::ForceJumpjetTurnToTarget(TechnoClass* pThis)
{
	const auto pFoot = abstract_cast<UnitClass*>(pThis);
	if (!pFoot)
		return;

	const auto pType = pThis->GetTechnoType();
	const auto pLoco = locomotion_cast<JumpjetLocomotionClass*, true>(pFoot->Locomotor);

	if (pLoco && pThis->IsInAir()
		&& !pType->TurretSpins)
	{
		if (TechnoTypeExt::ExtMap.Find(pType)->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget)
		   && pFoot->GetCurrentSpeed() == 0)
		{
			if (const auto pTarget = pThis->Target)
			{
				if (!pLoco->Facing.Is_Rotating() && TechnoExt::CheckIfCanFireAt(pThis, pTarget))
				{
					const CoordStruct source = pThis->Location;
					const CoordStruct target = pTarget->GetCoords();
					const DirStruct tgtDir = DirStruct(static_cast<double>(source.Y - target.Y), static_cast<double>(target.X - source.X));

					if (pThis->GetRealFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
						pLoco->Facing.Set_Desired(tgtDir);
				}
			}
		}
	}
}
#include <format>

// convert UTF-8 string to wstring
std::wstring Str2Wstr(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

void TechnoExt::DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage, WarheadTypeClass* pWH)
{
	if (!pThis || !pThis->IsAlive || pThis->InLimbo ||  !pThis->IsOnMyView())
		return;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	const ColorStruct color = isShieldDamage ? damage > 0 ? Phobos::Defines::ShieldPositiveDamageColor : Phobos::Defines::ShieldPositiveDamageColor :
		damage > 0 ? Drawing::DefaultColors[(int)DefaultColorList::Red] : Drawing::DefaultColors[(int)DefaultColorList::Green];

	std::wstring damageStr;

	if (Phobos::Otamaa::IsAdmin)
		damageStr = Str2Wstr(std::format("[{}] {} ({})", pThis->get_ID(), pWH->ID, damage));
	else
		damageStr = std::format(L"{}", damage);

	auto coords = pThis->GetCenterCoords();
	int maxOffset = 30;
	int width = 0, height = 0;
	BitFont::Instance->GetTextDimension(damageStr.c_str(), &width, &height, 120);

	if (pExt->DamageNumberOffset >= maxOffset || pExt->DamageNumberOffset.empty())
		pExt->DamageNumberOffset = -maxOffset;

	if (auto pBuilding = specific_cast<BuildingClass*>(pThis))
		coords.Z += 104 * pBuilding->Type->Height;
	else
		coords.Z += 256;

	if (auto const pCell = MapClass::Instance->TryGetCellAt(coords))
	{
		if (!pCell->IsFogged() && !pCell->IsShrouded())
		{
			if (pThis->VisualCharacter(0, HouseClass::CurrentPlayer()) != VisualType::Hidden)
			{
				FlyingStrings::Add(damageStr.c_str(), coords, color, Point2D { pExt->DamageNumberOffset - (width / 2), 0 });
			}
		}
	}

	pExt->DamageNumberOffset = pExt->DamageNumberOffset + width;
}

int TechnoExt::GetSizeLeft(FootClass* const pFoot)
{
	return pFoot->GetTechnoType()->Passengers - pFoot->Passengers.GetTotalSize();
}

void TechnoExt::Stop(TechnoClass* pThis, Mission const& eMission)
{
	pThis->ForceMission(eMission);
	pThis->CurrentTargets.Clear();
	pThis->SetFocus(nullptr);
	pThis->Stun();
}

bool TechnoExt::IsOnLimbo(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->InLimbo && !pThis->Transporter;
}

bool TechnoExt::IsDeactivated(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->Deactivated;
}

bool TechnoExt::IsUnderEMP(TechnoClass* pThis, bool bIgnore)
{
	return !bIgnore && pThis->IsUnderEMP();
}

bool TechnoExt::IsActive(TechnoClass* pThis, bool bCheckEMP, bool bCheckDeactivated, bool bIgnoreLimbo, bool bIgnoreIsOnMap, bool bIgnoreAbsorb)
{
	if (!TechnoExt::IsAlive(pThis, bIgnoreLimbo, bIgnoreIsOnMap, bIgnoreAbsorb))
		return false;

	if (pThis->BeingWarpedOut || pThis->TemporalTargetingMe || IsUnderEMP(pThis, !bCheckEMP) || IsDeactivated(pThis, !bCheckDeactivated))
		return false;

	return true;
}

bool TechnoExt::IsAlive(TechnoClass* pThis, bool bIgnoreLimbo, bool bIgnoreIsOnMap, bool bIgnoreAbsorb)
{
	if (!pThis)
		return false;

	if ((IsOnLimbo(pThis, !bIgnoreLimbo)) || (pThis->Absorbed && !bIgnoreAbsorb) || (!pThis->IsOnMap && !bIgnoreIsOnMap))
		return false;

	if (pThis->IsCrashing || pThis->IsSinking)
		return false;

	if (Is_Unit(pThis))
		return (static_cast<UnitClass*>(pThis)->DeathFrameCounter > 0) ? false : true;

	return pThis->IsAlive && pThis->Health > 0;
}

void TechnoExt::ObjectKilledBy(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (!pKiller || !pVictim)
		return;
	TechnoClass* pObjectKiller = (pKiller->GetTechnoType()->Spawned || pKiller->GetTechnoType()->MissileSpawn) && pKiller->SpawnOwner ?
		pKiller->SpawnOwner : pKiller;

	if (pObjectKiller && pObjectKiller->BelongsToATeam())
	{

		if (auto const pFootKiller = abstract_cast<const FootClass*>(pObjectKiller))
		{

			auto const pKillerTechnoData = TechnoExt::ExtMap.Find(pObjectKiller);
			auto const pFocus = abstract_cast<TechnoClass*>(pFootKiller->Team->Focus);
			pKillerTechnoData->LastKillWasTeamTarget = false;

			if (pFocus && pFocus->GetTechnoType() == pVictim->GetTechnoType())
			{
				pKillerTechnoData->LastKillWasTeamTarget = true;
			}

			// Conditional Jump Script Action stuff
			if (auto const pKillerTeamData = TeamExt::ExtMap.Find(pFootKiller->Team))
			{
				if (pKillerTeamData->ConditionalJump_EnabledKillsCount)
				{
					const bool isValidKill = pKillerTeamData->ConditionalJump_Index < 0 ?
						false : ScriptExt::EvaluateObjectWithMask(pVictim, pKillerTeamData->ConditionalJump_Index, -1, -1, pKiller);;

					if (isValidKill || pKillerTechnoData->LastKillWasTeamTarget)
						pKillerTeamData->ConditionalJump_Counter++;
				}

				// Special case for interrupting current action
				if (pKillerTeamData->AbortActionAfterKilling
					&& pKillerTechnoData->LastKillWasTeamTarget)
				{
					pKillerTeamData->AbortActionAfterKilling = false;
					const auto pTeam = pFootKiller->Team;
					auto const nAction = pTeam->CurrentScript->GetCurrentAction();
					auto const nActionNext = pTeam->CurrentScript->GetNextAction();

					//					Debug::Log("DEBUG: [%s] [%s] %d = %d,%d - Force next script action after successful kill: %d = %d,%d\n"
					//						, pTeam->Type->ID
					//						, pTeam->CurrentScript->Type->ID
					//						, pTeam->CurrentScript->CurrentMission
					//						, nAction.Action
					//						, nAction.Argument
					//						, pTeam->CurrentScript->CurrentMission + 1
					//						, nActionNext.Action
					//						, nActionNext.Argument);

										// Jumping to the next line of the script list
					pTeam->StepCompleted = true;

					return;
				}
			}
		}
	}
}

void TechnoExt::ExtData::UpdateMCRangeLimit()
{
	auto const pThis = this->Get();
	auto const pCManager = pThis->CaptureManager;

	if (!pCManager)
		return;

	const int Range = TechnoTypeExt::ExtMap.Find(Type)->MindControlRangeLimit.Get();

	if (Range <= 0)
		return;

	for (const auto& node : pCManager->ControlNodes)
	{
		if (pThis->DistanceFrom(node->Unit) > Range)
			pCManager->FreeUnit(node->Unit);
	}
}

void TechnoExt::ExtData::UpdateInterceptor()
{
	auto const pThis = this->Get();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(this->Type);

	if (!this->IsInterceptor() || pThis->Target)
		return;

	if ((Is_Aircraft(pThis) && !pThis->IsInAir()))
		return;

	if (auto const pTransport = pThis->Transporter)
	{
		if (Is_Aircraft(pTransport))
			if (!pTransport->IsInAir())
				return;

		if (TechnoExt::IsInWarfactory(pTransport))
			return;
	}

	if (TechnoExt::IsInWarfactory(pThis))
		return;

	BulletClass* pTargetBullet = nullptr;

	const auto& guardRange = pTypeExt->Interceptor_GuardRange.Get(pThis);
	const auto& minguardRange = pTypeExt->Interceptor_MinimumGuardRange.Get(pThis);

	for (auto pBullet : *BulletClass::Array)
	{
		const auto distance = pBullet->Location.DistanceFrom(pThis->Location);

		if (distance > guardRange || distance < minguardRange || pBullet->InLimbo)
			continue;

		const int weaponIndex = pThis->SelectWeapon(pBullet);
		const auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

		if (pTypeExt->Interceptor_ConsiderWeaponRange.Get() &&
			(distance > pWeapon->Range || distance < pWeapon->MinimumRange))
			continue;

		const auto pBulletExt = BulletExt::ExtMap.Find(pBullet);
		const auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

		if (!pBulletTypeExt->Interceptable ||
			pBulletExt->CurrentStrength <= 0 ||
			pBulletExt->InterceptedStatus == InterceptedStatus::Targeted)
			continue;

		if (pBulletTypeExt->Armor.isset())
		{
			auto const pWhExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
			if (std::abs(pWhExt->GetVerses(pBulletTypeExt->Armor).Verses)
				//GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead , pBulletTypeExt->Armor))
				< 0.001)
				continue;
		}

		const auto bulletOwner = pBullet->Owner ? pBullet->Owner->GetOwningHouse() : pBulletExt->Owner;

		if (EnumFunctions::CanTargetHouse(pTypeExt->Interceptor_CanTargetHouses.Get(), pThis->Owner, bulletOwner))
		{
			pTargetBullet = pBullet;
			break;
		}
	}

	if (pTargetBullet)
	{
		pThis->SetTarget(pTargetBullet);
	}
}

void TechnoExt::ExtData::UpdateSpawnLimitRange()
{
	auto const pThis = this->Get();
	const auto pExt = TechnoTypeExt::ExtMap.Find(Type);
	auto const pManager = pThis->SpawnManager;

	if (!pExt->Spawn_LimitedRange || !pManager)
		return;

	int weaponRange = 0;
	const int weaponRangeExtra = pExt->Spawn_LimitedExtraRange.Get() * 256;

	auto setWeaponRange = [&weaponRange](WeaponTypeClass* pWeaponType)
	{
		if (pWeaponType && pWeaponType->Spawner && pWeaponType->Range > weaponRange)
			weaponRange = pWeaponType->Range;
	};

	if (Type->IsGattling || pThis->CurrentWeaponNumber > 0)
	{
		if (auto const pCurWeapon = pThis->GetWeapon(pThis->CurrentWeaponNumber))
			setWeaponRange(pCurWeapon->WeaponType);
	}
	else
	{
		if (auto const pPriWeapon = pThis->GetWeapon(0))
			setWeaponRange(pPriWeapon->WeaponType);

		if (auto const pSecWeapon = pThis->GetWeapon(1))
			setWeaponRange(pSecWeapon->WeaponType);
	}

	weaponRange += weaponRangeExtra;

	if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
		pManager->ResetTarget();
}

bool TechnoExt::IsHarvesting(TechnoClass* pThis)
{
	if (!TechnoExt::IsActive(pThis, true, true))
		return false;

	const auto slave = pThis->SlaveManager;
	if (slave && slave->State != SlaveManagerStatus::Ready)
		return true;

	if (Is_Building(pThis) && pThis->IsPowerOnline())
		return true;

	const auto mission = pThis->GetCurrentMission();
	if ((mission == Mission::Harvest || mission == Mission::Unload || mission == Mission::Enter)
		&& TechnoExt::HasAvailableDock(pThis))
	{
		return true;
	}

	return false;
}

bool TechnoExt::HasAvailableDock(TechnoClass* pThis)
{
	for (auto const& pBld : pThis->GetTechnoType()->Dock)
	{
		if (pThis->Owner->CountOwnedAndPresent(pBld) > 0)
			return true;
	}

	return false;
}

void TechnoExt::InitializeLaserTrail(TechnoClass* pThis, bool bIsconverted)
{
	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pType = pThis->GetTechnoType();

	if (Is_Building(pThis) || pType->Invisible)
		return;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (bIsconverted)
		pExt->LaserTrails.clear();

	auto const pOwner = pThis->GetOwningHouse() ? pThis->GetOwningHouse() : HouseExt::FindCivilianSide();

	if (pExt->LaserTrails.empty())
	{
		for (auto const& entry : pTypeExt->LaserTrailData)
		{
			pExt->LaserTrails.emplace_back(
					LaserTrailTypeClass::Array[entry.idxType].get(), pOwner->LaserColor, entry.FLH, entry.IsOnTurret);
		}
	}

}

bool TechnoExt::FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType)
{
	if (!pWeaponType)
		return false;

	WeaponTypeExt::DetonateAt(pWeaponType, pThis, pThis , true);
	return true;
}

Matrix3D TechnoExt::GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey)
{
	Matrix3D Mtx { };
	// Step 1: get body transform matrix
	if (pThis && (pThis->AbstractFlags & AbstractFlags::Foot) && ((FootClass*)pThis)->Locomotor)
	{
		((FootClass*)pThis)->Locomotor.GetInterfacePtr()->Draw_Matrix(&Mtx, pKey);
		return Mtx;
	}

	// no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter

	Mtx.MakeIdentity();
	return 	Mtx;
}

// reversed from 6F3D60
CoordStruct TechnoExt::GetFLHAbsoluteCoords(TechnoClass* pThis, const CoordStruct& pCoord, bool isOnTurret, const CoordStruct& Overrider)
{
	auto const pType = pThis->GetTechnoType();
	Matrix3D mtx = TechnoExt::GetTransform(pThis);

	// Steps 2-3: turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		const auto& nOffs = TechnoTypeExt::ExtMap.Find(pType)->TurretOffset.Get();
			float x = static_cast<float>(nOffs.X * 1.0);
			float y = static_cast<float>(nOffs.Y * 1.0);
			float z = static_cast<float>(nOffs.Z * 1.0);
			mtx.Translate(x, y, z);

		double turretRad = (pThis->TurretFacing().GetFacing<32>() - 8) * -(Math::Pi / 16);
		double bodyRad = (pThis->PrimaryFacing.Current().GetFacing<32>() - 8) * -(Math::Pi / 16);
		float angle = static_cast<float>(turretRad - bodyRad);
		mtx.RotateZ(angle);
	}

	// Step 4: apply FLH offset
	mtx.Translate(static_cast<float>(pCoord.X), static_cast<float>(pCoord.Y), static_cast<float>(pCoord.Z));

	Vector3D<float> result {};
	Matrix3D::MatrixMultiply(result, &mtx, Vector3D<float>::Empty);
	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// Step 5: apply as an offset to global object coords
	CoordStruct location = Overrider ? Overrider : pThis->GetCoords();
	location += { static_cast<int>(result.X), static_cast<int>(result.Y), static_cast<int>(result.Z) };
	// += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

	return location;
}

double TechnoExt::GetROFMult(TechnoClass const* pTech)
{
	const bool rofAbility = pTech->HasAbility(AbilityType::ROF);

	return !rofAbility ? 1.0 :
		RulesClass::Instance->VeteranROF * ((!pTech->Owner || !pTech->Owner->Type) ?
			1.0 : pTech->Owner->Type->ROFMult);
}

int TechnoExt::ExtData::GetEatPassangersTotalTime(TechnoTypeClass* pTransporterData, FootClass const* pPassenger)
{
	auto const pData = TechnoTypeExt::ExtMap.Find(pTransporterData);
	auto const pThis = this->Get();
	auto const& pDelType = pData->PassengerDeletionType;

	int nRate = 0;

	if (pDelType->UseCostAsRate.Get())
	{
		// Use passenger cost as countdown.
		auto timerLength = static_cast<int>(pPassenger->GetTechnoType()->Cost * pDelType->CostMultiplier);
		const auto nCostRateCap = pDelType->CostRateCap.Get(-1);
		if (nCostRateCap > 0)
			timerLength = MinImpl(timerLength, nCostRateCap);

		nRate = timerLength;
	}
	else
	{
		// Use explicit rate optionally multiplied by unit size as countdown.
		auto timerLength = pDelType->Rate.Get();
		if (pDelType->Rate_SizeMultiply.Get() && pPassenger->GetTechnoType()->Size > 1.0)
			timerLength *= static_cast<int>(pPassenger->GetTechnoType()->Size + 0.5);

		nRate = timerLength;
	}

	if (pDelType->Rate_AffectedByVeterancy)
	{
		auto const nRof = TechnoExt::GetROFMult(pThis);
		if (nRof != 1.0)
		{
			nRate = static_cast<int>(nRate * nRof);
		}
	}

	return nRate;
}

void TechnoExt::ExtData::UpdateEatPassengers()
{
	auto const pThis = this->Get();

	if (!TechnoExt::IsActive(pThis))
		return;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(Type);
	if (!pTypeExt || !pTypeExt->PassengerDeletionType)
		return;

	auto const& pDelType = pTypeExt->PassengerDeletionType;

	if ((pDelType->Rate > 0 || pDelType->UseCostAsRate))
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			// Passengers / CargoClass is essentially a stack, last in, first out (LIFO) kind of data structure
			FootClass* pPassenger = nullptr;          // Passenger to potentially delete
			FootClass* pPreviousPassenger = nullptr;  // Passenger immediately prior to the deleted one in the stack
			ObjectClass* pLastPassenger = nullptr;    // Passenger that is last in the stack
			auto pCurrentPassenger = pThis->Passengers.GetFirstPassenger();

			// Find the first entered passenger that is eligible for deletion.
			while (pCurrentPassenger)
			{
				if (EnumFunctions::CanTargetHouse(pDelType->AllowedHouses, pThis->Owner, pCurrentPassenger->Owner))
				{
					pPreviousPassenger = abstract_cast<FootClass*>(pLastPassenger);
					pPassenger = pCurrentPassenger;
				}

				pLastPassenger = pCurrentPassenger;
				pCurrentPassenger = abstract_cast<FootClass*>(pCurrentPassenger->NextObject);
			}

			if (!pPassenger)
			{
				this->PassengerDeletionTimer.Stop();
				return;
			}

			if (!this->PassengerDeletionTimer.HasStarted()) // Execute only if timer has been stopped or not started
			{
				// Setting & start countdown. Bigger units needs more time
				int timerLength = this->GetEatPassangersTotalTime(Type, pPassenger);

				if (timerLength <= 0)
					return;

				this->PassengerDeletionTimer.Start(timerLength);
			}

			if (this->PassengerDeletionTimer.Completed()) // Execute only if timer has ran out after being started
			{
				--pThis->Passengers.NumPassengers;

				if (pLastPassenger)
					pLastPassenger->NextObject = nullptr;

				if (pPreviousPassenger)
					pPreviousPassenger->NextObject = pPassenger->NextObject;

				if (pThis->Passengers.NumPassengers <= 0)
					pThis->Passengers.FirstPassenger = nullptr;

				if (auto const pPassengerType = pPassenger->GetTechnoType())
				{
					pPassenger->LiberateMember();

					auto const& nReportSound = pDelType->ReportSound;
					VocClass::PlayIndexAtPos(nReportSound.Get(), pThis->Location);

					auto const pThisOwner = pThis->GetOwningHouse();

					if (const auto pAnimType = pDelType->Anim.Get(nullptr))
					{
						if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location))
						{
							pAnim->SetOwnerObject(pThis);
							AnimExt::SetAnimOwnerHouseKind(pAnim, pThisOwner, pPassenger->GetOwningHouse(), pThis, false);
						}
					}

					// Check if there is money refund
					if (pDelType->Soylent &&
						EnumFunctions::CanTargetHouse(pDelType->SoylentAllowedHouses, pThis->Owner, pPassenger->Owner))
					{
						if (const int nMoneyToGive = (int)(pPassenger->GetTechnoType()->GetRefund(pPassenger->Owner, true) *
							pDelType->SoylentMultiplier))
						{
							pThis->Owner->TransactMoney(nMoneyToGive);

							if (pDelType->DisplaySoylent)
							{
								FlyingStrings::AddMoneyString(true, nMoneyToGive, pThis,
									pDelType->DisplaySoylentToHouses, pThis->Location, pDelType->DisplaySoylentOffset);
							}
						}
					}

					// Handle gunner change.
					if (pThis->GetTechnoType()->Gunner)
					{
						if (auto const pFoot = abstract_cast<FootClass*>(pThis))
						{
							pFoot->RemoveGunner(pPassenger);

							if (pThis->Passengers.NumPassengers > 0)
								pFoot->ReceiveGunner(pThis->Passengers.FirstPassenger);
						}
					}

					if (pThis->GetTechnoType()->OpenTopped)
					{
						pThis->ExitedOpenTopped(pPassenger);
					}

					if (const auto pBld = specific_cast<BuildingClass*>(pThis))
					{
						if (pBld->Absorber())
						{
							pPassenger->Absorbed = false;

							if (pBld->Type->ExtraPowerBonus > 0)
							{
								pBld->Owner->RecheckPower = true;
							}
						}
					}

					//auto const pPassengerOwner = pPassenger->Owner;

					//if (!pPassengerOwner->IsNeutral() && !pThis->GetTechnoType()->Insignificant)
					//{
					//	pPassengerOwner->RegisterLoss(pPassenger, false);
					//	pPassengerOwner->RemoveTracking(pPassenger);

					//	if (!pPassengerOwner->RecheckTechTree)
					//		pPassengerOwner->RecheckTechTree = true;
					//}


					pPassenger->RegisterDestruction(pDelType->DontScore ? nullptr : pThis);
					TechnoExt::HandleRemove(pPassenger, pDelType->DontScore ? nullptr : pThis ,false ,false);
				}

				this->PassengerDeletionTimer.Stop();
			}
		}
	}
}

void TechnoExt::ExtData::UpdateDelayFireAnim()
{
	auto const pThis = this->Get();

	if (TechnoExt::IsAlive(pThis) && pThis->Target || pThis->GetCurrentMission() == Mission::Attack)
		return;

	pThis->DiskLaserTimer.Start(pThis->DiskLaserTimer.GetTimeLeft() + 5);

	// Reset Delayed fire animation
	TechnoExt::ResetDelayFireAnim(pThis);
}

bool TechnoExt::CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex)
{
	if (pThis->GetTechnoType()->Ammo > 0)
	{
		const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		if (pThis->Ammo <= pExt->NoAmmoAmount && (pExt->NoAmmoWeapon == weaponIndex || pExt->NoAmmoWeapon == -1))
			return true;
	}

	return false;
}

void TechnoExt::HandleRemove(TechnoClass* pThis, TechnoClass* pSource, bool SkipTrackingRemove, bool Delete)
{
	// kill passenger cargo to prevent memleak
	pThis->KillPassengers(pSource);

	if (const auto pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		pBuilding->KillOccupants(nullptr);
	}

	if (!SkipTrackingRemove)
	{
		if (const auto pOwner = pThis->GetOwningHouse())
		{
			if (!pOwner->IsNeutral() && !pThis->GetTechnoType()->Insignificant && !pThis->InLimbo)
			{
				pOwner->RegisterLoss(pThis, false);
				pOwner->RemoveTracking(pThis);

				if (!pOwner->RecheckTechTree)
					pOwner->RecheckTechTree = true;
			}
		}
	}

	if (auto const pFoot = generic_cast<FootClass*>(pThis))
		pFoot->LiberateMember();

	pThis->RemoveFromTargetingAndTeam();

	if (Delete)
		GameDelete<true, false>(pThis);
	else
		pThis->UnInit();
}

void TechnoExt::KillSelf(TechnoClass* pThis, bool isPeaceful)
{
	if (isPeaceful)
	{
		// this shit is not really good idea to pull of
		// some stuffs doesnt really handled properly , wtf
		if (!pThis->InLimbo)
			pThis->Limbo();

		TechnoExt::HandleRemove(pThis, nullptr, false, false);
	}
	else
	{
		pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, false, false, pThis->Owner);
	}
}

KillMethod NOINLINE GetKillMethod(KillMethod deathOption)
{
	if (deathOption == KillMethod::Random) {
		return static_cast<KillMethod>(ScenarioClass::Instance->Random.RandomRanged((int)KillMethod::Explode, (int)KillMethod::Sell));
	}

	return deathOption;
}

void TechnoExt::KillSelf(TechnoClass* pThis, const KillMethod& deathOption, bool RegisterKill, AnimTypeClass* pVanishAnim)
{
	if (!pThis || deathOption == KillMethod::None || !pThis->IsAlive)
		return;

	auto const pWhat = VTable::Get(pThis);

	switch (GetKillMethod(deathOption))
	{
	case KillMethod::Explode:
	{
		if(pThis) { 
			auto nHealth = pThis->Health;
			pThis->ReceiveDamage(&nHealth, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, nullptr);
		}

	}break;
	case KillMethod::Vanish:
	{
		// this shit is not really good idea to pull off
		// some stuffs doesnt really handled properly , wtf

		if (pVanishAnim) {
			if (auto const pAnim = GameCreate<AnimClass>(pVanishAnim, pThis->GetCoords())) {
				AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, true);
			}
		}

		pThis->Stun();

		if (!pThis->InLimbo)
			pThis->Limbo();

		if (RegisterKill)
			pThis->RegisterKill(pThis->Owner);

		TechnoExt::HandleRemove(pThis ,nullptr, false , false);

	}break;
	case KillMethod::Sell:
	{
		if (pWhat == BuildingClass::vtable)
		{
			const auto pBld = static_cast<BuildingClass*>(pThis);

			if (pBld->HasBuildup && (pBld->CurrentMission != Mission::Selling || pBld->CurrentMission != Mission::Unload))
			{
				BuildingExt::ExtMap.Find(pBld)->Silent = true;
				pBld->Sell(true);
				return;
			}
		}
		else if (pThis->AbstractFlags & AbstractFlags::Foot)
		{
			const auto pFoot = static_cast<FootClass*>(pThis);

			if (pWhat != InfantryClass::vtable && pFoot->CurrentMission != Mission::Unload)
			{
				if (auto const pCell = pFoot->GetCell())
				{
					if (auto const pBuilding = pCell->GetBuilding())
					{
						if (pBuilding->Type->UnitRepair)
						{
							pFoot->Sell(true);
							return;
						}
					}
				}
			}
		}

		if (pThis && pThis->IsAlive) {
			pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, nullptr);
		}

	}break;
	}
}

// Feature: Kill Object Automatically
bool TechnoExt::ExtData::CheckDeathConditions()
{
	auto const pThis = this->Get();
	const auto pTypeThis = this->Type;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(this->Type);

	const KillMethod nMethod = pTypeExt->Death_Method.Get();
	const auto pVanishAnim = pTypeExt->AutoDeath_VanishAnimation.Get();

	if (nMethod == KillMethod::None)
		return false;

	// Death if no ammo
	if (pTypeExt->Death_NoAmmo)
	{
		if (pTypeThis->Ammo > 0 && pThis->Ammo <= 0)
		{
			TechnoExt::KillSelf(pThis, nMethod , pVanishAnim);
			return !pThis->IsAlive;
		}
	}

	const auto existTechnoTypes = [pThis](const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any, bool allowLimbo)
	{
		const auto existSingleType = [pThis, affectedHouse, allowLimbo](const TechnoTypeClass* pType)
		{
			for (HouseClass* pHouse : *HouseClass::Array)
			{
				if (EnumFunctions::CanTargetHouse(affectedHouse, pThis->Owner, pHouse)
					&& (allowLimbo ? pHouse->CountOwnedNow(pType) > 0 : pHouse->CountOwnedAndPresent(pType) > 0))
					return true;
			}

			return false;
		};

		return any
			? std::any_of(vTypes.begin(), vTypes.end(), existSingleType)
			: std::all_of(vTypes.begin(), vTypes.end(), existSingleType);
	};

	// Death if nonexist
	if (!pTypeExt->AutoDeath_Nonexist.empty())
	{
		if (!existTechnoTypes(pTypeExt->AutoDeath_Nonexist,
			pTypeExt->AutoDeath_Nonexist_House,
			!pTypeExt->AutoDeath_Nonexist_Any, pTypeExt->AutoDeath_Nonexist_AllowLimboed))
		{
			TechnoExt::KillSelf(pThis, nMethod, pVanishAnim);

			return !pThis->IsAlive;
		}
	}

	// Death if exist
	if (!pTypeExt->AutoDeath_Exist.empty())
	{
		if (existTechnoTypes(pTypeExt->AutoDeath_Exist,
			pTypeExt->AutoDeath_Exist_House,
			pTypeExt->AutoDeath_Exist_Any,
			pTypeExt->AutoDeath_Exist_AllowLimboed))
		{
			TechnoExt::KillSelf(pThis, nMethod, pVanishAnim);

			return !pThis->IsAlive;
		}
	}

	// Death if countdown ends
	if (pTypeExt->Death_Countdown > 0)
	{
		if (!Death_Countdown.HasStarted())
		{
			Death_Countdown.Start(pTypeExt->Death_Countdown);
			if (pThis->Owner)
				HouseExt::ExtMap.Find(pThis->Owner)->AutoDeathObjects.insert(pThis, nMethod);
		}
		else if (Death_Countdown.Completed())
		{
			TechnoExt::KillSelf(pThis, nMethod, pVanishAnim);
			return !pThis->IsAlive;
		}
	}

	return false;
}

void TechnoExt::ApplyGainedSelfHeal(TechnoClass* pThis)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();
	const int healthDeficit = pType->Strength - pThis->Health;

	if (pThis->Health && healthDeficit > 0)
	{
		const auto pWhat = pThis->WhatAmI();
		const bool isBuilding = pWhat == AbstractType::Building;
		const bool isOrganic = pWhat == AbstractType::Infantry || (pWhat == AbstractType::Unit && pType->Organic);
		const auto defaultSelfHealType = isBuilding ? SelfHealGainType::None : isOrganic ? SelfHealGainType::Infantry : SelfHealGainType::Units;
		bool applyHeal = false;
		int amount = 0;

		switch (TechnoTypeExt::ExtMap.Find(pType)->SelfHealGainType.Get(defaultSelfHealType))
		{
		case SelfHealGainType::Infantry:
		{
			if (!pThis->Owner->InfantrySelfHeal)
				return;

			const int count = RulesExt::Global()->InfantryGainSelfHealCap.isset() ?
				std::clamp(pThis->Owner->InfantrySelfHeal, 1, RulesExt::Global()->InfantryGainSelfHealCap.Get()) :
				pThis->Owner->InfantrySelfHeal;

			amount = RulesClass::Instance->SelfHealInfantryAmount * count;

			if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealInfantryFrames) && amount)
				applyHeal = true;
		}
		break;
		case SelfHealGainType::Units:
		{
			if (!pThis->Owner->UnitsSelfHeal)
				return;

			const int count = RulesExt::Global()->UnitsGainSelfHealCap.isset() ?
				std::clamp(pThis->Owner->UnitsSelfHeal, 1, RulesExt::Global()->UnitsGainSelfHealCap.Get()) :
				pThis->Owner->UnitsSelfHeal;

			amount = RulesClass::Instance->SelfHealUnitAmount * count;

			if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealUnitFrames) && amount)
				applyHeal = true;
		}
		break;
		default:
			return;
		}

		if (applyHeal && amount)
		{
			if (amount >= healthDeficit)
				amount = healthDeficit;

			if (Phobos::Debug_DisplayDamageNumbers)
				FlyingStrings::AddNumberString(amount, pThis->Owner, AffectedHouse::All, Drawing::DefaultColors[(int)DefaultColorList::White], pThis->Location, Point2D::Empty, false, L"");

			const bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

			pThis->Health += amount;

			if (wasDamaged && (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
				|| pThis->GetHeight() < -10))
			{
				if (isBuilding)
				{
					const auto pBuilding = static_cast<BuildingClass*>(pThis);
					pBuilding->UpdatePlacement(PlacementType::Redraw);
					pBuilding->ToggleDamagedAnims(false);
				}

				if (pWhat == AbstractType::Unit || isBuilding)
				{
					if (auto& dmgParticle = pThis->DamageParticleSystem)
					{
						//GameDelete<true,false>(dmgParticle);
						dmgParticle->UnInit();
						dmgParticle = nullptr;
					}
				}
			}
		}
	}

	return;
}

void TechnoExt::ApplyDrainMoney(TechnoClass* pThis)
{
	const auto pSource = pThis->DrainingMe;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
	const auto pRules = RulesClass::Instance();
	const auto nDrainDelay = pTypeExt->DrainMoneyFrameDelay.Get(pRules->DrainMoneyFrameDelay);

	if ((Unsorted::CurrentFrame % nDrainDelay) == 0)
	{
		auto nDrainAmount = pTypeExt->DrainMoneyAmount.Get(pRules->DrainMoneyAmount);
		if ( nDrainAmount != 0) {
			if(!pThis->Owner->CanTransactMoney(nDrainAmount)
				|| !pSource->Owner->CanTransactMoney(nDrainAmount) )
				return;

				pThis->Owner->TransactMoney(-nDrainAmount);
				pSource->Owner->TransactMoney(nDrainAmount);

			if (pTypeExt->DrainMoney_Display)
			{
				auto const pDest = pTypeExt->DrainMoney_Display_AtFirer.Get() ? pSource : pThis;
				FlyingStrings::AddMoneyString(true, nDrainAmount, pDest,
					pTypeExt->DrainMoney_Display_Houses, pDest->Location,
					pTypeExt->DrainMoney_Display_Offset, ColorStruct::Empty);
			}
		}
	}
}

void TechnoExt::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	bool drawPip = false;
	bool isInfantryHeal = false;
	int selfHealFrames = 0;

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto const& nSelfHealType = pExt->SelfHealGainType;

	if (nSelfHealType.isset() && nSelfHealType.Get() == SelfHealGainType::None)
		return;

	auto const pWhat = GetVtableAddr(pThis);
	const bool hasInfantrySelfHeal = nSelfHealType.isset() && nSelfHealType.Get() == SelfHealGainType::Infantry;
	const bool hasUnitSelfHeal = nSelfHealType.isset() && nSelfHealType.Get() == SelfHealGainType::Units;
	const bool isOrganic = pWhat == InfantryClass::vtable 
	|| (pThis->GetTechnoType()->Organic && (pWhat == UnitClass::vtable));

	if (pThis->Owner->InfantrySelfHeal > 0 && (hasInfantrySelfHeal || isOrganic))
	{
		drawPip = true;
		selfHealFrames = RulesClass::Instance->SelfHealInfantryFrames;
		isInfantryHeal = true;
	}
	else if (pThis->Owner->UnitsSelfHeal > 0
		&& (hasUnitSelfHeal || (pWhat == UnitClass::vtable && !isOrganic)))
	{
		drawPip = true;
		selfHealFrames = RulesClass::Instance->SelfHealUnitFrames;
	}

	if (drawPip)
	{
		Point2D pipFrames { 0,0 };
		bool isSelfHealFrame = false;
		int xOffset = 0;
		int yOffset = 0;

		if (Unsorted::CurrentFrame % selfHealFrames <= 5
			&& pThis->Health < pThis->GetTechnoType()->Strength)
		{
			isSelfHealFrame = true;
		}

		int nBracket = TechnoExt::GetDisguiseType(pThis, false, true).first->PixelSelectionBracketDelta;

		switch (pWhat)
		{
		case UnitClass::vtable:
		case AircraftClass::vtable:
		{
			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Units_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Units.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case InfantryClass::vtable:
		{
			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Infantry_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Infantry.Get();
			xOffset = offset.X;
			yOffset = offset.Y + nBracket;
		}
		break;
		case BuildingClass::vtable:
		{
			const auto pType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			int fHeight = pType->GetFoundationHeight(false);
			int yAdjust = -Unsorted::CellHeightInPixels / 2;

			const auto& offset = RulesExt::Global()->Pips_SelfHeal_Buildings_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Buildings.Get();
			xOffset = offset.X + Unsorted::CellWidthInPixels / 2 * fHeight;
			yOffset = offset.Y + yAdjust * fHeight + pType->Height * yAdjust;
		}
		break;
		}

		int pipFrame = isInfantryHeal ? pipFrames.X : pipFrames.Y;

		Point2D position { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		if (isSelfHealFrame)
			flags = flags | BlitterFlags::Darken;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
			pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExt::TransformFLHForTurret(TechnoClass* pThis, Matrix3D& mtx, bool isOnTurret)
{
	auto const pType = pThis->GetTechnoType();

	// turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		const auto& nOffs = TechnoTypeExt::ExtMap.Find(pType)->TurretOffset.Get();
		float x = static_cast<float>(nOffs.X * TechnoTypeExt::TurretMultiOffsetDefaultMult);
		float y = static_cast<float>(nOffs.Y * TechnoTypeExt::TurretMultiOffsetDefaultMult);
		float z = static_cast<float>(nOffs.Z * TechnoTypeExt::TurretMultiOffsetDefaultMult);

		mtx.Translate(x, y, z);

		double turretRad = (pThis->TurretFacing().GetFacing<32>() - 8) * -(Math::Pi / 16);
		double bodyRad = (pThis->PrimaryFacing.Current().GetFacing<32>() - 8) * -(Math::Pi / 16);
		float angle = (float)(turretRad - bodyRad);

		mtx.RotateZ(angle);
	}
}

Matrix3D TechnoExt::GetFLHMatrix(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret)
{
	Matrix3D nMTX = TechnoExt::GetTransform(abstract_cast<FootClass*>(pThis));
	TechnoExt::TransformFLHForTurret(pThis, nMTX, isOnTurret);

	// apply FLH offset
	nMTX.Translate((float)nCoord.X, (float)nCoord.Y, (float)nCoord.Z);

	return nMTX;
}

CoordStruct TechnoExt::GetFLHAbsoluteCoordsB(TechnoClass* pThis, const CoordStruct& nCoord, bool isOnTurret)
{
	Vector3D<float> result = Matrix3D::MatrixMultiply(
	TechnoExt::GetFLHMatrix(pThis, nCoord, isOnTurret), Vector3D<float>::Empty);

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// apply as an offset to global object coords
	CoordStruct location = pThis->GetCoords();
	location += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

	return location;
}

void TechnoExt::UpdateSharedAmmo(TechnoClass* pThis)
{
	if (!pThis)
		return;

	const auto pType = pThis->GetTechnoType();

	if (!pType->OpenTopped || !pThis->Passengers.NumPassengers)
		return;

	const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (!pExt->Ammo_Shared || !pType->Ammo)
		return;

	auto passenger = pThis->Passengers.FirstPassenger;

	do
	{

		TechnoTypeClass* passengerType = passenger->GetTechnoType();

		if (TechnoTypeExt::ExtMap.Find(passengerType)->Ammo_Shared.Get())
		{
			if (pExt->Ammo_Shared_Group.Get() < 0 ||
				pExt->Ammo_Shared_Group.Get() == TechnoTypeExt::ExtMap.Find(passengerType)->Ammo_Shared_Group.Get())
			{
				if (pThis->Ammo > 0 && (passenger->Ammo < passengerType->Ammo))
				{
					pThis->Ammo--;
					passenger->Ammo++;
				}
			}
		}

		passenger = static_cast<FootClass*>(passenger->NextObject);

	}
	while (passenger);
}

double TechnoExt::GetCurrentSpeedMultiplier(FootClass* pThis)
{
	double houseMultiplier = 1.0;

	switch (GetVtableAddr(pThis))
	{
	case AircraftClass::vtable:
		houseMultiplier = pThis->Owner->Type->SpeedAircraftMult;
		break;
	case InfantryClass::vtable:
		houseMultiplier = pThis->Owner->Type->SpeedInfantryMult;
		break;
	case UnitClass::vtable:
		houseMultiplier = pThis->Owner->Type->SpeedUnitsMult;
		break;
	}

	return pThis->SpeedMultiplier * houseMultiplier *
		(pThis->HasAbility(AbilityType::Faster) ? RulesClass::Instance->VeteranSpeed : 1.0);
}

void TechnoExt::ExtData::UpdateMindControlAnim()
{
	const auto pThis = this->Get();

	if (pThis->IsMindControlled())
	{
		if (pThis->MindControlRingAnim && !MindControlRingAnimType)
		{
			MindControlRingAnimType = pThis->MindControlRingAnim->Type;
		}
		else if (!pThis->MindControlRingAnim && MindControlRingAnimType &&
			pThis->CloakState == CloakState::Uncloaked && !pThis->InLimbo && pThis->IsAlive)
		{
			auto coords = pThis->GetCoords();
			int offset = 0;
			const auto pBuilding = specific_cast<BuildingClass*>(pThis);

			if (pBuilding)
				offset = Unsorted::LevelHeight * pBuilding->Type->Height;
			else
				offset = pThis->GetTechnoType()->MindControlRingOffset;

			coords.Z += offset;

			if (auto anim = GameCreate<AnimClass>(MindControlRingAnimType, coords, 0, 1))
			{
				pThis->MindControlRingAnim = anim;
				pThis->MindControlRingAnim->SetOwnerObject(pThis);

				if (pBuilding)
					pThis->MindControlRingAnim->ZAdjust = -1024;
			}
		}
	}
	else if (MindControlRingAnimType)
	{
		MindControlRingAnimType = nullptr;
	}
}

std::pair<const std::vector<WeaponTypeClass*>*, const std::vector<int>*> TechnoExt::ExtData::GetFireSelfData()
{
	const auto pThis = this->Get();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(Type);

	if (pThis->IsRedHP() && !pTypeExt->FireSelf_Weapon_RedHeath.empty() && !pTypeExt->FireSelf_ROF_RedHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_RedHeath , &pTypeExt->FireSelf_ROF_RedHeath };
	}
	else if (pThis->IsYellowHP() && !pTypeExt->FireSelf_Weapon_YellowHeath.empty() && !pTypeExt->FireSelf_ROF_YellowHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_YellowHeath , &pTypeExt->FireSelf_ROF_YellowHeath };
	}
	else if (pThis->IsGreenHP() && !pTypeExt->FireSelf_Weapon_GreenHeath.empty() && !pTypeExt->FireSelf_ROF_GreenHeath.empty())
	{
		return  { &pTypeExt->FireSelf_Weapon_GreenHeath , &pTypeExt->FireSelf_ROF_GreenHeath };
	}

	return  { &pTypeExt->FireSelf_Weapon , &pTypeExt->FireSelf_ROF };

}

void TechnoExt::ExtData::UpdateFireSelf()
{
	const auto pThis = this->Get();

	auto const& [FireSelf_Weapon, FireSelf_ROF] = TechnoExt::ExtData::GetFireSelfData();

	if (!FireSelf_Weapon || !FireSelf_ROF || FireSelf_Weapon->empty() || FireSelf_ROF->empty()) return;

	if (FireSelf_Count.size() < FireSelf_Weapon->size())
	{
		const int p = int(FireSelf_Count.size());
		while (FireSelf_Count.size() < FireSelf_Weapon->size())
		{
			int ROF = 10;

			if (p >= (int)FireSelf_ROF->size())
				ROF = FireSelf_Weapon->at(p)->ROF;
			else
				ROF = FireSelf_ROF->at(p);

			FireSelf_Count.emplace_back(ROF);
		}
	}

	for (int i = 0; i < (int)FireSelf_Count.size(); i++)
	{
		FireSelf_Count[i]--;
		if (FireSelf_Count[i] > 0) continue;
		else
		{
			int ROF = 10;

			if (i >= (int)FireSelf_ROF->size())
				ROF = FireSelf_Weapon->at(i)->ROF;
			else
				ROF = FireSelf_ROF->at(i);

			FireSelf_Count[i] = ROF;

			WeaponTypeExt::DetonateAt(FireSelf_Weapon->at(i), pThis, pThis , true);
		}
	}
}

void TechnoExt::ExtData::UpdateOnTunnelEnter()
{
	if (!this->IsInTunnel)
	{
		if (auto& pShieldData = this->Shield)
			pShieldData->SetAnimationVisibility(false);

		for(auto pos =this->LaserTrails.begin();
			pos != this->LaserTrails.end() ; ++pos)
		{
			pos->Visible = false;
			pos->LastLocation.clear();
		}

		TrailsManager::Hide(Get());

		this->IsInTunnel = true;
	}
}

std::pair<WeaponTypeClass*, int> TechnoExt::GetDeployFireWeapon(TechnoClass* pThis, AbstractClass* pTarget)
{
	auto const pType = pThis->GetTechnoType();
	int weaponIndex = pType->DeployFireWeapon;

	if (Is_Unit(pThis))
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType))
		{
			// Only apply DeployFireWeapon on vehicles if explicitly set.
			if (!pTypeExt->DeployFireWeapon.isset())
			{
				weaponIndex = 0;
				if (pThis->GetFireError(pThis->GetCell(), 0, true) != FireError::OK)
					weaponIndex = 1;
			}
		}
	}

	if (weaponIndex == -1)
	{
		weaponIndex = pThis->SelectWeapon(pTarget);
	}
	else if (weaponIndex < -1)
		return { nullptr   , -1 };

	if (auto const pWs = pThis->GetWeapon(weaponIndex))
	{
		return { pWs->WeaponType  , weaponIndex };
	}

	return { nullptr , -1 };
}

void TechnoExt::ExtData::UpdateType(TechnoTypeClass* currentType)
{
	auto const pThis = this->Get();

	this->Type = currentType;
	auto const pTypeExtData = TechnoTypeExt::ExtMap.Find(currentType);

	if (auto pSpawnManager = pThis->SpawnManager) {
		if (currentType->Spawns && pSpawnManager->SpawnType != currentType->Spawns) {
			pSpawnManager->SpawnType = currentType->Spawns;

			if(currentType->SpawnsNumber > 0)
				pSpawnManager->SpawnCount = currentType->SpawnsNumber;

			pSpawnManager->RegenRate = currentType->SpawnRegenRate;
			pSpawnManager->ReloadRate = currentType->SpawnReloadRate;
		}
	}
	//else if(currentType->Spawns && currentType->SpawnsNumber > 0)
	//{
	//	pThis->SpawnManager = GameCreate<SpawnManagerClass>(
	//		pThis,
	//		currentType->Spawns , 
	//		currentType->SpawnsNumber , 
	//		currentType->SpawnRegenRate ,
	//		currentType->SpawnReloadRate);
	//}

	TechnoExt::InitializeLaserTrail(pThis, true);

	// Reset Shield
	// This part should have been done by UpdateShield

	// Reset AutoDeath Timer
	if (this->Death_Countdown.HasStarted())
	{
		this->Death_Countdown.Stop();

		if (pThis->Owner)
		{
			HouseExt::ExtMap.Find(pThis->Owner)->AutoDeathObjects.erase(pThis);
		}
	}

	// Reset PassengerDeletion Timer - TODO : unchecked
	if (this->PassengerDeletionTimer.IsTicking()
		&& pTypeExtData->PassengerDeletionType
		&& pTypeExtData->PassengerDeletionType->Rate <= 0)
		this->PassengerDeletionTimer.Stop();

	TrailsManager::Construct(static_cast<TechnoClass*>(pThis), true);

	/*if (!pTypeExtData->MyFighterData.Enable && this->MyFighterData)
		this->MyFighterData.reset(nullptr);

	else if (pTypeExtData->MyFighterData.Enable && !this->MyFighterData)
	{
		this->MyFighterData = std::make_unique<FighterAreaGuard>();
		this->MyFighterData->OwnerObject = (AircraftClass*)pThis;
	}

	if(!pTypeExtData->DamageSelfData.Enable && this->DamageSelfState)
		this->DamageSelfState.reset(nullptr);
	else if (pTypeExtData->DamageSelfData.Enable && !this->DamageSelfState)
		DamageSelfState::OnPut(this->DamageSelfState, pTypeExtData->DamageSelfData);

	if (!pTypeExtData->MyGiftBoxData.Enable && this->MyGiftBox)
		this->MyGiftBox.reset(nullptr);
	else if (pTypeExtData->MyGiftBoxData.Enable && !this->MyGiftBox)
		GiftBoxFunctional::Init(this, pTypeExtData);*/

}

void TechnoExt::ExtData::UpdateBuildingLightning()
{
	auto const pThis = this->Get();

	if (!Is_Building(pThis))
		return;

	auto pBldExt = BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis));
	if (pBldExt->LighningNeedUpdate)
	{
		pThis->UpdatePlacement(PlacementType::Redraw);
		pBldExt->LighningNeedUpdate = false;

	}
}

void TechnoExt::ExtData::UpdateShield()
{
	auto const pThis = this->Get();

	if (!this->CurrentShieldType)
		Debug::FatalErrorAndExit("Techno[%s] Missing CurrentShieldType ! \n", pThis->get_ID());

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(this->Type);

	// Set current shield type if it is not set.
	if (!this->CurrentShieldType->Strength && pTypeExt->ShieldType->Strength)
		this->CurrentShieldType = pTypeExt->ShieldType;

	// Create shield class instance if it does not exist.
	if (this->CurrentShieldType && this->CurrentShieldType->Strength && !this->Shield)
	{
		this->Shield = std::make_unique<ShieldClass>(pThis);
		this->Shield->OnInit();
	}

	if (const  auto pShieldData = this->GetShield())
		pShieldData->OnUpdate();
}

void TechnoExt::ExtData::UpdateMobileRefinery()
{
	auto const pThis = this->Get();

	if (!(pThis->AbstractFlags & AbstractFlags::Foot))
		return;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(Type);

	if (!pTypeExt->MobileRefinery || !pThis->Owner || (pTypeExt->MobileRefinery_TransRate > 0 &&
		Unsorted::CurrentFrame % pTypeExt->MobileRefinery_TransRate))
		return;

	const int cellCount =
		std::clamp(static_cast<int>(pTypeExt->MobileRefinery_FrontOffset.size()), 1,
				static_cast<int>(pTypeExt->MobileRefinery_LeftOffset.size()));

	CoordStruct flh = { 0,0,0 };

	for (int idx = 0; idx < cellCount; idx++)
	{
		flh.X = static_cast<int>(pTypeExt->MobileRefinery_FrontOffset.size()) > idx ? pTypeExt->MobileRefinery_FrontOffset[idx] * Unsorted::LeptonsPerCell : 0;
		flh.Y = static_cast<int>(pTypeExt->MobileRefinery_LeftOffset.size()) > idx ? pTypeExt->MobileRefinery_LeftOffset[idx] * Unsorted::LeptonsPerCell : 0;
		auto nPos = TechnoExt::GetFLHAbsoluteCoords(pThis, flh, false);
		const CellClass* pCell = MapClass::Instance->GetCellAt(nPos);

		if (!pCell)
			continue;

		nPos.Z += pThis->Location.Z;

		if (const int tValue = pCell->GetContainedTiberiumValue())
		{
			const int tibValue = TiberiumClass::Array->GetItem(pCell->GetContainedTiberiumIndex())->Value;
			const int tAmount = static_cast<int>(tValue * 1.0 / tibValue);
			const int amount = pTypeExt->MobileRefinery_AmountPerCell ? MinImpl(tAmount , pTypeExt->MobileRefinery_AmountPerCell.Get()) : tAmount;
			pCell->ReduceTiberium(amount);
			const int value = static_cast<int>(amount * tibValue * pTypeExt->MobileRefinery_CashMultiplier);

			if (pThis->Owner->CanTransactMoney(value))
			{
				pThis->Owner->TransactMoney(value);
				FlyingStrings::AddMoneyString(pTypeExt->MobileRefinery_Display, value, pThis, AffectedHouse::All, nPos, Point2D::Empty, pTypeExt->MobileRefinery_DisplayColor);
			}

			if (!pTypeExt->MobileRefinery_Anims.empty())
			{
				AnimTypeClass* pAnimType = nullptr;
				int facing = pThis->PrimaryFacing.Current().GetFacing<8>();

				if (facing >= 7)
					facing = 0;
				else
					facing++;

				switch (pTypeExt->MobileRefinery_Anims.size())
				{
				case 1:
					pAnimType = pTypeExt->MobileRefinery_Anims[0];
					break;
				case 8:
					pAnimType = pTypeExt->MobileRefinery_Anims[facing];
					break;
				default:
					pAnimType = pTypeExt->MobileRefinery_Anims[
						ScenarioClass::Instance->Random.RandomFromMax(pTypeExt->MobileRefinery_Anims.size() - 1)];
					break;
				}

				if (pAnimType)
				{
					if (auto pAnim = GameCreate<AnimClass>(pAnimType, nPos))
					{
						AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);

						if (pTypeExt->MobileRefinery_AnimMove)
							pAnim->SetOwnerObject(pThis);
					}
				}
			}
		}
	}
}

void TechnoExt::ExtData::UpdateRevengeWeapons()
{
	if (this->RevengeWeapons.empty())
		return;

	for (size_t i = 0; i < this->RevengeWeapons.size(); i++)
	{
		auto const& weapon = this->RevengeWeapons[i];

		if (weapon.Timer.Expired())
			this->RevengeWeapons.erase(this->RevengeWeapons.begin() + i);
	}
}

void TechnoExt::ExtData::UpdateAircraftOpentopped()
{
	auto const pThis = this->Get();

	if (!TechnoExt::IsAlive(pThis, true, true, true))
		return;

	if (Type->Passengers > 0 && !AircraftOpentoppedInitEd)
	{

		for (NextObject object(pThis->Passengers.GetFirstPassenger()); object; ++object)
		{
			if (auto const pInf = generic_cast<FootClass*>(*object))
			{
				if (!pInf->Transporter || !pInf->InOpenToppedTransport)
				{
					if (Type->OpenTopped)
						pThis->EnteredOpenTopped(pInf);

					if (Type->Gunner)
						abstract_cast<FootClass*>(pThis)->ReceiveGunner(pInf);

					pInf->Transporter = pThis;
					pInf->Undiscover();
				}
			}
		}

		AircraftOpentoppedInitEd = true;
	}
}

void TechnoExt::ExtData::UpdateLaserTrails()
{
	auto const pThis = this->Get();

	if (LaserTrails.empty())
		return;

	for (auto& trail : LaserTrails)
	{
		if (pThis->CloakState == CloakState::Cloaked && !trail.Type->CloakVisible)
			continue;

		if (!IsInTunnel)
			trail.Visible = true;

		if (Is_Aircraft(pThis) && !pThis->IsInAir() && trail.LastLocation.isset())
			trail.LastLocation.clear();

		CoordStruct trailLoc = TechnoExt::GetFLHAbsoluteCoords(pThis, trail.FLH, trail.IsOnTurret);

		if (pThis->CloakState == CloakState::Uncloaking && !trail.Type->CloakVisible)
			trail.LastLocation = trailLoc;
		else
			trail.Update(trailLoc);
	}
}

void TechnoExt::ExtData::UpdateGattlingOverloadDamage()
{
	auto const pThis = this->Get();

	if (!pThis->IsAlive)
		return;

	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(Type);
	const auto pType = Type;

	if (!pType->IsGattling || !pTypeExt->Gattling_Overload.Get())
		return;

	auto const nDelay = abs(pTypeExt->Gattling_Overload_Frames.Get(0));

	if (!nDelay)
		return;

	auto const curValue = pThis->GattlingValue;
	auto const maxValue = pThis->Veterancy.IsElite() ? pType->EliteStage[pType->WeaponStages - 1] : pType->WeaponStage[pType->WeaponStages - 1];

	if (GattlingDmageDelay <= 0)
	{
		int nStage = curValue;
		if (nStage < maxValue)
		{
			GattlingDmageDelay = -1;
			GattlingDmageSound = false;
			return;
		}

		GattlingDmageDelay = nDelay;
		auto nDamage = pTypeExt->Gattling_Overload_Damage.Get();

		if (nDamage <= 0)
		{
			GattlingDmageSound = false;
		}
		else
		{
			auto const pWarhead = pTypeExt->Gattling_Overload_Warhead.Get(RulesClass::Instance->C4Warhead);
			pThis->ReceiveDamage(&nDamage, 0, pWarhead, 0, 0, 0, 0);

			if (!GattlingDmageSound)
			{
				if (pTypeExt->Gattling_Overload_DeathSound.isset())
					VocClass::PlayIndexAtPos(pTypeExt->Gattling_Overload_DeathSound, pThis->Location, 0);

				GattlingDmageSound = true;
			}

			if (auto const pParticle = pTypeExt->Gattling_Overload_ParticleSys.Get(nullptr))
			{
				for (int i = pTypeExt->Gattling_Overload_ParticleSysCount.Get(1); i > 0; --i)
				{
					auto const nRandomY = ScenarioClass::Instance->Random(-200, 200);
					auto const nRamdomX = ScenarioClass::Instance->Random(-200, 200);
					auto nLoc = pThis->Location;

					if (pParticle->BehavesLike == BehavesLike::Smoke)
						nLoc.Z += 100;

					CoordStruct nParticleCoord { pThis->Location.X + nRamdomX, nRandomY + pThis->Location.Y, pThis->Location.Z + 100 };
					GameCreate<ParticleSystemClass>(pParticle, nParticleCoord, pThis->GetCell(), pThis, CoordStruct::Empty, pThis->Owner);
				}
			}

			if (Is_Unit(pThis) && pThis->IsAlive && pThis->IsVoxel())
			{
				double const nBase = ScenarioClass::Instance->Random.RandomBool() ? 0.015 : 0.029999999;
				double const nCopied_base = (ScenarioClass::Instance->Random.RandomFromMax(100) < 50) ? -nBase : nBase;
				pThis->RockingSidewaysPerFrame = (float)nCopied_base;
			}
		}
	}
	else
	{
		--GattlingDmageDelay;
	}
}

bool TechnoExt::ExtData::UpdateKillSelf_Slave()
{
	auto const pThis = this->Get();

	if (VTable::Get(pThis) != InfantryClass::vtable)
		return false;

	const auto pInf = static_cast<InfantryClass*>(pThis);

	if (!pInf->Type->Slaved)
		return false;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(Type);

	if (!pInf->SlaveOwner && (pTypeExt->Death_WithMaster.Get()
		|| pTypeExt->Slaved_ReturnTo == SlaveReturnTo::Suicide)) {

		const KillMethod nMethod = pTypeExt->Death_Method.Get();

		if (nMethod != KillMethod::None)
			TechnoExt::KillSelf(pInf, nMethod, pTypeExt->AutoDeath_VanishAnimation);
	}

	return !pThis->IsAlive;
}

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
int TechnoExt::PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno,
 AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback, bool allowAAFallback)
{
	CellClass* targetCell = nullptr;

	// Ignore target cell for airborne target technos.
	if (pTarget)
	{
		if (const auto pCell = abstract_cast<CellClass*>(pTarget))
			targetCell = pCell;
		else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
		{
			// Ignore target cell for technos that are in air.
			if ((pTargetTechno && !pTargetTechno->IsInAir()) || pObject != pTargetTechno)
				targetCell = pObject->GetCell();
		}
	}

	const auto pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
	const auto pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);

	if (!pWeaponStructOne && !pWeaponStructTwo)
		return -1;
	else if (!pWeaponStructTwo)
		return weaponIndexOne;
	else if (!pWeaponStructOne)
		return weaponIndexTwo;

	const auto pWeaponOne = pWeaponStructOne->WeaponType;
	const auto pWeaponTwo = pWeaponStructTwo->WeaponType;

	if (const auto pSecondExt = WeaponTypeExt::ExtMap.Find(pWeaponTwo))
	{
		if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pSecondExt->CanTarget, true)) ||
			(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
		{
			return weaponIndexOne;
		}
		else if (const auto pFirstExt = WeaponTypeExt::ExtMap.Find(pWeaponOne))
		{
			const bool secondaryIsAA = pTargetTechno && pTargetTechno->IsInAir() && pWeaponTwo->Projectile->AA;

			if (!allowFallback && (!allowAAFallback || !secondaryIsAA) && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1))
				return weaponIndexOne;

			if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pFirstExt->CanTarget, true)) ||
				(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget) ||
					!EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner))))
			{
				return weaponIndexTwo;
			}
		}
	}

	const auto pType = pThis->GetTechnoType();

	// Handle special case with NavalTargeting / LandTargeting.
	if (!pTargetTechno &&
		(pType->NavalTargeting == NavalTargetingType::Naval_primary || pType->LandTargeting == LandTargetingType::Land_secondary))
	{
		if (targetCell)
		{
			if ((targetCell->LandType != LandType::Water && targetCell->LandType != LandType::Beach) || targetCell->ContainsBridge())
				return weaponIndexTwo;
		}
	}

	return -1;
}

bool TechnoExt::IsInWarfactory(TechnoClass* pThis, bool bCheckNaval)
{
	if (!Is_Unit(pThis) || pThis->IsInAir())
		return false;

	//if (pThis->IsTethered)
	//	return true;

	auto const pContact = pThis->GetNthLink();

	if (!pContact)
		return false;

	auto const pCell = pThis->GetCell();

	if (!pCell)
		return false;

	auto const pBld = pCell->GetBuilding();

	if (!pBld || pBld != pContact)
		return false;

	if (pBld->Type->WeaponsFactory || (bCheckNaval && pBld->Type->Naval)) {
		return true;
	}

	return false;
}

CoordStruct TechnoExt::GetPutLocation(CoordStruct current, int distance)
{
	// this whole thing does not at all account for cells which are completely occupied.
	const auto& tmpCoords = CellSpread::AdjacentCell[ScenarioClass::Instance->Random.RandomFromMax(7)];

	current.X += tmpCoords.X * distance;
	current.Y += tmpCoords.Y * distance;

	const auto tmpCell = MapClass::Instance->TryGetCellAt(current);

	if(!tmpCell) {
		return CoordStruct::Empty;
	}

	return tmpCell->FindInfantrySubposition(current, false, false, false, current.Z);
}

bool TechnoExt::EjectSurvivor(FootClass* Survivor, CoordStruct loc, bool Select)
{
	const CellClass* pCell = MapClass::Instance->TryGetCellAt(loc);

	if (const auto pBld = pCell->GetBuilding())
	{
		if (!Is_Passable(pBld->Type))
			return false;

		if (Is_FirestromWall(pBld->Type) &&
			pBld->Owner && 
			pBld->Owner->FirestormActive)
			return false;
	}

	Survivor->OnBridge = pCell->ContainsBridge();

	const int floorZ = pCell->GetCoordsWithBridge().Z;
	const bool chuted = (loc.Z - floorZ > 2 * Unsorted::LevelHeight);

	if (chuted)
	{
		// HouseClass::CreateParadrop does this when building passengers for a paradrop... it might be a wise thing to mimic!
		Survivor->Limbo();

		if (!Survivor->SpawnParachuted(loc) || pCell->GetBuilding())
		{
			return false;
		}
	}
	else
	{
		loc.Z = floorZ;
		if (!Survivor->Unlimbo(loc, static_cast<DirType>(ScenarioClass::Instance->Random.RandomFromMax(7))))
		{
			return false;
		}
	}

	Survivor->Transporter = nullptr;
	Survivor->LastMapCoords = pCell->MapCoords;

	// don't ask, don't tell
	if (chuted)
	{
		const bool scat = Survivor->OnBridge;
		const auto occupation = scat ? pCell->AltOccupationFlags : pCell->OccupationFlags;

		if ((occupation & 0x1C) == 0x1C)
		{
			pCell->ScatterContent(CoordStruct::Empty, true, true, scat);
		}
	}
	else
	{
		Survivor->Scatter(CoordStruct::Empty, true, false);
		Survivor->QueueMission(!Survivor->Owner->IsControlledByHuman() ?  Mission::Hunt : Mission::Guard, 0);
	}

	Survivor->ShouldEnterOccupiable = false;
	Survivor->ShouldGarrisonStructure = false;

	if (Select) {
		Survivor->Select();
	}

	return true;
}

bool TechnoExt::EjectRandomly(FootClass* pEjectee, CoordStruct const& location, int distance, bool select)
{
	CoordStruct destLoc = GetPutLocation(location, distance);

	if(destLoc == CoordStruct::Empty || !MapClass::Instance->IsWithinUsableArea(destLoc))
		return false;

	return EjectSurvivor(pEjectee, destLoc, select);
}

bool TechnoExt::ReplaceArmor(REGISTERS* R, TechnoClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto const pTargetTechnoExt = TechnoExt::ExtMap.Find(pTarget);

	if (!pTargetTechnoExt)
		return false;

	auto const pShieldData = pTargetTechnoExt->Shield.get();

	if (!pShieldData)
		return false;

	if (pShieldData->IsActive())
	{
		if (pShieldData->CanBePenetrated(pWeapon->Warhead))
			return false;

		R->EAX(pShieldData->GetType()->Armor.Get());
		return true;
	}

	return false;
}

void TechnoExt::ResetDelayFireAnim(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->DelayedFire_Anim)
	{
		pExt->DelayedFire_Anim.reset(nullptr);
	}

	pExt->DelayedFire_Anim_LoopCount = 1;

}

int TechnoExt::GetInitialStrength(TechnoTypeClass* pType, int nHP)
{
	return TechnoTypeExt::ExtMap.Find(pType)->InitialStrength.Get(nHP);
}

// Feature for common usage : TechnoType conversion -- Trsdy
// BTW, who said it was merely a Type pointer replacement and he could make a better one than Ares?
bool TechnoExt::ConvertToType(FootClass* pThis, TechnoTypeClass* pToType)
{
	// In case not using Ares 3.0. Only update necessary vanilla properties
	AbstractType rtti;
	TechnoTypeClass** nowTypePtr;

	// Different types prohibited
	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<InfantryClass*>(pThis)->Type));
		rtti = AbstractType::InfantryType;
		break;
	case AbstractType::Unit:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<UnitClass*>(pThis)->Type));
		rtti = AbstractType::UnitType;
		break;
	case AbstractType::Aircraft:
		nowTypePtr = reinterpret_cast<TechnoTypeClass**>(&(static_cast<AircraftClass*>(pThis)->Type));
		rtti = AbstractType::AircraftType;
		break;
	default:
		Debug::Log("%s is not FootClass, conversion not allowed\n", pToType->get_ID());
		return false;
	}

	if (pToType->WhatAmI() != rtti)
	{
		Debug::Log("Incompatible types between %s and %s\n", pThis->get_ID(), pToType->get_ID());
		return false;
	}

	// Detach CLEG targeting
	auto tempUsing = pThis->TemporalImUsing;
	if (tempUsing && tempUsing->Target)
		tempUsing->Detach();

	HouseClass* const pOwner = pThis->Owner;

	// Remove tracking of old techno
	if (!pThis->InLimbo)
		pOwner->RegisterLoss(pThis, false);
	pOwner->RemoveTracking(pThis);

	int oldHealth = pThis->Health;

	// Generic type-conversion
	TechnoTypeClass* prevType = *nowTypePtr;
	*nowTypePtr = pToType;

	// Readjust health according to percentage
	pThis->AdjustStrength((double)(oldHealth) / (double)prevType->Strength);
	pThis->EstimatedHealth = pThis->Health;

	// Add tracking of new techno
	pOwner->AddTracking(pThis);
	if (!pThis->InLimbo)
		pOwner->RegisterGain(pThis, false);
	pOwner->RecheckTechTree = true;

	// Update Ares AttachEffects -- skipped
	// Ares RecalculateStats -- skipped

	// Adjust ammo
	pThis->Ammo = MinImpl(pThis->Ammo, pToType->Ammo);
	// Ares ResetSpotlights -- skipped

	// Adjust ROT
	if (rtti == AbstractType::AircraftType)
		pThis->SecondaryFacing.Set_ROT(pToType->ROT);
	else
		pThis->PrimaryFacing.Set_ROT(pToType->ROT);
	// Adjust Ares TurretROT -- skipped
	//  pThis->SecondaryFacing.SetROT(TechnoTypeExt::ExtMap.Find(pToType)->TurretROT.Get(pToType->ROT));

	// Locomotor change, referenced from Ares 0.A's abduction code, not sure if correct, untested
	CLSID nowLocoID;
	ILocomotion* iloco = pThis->Locomotor.GetInterfacePtr();
	const auto& toLoco = pToType->Locomotor;
	if ((SUCCEEDED(static_cast<LocomotionClass*>(iloco)->GetClassID(&nowLocoID)) && nowLocoID != toLoco))
	{
		// because we are throwing away the locomotor in a split second, piggybacking
		// has to be stopped. otherwise the object might remain in a weird state.
		while (LocomotionClass::End_Piggyback(pThis->Locomotor));
		// throw away the current locomotor and instantiate
		// a new one of the default type for this unit.
		if (auto newLoco = LocomotionClass::CreateInstance(toLoco))
		{
			newLoco->Link_To_Object(pThis);
			pThis->Locomotor = std::move(newLoco);
		}
	}

	// TODO : Jumpjet locomotor special treatement, some brainfart, must be uncorrect, HELP ME!
	const auto& jjLoco = CLSIDs::Jumpjet();
	if (pToType->BalloonHover && pToType->DeployToLand && prevType->Locomotor != jjLoco && toLoco == jjLoco)
		pThis->Locomotor->Move_To(pThis->Location);

	return true;
}

bool TechnoExt::IsEligibleSize(TechnoClass* pThis, TechnoClass* pPassanger)
{
	auto pThisType = pThis->GetTechnoType();
	auto const pThisTypeExt = TechnoTypeExt::ExtMap.Find(pThisType);
	auto pThatType = pPassanger->GetTechnoType();

	if (pThatType->Size > pThisType->SizeLimit)
		return false;

	if (pThisTypeExt->Passengers_BySize.Get())
	{
		if (pThatType->Size > (pThisType->Passengers - pThis->Passengers.GetTotalSize()))
			return false;
	}
	else if (pThis->Passengers.NumPassengers >= pThisType->Passengers)
	{
		return false;
	}

	return true;
}

// =============================
// load / save

template <typename T>
void TechnoExt::ExtData::Serialize(T& Stm)
{
	//Debug::Log("Processing Element From TechnoExt ! \n");

	Stm
		.Process(this->Initialized)
		.Process(this->Type)
		.Process(this->AbsType)
		.Process(this->Shield)
		.Process(this->LaserTrails)
		.Process(this->ReceiveDamage)
		.Process(this->PassengerDeletionTimer)
		.Process(this->CurrentShieldType)
		.Process(this->LastWarpDistance)
		.Process(this->Death_Countdown)
		.Process(this->MindControlRingAnimType)
		.Process(this->DamageNumberOffset)
		.Process(this->CurrentLaserWeaponIndex)
		.Process(this->OriginalPassengerOwner)
		.Process(this->DelayedFire_Anim)
		.Process(this->DelayedFire_Anim_LoopCount)
		.Process(this->DelayedFire_DurationTimer)
		.Process(this->IsInTunnel)
		.Process(this->DeployFireTimer)
		.Process(this->DisableWeaponTimer)
		.Process(this->RevengeWeapons)
		.Process(this->IsDriverKilled)
		.Process(this->GattlingDmageDelay)
		.Process(this->GattlingDmageSound)
		.Process(this->AircraftOpentoppedInitEd)
		.Process(this->FireSelf_Count)
		.Process(this->EngineerCaptureDelay)
		.Process(this->FlhChanged)
		.Process(this->IsMissisleSpawn)
		.Process(this->LastAttacker)
		.Process(this->Attempt)
		.Process(this->ReceiveDamageMultiplier)
		.Process(this->SkipLowDamageCheck)
		.Process(this->AttachedAnim)
		.Process(this->KillActionCalled)
		.Process(this->WarpedOutDelay)
		.Process(this->AltOccupation)
		.Process(this->MyOriginalTemporal)

		.Process(this->aircraftPutOffsetFlag)
		.Process(this->aircraftPutOffset)
		.Process(this->VirtualUnit)
		.Process(this->IsMissileHoming)
		.Process(this->SkipVoice)
		.Process(this->HomingTargetLocation)
		.Process(this->ExtraWeaponTimers)
		.Process(this->Trails)
		.Process(this->MyGiftBox)
		.Process(this->PaintBallState)
		.Process(this->DamageSelfState)
		.Process(this->CurrentWeaponIdx)
		.Process(this->CurrentArmor)
		.Process(this->SupressEVALost)
		.Process(this->MyFighterData)
		.Process(this->SelfHealing_CombatDelay)
		.Process(this->PayloadCreated)
		.Process(this->LinkedSW)
		.Process(this->SuperTarget)
#ifdef ENABLE_HOMING_MISSILE
		.Process(this->MissileTargetTracker)
#endif
		;
	//should put this inside techo ext , ffs

	this->MyWeaponManager.Serialize(Stm);
	this->MyDriveData.Serialize(Stm);
	this->MyDiveData.Serialize(Stm);
	//this->MyJJData.Serialize(Stm);
	this->MySpawnSuport.Serialize(Stm);
}

bool TechnoExt::ExtData::InvalidateIgnorable(void* ptr) const
{
	switch (GetVtableAddr(ptr))
	{
	case HouseClass::vtable:
	case BuildingClass::vtable:
	case AircraftClass::vtable:
	case UnitClass::vtable:
	case InfantryClass::vtable:
	case TemporalClass::vtable:
	case SuperClass::vtable:
		return false;
	}

	return true;
}

void TechnoExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved)
{

	MyWeaponManager.InvalidatePointer(ptr, bRemoved);

	AnnounceInvalidPointer(LinkedSW, ptr);
	AnnounceInvalidPointer(OriginalPassengerOwner, ptr);
	AnnounceInvalidPointer(LastAttacker, ptr);
#ifdef ENABLE_HOMING_MISSILE
	if (MissileTargetTracker)
		MissileTargetTracker->InvalidatePointer(ptr, bRemoved);
#endif
}

TechnoExt::ExtContainer TechnoExt::ExtMap;
TechnoExt::ExtContainer::ExtContainer() : Container("TechnoClass") { }
TechnoExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x6F3260, TechnoClass_CTOR, 0x5)
{
	GET(TechnoClass*, pItem, ESI);
	TechnoExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6F4500, TechnoClass_DTOR, 0x5)
{
	GET(TechnoClass*, pItem, ECX);
	TechnoExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x70C250, TechnoClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x70BF50, TechnoClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	TechnoExt::ExtMap.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x70C249, TechnoClass_Load_Suffix, 0x5)
{
	TechnoExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x70C264, TechnoClass_Save_Suffix, 0x5)
{
	TechnoExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x70783B, TechnoClass_Detach, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(void*, target, EBP);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	//if (!Is_Techno(pThis))
	//	Debug::Log("TechnoClass_Detach Called with gargabage ptr[%x] !\n", pThis);

	TechnoExt::ExtMap.InvalidatePointerFor(pThis, target, all);

	return pThis->BeingManipulatedBy == target ? 0x707843 : 0x707849;
}

DEFINE_HOOK(0x710443, TechnoClass_AnimPointerExpired_PhobosAdd, 6)
{
	GET(AnimClass*, pAnim, EAX);
	GET(TechnoClass*, pThis, ECX);

	if (auto pExt = TechnoExt::ExtMap.Find(pThis)) {
		if (auto& pShield = pExt->Shield)
			pShield->InvalidatePointer(pAnim, false);
	}

	return 0x0;
}