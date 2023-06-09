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

void AnimTypeExt::ExtData::Initialize()
{
	SplashList.reserve(RulesClass::Instance->SplashList.Count);
	SpawnsMultiple.reserve(8);
	SpawnsMultiple_amouts.reserve(8);
	ConcurrentAnim.reserve(8);
	Launchs.reserve(8);
	const char* pID = this->Get()->ID;

	SpecialDraw = IS_SAME_STR_(pID, RING1_NAME);
	IsInviso = IS_SAME_STR_(pID, INVISO_NAME);
}

// AnimType Class is readed before Unit and weapon 
// so it is safe to `allocate` them before

void AnimTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	const char* pID = this->Get()->ID;

	if (!pINI->GetSection(pID))
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

	this->Weapon.Read(exINI, pID, "Weapon", true);
	this->Damage_Delay.Read(exINI, pID, "Damage.Delay");
	this->Damage_DealtByInvoker.Read(exINI, pID, "Damage.DealtByInvoker");
	this->Damage_ApplyOnce.Read(exINI, pID, "Damage.ApplyOnce");
	this->Damage_ConsiderOwnerVeterancy.Read(exINI, pID, "Damage.ConsiderOwnerVeterancyBonus");
	this->Warhead_Detonate.Read(exINI, pID, "Warhead.Detonate");
	this->Damage_TargetFlag.Read(exINI, pID, "Damage.TargetFlag");

	Nullable<bool> Damage_TargetInvoker {};
	Damage_TargetInvoker.Read(exINI, pID, "Damage.TargetInvoker");
	if (Damage_TargetInvoker.isset())
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

	if (!this->SpawnsMultiple.empty())
	{
		auto const nBaseSize = (int)this->SpawnsMultiple.size();
		this->SpawnsMultiple_amouts.resize(nBaseSize, 1);

		if (exINI.ReadString(pID, "SpawnsMultiple.Amount"))
		{
			int nCount = 0;
			char* context = nullptr;
			for (char* cur = strtok_s(exINI.value(), Phobos::readDelims, &context);
				cur;
				cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				int buffer;
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
	this->ShouldFogRemove.Read(exINI, pID, "ShouldFogRemove");
	this->AttachedSystem.Read(exINI, pID, "AttachedSystem", true);

	//Launchs
	this->Launchs.clear();
	for (size_t i = 0; ; ++i)
	{
		char nBuff[0x30];
		Nullable<SuperWeaponTypeClass*> LaunchWhat_Dummy { };
		IMPL_SNPRNINTF(nBuff, sizeof(nBuff), "LaunchSW%d.Type", i);
		LaunchWhat_Dummy.Read(exINI, pID, nBuff, true);

		if (!LaunchWhat_Dummy.isset() || !LaunchWhat_Dummy.Get())
			break;

		LauchSWData nData;
		if (!nData.Read(exINI, pID, i, LaunchWhat_Dummy))
			break;

		this->Launchs.push_back(std::move(nData));
	}

#pragma endregion
}

void AnimTypeExt::CreateUnit_MarkCell(AnimClass* pThis)
{
	if (!pThis->Type)
		return;

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	auto pExt = AnimExt::ExtMap.Find(pThis);

	if (pExt->AllowCreateUnit)
		return;

	if (const auto pUnit = pTypeExt->CreateUnit.Get())
	{
		auto Location = pThis->GetCoords();

		if (pTypeExt->CreateUnit_ConsiderPathfinding)
		{
			const auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(Location),
				pUnit->SpeedType, -1, pUnit->MovementZone, false, 1, 1, true,
				false, false, pUnit->SpeedType != SpeedType::Float, CellStruct::Empty, false, false);

			CellClass* pCell = MapClass::Instance->TryGetCellAt(nCell);
			if (!pCell)
				return;

			Location = pTypeExt->CreateUnit->SpeedType != SpeedType::Float ?
				pCell->GetCoordsWithBridge() : pCell->GetCoords();
		}

		// TODO : this shit break when the unit on top of bridge
		const int z = pTypeExt->CreateUnit_AlwaysSpawnOnGround ? INT32_MIN : pThis->GetCoords().Z;
		const auto nCellHeight = MapClass::Instance->GetCellFloorHeight(Location);
		Location.Z = MaxImpl(nCellHeight, z);

		const auto pCellAfter = MapClass::Instance->GetCellAt(Location);

		//there is some unsolved vanilla bug that causing unit uneable to die 
		//when it on brige , idk what happen there , but it is what it is for now
		//will reenable if fixed !
		if (pCellAfter->ContainsBridge())
			return;

		if (!MapClass::Instance->IsWithinUsableArea(Location))
			return;

		if(pCellAfter->GetBuilding())
			return;

		pExt->AllowCreateUnit = true;
		pThis->MarkAllOccupationBits(Location);
		pExt->CreateUnitLocation = Location;
	}
}

HouseClass* GetOwnerForSpawned(AnimClass* pThis)
{
	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	if (!pThis->Owner || ((!pTypeExt->CreateUnit_KeepOwnerIfDefeated && pThis->Owner->Defeated)))
		return HouseExt::FindCivilianSide();

	return pThis->Owner;
}

void AnimTypeExt::CreateUnit_Spawn(AnimClass* pThis)
{
	if (!pThis->Type)
		return;

	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);
	const auto pAnimExt = AnimExt::ExtMap.Find(pThis);

	//location is not marked , so dont !
	if (!pAnimExt->AllowCreateUnit)
		return;

	if (const auto unit = pTypeExt->CreateUnit.Get())
	{
		HouseClass* decidedOwner = GetOwnerForSpawned(pThis);
		//const auto pCell = MapClass::Instance->TryGetCellAt(pAnimExt->CreateUnitLocation);
		pThis->UnmarkAllOccupationBits(pAnimExt->CreateUnitLocation);

		if (const auto pTechno = static_cast<UnitClass*>(unit->CreateObject(decidedOwner)))
		{
			bool success = false;

			const short resultingFacing = (pTypeExt->CreateUnit_InheritDeathFacings.Get() &&
				  pAnimExt->DeathUnitFacing.has_value())
				? pAnimExt->DeathUnitFacing.get() : pTypeExt->CreateUnit_RandomFacing.Get()
				? ScenarioClass::Instance->Random.RandomRangedSpecific<short>(0, 255) : (short)pTypeExt->CreateUnit_Facing.Get();

			if (!pTypeExt->CreateUnit_ConsiderPathfinding.Get())
			{
				++Unsorted::IKnowWhatImDoing;
				success = pTechno->Unlimbo(pAnimExt->CreateUnitLocation, DirType(resultingFacing));
				--Unsorted::IKnowWhatImDoing;
			}
			else
			{
				success = pTechno->Unlimbo(pAnimExt->CreateUnitLocation, DirType(resultingFacing));
			}

			if (success)
			{
				//pTechno->UpdatePlacement(PlacementType::Remove);
				//pTechno->OnBridge = pCell->ContainsBridge();
				//pTechno->UpdatePlacement(PlacementType::Put);

				if (const auto pCreateUnitAnimType = pTypeExt->CreateUnit_SpawnAnim.Get(nullptr))
				{
					if (auto const pCreateUnitAnim = GameCreate<AnimClass>(pCreateUnitAnimType, pAnimExt->CreateUnitLocation))
					{
						pCreateUnitAnim->Owner = decidedOwner;
						if (auto pCreateUnitAnimExt = AnimExt::ExtMap.Find(pCreateUnitAnim))
							pCreateUnitAnimExt->Invoker = AnimExt::GetTechnoInvoker(pThis, pTypeExt->Damage_DealtByInvoker.Get());
					}
				}

				if (!decidedOwner->IsNeutral() && !unit->Insignificant)
				{
					decidedOwner->RegisterGain(pTechno, false);
					decidedOwner->AddTracking(pTechno);
					decidedOwner->RecheckTechTree = 1;
				}

				if (pTechno->HasTurret() && pAnimExt->DeathUnitTurretFacing.has_value())
				{
					pTechno->SecondaryFacing.Set_Desired(pAnimExt->DeathUnitTurretFacing.get());
				}


				pTechno->QueueMission(pTypeExt->CreateUnit_Mission.Get(), false);
			}
			else
			{
				TechnoExt::HandleRemove(pTechno);
			}
		}
	}
}

