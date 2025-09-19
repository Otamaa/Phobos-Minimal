#include "Body.h"
#include <Phobos.h>
#include <Helpers/Macro.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>
#include <HouseTypeClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <UnitClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Locomotor/Cast.h>
#include <Locomotor/JumpjetLocomotionClass.h>

#include <AircraftTrackerClass.h>

// AnimType Class is readed before Unit and weapon
// so it is safe to `allocate` them before

bool AnimTypeExtData::LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
{
	const char* pID = this->Name();

	if (parseFailAddr)
		return false;

	INI_EX exINI(pINI);
	this->Palette.Read(exINI, pID, "CustomPalette");

	Valueable<TechnoTypeClass*> createUnit { nullptr };
	createUnit.Read(exINI, pID, "CreateUnit");

	if(createUnit && !this->CreateUnitType)
		this->CreateUnitType = std::make_unique<CreateUnitTypeClass>();

	if (this->CreateUnitType)
	{
		if (!createUnit)
			this->CreateUnitType.reset();

		this->CreateUnitType->LoadFromINI(pINI, pID);
	}

	this->XDrawOffset.Read(exINI, pID, "XDrawOffset");
	this->YDrawOffset_ApplyBracketHeight.Read(exINI, pID, "YDrawOffset.ApplyBracketHeight");
	this->YDrawOffset_InvertBracketShift.Read(exINI, pID, "YDrawOffset.InvertBracketShift");
	this->YDrawOffset_BracketAdjust.Read(exINI, pID, "YDrawOffset.BracketAdjust");
	this->YDrawOffset_BracketAdjust_Buildings.Read(exINI, pID, "YDrawOffset.BracketAdjust.Buildings");
	this->HideIfNoOre_Threshold.Read(exINI, pID, "HideIfNoOre.Threshold");
	this->Layer_UseObjectLayer.Read(exINI, pID, "Layer.UseObjectLayer");

	Nullable<bool> UseCenterCoordsIfAttached {};

	UseCenterCoordsIfAttached.Read(exINI, pID, "UseCenterCoordsIfAttached");

	auto att = this->AttachedAnimPosition.Get();
	if (UseCenterCoordsIfAttached.isset() && UseCenterCoordsIfAttached.Get()){
		att |= AttachedAnimPosition::Center;
		this->AttachedAnimPosition = att;
	}

	this->AttachedAnimPosition.Read(exINI, pID, "AttachedAnimPosition");

	this->Weapon.Read(exINI, pID, "Weapon", true);
	this->WeaponToCarry.Read(exINI, pID, "WeaponToCarry", true);

	if (auto& pWeapon = this->Weapon) {
		if (!pWeapon->Projectile || !pWeapon->Warhead)
			pWeapon = nullptr;
	}

	if (auto& pWeaponC = this->WeaponToCarry)
	{
		if (!pWeaponC->Projectile || !pWeaponC->Warhead)
			pWeaponC = nullptr;
	}

	this->Damage_Delay.Read(exINI, pID, "Damage.Delay");
	this->Damage_DealtByInvoker.Read(exINI, pID, "Damage.DealtByInvoker");
	this->Damage_ApplyOnce.Read(exINI, pID, "Damage.ApplyOnce");
	this->Damage_ConsiderOwnerVeterancy.Read(exINI, pID, "Damage.ConsiderOwnerVeterancyBonus");
	this->Damage_ConsiderOwnerVeterancy.Read(exINI, pID, "Damage.ApplyFirepowerMult");
	this->Warhead_Detonate.Read(exINI, pID, "Warhead.Detonate");
	this->Damage_TargetFlag.Read(exINI, pID, "Damage.TargetFlag");

	bool Damage_TargetInvoker;
	if (detail::read(Damage_TargetInvoker, exINI, pID, "Damage.TargetInvoker") && Damage_TargetInvoker)
		this->Damage_TargetFlag = DamageDelayTargetFlag::Invoker;

	this->MakeInfantryOwner.Read(exINI, pID, "MakeInfantryOwner");

	if (exINI.ReadString(pID, "MakeInfantry.Mission"))
	{
		auto result = MissionClass::GetMissionById(exINI.value());
		if (result == Mission::None && IS_SAME_STR_(exINI.c_str(), "scatter"))
		{
			this->MakeInfantry_Scatter = true;
		}
		else if (result != Mission::None)
		{
			this->MakeInfantry_Scatter = false;
			this->MakeInfantry_Mission = result;
		}
	}

	if (exINI.ReadString(pID, "MakeInfantry.Mission.AI"))
	{
		auto result = MissionClass::GetMissionById(exINI.value());
		if (result == Mission::None && IS_SAME_STR_(exINI.c_str(), "scatter"))
		{
			this->MakeInfantry_AI_Scatter = true;
		}
		else if (result != Mission::None)
		{
			this->MakeInfantry_AI_Scatter = false;
			this->MakeInfantry_AI_Mission = result;
		}
	}

	this->DetachedReport.Read(exINI, pID, "DetachedReport");
#pragma region Otamaa

	this->ParticleRangeMin.Read(exINI, pID, "SpawnsParticle.RangeMinimum");
	this->ParticleRangeMax.Read(exINI, pID, "SpawnsParticle.RangeMaximum");
	this->ParticleChance.Read(exINI, pID, "SpawnsParticle.Chance");
	this->SpawnParticleModeUseAresCode.Read(exINI, pID, "SpawnsParticle.UseAresCode");

	this->SplashList.Read(exINI, pID, "SplashAnims");
	this->SplashIndexRandom.Read(exINI, pID, "SplashAnims.PickRandom");

	this->WakeAnim.Read(exINI, pID, "WakeAnim");
	this->ExplodeOnWater.Read(exINI, pID, "ExplodeOnWater");

	//set allocate to true to shut off debug warning
	this->SpawnsMultiple.Read(exINI, pID, "SpawnsMultiple", true);
	this->SpawnsMultiple_Random.Read(exINI, pID, "SpawnsMultiple.Random");

	this->SpawnsMultiple_amouts.clear();

	if (!this->SpawnsMultiple.empty())
	{
		auto const nBaseSize = (int)this->SpawnsMultiple.size();
		this->SpawnsMultiple_amouts.resize(nBaseSize, 1);

		if (exINI.ReadString(pID, "SpawnsMultiple.Amount") > 0)
		{
			int nCount = 0;
			char* context = nullptr;
			for (char* cur = strtok_s(exINI.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				int buffer { 1 };
				if (Parser<int>::TryParse(cur, &buffer))
					this->SpawnsMultiple_amouts[nCount] = buffer;

				if (++nCount >= nBaseSize)
					break;
			}
		}
	}

	this->CraterDecreaseTiberiumAmount.Read(exINI, pID, "Crater.DecreaseTiberiumAmount");
	this->CraterChance.Read(exINI, pID, "Crater.Chance");
	this->SpawnCrater.Read(exINI, pID, "Crater.Spawn");
	this->ScorchChance.Read(exINI, pID, "Scorch.Chance");
	this->SpecialDraw.Read(exINI, pID, "SpecialDraw");
	this->NoOwner.Read(exINI, pID, "NoOwner");
	this->Spawns_Delay.Read(exINI, pID, "Spawns.InitialDelay");

	this->ConcurrentChance.Read(exINI, pID, "ConcurrentChance");
	this->ConcurrentAnim.Read(exINI, pID, "ConcurrentAnim");
	this->AttachedSystem.Read(exINI, pID, "AttachedSystem", true);

	this->AltPalette_ApplyLighting.Read(exINI, pID, "AltPalette.ApplyLighting");

	//Launchs
	LauchSWData::ReadVector(this->Launchs, exINI , pID, Phobos::Otamaa::CompatibilityMode);

	this->RemapAnim.Read(exINI, pID, "RemapAnim");
	this->ExtraShadow.Read(exINI, pID, "ExtraShadow");

	this->AdditionalHeight.Read(exINI, pID, "AdditionalHeight");
	this->AltReport.Read(exINI, pID, "AltReport");

	//this->SpawnsData.Read(exINI, pID);

	this->VisibleTo.Read(exINI, pID, "VisibleTo");
	this->VisibleTo_ConsiderInvokerAsOwner.Read(exINI, pID, "VisibleTo.ConsiderInvokerAsOwner");
	this->RestrictVisibilityIfCloaked.Read(exINI, pID, "RestrictVisibilityIfCloaked");
	this->DetachOnCloak.Read(exINI, pID, "DetachOnCloak");
	this->Translucency_Cloaked.Read(exINI, pID, "Translucency.Cloaked");

	if (This()->Translucent) {
		this->Translucent_Keyframes.Read(exINI, pID, "Translucent.%s", This()->End);
	}

#pragma endregion

	this->ConstrainFireAnimsToCellSpots.Read(exINI, pID, "ConstrainFireAnimsToCellSpots");
	this->FireAnimDisallowedLandTypes.Read(exINI, pID, "FireAnimDisallowedLandTypes");
	this->AttachFireAnimsToParent.Read(exINI, pID, "AttachFireAnimsToParent");
	this->SmallFireCount.Read(exINI, pID, "SmallFireCount");
	this->SmallFireAnims.Read(exINI, pID, "SmallFireAnims");
	this->SmallFireChances.Read(exINI, pID, "SmallFireChances");
	this->SmallFireDistances.Read(exINI, pID, "SmallFireDistances");
	this->LargeFireCount.Read(exINI, pID, "LargeFireCount");
	this->LargeFireAnims.Read(exINI, pID, "LargeFireAnims");
	this->LargeFireChances.Read(exINI, pID, "LargeFireChances");
	this->LargeFireDistances.Read(exINI, pID, "LargeFireDistances");

	this->Damaging_UseSeparateState.Read(exINI, pID, "Damaging.UseSeparateState");
	this->Damaging_Rate.Read(exINI, pID, "Damaging.Rate");

	return true;
}

void AnimTypeExtData::CreateUnit_MarkCell(AnimClass* pThis)
{
	auto pExt = ((FakeAnimClass*)pThis)->_GetExtData();
	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);

	if (pExt->AllowCreateUnit || !pTypeExt->CreateUnitType)
		return;

	auto& c_type = pTypeExt->CreateUnitType;
	const auto pUnit = c_type->Type;

	{
		auto Location = pThis->GetCoords();

		if (!MapClass::Instance->IsWithinUsableArea(Location))
			return;

		auto pCell = pThis->GetCell();

		bool allowBridges = pExt->WasOnBridge || GroundType::GetCost(LandType::Clear, pUnit->SpeedType) > 0.0;
		bool isBridge = allowBridges && pCell->ContainsBridge();

		if (c_type->ConsiderPathfinding
			&& (!pCell || !pCell->IsClearToMove(pUnit->SpeedType, false, false, ZoneType::None, pUnit->MovementZone, -1, isBridge)))
		{
			const auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(Location),
				pUnit->SpeedType, ZoneType::None, pUnit->MovementZone, isBridge, 1, 1, true,
				false, false, isBridge, CellStruct::Empty, false, false);

			pCell = MapClass::Instance->TryGetCellAt(nCell);
			if (!pCell)
				return;

			Location = pCell->GetCoords();
		}

		isBridge = allowBridges && pCell->ContainsBridge();
		int bridgeZ = isBridge ? Unsorted::BridgeHeight : 0;

		const int z = c_type->AlwaysSpawnOnGround ? INT32_MIN : Location.Z;
		const auto nCellHeight = MapClass::Instance->GetCellFloorHeight(Location);
		Location.Z = MaxImpl(nCellHeight + bridgeZ, z);

		const int baseHeight = pTypeExt->CreateUnit_SpawnHeight != -1 ? pTypeExt->CreateUnit_SpawnHeight : Location.Z;
		const int zCoord = c_type->AlwaysSpawnOnGround ? INT32_MIN : baseHeight;
		Location.Z = MaxImpl(MapClass::Instance->GetCellFloorHeight(Location) + bridgeZ, zCoord);

		//const auto pCellAfter = MapClass::Instance->GetCellAt(Location);

		if (!MapClass::Instance->IsWithinUsableArea(Location))
			return;

		pExt->AllowCreateUnit = true;
		pThis->MarkAllOccupationBits(Location);
		pExt->CreateUnitLocation = Location;
	}
}

