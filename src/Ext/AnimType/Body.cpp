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
	SplashList.reserve(RulesClass::Instance->SplashList.Count);
	SpawnsMultiple.reserve(8);
	SpawnsMultiple_amouts.reserve(8);
	ConcurrentAnim.reserve(8);
	Launchs.reserve(8);
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
	this->CreateUnit_Mission.Read(exINI, pID, "CreateUnit.Mission");
	this->CreateUnit_Owner.Read(exINI, pID, "CreateUnit.Owner");
	this->CreateUnit_RandomFacing.Read(exINI, pID, "CreateUnit.RandomFacing");
	this->CreateUnit_ConsiderPathfinding.Read(exINI, pID, "CreateUnit.ConsiderPathfinding");
	this->CreateUnit_SpawnAnim.Read(exINI, pID, "CreateUnit.SpawnAnim");
	this->CreateUnit_AlwaysSpawnOnGround.Read(exINI, pID, "CreateUnit.AlwaysSpawnOnGround");
	this->CreateUnit_KeepOwnerIfDefeated.Read(exINI, pID, "CreateUnit.KeepOwnerIfDefeated");

	this->XDrawOffset.Read(exINI, pID, "XDrawOffset");
	this->HideIfNoOre_Threshold.Read(exINI, pID, "HideIfNoOre.Threshold");
	this->Layer_UseObjectLayer.Read(exINI, pID, "Layer.UseObjectLayer");
	this->UseCenterCoordsIfAttached.Read(exINI, pID, "UseCenterCoordsIfAttached");

	this->Weapon.Read(exINI, pID, "Weapon" , true);
	this->Damage_Delay.Read(exINI, pID, "Damage.Delay");
	this->Damage_DealtByInvoker.Read(exINI, pID, "Damage.DealtByInvoker");
	this->Damage_ApplyOnce.Read(exINI, pID, "Damage.ApplyOnce");
	this->Damage_ConsiderOwnerVeterancy.Read(exINI, pID, "Damage.ConsiderOwnerVeterancyBonus");
	this->Warhead_Detonate.Read(exINI, pID, "Warhead.Detonate");
	this->Damage_TargetFlag.Read(exINI, pID, "Damage.TargetFlag");

	bool Damage_TargetInvoker;
	if (detail::read(Damage_TargetInvoker , exINI, pID, "Damage.TargetInvoker") && Damage_TargetInvoker)
		this->Damage_TargetFlag = DamageDelayTargetFlag::Invoker;

	this->MakeInfantryOwner.Read(exINI, pID, "MakeInfantryOwner");

#pragma region Otamaa

	this->ParticleRangeMin.Read(exINI, pID, "SpawnsParticle.RangeMinimum");
	this->ParticleRangeMax.Read(exINI, pID, "SpawnsParticle.RangeMaximum");
	this->ParticleChance.Read(exINI, pID, "SpawnsParticle.Chance");

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
		char nBuff[0x30];
		SuperWeaponTypeClass* LaunchWhat_Dummy;
		IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Type", i);

		if (!detail::read(LaunchWhat_Dummy , exINI, pID, nBuff, true) || !LaunchWhat_Dummy)
			break;

		this->Launchs.emplace_back().Read(exINI, pID, i, LaunchWhat_Dummy);
	}

	this->RemapAnim.Read(exINI, pID, "RemapAnim");
#pragma endregion
}