OwnerHouseKind AnimTypeExt::SetMakeInfOwner(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim)
{
	if (!pAnim)
		return OwnerHouseKind::Neutral;

	auto pAnimData = AnimTypeExt::ExtMap.Find(pAnim->Type);

	auto newOwner = HouseExt::GetHouseKind(pAnimData->MakeInfantryOwner, true,
		nullptr, pInvoker, pVictim);

	if (newOwner)
	{
		pAnim->Owner = newOwner;
		if (pAnim->Type->MakeInfantry > -1)
		{
			pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;
		}
	}

	return pAnimData->MakeInfantryOwner;
}

void AnimTypeExt::ProcessDestroyAnims(FootClass* pThis, TechnoClass* pKiller)
{
	const auto location = pThis->GetCoords();

	if (!MapClass::Instance->IsWithinUsableArea(location))
		return;

	const auto pType = pThis->GetTechnoType();
	if (!pType->DestroyAnim.Count)
		return;

	const auto facing = pThis->PrimaryFacing.Current().GetFacing<256>();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	int idxAnim = 0;
	GeneralUtils::GetRandomAnimVal(idxAnim, pType->DestroyAnim.Count, facing, pTypeExt->DestroyAnim_Random.Get());
	AnimTypeClass* pAnimType = pType->DestroyAnim[idxAnim];

	if (!pAnimType)
		return;

	if (auto pAnim = GameCreate<AnimClass>(pAnimType, location))
	{
		const auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pAnimType);
		auto pAnimExt = AnimExt::ExtMap.Find(pAnim);
		HouseClass* const pInvoker = pKiller ? pKiller->Owner : nullptr;

		if (AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner, true))
			pAnimExt->Invoker = pThis;

		if (pAnimTypeExt->CreateUnit_InheritDeathFacings.Get())
			pAnimExt->DeathUnitFacing = static_cast<short>(facing);

		if (pAnimTypeExt->CreateUnit_InheritTurretFacings.Get())
		{
			if (pThis->HasTurret())
			{
				pAnimExt->DeathUnitTurretFacing = pThis->SecondaryFacing.Current();
			}
		}
	}
}