static HouseClass* GetOwnerForSpawned(AnimClass* pThis)
{
	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
	auto& c_type = pTypeExt->CreateUnitType;

	if (!pThis->Owner || pThis->Owner->Defeated)
	{
		if (c_type->RequireOwner)
			return nullptr;

		return HouseExtData::FindFirstCivilianHouse();
	}

	return pThis->Owner;
}

static TechnoClass* CreateFoot(
	TechnoTypeClass* pType,
	CoordStruct location,
	DirType facing,
	std::optional<DirType>& secondaryFacing,
	bool Scatter,
	HouseClass* pOwner,
	bool checkPathfinding,
	bool parachuteIfInAir,
	bool alwaysOnGround,
	Mission mission)
{
	auto const rtti = pType->WhatAmI();

	if (rtti == AbstractType::BuildingType)
		return nullptr;

	HouseClass* decidedOwner = pOwner && !pOwner->Defeated
		? pOwner : HouseClass::FindCivilianSide();

	const auto pCell = MapClass::Instance->GetCellAt(location);
	const auto speedType = rtti != AbstractType::AircraftType ? pType->SpeedType : SpeedType::Wheel;
	//auto const mZone = rtti != AbstractType::AircraftType ? pType->MovementZone : MovementZone::Normal;
	const bool allowBridges = GroundType::Array[static_cast<int>(LandType::Clear)].Cost[static_cast<int>(speedType)] > 0.0;
	const bool isBridge = allowBridges && pCell->ContainsBridge();
	const bool inAir = location.Z >= Unsorted::CellHeight * 2;

	if (auto const pTechno = static_cast<FootClass*>(pType->CreateObject(decidedOwner)))
	{
		bool success = false;
		bool parachuted = false;

		pTechno->OnBridge = isBridge;

		if (rtti != AbstractType::AircraftType && parachuteIfInAir && !alwaysOnGround && inAir) {
			parachuted = true;
			success = pTechno->SpawnParachuted(location);
		} else if (!pCell->GetBuilding() || !checkPathfinding) {
			++Unsorted::ScenarioInit;
			success = pTechno->Unlimbo(location, facing);
			--Unsorted::ScenarioInit;
		} else {
			success = pTechno->Unlimbo(location, facing);
		}

		if (success)
		{
			if (secondaryFacing.has_value())
				pTechno->SecondaryFacing.Set_Current(DirStruct(*secondaryFacing));

			if (!pTechno->InLimbo)
			{
				if (!alwaysOnGround)
				{
					if (auto const pFlyLoco = locomotion_cast<FlyLocomotionClass*>(pTechno->Locomotor))
					{
						pTechno->SetLocation(location);
						if(pType->Speed != 0) {
							bool airportBound = rtti == AbstractType::AircraftType && static_cast<AircraftTypeClass*>(pType)->AirportBound;
							if (pCell->GetContent() || airportBound)
								pTechno->EnterIdleMode(false, true);
							else
								pFlyLoco->Move_To(pCell->GetCoordsWithBridge());


						} else if (inAir) {
							AircraftTrackerClass::Instance->Add(pTechno);
						}
					}
					else if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pTechno->Locomotor))
					{
						pJJLoco->Facing.Set_Current(DirStruct(facing));
						if(pType->Speed != 0) {
							if (pType->BalloonHover)
							{
								// Makes the jumpjet think it is hovering without actually moving.
								pJJLoco->NextState = JumpjetLocomotionClass::State::Hovering;
								pJJLoco->IsMoving = true;
								pJJLoco->HeadToCoord = location;
								pJJLoco->Height = pType->JumpJetData.Height;

								if (!inAir)
									AircraftTrackerClass::Instance->Add(pTechno);

							}
							else if (inAir)
							{
								// Order non-BalloonHover jumpjets to land.
								pJJLoco->Move_To(location);
							}
						} else if (inAir) {
							AircraftTrackerClass::Instance->Add(pTechno);
						}
					}
					else if (inAir && !parachuted)
					{
						pTechno->IsFallingDown = true;
					}
				}

				if (!Scatter)
					pTechno->QueueMission(mission, false);
				else
					pTechno->Scatter(CoordStruct::Empty, false, false);

			}

			if (!decidedOwner->Type->MultiplayPassive)
				decidedOwner->RecheckTechTree = true;

			return pTechno;
		}
		else {
			TechnoExtData::HandleRemove(pTechno);
		}
	}

	return nullptr;
}

