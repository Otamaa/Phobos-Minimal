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
#include <Ext/TechnoType/Body.h>

AnimTypeExt::ExtContainer AnimTypeExt::ExtMap;

void AnimTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->Get()->ID;

	if (!pINI) {
		Debug::FatalErrorAndExit(__FUNCTION__" Missing CCINIClass Pointer somehow WTF ?  , at [%x - %s]\n", this->Get(), pID);
	}

	INI_EX exINI(pINI);

	if (!pINI->GetSection(pID))
		return;

	SpecialDraw = IS_SAME_STR_(pID, RING1_NAME);
	IsInviso = IS_SAME_STR_(pID, INVISO_NAME);

	this->Palette.Read(pINI, pID, "CustomPalette");
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
	this->Damage_TargetInvoker.Read(exINI, pID, "Damage.TargetInvoker");

#pragma region Otamaa

	//Launchs
	this->Launchs.clear();
	for (size_t i = 0; ; ++i)
	{
		LauchSWData nData;
		if (!nData.Read(exINI, pID, i))
			break;

		this->Launchs.push_back(std::move(nData));
	}

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
		this->SpawnsMultiple_amouts.Clear();
		this->SpawnsMultiple_amouts.Reserve(nBaseSize);
		this->SpawnsMultiple_amouts.Count = nBaseSize;
		auto const pKey = "SpawnsMultiple.Amount";

		for (auto& nSpawnMult : this->SpawnsMultiple_amouts)
			nSpawnMult = 1;

		if (exINI.ReadString(pID, pKey))
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
	this->AttachedSystem.Read(exINI, pID, "AttachedSystem");

	//if (AttachedSystem && AttachedSystem->BehavesLike != BehavesLike::Smoke)
	//	AttachedSystem = nullptr;

	//this->SpawnerDatas.Read(exINI, pID);
#pragma endregion
}

const void AnimTypeExt::ProcessDestroyAnims(UnitClass* pThis, TechnoClass* pKiller)
{
	if (pThis->Type->DestroyAnim.Count > 0)
	{
		HouseClass* pInvoker = pKiller ? pKiller->Owner : nullptr;
		auto facing = pThis->PrimaryFacing.Current().GetFacing<256>();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

		int idxAnim = 0;

		if (!pTypeExt->DestroyAnim_Random.Get())
		{
			if (pThis->Type->DestroyAnim.Count >= 8)
			{
				idxAnim = pThis->Type->DestroyAnim.Count;
				if (pThis->Type->DestroyAnim.Count % 2 == 0)
					idxAnim *= static_cast<int>(facing / 256.0);
			}
		}
		else
		{
			idxAnim = ScenarioGlobal->Random.RandomFromMax(pThis->Type->DestroyAnim.Count - 1);
		}

		if (AnimTypeClass* pAnimType = pThis->Type->DestroyAnim[idxAnim])
		{
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, pThis->GetCoords()))
			{
				auto pAnimTypeExt = AnimTypeExt::ExtMap.Find(pAnimType);
				auto pAnimExt = AnimExt::ExtMap.Find(pAnim);

				if (AnimExt::SetAnimOwnerHouseKind(pAnim, pInvoker, pThis->Owner,true)) {
					pAnimExt->Invoker = pThis;
				}

				if (pAnimTypeExt->CreateUnit_InheritDeathFacings.Get())
					pAnimExt->DeathUnitFacing = static_cast<short>(facing);

				if (pAnimTypeExt->CreateUnit_InheritTurretFacings.Get()) {
					if (pThis->HasTurret()) {
						pAnimExt->DeathUnitTurretFacing = pThis->SecondaryFacing.Current();
					}
				}
			}
		}
	}
}

template <typename T>
void AnimTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
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
		.Process(this->XDrawOffset)
		.Process(this->HideIfNoOre_Threshold)
		.Process(this->Layer_UseObjectLayer)
		.Process(this->UseCenterCoordsIfAttached)
		.Process(this->Weapon)
		.Process(this->Damage_Delay)
		.Process(this->Damage_DealtByInvoker)
		.Process(this->Damage_ApplyOnce)
		.Process(this->Damage_ConsiderOwnerVeterancy)
		.Process(this->Damage_TargetInvoker)
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
		.Process(AttachedSystem)
		.Process(IsInviso)
		//.Process(SpawnerDatas)
		;
}

void AnimTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AnimTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AnimTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AnimTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool AnimTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool AnimTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

AnimTypeExt::ExtContainer::ExtContainer() : Container("AnimTypeClass") { }
AnimTypeExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x42784B, AnimTypeClass_CTOR, 0x5)
{
	GET(AnimTypeClass*, pItem, EAX);

#ifndef ENABLE_NEWEXT
	AnimTypeExt::ExtMap.JustAllocate(pItem, pItem, "Trying To Allocate from nullptr !");
#else
	AnimTypeExt::ExtMap.FindOrAllocate(pItem);
#endif

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

	AnimTypeExt::ExtMap.LoadFromINI(pItem , pINI);
	return 0;
}

#ifndef ENABLE_NEWEXT

//DEFINE_HOOK(0x42772A, AnimTypeClass_CTOR_ShouldFogRemove, 0x7)
//{
//	GET(AnimTypeClass*, pItem, ESI);
//	pItem->ShouldFogRemove = 0;
//	return 0x427731;
//}

DEFINE_HOOK(0x4282C2, AnimTypeClass_ReadFromINI_Replace, 0x6)
{
	GET(AnimTypeClass*, pItem, ESI);
	pItem->IsAnimatedTiberium = R->AL();
	return 0x4282DC;
}

DEFINE_JUMP(LJMP,0x4282DC, 0x4282E2);

DEFINE_HOOK(0x42301E, AnimClass_DrawIt_ShouldFogRemove_Ext, 0x6)
{
	GET(AnimTypeClass*, pType, EAX);
	R->CL(AnimTypeExt::ExtMap.Find(pType)->ShouldFogRemove);
	return 0x423024;
}
#endif