void AnimTypeExtData::CreateUnit_MarkCell(AnimClass* pThis)
{
	if (!pThis->Type)
		return;

	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
	auto pExt = AnimExtContainer::Instance.Find(pThis);

	if (pExt->AllowCreateUnit)
		return;

	if (const auto pUnit = pTypeExt->CreateUnit.Get())
	{
		auto Location = pThis->GetCoords();

		if (!MapClass::Instance->IsWithinUsableArea(Location))
			return;

		auto pCell = pThis->GetCell();

		bool allowBridges = GroundType::GetCost(LandType::Clear , pUnit->SpeedType) > 0.0;
		bool isBridge = allowBridges && pCell->ContainsBridge();

		if (pTypeExt->CreateUnit_ConsiderPathfinding
			&& (!pCell || !pCell->IsClearToMove(pUnit->SpeedType, false, false, -1, pUnit->MovementZone, -1, isBridge)))
		{
			const auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(Location),
				pUnit->SpeedType, -1, pUnit->MovementZone, isBridge, 1, 1, true,
				false, false, isBridge, CellStruct::Empty, false, false);

			CellClass* pCell = MapClass::Instance->TryGetCellAt(nCell);
			if (!pCell)
				return;

			Location = pCell->GetCoords();
		}

		if (!pCell)
			return;

		isBridge = allowBridges && pCell->ContainsBridge();
		int bridgeZ = isBridge ? Unsorted::BridgeHeight : 0;

		const int z = pTypeExt->CreateUnit_AlwaysSpawnOnGround ? INT32_MIN : Location.Z;
		const auto nCellHeight = MapClass::Instance->GetCellFloorHeight(Location);
		Location.Z = MaxImpl(nCellHeight + bridgeZ, z);

		const auto pCellAfter = MapClass::Instance->GetCellAt(Location);

		//there is some unsolved vanilla bug that causing unit uneable to die
		//when it on brige , idk what happen there , but it is what it is for now
		//will reenable if fixed !
		//if (pCellAfter->ContainsBridge())
			//return;

		if (!MapClass::Instance->IsWithinUsableArea(Location))
			return;

		pExt->AllowCreateUnit = true;
		pThis->MarkAllOccupationBits(Location);
		pExt->CreateUnitLocation = Location;
	}
}

HouseClass* GetOwnerForSpawned(AnimClass* pThis)
{
	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
	if (!pThis->Owner || ((!pTypeExt->CreateUnit_KeepOwnerIfDefeated && pThis->Owner->Defeated)))
		return HouseExtData::FindCivilianSide();

	return pThis->Owner;
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

	if (const auto pTechno = static_cast<UnitClass*>(pTypeExt->CreateUnit->CreateObject(decidedOwner)))
	{
		const DirType resultingFacing = (pTypeExt->CreateUnit_InheritDeathFacings.Get() &&
			  pAnimExt->DeathUnitFacing.has_value())
			? pAnimExt->DeathUnitFacing.get() : pTypeExt->CreateUnit_RandomFacing.Get()
			? ScenarioClass::Instance->Random.RandomRangedSpecific<DirType>(DirType::North, DirType::Max) : pTypeExt->CreateUnit_Facing.Get();

		auto pCell = MapClass::Instance->GetCellAt(pAnimExt->CreateUnitLocation);
		if (!pTypeExt->CreateUnit_ConsiderPathfinding.Get() || !pCell->GetBuilding() || !pCell->ContainsBridge())
		{
			++Unsorted::ScenarioInit;
			pTechno->Unlimbo(pAnimExt->CreateUnitLocation, resultingFacing);
			--Unsorted::ScenarioInit;
		}
		else
		{
			 pTechno->Unlimbo(pAnimExt->CreateUnitLocation, resultingFacing);
		}

		if (!pTechno->InLimbo)
		{
			if (const auto pCreateUnitAnimType = pTypeExt->CreateUnit_SpawnAnim.Get(nullptr))
			{
				if (auto const pCreateUnitAnim = GameCreate<AnimClass>(pCreateUnitAnimType, pAnimExt->CreateUnitLocation))
				{
					pCreateUnitAnim->Owner = decidedOwner;
					if (auto pCreateUnitAnimExt = AnimExtContainer::Instance.Find(pCreateUnitAnim))
						pCreateUnitAnimExt->Invoker = AnimExtData::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker.Get());
				}
			}

			if (!decidedOwner->IsNeutral() && !pTypeExt->CreateUnit->Insignificant)
			{
				decidedOwner->RegisterGain(pTechno, false);
				decidedOwner->AddTracking(pTechno);
				decidedOwner->RecheckTechTree = 1;
			}

			if (pTechno->HasTurret() && pAnimExt->DeathUnitTurretFacing.has_value())
			{
				pTechno->SecondaryFacing.Set_Current(pAnimExt->DeathUnitTurretFacing.get());
			}

			if (pThis->IsInAir() && !pTypeExt->CreateUnit_AlwaysSpawnOnGround)
			{
				if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pTechno->Locomotor))
				{
					auto const pType = pTechno->GetTechnoType();
					pJJLoco->Facing.Set_Current(DirStruct(static_cast<DirType>(resultingFacing)));

					if (pType->BalloonHover)
					{
						// Makes the jumpjet think it is hovering without actually moving.
						pJJLoco->NextState = JumpjetLocomotionClass::State::Hovering;
						pJJLoco->IsMoving = true;
						pJJLoco->HeadToCoord = pAnimExt->CreateUnitLocation;
						pJJLoco->Height = pType->JumpjetHeight;
					}
					else
					{
						// Order non-BalloonHover jumpjets to land.
						pJJLoco->Move_To(pAnimExt->CreateUnitLocation);
					}
				}
				else
				{
					pTechno->IsFallingDown = true;
				}
			}
			else
			{
				pTechno->UpdatePlacement(PlacementType::Remove);
				pTechno->OnBridge = pCell->ContainsBridge();
				pTechno->UpdatePlacement(PlacementType::Put);
				//pTechno->MarkForRedraw();
			}

			pTechno->QueueMission(pTypeExt->CreateUnit_Mission.Get(), false);
		}
		else
		{
			Debug::Log(__FUNCTION__" Called \n");
			TechnoExtData::HandleRemove(pTechno);
		}
	}
}