void AnimTypeExtData::CreateUnit_Spawn(AnimClass* pThis)
{
	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
	const auto pAnimExt = ((FakeAnimClass*)pThis)->_GetExtData();

	//location is not marked , so dont !
	if (!pAnimExt->AllowCreateUnit)
		return;

	pThis->UnmarkAllOccupationBits(pAnimExt->CreateUnitLocation);

	HouseClass* decidedOwner = GetOwnerForSpawned(pThis);
	auto& c_type = pTypeExt->CreateUnitType;
	const auto pTechnoType = c_type->Type;

	{
		const auto Is_AI = !decidedOwner->IsControlledByHuman();
		DirType primaryFacing = c_type->Facing;
		if(c_type->InheritDeathFacings && pAnimExt->DeathUnitFacing.has_value())
			primaryFacing = pAnimExt->DeathUnitFacing;
		else if(c_type->RandomFacing)
			primaryFacing = ScenarioClass::Instance->Random.RandomRangedSpecific<DirType>(DirType::Min, DirType::Max);

		std::optional<DirType> secondaryFacing {};
		bool Scatter = false;
		Mission missionAI = Mission::None;

		if (pTechnoType->WhatAmI() == AbstractType::UnitType &&
			pTechnoType->Turret &&
			pAnimExt->DeathUnitTurretFacing.has_value() &&
			c_type->InheritTurretFacings)
		{
			secondaryFacing = pAnimExt->DeathUnitTurretFacing.get().GetDir();
		}

		if (!pTypeExt->ScatterCreateUnit(Is_AI))
			missionAI = pTypeExt->GetCreateUnitMission(Is_AI);
		else
			Scatter = true;

		if (CreateFoot(pTechnoType,
			pAnimExt->CreateUnitLocation,
			primaryFacing,
			secondaryFacing,
			Scatter,
			decidedOwner,
			c_type->ConsiderPathfinding,
			c_type->SpawnParachutedInAir,
			c_type->AlwaysSpawnOnGround,
			missionAI
		)) {
			if (auto pSpawnAnim = c_type->SpawnAnim) {
				auto pCreateUnitAnim = GameCreate<AnimClass>(pSpawnAnim, pAnimExt->CreateUnitLocation);
				pCreateUnitAnim->Owner = decidedOwner;
				((FakeAnimClass*)pCreateUnitAnim)->_GetExtData()->Invoker = AnimExtData::GetTechnoInvoker(pThis);
			}
		}
	}
}