void AnimTypeExt::ExtData::ValidateSpalshAnims()
{
	AnimTypeClass* pWake = nullptr;
	if (WakeAnim.isset() && this->Get()->IsMeteor)
		pWake = WakeAnim.Get();
	else
		pWake = RulesClass::Instance->Wake;

	//what if the anim type loaded twice ?
	if (SplashList.empty())
	{
		SplashList.push_back(pWake);
	}
}

template <typename T>
void AnimTypeExt::ExtData::Serialize(T& Stm)
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

		.Process(SplashList)
		.Process(SplashIndexRandom)
		.Process(WakeAnim)
		.Process(ExplodeOnWater)

		.Process(SpawnsMultiple)
		.Process(SpawnsMultiple_Random)
		.Process(SpawnsMultiple_amouts)

		.Process(ParticleRangeMin)
		.Process(ParticleRangeMax)
		.Process(ParticleChance)

		.Process(Launchs)

		.Process(CraterDecreaseTiberiumAmount)
		.Process(CraterChance)
		.Process(SpawnCrater)
		.Process(ScorchChance)
		.Process(SpecialDraw)
		.Process(NoOwner)
		.Process(Spawns_Delay)

		.Process(ConcurrentChance)
		.Process(ConcurrentAnim)
		.Process(ShouldFogRemove)
		.Process(this->MakeInfantryOwner)
		.Process(AttachedSystem)
		.Process(IsInviso)
		//.Process(SpawnerDatas)
		;
}

AnimTypeExt::ExtContainer AnimTypeExt::ExtMap;
AnimTypeExt::ExtContainer::ExtContainer() : Container("AnimTypeClass") { }
AnimTypeExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x42784B, AnimTypeClass_CTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, EAX);
	AnimTypeExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x428EA8, AnimTypeClass_SDDTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, ECX);

	AnimTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x428970, AnimTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x428800, AnimTypeClass_SaveLoad_Prefix, 0xA)
{
	GET_STACK(AnimTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AnimTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

// Before : 
DEFINE_HOOK_AGAIN(0x42892C, AnimTypeClass_Load_Suffix, 0x6)
DEFINE_HOOK(0x428958, AnimTypeClass_Load_Suffix, 0x6)
{
	AnimTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x42898A, AnimTypeClass_Save_Suffix, 0x3)
{
	AnimTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x4287E9, AnimTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x4287DC, AnimTypeClass_LoadFromINI, 0xA)
{
	GET(AnimTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xBC);

	AnimTypeExt::ExtMap.LoadFromINI(pItem, pINI, R->Origin() == 0x4287E9);
	return 0;
}