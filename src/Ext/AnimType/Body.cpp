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

void AnimTypeExtData::Initialize()
{
	const char* pID = this->AttachedToObject->ID;

	SpecialDraw = IS_SAME_STR_(pID, RING1_NAME);
	IsInviso = IS_SAME_STR_(pID, INVISO_NAME);
}

// AnimType Class is readed before Unit and weapon
// so it is safe to `allocate` them before

void AnimTypeExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	const char* pID = this->AttachedToObject->ID;

	if (parseFailAddr)
		return;

	INI_EX exINI(pINI);
	this->Palette.Read(exINI, pID, "CustomPalette");
	this->CreateUnit.Read(exINI, pID, "CreateUnit", true);
	this->CreateUnit_Facing.Read(exINI, pID, "CreateUnit.Facing");
	this->CreateUnit_InheritDeathFacings.Read(exINI, pID, "CreateUnit.InheritFacings");
	this->CreateUnit_InheritTurretFacings.Read(exINI, pID, "CreateUnit.InheritTurretFacings");
	this->CreateUnit_RemapAnim.Read(exINI, pID, "CreateUnit.RemapAnim");

	if (exINI.ReadString(pID, "CreateUnit.Mission"))
	{
		auto result = MissionClass::GetMissionById(exINI.value());
		if (result == Mission::None && IS_SAME_STR_(exINI.c_str(), "scatter"))
		{
			this->CreateUnit_Scatter = true;
		}
		else if (result != Mission::None)
		{
			this->CreateUnit_Scatter = false;
			this->CreateUnit_Mission = result;
		}
	}

	if (exINI.ReadString(pID, "CreateUnit.Mission.AI"))
	{
		auto result = MissionClass::GetMissionById(exINI.value());
		if (result == Mission::None && IS_SAME_STR_(exINI.c_str(), "scatter"))
		{
			this->CreateUnit_AI_Scatter = true;
		}
		else if (result != Mission::None)
		{
			this->CreateUnit_AI_Scatter = false;
			this->CreateUnit_AI_Mission = result;
		}
	}

	this->CreateUnit_Owner.Read(exINI, pID, "CreateUnit.Owner");
	this->CreateUnit_RandomFacing.Read(exINI, pID, "CreateUnit.RandomFacing");
	this->CreateUnit_ConsiderPathfinding.Read(exINI, pID, "CreateUnit.ConsiderPathfinding");
	this->CreateUnit_SpawnAnim.Read(exINI, pID, "CreateUnit.SpawnAnim");
	this->CreateUnit_AlwaysSpawnOnGround.Read(exINI, pID, "CreateUnit.AlwaysSpawnOnGround");
	this->CreateUnit_KeepOwnerIfDefeated.Read(exINI, pID, "CreateUnit.KeepOwnerIfDefeated");
	this->CreateUnit_SpawnParachutedInAir.Read(exINI, pID, "CreateUnit.SpawnParachutedInAir");

	this->XDrawOffset.Read(exINI, pID, "XDrawOffset");
	this->HideIfNoOre_Threshold.Read(exINI, pID, "HideIfNoOre.Threshold");
	this->Layer_UseObjectLayer.Read(exINI, pID, "Layer.UseObjectLayer");
	this->UseCenterCoordsIfAttached.Read(exINI, pID, "UseCenterCoordsIfAttached");

	this->Weapon.Read(exINI, pID, "Weapon", true);
	this->WeaponToCarry.Read(exINI, pID, "WeaponToCarry", true);
	this->Damage_Delay.Read(exINI, pID, "Damage.Delay");
	this->Damage_DealtByInvoker.Read(exINI, pID, "Damage.DealtByInvoker");
	this->Damage_ApplyOnce.Read(exINI, pID, "Damage.ApplyOnce");
	this->Damage_ConsiderOwnerVeterancy.Read(exINI, pID, "Damage.ConsiderOwnerVeterancyBonus");
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
	this->Launchs.clear();
	for (size_t i = 0; ; ++i)
	{
		//char nBuff[0x30];
		SuperWeaponTypeClass* LaunchWhat_Dummy;
		std::string _buffer = "LaunchSW";
		_buffer += std::to_string(i);
		_buffer += ".Type";

		if (!detail::read(LaunchWhat_Dummy, exINI, pID, _buffer.c_str(), true) || !LaunchWhat_Dummy)
			break;

		this->Launchs.emplace_back().Read(exINI, pID, i, LaunchWhat_Dummy);
	}

	this->RemapAnim.Read(exINI, pID, "RemapAnim");
	this->ExtraShadow.Read(exINI, pID, "ExtraShadow");

	this->AdditionalHeight.Read(exINI, pID, "AdditionalHeight");
	this->AltReport.Read(exINI, pID, "AltReport");

	this->SpawnsData.Read(exINI, pID);

	this->VisibleTo.Read(exINI, pID, "VisibleTo");
	this->VisibleTo_ConsiderInvokerAsOwner.Read(exINI, pID, "VisibleTo.ConsiderInvokerAsOwner");
	this->RestrictVisibilityIfCloaked.Read(exINI, pID, "RestrictVisibilityIfCloaked");
	this->DetachOnCloak.Read(exINI, pID, "DetachOnCloak");
	this->Translucency_Cloaked.Read(exINI, pID, "Translucency.Cloaked");

	if (this->AttachedToObject->Translucent)
	{
		this->Translucent_Keyframes.Read(exINI, pID, "Translucent.%s", this->AttachedToObject->End);
		this->Translucent_Keyframes.InterpolationMode = InterpolationMode::None;
	}

	this->CreateUnit_SpawnHeight.Read(exINI, pID, "CreateUnit.SpawnHeight");
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
}