void AnimTypeExtData::ValidateData()
{

	if (this->CreateUnitType && this->CreateUnitType->Type->Strength == 0)
	{
		Debug::LogInfo("AnimType[{}] With[{}] CreateUnit strength 0 !", Name(), this->CreateUnitType->Type->ID);
		this->CreateUnitType.reset();
		Debug::RegisterParserError();
	}
}

#include <Ext/WarheadType/Body.h>

void AnimTypeExtData::ProcessDestroyAnims(FootClass* pThis, TechnoClass* pKiller, WarheadTypeClass* pWH)
{
	const auto location = pThis->GetCoords();

	if (!MapClass::Instance->IsWithinUsableArea(location))
		return;

	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);
	AnimTypeClass** begin = pType->DestroyAnim.begin();
	int count = pType->DestroyAnim.Count;

	if (pWH)
	{
		for (auto walk = pTypeExt->DestroyAnimSpecific.begin();
			 walk != pTypeExt->DestroyAnimSpecific.end();
			 ++walk
		)
		{
			if (walk->first == pWH)
			{
				begin = walk->second.data();
				count = (int)walk->second.size();
				break;
			}
		}

	}

	if (!count)
		return;

	const DirType facing = (DirType)pThis->PrimaryFacing.Current().GetFacing<256>();

	int idxAnim = 0;
	GeneralUtils::GetRandomAnimVal(idxAnim, count, (short)facing, pTypeExt->DestroyAnim_Random.Get());
	AnimTypeClass* pAnimType = begin[idxAnim];

	if (!pAnimType)
		return;

	auto pAnim = GameCreate<AnimClass>(pAnimType, location);
	const auto pAnimTypeExt = AnimTypeExtContainer::Instance.Find(pAnimType);
	auto pAnimExt = ((FakeAnimClass*)pAnim)->_GetExtData();
	HouseClass* const pInvoker = pKiller ? pKiller->Owner : nullptr;
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, pThis, true, false);

	if(auto& c_type = pAnimTypeExt->CreateUnitType){
		if (c_type->InheritDeathFacings.Get())
			pAnimExt->DeathUnitFacing = facing;

		if (c_type->InheritTurretFacings.Get())
		{
			if (pThis->HasTurret())
			{
				pAnimExt->DeathUnitTurretFacing = pThis->SecondaryFacing.Current();
			}
		}
	}

	pAnimExt->WasOnBridge = pThis->OnBridge;
}