void AnimTypeExtData::ProcessDestroyAnims(FootClass* pThis, TechnoClass* pKiller)
{
	const auto location = pThis->GetCoords();

	if (!MapClass::Instance->IsWithinUsableArea(location))
		return;

	const auto pType = pThis->GetTechnoType();
	if (!pType->DestroyAnim.Count)
		return;

	const DirType facing = (DirType)pThis->PrimaryFacing.Current().GetFacing<256>();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

	int idxAnim = 0;
	GeneralUtils::GetRandomAnimVal(idxAnim, pType->DestroyAnim.Count, (short)facing, pTypeExt->DestroyAnim_Random.Get());
	AnimTypeClass* pAnimType = pType->DestroyAnim[idxAnim];

	if (!pAnimType)
		return;

	if (auto pAnim = GameCreate<AnimClass>(pAnimType, location))
	{
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
	}
}

void AnimTypeExtData::ValidateSpalshAnims()
{
	AnimTypeClass* pWake = nullptr;
	if (WakeAnim.isset() && this->AttachedToObject->IsMeteor)
		pWake = WakeAnim.Get();
	else
		pWake = RulesClass::Instance->Wake;

	//what if the anim type loaded twice ?
	if (SplashList.empty())
	{
		SplashList.push_back(pWake);
	}
}

OwnerHouseKind AnimTypeExtData::GetAnimOwnerHouseKind() {

	if (this->CreateUnit && !this->CreateUnit_Owner.isset())
		return OwnerHouseKind::Victim;

	if(this->AttachedToObject->MakeInfantry > -1 && !this->MakeInfantryOwner.isset())
		return OwnerHouseKind::Invoker;

	if (this->CreateUnit_Owner.isset())
		return this->CreateUnit_Owner;

	if(this->MakeInfantryOwner.isset())
		return this->MakeInfantryOwner;

	return OwnerHouseKind::Invoker;
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
		.Process(this->CreateUnit_Owner)
		.Process(this->CreateUnit_ConsiderPathfinding)
		.Process(this->CreateUnit_SpawnAnim)
		.Process(this->CreateUnit_AlwaysSpawnOnGround)
		.Process(this->CreateUnit_KeepOwnerIfDefeated)
		.Process(this->XDrawOffset)
		.Process(this->HideIfNoOre_Threshold)
		.Process(this->Layer_UseObjectLayer)
		.Process(this->UseCenterCoordsIfAttached)
		.Process(this->Weapon)
		.Process(this->Damage_Delay)
		.Process(this->Damage_DealtByInvoker)
		.Process(this->Damage_ApplyOnce)
		.Process(this->Damage_ConsiderOwnerVeterancy)
		.Process(this->Damage_TargetFlag)
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