void AnimTypeExtData::CreateUnit_MarkCell(AnimClass* pThis)
{
	if (!pThis->Type)
		return;

	auto pExt = AnimExtContainer::Instance.Find(pThis);

	if (pExt->AllowCreateUnit)
		return;

	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);

	if (const auto pUnit = pTypeExt->CreateUnit.Get())
	{
		auto Location = pThis->GetCoords();

		if (!MapClass::Instance->IsWithinUsableArea(Location))
			return;

		auto pCell = pThis->GetCell();

		bool allowBridges = pExt->WasOnBridge || GroundType::GetCost(LandType::Clear, pUnit->SpeedType) > 0.0;
		bool isBridge = allowBridges && pCell->ContainsBridge();

		if (pTypeExt->CreateUnit_ConsiderPathfinding
			&& (!pCell || !pCell->IsClearToMove(pUnit->SpeedType, false, false, ZoneType::None, pUnit->MovementZone, -1, isBridge)))
		{
			const auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(Location),
				pUnit->SpeedType, -1, pUnit->MovementZone, isBridge, 1, 1, true,
				false, false, isBridge, CellStruct::Empty, false, false);

			pCell = MapClass::Instance->TryGetCellAt(nCell);
			if (!pCell)
				return;

			Location = pCell->GetCoords();
		}

		isBridge = allowBridges && pCell->ContainsBridge();
		int bridgeZ = isBridge ? Unsorted::BridgeHeight : 0;

		const int z = pTypeExt->CreateUnit_AlwaysSpawnOnGround ? INT32_MIN : Location.Z;
		const auto nCellHeight = MapClass::Instance->GetCellFloorHeight(Location);
		Location.Z = MaxImpl(nCellHeight + bridgeZ, z);

		const int baseHeight = pTypeExt->CreateUnit_SpawnHeight != -1 ? pTypeExt->CreateUnit_SpawnHeight : Location.Z;
		const int zCoord = pTypeExt->CreateUnit_AlwaysSpawnOnGround ? INT32_MIN : baseHeight;
		Location.Z = MaxImpl(MapClass::Instance->GetCellFloorHeight(Location) + bridgeZ, zCoord);

		const auto pCellAfter = MapClass::Instance->GetCellAt(Location);

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
	if (!pThis->Owner || ((!pTypeExt->CreateUnit_KeepOwnerIfDefeated && pThis->Owner->Defeated)))
		return HouseExtData::FindFirstCivilianHouse();

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
	auto const mZone = rtti != AbstractType::AircraftType ? pType->MovementZone : MovementZone::Normal;
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
						bool airportBound = rtti == AbstractType::AircraftType && abstract_cast<AircraftTypeClass*>(pType)->AirportBound;
						if (pCell->GetContent() || airportBound)
							pTechno->EnterIdleMode(false, true);
						else
							pFlyLoco->Move_To(pCell->GetCoordsWithBridge());
					}
					else if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pTechno->Locomotor))
					{
						pJJLoco->Facing.Set_Current(DirStruct(facing));
						if (pType->BalloonHover)
						{
							// Makes the jumpjet think it is hovering without actually moving.
							pJJLoco->NextState = JumpjetLocomotionClass::State::Hovering;
							pJJLoco->IsMoving = true;
							pJJLoco->HeadToCoord = location;
							pJJLoco->Height = pType->JumpjetData.Height;
							if (!inAir)
								AircraftTrackerClass::Instance->Add(pTechno);
						}
						else if (inAir)
						{
							// Order non-BalloonHover jumpjets to land.
							pJJLoco->Move_To(location);
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
	if (!pThis->Type)
		return;

	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
	const auto pAnimExt = AnimExtContainer::Instance.Find(pThis);

	//location is not marked , so dont !
	if (!pAnimExt->AllowCreateUnit)
		return;

	pThis->UnmarkAllOccupationBits(pAnimExt->CreateUnitLocation);

	HouseClass* decidedOwner = GetOwnerForSpawned(pThis);

	if (const auto pTechnoType = pTypeExt->CreateUnit)
	{
		const auto Is_AI = !decidedOwner->IsControlledByHuman();
		DirType facing = pTypeExt->CreateUnit_RandomFacing
			? ScenarioClass::Instance->Random.RandomRangedSpecific<DirType>(DirType::Min, DirType::Max) : pTypeExt->CreateUnit_Facing;

		DirType primaryFacing = pTypeExt->CreateUnit_InheritDeathFacings && pAnimExt->DeathUnitFacing.has_value() ? pAnimExt->DeathUnitFacing : facing;
		std::optional<DirType> secondaryFacing {};
		bool Scatter = false;
		Mission missionAI = Mission::None;

		if (pTechnoType->WhatAmI() == AbstractType::UnitType &&
			pTechnoType->Turret &&
			pAnimExt->DeathUnitTurretFacing.has_value() &&
			pTypeExt->CreateUnit_InheritTurretFacings)
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
			pTypeExt->CreateUnit_ConsiderPathfinding,
			pTypeExt->CreateUnit_SpawnParachutedInAir,
			pTypeExt->CreateUnit_AlwaysSpawnOnGround,
			missionAI
		)) {
			if (auto pSpawnAnim = pTypeExt->CreateUnit_SpawnAnim) {
				auto pCreateUnitAnim = GameCreate<AnimClass>(pSpawnAnim, pAnimExt->CreateUnitLocation);
				pCreateUnitAnim->Owner = decidedOwner;
				AnimExtContainer::Instance.Find(pCreateUnitAnim)->Invoker = AnimExtData::GetTechnoInvoker(pThis);
			}
		}
	}
}