void AnimTypeExtData::ValidateSpalshAnims()
{
	AnimTypeClass* pWake = nullptr;
	if (This()->IsMeteor)
		pWake = WakeAnim.Get(RulesClass::Instance->Wake);

	//what if the anim type loaded twice ?
	if (SplashList.empty())
	{
		SplashList.push_back(pWake);
	}
}

template <typename T>
void AnimTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Palette)
		.Process(this->CreateUnitType)
		.Process(this->XDrawOffset)
		.Process(this->YDrawOffset_ApplyBracketHeight)
		.Process(this->YDrawOffset_InvertBracketShift)
		.Process(this->YDrawOffset_BracketAdjust)
		.Process(this->YDrawOffset_BracketAdjust_Buildings)
		.Process(this->HideIfNoOre_Threshold)
		.Process(this->Layer_UseObjectLayer)
		.Process(this->AttachedAnimPosition)
		.Process(this->Weapon)
		.Process(this->WeaponToCarry)
		.Process(this->Damage_Delay)
		.Process(this->Damage_DealtByInvoker)
		.Process(this->Damage_ApplyOnce)
		.Process(this->Damage_ConsiderOwnerVeterancy)
		.Process(this->Damage_TargetFlag)
		.Process(this->MakeInfantry_Mission)
		.Process(this->MakeInfantry_AI_Mission)
		.Process(this->Warhead_Detonate)

		.Process(this->SplashList)
		.Process(this->SplashIndexRandom)
		.Process(this->WakeAnim)
		.Process(this->ExplodeOnWater)

		.Process(this->SpawnsMultiple)
		.Process(this->SpawnsMultiple_Random)
		.Process(this->SpawnsMultiple_amouts)

		.Process(this->ParticleRangeMin)
		.Process(this->ParticleRangeMax)
		.Process(this->ParticleChance)
		.Process(this->SpawnParticleModeUseAresCode)
		.Process(this->Launchs)

		.Process(this->CraterDecreaseTiberiumAmount)
		.Process(this->CraterChance)
		.Process(this->SpawnCrater)
		.Process(this->ScorchChance)
		.Process(this->SpecialDraw)
		.Process(this->NoOwner)
		.Process(this->Spawns_Delay)

		.Process(this->ConcurrentChance)
		.Process(this->ConcurrentAnim)
		.Process(this->MakeInfantryOwner)
		.Process(this->AttachedSystem)
		.Process(this->IsInviso)
		.Process(this->RemapAnim)
		//.Process(SpawnerDatas)

		.Process(this->AltPalette_ApplyLighting)
		.Process(this->ExtraShadow)
		.Process(this->DetachedReport)

		.Process(this->AdditionalHeight)
		.Process(this->AltReport)
		//.Process(this->SpawnsData)

		.Process(this->MakeInfantry_Scatter)
		.Process(this->MakeInfantry_AI_Scatter)

		.Process(this->VisibleTo)
		.Process(this->VisibleTo_ConsiderInvokerAsOwner)
		.Process(this->RestrictVisibilityIfCloaked)
		.Process(this->DetachOnCloak)
		.Process(this->Translucency_Cloaked)
		.Process(this->Translucent_Keyframes)

		.Process(this->CreateUnit_SpawnHeight)

		.Process(this->ConstrainFireAnimsToCellSpots)
		.Process(this->FireAnimDisallowedLandTypes)
		.Process(this->AttachFireAnimsToParent)
		.Process(this->SmallFireCount)
		.Process(this->SmallFireAnims)
		.Process(this->SmallFireChances)
		.Process(this->SmallFireDistances)
		.Process(this->LargeFireCount)
		.Process(this->LargeFireAnims)
		.Process(this->LargeFireChances)
		.Process(this->LargeFireDistances)
		.Process(this->Damaging_UseSeparateState)
		.Process(this->Damaging_Rate)
		;
}