void AnimTypeExtData::ValidateData()
{

	if (this->CreateUnit && this->CreateUnit->Strength == 0)
	{
		Debug::Log("AnimType[%s] With[%s] CreateUnit strength 0 !\n", this->AttachedToObject->ID, this->CreateUnit->ID);
		this->CreateUnit = nullptr;
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
	auto pAnimExt = AnimExtContainer::Instance.Find(pAnim);
	HouseClass* const pInvoker = pKiller ? pKiller->Owner : nullptr;
	AnimExtData::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, pThis, true);

	if (pAnimTypeExt->CreateUnit_InheritDeathFacings.Get())
		pAnimExt->DeathUnitFacing = facing;

	if (pAnimTypeExt->CreateUnit_InheritTurretFacings.Get())
	{
		if (pThis->HasTurret())
		{
			pAnimExt->DeathUnitTurretFacing = pThis->SecondaryFacing.Current();
		}
	}

	pAnimExt->WasOnBridge = pThis->OnBridge;
}

void AnimTypeExtData::ValidateSpalshAnims()
{
	AnimTypeClass* pWake = nullptr;
	if (this->AttachedToObject->IsMeteor)
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
		.Process(this->Initialized)

		.Process(this->Palette)
		.Process(this->CreateUnit)
		.Process(this->CreateUnit_Facing)
		.Process(this->CreateUnit_InheritDeathFacings)
		.Process(this->CreateUnit_InheritTurretFacings)
		.Process(this->CreateUnit_RemapAnim)
		.Process(this->CreateUnit_RandomFacing)
		.Process(this->CreateUnit_Mission)
		.Process(this->CreateUnit_AI_Mission)
		.Process(this->CreateUnit_Owner)
		.Process(this->CreateUnit_ConsiderPathfinding)
		.Process(this->CreateUnit_SpawnAnim)
		.Process(this->CreateUnit_AlwaysSpawnOnGround)
		.Process(this->CreateUnit_KeepOwnerIfDefeated)
		.Process(this->CreateUnit_SpawnParachutedInAir)
		.Process(this->XDrawOffset)
		.Process(this->HideIfNoOre_Threshold)
		.Process(this->Layer_UseObjectLayer)
		.Process(this->UseCenterCoordsIfAttached)
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
		.Process(this->SpawnsData)

		.Process(this->CreateUnit_Scatter)
		.Process(this->CreateUnit_AI_Scatter)
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
		;
}

AnimTypeExtContainer AnimTypeExtContainer::Instance;

DEFINE_HOOK(0x42784B, AnimTypeClass_CTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, EAX);
	AnimTypeExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x428EA8, AnimTypeClass_SDDTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, ECX);

	AnimTypeExtContainer::Instance.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x428970, AnimTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x428800, AnimTypeClass_SaveLoad_Prefix, 0xA)
{
	GET_STACK(AnimTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AnimTypeExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

// Before :
DEFINE_HOOK_AGAIN(0x42892C, AnimTypeClass_Load_Suffix, 0x6)
DEFINE_HOOK(0x428958, AnimTypeClass_Load_Suffix, 0x6)
{
	AnimTypeExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x42898A, AnimTypeClass_Save_Suffix, 0x3)
{
	AnimTypeExtContainer::Instance.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x4287E9, AnimTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x4287DC, AnimTypeClass_LoadFromINI, 0xA)
{
	GET(AnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xBC);

	AnimTypeExtContainer::Instance.LoadFromINI(pItem, pINI, R->Origin() == 0x4287E9);
	return 0;
}