AnimTypeExtContainer AnimTypeExtContainer::Instance;
std::vector<AnimTypeExtData*> Container<AnimTypeExtData>::Array;

void Container<AnimTypeExtData>::Clear()
{
	Array.clear();
}

bool AnimTypeExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	return LoadGlobalArrayData(Stm);
}

bool AnimTypeExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	return SaveGlobalArrayData(Stm);;
}

ASMJIT_PATCH(0x42784B, AnimTypeClass_CTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, EAX);
	AnimTypeExtContainer::Instance.Allocate(pItem);
	return 0;

}

ASMJIT_PATCH(0x428EA8, AnimTypeClass_SDDTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, ECX);

	AnimTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

bool FakeAnimTypeClass::_ReadFromINI(CCINIClass* pINI)
{
	bool status = this->AnimTypeClass::LoadFromINI(pINI);
	AnimTypeExtContainer::Instance.LoadFromINI(this, pINI, !status);
	return status;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E366C, FakeAnimTypeClass::_ReadFromINI)


HRESULT __stdcall FakeAnimTypeClass::_Load(IStream* pStm)
{
	HRESULT hr = this->AnimTypeClass::Load(pStm);
	if (SUCCEEDED(hr))
		hr = AnimTypeExtContainer::Instance.LoadKey(this, pStm);

	return hr;
}

HRESULT __stdcall FakeAnimTypeClass::_Save(IStream* pStm, BOOL clearDirty)
{
	//temporarely remove it
	auto ext = this->_GetExtData();
	AnimTypeExtContainer::Instance.ClearExtAttribute(this);
	HRESULT hr = this->AnimTypeClass::Save(pStm, clearDirty);
	AnimTypeExtContainer::Instance.SetExtAttribute(this, ext);
	if (SUCCEEDED(hr))
		hr = AnimTypeExtContainer::Instance.SaveKey(this, pStm);

	return hr;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E361C, FakeAnimTypeClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3620, FakeAnimTypeClass::_Save)