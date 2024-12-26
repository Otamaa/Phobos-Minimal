#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Bullet/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>
#include <Utilities/AnimHelpers.h>

#include <Misc/Hooks.Otamaa.h>

#include <ParticleSystemClass.h>
#include <ColorScheme.h>
#include <SmudgeTypeClass.h>
#include <CoordStruct.h>
#include <GameOptionsClass.h>

//std::vector<CellClass*> AnimExtData::AnimCellUpdater::Marked;
void AnimExtData::OnInit(AnimClass* pThis, CoordStruct* pCoord)
{
	if (!pThis->Type)
		return;

	const auto pTypeExt = ((FakeAnimTypeClass*)pThis->Type)->_GetExtData();

	if (pTypeExt->ConcurrentChance.Get() >= 1.0 && !pTypeExt->ConcurrentAnim.empty())
	{
		if (ScenarioClass::Instance->Random.RandomDouble() <= pTypeExt->ConcurrentChance.Get())
		{

			auto const nIdx = pTypeExt->ConcurrentAnim.size() == 1 ?
				0 : ScenarioClass::Instance->Random.RandomFromMax(pTypeExt->ConcurrentAnim.size() - 1);

			if (auto pType = pTypeExt->ConcurrentAnim[nIdx])
			{

				if (pType == pThis->Type)
					return;

				GameCreate<AnimClass>(pType, pCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, false)->Owner = pThis->GetOwningHouse();
			}
		}
	}

	//if (auto const& pSpawns = pExt->SpawnData) {
	//	pSpawns->OnInit(pCoord);
	//}
}

bool AnimExtData::OnMiddle_SpawnSmudge(AnimClass* pThis, CellClass* pCell, Point2D nOffs)
{
	const auto pType = pThis->Type;
	if (!pType)
		return false;

	const auto pTypeExt = ((FakeAnimTypeClass*)pThis->Type)->_GetExtData();

	if (pTypeExt->SpawnCrater.Get(pThis->GetHeight() < 30))
	{
		if (pType->Flamer || pType->Scorch)
			AnimExtData::SpawnFireAnims(pThis);

		auto nCoord = pThis->GetCoords();
		if (!pType->Scorch || (pType->Crater && ScenarioClass::Instance->Random.RandomDouble() >= pTypeExt->CraterChance.Get()))
		{
			if (pType->Crater)
			{
				if (pTypeExt->CraterDecreaseTiberiumAmount.Get() > 0)
					pCell->ReduceTiberium(pTypeExt->CraterDecreaseTiberiumAmount.Get());

				if (pType->ForceBigCraters)
					SmudgeTypeClass::CreateRandomSmudgeFromTypeList(nCoord, 300, 300, true);
				else
					SmudgeTypeClass::CreateRandomSmudgeFromTypeList(nCoord, nOffs.X, nOffs.Y, false);
			}
		}
		else
		{
			const bool bSpawn = (pTypeExt->ScorchChance.isset()) ? (
				ScenarioClass::Instance->Random.RandomDouble() >= pTypeExt->ScorchChance.Get()) : true;

			if (bSpawn)
				SmudgeTypeClass::CreateRandomSmudge(nCoord, nOffs.X, nOffs.Y, false);
		}
	}

	return true;
}

bool AnimExtData::OnExpired(AnimClass* pThis, bool LandIsWater, bool EligibleHeight)
{
	if (!pThis->Type)
		return false;

	auto const pAnimTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);

	{
		TechnoClass* const pTechOwner = AnimExtData::GetTechnoInvoker(pThis);
		auto const pOwner = !pThis->Owner && pTechOwner ? pTechOwner->Owner : pThis->Owner;

		if (!LandIsWater || EligibleHeight)
		{
			Helper::Otamaa::DetonateWarhead(int(pThis->Type->Damage), pThis->Type->Warhead, pAnimTypeExt->Warhead_Detonate, pThis->Bounce.GetCoords(), pTechOwner, pOwner, pAnimTypeExt->Damage_ConsiderOwnerVeterancy.Get());

			if (auto const pExpireAnim = pThis->Type->ExpireAnim)
			{
				AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pExpireAnim, pThis->Bounce.GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30, 0),
					pOwner,
					nullptr,
					pTechOwner,
					false
				);
			}
		}
		else
		{
			if (!pAnimTypeExt->ExplodeOnWater)
			{
				if (auto pSplashAnim = Helper::Otamaa::PickSplashAnim(pAnimTypeExt->SplashList, pAnimTypeExt->WakeAnim, pAnimTypeExt->SplashIndexRandom.Get(), pThis->Type->IsMeteor))
				{
					AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pSplashAnim, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, false),
						pOwner,
						nullptr,
						pTechOwner,
						false
					);
				}
			}
			else
			{
				auto const& [bPlayWHAnim, nDamage] = Helper::Otamaa::DetonateWarhead(int(pThis->Type->Damage), pThis->Type->Warhead, pAnimTypeExt->Warhead_Detonate, pThis->GetCoords(), pTechOwner, pOwner, pAnimTypeExt->Damage_ConsiderOwnerVeterancy.Get());
				if (bPlayWHAnim)
				{
					if (auto pSplashAnim = MapClass::SelectDamageAnimation(nDamage, pThis->Type->Warhead, pThis->GetCell()->LandType, pThis->GetCoords()))
					{
						AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pSplashAnim, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -30),
							pOwner,
							nullptr,
							pTechOwner,
							false
						);
					}
				}
			}
		}
	}

	return true;
}

#include <Misc/PhobosGlobal.h>

DWORD AnimExtData::DealDamageDelay(AnimClass* pThis)
{
	enum { SkipDamage = 0x42465D, CheckIsActive = 0x42464C };

	if (!pThis->Type)
		return CheckIsActive;

	const auto pExt = ((FakeAnimClass*)pThis)->_GetExtData();
	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pThis->Type);
	const int delay = pTypeExt->Damage_Delay.Get();
	TechnoClass* const pInvoker = AnimExtData::GetTechnoInvoker(pThis);
	const double damageMultiplier = (pThis->OwnerObject && pThis->OwnerObject->WhatAmI() == TerrainClass::AbsID) ? 5.0 : 1.0;

	int appliedDamage = 0;

	if (pTypeExt->Damage_ApplyOnce.Get()) // If damage is to be applied only once per animation loop
	{
		if (pThis->Animation.Value == MaxImpl(delay - 1, 1))
			appliedDamage = static_cast<int>(std::round(pThis->Type->Damage) * damageMultiplier);
		else
			return SkipDamage;
	}
	else if (delay <= 0 || pThis->Type->Damage < 1.0) // If Damage.Delay is less than 1 or Damage is a fraction.
	{
		double damage = damageMultiplier * pThis->Type->Damage + pThis->Accum;

		// Deal damage if it is at least 1, otherwise accumulate it for later.
		if (damage >= 1.0) {
			appliedDamage = static_cast<int>(std::round(damage));
			pThis->Accum = damage - appliedDamage;

		} else {
			pThis->Accum = damage;
			return SkipDamage;
		}
	}
	else
	{
		// Accum here is used as a counter for Damage.Delay, which cannot deal fractional damage.
		pThis->Accum += 1.0;

		if (pThis->Accum < delay)
			return SkipDamage;

		// Use Type->Damage as the actually dealt damage.
		appliedDamage = static_cast<int>((pThis->Type->Damage) * damageMultiplier);
		pThis->Accum = 0.0;
	}

	if (appliedDamage <= 0 || pThis->IsPlaying)
		return  SkipDamage;

	const CoordStruct nCoord = pExt->BackupCoords.has_value() ? pExt->BackupCoords.get() : pThis->GetCoords();
	const auto pOwner = pThis->Owner ? pThis->Owner : pInvoker ? pInvoker->Owner : nullptr;

	if (auto const pWeapon = pTypeExt->Weapon)
	{
		AbstractClass* pTarget = AnimExtData::GetTarget(pThis);
		WeaponTypeExtData::DetonateAt(pWeapon, nCoord, pTarget, pInvoker, appliedDamage, pTypeExt->Damage_ConsiderOwnerVeterancy.Get(), pOwner);
	}
	else
	{
		auto const pWarhead = pThis->Type->Warhead ? pThis->Type->Warhead :
			!pTypeExt->IsInviso ? RulesClass::Instance->FlameDamage2 : RulesClass::Instance->C4Warhead;

		const auto nDamageResult = static_cast<int>(appliedDamage * TechnoExtData::GetDamageMult(pInvoker, !pTypeExt->Damage_ConsiderOwnerVeterancy.Get()));

		if (pTypeExt->Warhead_Detonate.Get())
		{
			AbstractClass* pTarget = AnimExtData::GetTarget(pThis);
			// use target loc instead of anim loc , it doesnt work well with bridges
			WarheadTypeExtData::DetonateAt(pWarhead, pTarget, pTarget ? pTarget->GetCoords() : nCoord, pInvoker, nDamageResult, pOwner);
		}
		else
		{
			// Ares keep the `Source` nullptr so it can affect everything
			// if the `Source` contains `OwnerObject` it will cause problem because the techno need `DamageSelf`
			// in order to deal damage to itself ,..
			MapClass::DamageArea(nCoord, nDamageResult, pInvoker, pWarhead, pWarhead->Tiberium, pOwner);
			//PhobosGlobal::Instance()->AnimAttachedto = nullptr;
			MapClass::FlashbangWarheadAt(nDamageResult, pWarhead, nCoord);
		}
	}

	return CheckIsActive;
}

bool AnimExtData::OnMiddle(AnimClass* pThis)
{
	const auto pType = pThis->Type;
	if (!pType)
		return false;

	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pType);

	{
		auto pAnimTypeExt = pTypeExt;
		const auto pObject = AnimExtData::GetTechnoInvoker(pThis);
		const auto pHouse = !pThis->Owner && pObject ? pObject->Owner : pThis->Owner;
		const auto nCoord = pThis->Location;

		Helper::Otamaa::SpawnMultiple(
			pAnimTypeExt->SpawnsMultiple,
			pAnimTypeExt->SpawnsMultiple_amouts,
			nCoord, pObject, pHouse, pAnimTypeExt->SpawnsMultiple_Random.Get());

		if (pType->SpawnsParticle != -1)
		{
			if (const auto pParticleType = ParticleTypeClass::Array->Items[pType->SpawnsParticle])
			{
				CoordStruct InitialCoord = nCoord;
				InitialCoord.Z -= MapClass::Instance->GetCellFloorHeight(nCoord);

				if (!pAnimTypeExt->SpawnParticleModeUseAresCode)
				{
					for (int i = 0; i < pType->NumParticles; ++i)
					{
						CoordStruct nDestCoord = CoordStruct::Empty;
						if (!pAnimTypeExt->ParticleChance.isset() ||
							(ScenarioClass::Instance->Random.RandomFromMax(99) < Math::abs(pAnimTypeExt->ParticleChance.Get())))
						{
							nDestCoord = Helper::Otamaa::GetRandomCoordsInsideLoops(pAnimTypeExt->ParticleRangeMin.Get(), pAnimTypeExt->ParticleRangeMax.Get(), InitialCoord, i);
							ParticleSystemClass::Instance->SpawnParticle(pParticleType, &nDestCoord);
						}
					}
				}
				else
				{
					int numParticle = pType->NumParticles;

					if (numParticle > 0) {

						const auto nMin = pAnimTypeExt->ParticleRangeMin.Get();
						const auto nMax = pAnimTypeExt->ParticleRangeMax.Get();

						if (nMin || nMax) {

							double rad = 6.283185307179586 / numParticle;
							double start_distance = 0.0;

							for (; numParticle > 0; --numParticle) {

								int rand = Math::abs(ScenarioClass::Instance->Random.RandomRanged((int)nMin, (int)nMax));
								double randDouble = ScenarioClass::Instance->Random.RandomDouble() * rad + start_distance;
								CoordStruct dest {
									InitialCoord.X + int(rand * Math::cos(randDouble)),
									InitialCoord.Y - int(Math::sin(randDouble) * rand),
									nCoord.Z
								};

								dest.Z = InitialCoord.Z + MapClass::Instance->GetCellFloorHeight(dest);
								ParticleSystemClass::Instance->SpawnParticle(pParticleType, &dest);
								start_distance += rad;
							}

						}
						else {
							for (int i = 0; i < numParticle; ++i) {
								ParticleSystemClass::Instance->SpawnParticle(pParticleType, &pThis->Location);
							}
						}
					}
				}
			}
		}

		for (const auto& nLauch : pTypeExt->Launchs)
		{
			if (nLauch.LaunchWhat)
			{
				Helpers::Otamaa::LauchSW(nLauch, pHouse, nCoord, pObject);
			}
		}

		if (auto pWeapon = pTypeExt->WeaponToCarry) {
			AbstractClass* pTarget = AnimExtData::GetTarget(pThis);
			TechnoClass* const pInvoker = AnimExtData::GetTechnoInvoker(pThis);
			const auto nDamageResult = static_cast<int>(pWeapon->Damage * TechnoExtData::GetDamageMult(pInvoker, !pTypeExt->Damage_ConsiderOwnerVeterancy.Get()));
			const auto pOwner = pThis->Owner ? pThis->Owner : pInvoker ? pInvoker->Owner : nullptr;

			WeaponTypeExtData::DetonateAt(pWeapon, pTarget, pInvoker, pTypeExt->Damage_ConsiderOwnerVeterancy, pOwner);
		}
	}

	return true;
}

AbstractClass* AnimExtData::GetTarget(AnimClass* pThis)
{
	auto const pType = pThis->Type;
	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(pType);

	if (!pTypeExt->Damage_TargetFlag.isset())
	{
		return pThis->GetCell();
	}

	switch (pTypeExt->Damage_TargetFlag.Get())
	{
	case DamageDelayTargetFlag::Cell:
		return  pThis->GetCell();
	case DamageDelayTargetFlag::AttachedObject:
	{
		if (pThis->AttachedBullet)
		{
			return pThis->AttachedBullet->Owner;
		}
		else
		{
			if (auto const pBullet = cast_to<BulletClass*, false>(pThis->OwnerObject))
				return pBullet->Owner;
			else
				return pThis->OwnerObject;
		}
	}
	case DamageDelayTargetFlag::Invoker:
	{
		return ((FakeAnimClass*)pThis)->_GetExtData()->Invoker;
	}
	}

	return nullptr;
}

void AnimExtData::InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
{
	AnnounceInvalidPointer(this->Invoker, ptr, bRemoved);
	AnnounceInvalidPointer(this->ParentBuilding, ptr, bRemoved);

	if (this->AttachedSystem == ptr)
		this->AttachedSystem = nullptr;
}

void AnimExtData::CreateAttachedSystem()
{
	const auto pThis = this->AttachedToObject;
	const auto pData = AnimTypeExtContainer::Instance.TryFind(pThis->Type);

	if (!pData || !pData->AttachedSystem || this->AttachedSystem)
		return;

	auto nLoc = pThis->Location;

	if (pData->AttachedSystem->BehavesLike == ParticleSystemTypeBehavesLike::Smoke)
		nLoc.Z += 100;

	this->AttachedSystem = (GameCreate<ParticleSystemClass>(
		pData->AttachedSystem.Get(),
		nLoc,
		pThis->GetCell(),
		nullptr,
		CoordStruct::Empty,
		pThis->GetOwningHouse()
	));
}

//Modified from Ares
const std::pair<bool, OwnerHouseKind> AnimExtData::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner)
{
	if (!pAnim || !pAnim->Type)
		return { false ,OwnerHouseKind::Default };

	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(pAnim->Type);
	if (!pTypeExt->NoOwner)
	{

		const auto Owner = pTypeExt->GetAnimOwnerHouseKind();

		if (Owner == OwnerHouseKind::Invoker && !pInvoker || Owner == OwnerHouseKind::Victim && !pVictim)
			return { false , OwnerHouseKind::Default };

		const auto newOwner = HouseExtData::GetHouseKind(Owner, true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim);

		if (!pAnim->Owner || pAnim->Owner != newOwner)
		{
			pAnim->SetHouse(newOwner);
			return { false , Owner };
		}
	}
	return { false , OwnerHouseKind::Default }; //yes return true
}

const std::pair<bool, OwnerHouseKind> AnimExtData::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, TechnoClass* pTechnoInvoker, bool defaultToVictimOwner, bool forceOwnership)
{
	if (!pAnim || !pAnim->Type)
		return { false ,OwnerHouseKind::Default };

	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(pAnim->Type);

	if (forceOwnership || !pTypeExt->NoOwner)
	{
		((FakeAnimClass*)pAnim)->_GetExtData()->Invoker = pTechnoInvoker;

		const auto Owner = pTypeExt->GetAnimOwnerHouseKind();

		if (Owner == OwnerHouseKind::Invoker && !pInvoker || Owner == OwnerHouseKind::Victim && !pVictim)
			return { false , OwnerHouseKind::Default };

		const auto newOwner = HouseExtData::GetHouseKind(Owner, true, defaultToVictimOwner ? pVictim : nullptr, pInvoker, pVictim);

		if (!pAnim->Owner || pAnim->Owner != newOwner)
		{
			pAnim->SetHouse(newOwner);
			return { false , Owner };
		}
	}

	return { false , OwnerHouseKind::Default };
}

TechnoClass* AnimExtData::GetTechnoInvoker(AnimClass* pThis)
{
	if (!AnimTypeExtContainer::Instance.Find(pThis->Type)->Damage_DealtByInvoker)
		return nullptr;

	if (pThis->OwnerObject)
	{
		switch (pThis->OwnerObject->WhatAmI())
		{
		case BuildingClass::AbsID:
		case UnitClass::AbsID:
		case InfantryClass::AbsID:
		case AircraftClass::AbsID:
			return static_cast<TechnoClass*>(pThis->OwnerObject);
		case BulletClass::AbsID:
			return static_cast<BulletClass*>(pThis->OwnerObject)->Owner;
		}
	}

	//additional behaviour 1
	auto const pExt = ((FakeAnimClass*)pThis)->_GetExtData();
	if (pExt->Invoker)
		return pExt->Invoker;

	//additional behaviour 2
	if (auto const pBullet = pThis->AttachedBullet)
		return pBullet->Owner;

	return nullptr;
}

Layer __fastcall AnimExtData::GetLayer_patch(AnimClass* pThis, void* _)
{
	if (!pThis->OwnerObject)
		return pThis->Type ? pThis->Type->Layer : Layer::Air;

	const auto pExt = AnimTypeExtContainer::Instance.Find(pThis->Type);

	if (!pExt || !pExt->Layer_UseObjectLayer.isset())
		return Layer::Ground;

	if (pExt->Layer_UseObjectLayer.Get())
	{
		if (auto const pFoot = flag_cast_to<FootClass* , false>(pThis->OwnerObject))
		{
			if (auto const pLocomotor = pFoot->Locomotor.GetInterfacePtr())
				return pLocomotor->In_Which_Layer();
		}
		else if (auto const pBullet = cast_to<BulletClass*, false>(pThis->OwnerObject))
			return pBullet->InWhichLayer();

		return pThis->OwnerObject->ObjectClass::InWhichLayer();
	}

	return pThis->Type ? pThis->Type->Layer : Layer::Air;
}

void AnimExtData::SpawnFireAnims(AnimClass* pThis)
{
	auto const pType = pThis->Type;
	auto const pExt = ((FakeAnimClass*)pThis)->_GetExtData();
	auto const pTypeExt = AnimTypeExtContainer::Instance.Find(pType);
	auto const coords = pThis->GetCoords();

	auto SpawnAnim = [&coords, pThis, pExt](AnimTypeClass* pType, int distance, bool constrainToCellSpots, bool attach)
		{
			if (!pType)
				return;

			CoordStruct newCoords = coords;

			if (distance > 0)
			{
				newCoords = MapClass::GetRandomCoordsNear(coords, distance, false);

				if (constrainToCellSpots)
					newCoords = MapClass::PickInfantrySublocation(newCoords, true);
			}

			auto const loopCount = ScenarioClass::Instance->Random.RandomRanged(1, 2);
			auto const pAnim = GameCreate<AnimClass>(pType, newCoords, 0, loopCount, 0x600u, 0, false);
			pAnim->Owner = pThis->Owner;

			if (attach && pThis->OwnerObject)
				pAnim->SetOwnerObject(pThis->OwnerObject);

			auto const pExtNew = ((FakeAnimClass*)pAnim)->_GetExtData();
			pExtNew->Invoker = pExt->Invoker ? pExt->Invoker : pExtNew->Invoker;
		};

	auto LoopAnims = [&coords, SpawnAnim](std::vector<AnimTypeClass*>* const anims, std::vector<double>* const chances, std::vector<double>* const distances,
		int count, AnimTypeClass* defaultAnimType, double defaultChance0, double defaultChanceRest, int defaultDistance0, int defaultDistanceRest, bool constrainToCellSpots, bool attach)
		{
			double chance = 0.0;
			int distance = 0;
			AnimTypeClass* pAnimType = nullptr;

			for (size_t i = 0; i < static_cast<unsigned int>(count); i++)
			{
				if (chances->size() > 0 && chances->size() > i)
					chance = (*chances)[i];
				else if (chances->size() > 0)
					chance = (*chances)[chances->size() - 1];
				else
					chance = i == 0 ? defaultChance0 : defaultChanceRest;

				if (chance < ScenarioClass::Instance->Random.RandomDouble())
					continue;

				if (anims->size() > 1)
					pAnimType = (*anims)[ScenarioClass::Instance->Random.RandomRanged(0, anims->size() - 1)];
				else if (anims->size() > 0)
					pAnimType = (*anims)[0];
				else
					pAnimType = defaultAnimType;

				if (distances->size() > 0 && distances->size() < i)
					distance = static_cast<int>((*distances)[i] * Unsorted::LeptonsPerCell);
				else if (distances->size() > 0)
					distance = static_cast<int>((*distances)[distances->size() - 1] * Unsorted::LeptonsPerCell);
				else
					distance = i == 0 ? defaultDistance0 : defaultDistanceRest;

				SpawnAnim(pAnimType, distance, constrainToCellSpots, attach);
			}
		};

	auto const disallowedLandTypes = pTypeExt->FireAnimDisallowedLandTypes.Get(pType->Scorch ? LandTypeFlags::Default : LandTypeFlags::None);

	if (IsLandTypeInFlags(disallowedLandTypes, pThis->GetCell()->LandType))
		return;

	std::vector<AnimTypeClass*>* anims = &pTypeExt->SmallFireAnims;
	std::vector<double>* chances = &pTypeExt->SmallFireChances;
	std::vector<double>* distances = &pTypeExt->SmallFireDistances;
	bool constrainToCellSpots = pTypeExt->ConstrainFireAnimsToCellSpots;
	bool attach = pTypeExt->AttachFireAnimsToParent.Get(pType->Scorch);
	int smallCount = pTypeExt->SmallFireCount.Get(1 + pType->Flamer);

	if (pType->Flamer)
	{
		LoopAnims(anims, chances, distances, smallCount, RulesClass::Instance->SmallFire, 0.5, 1.0, 64, 160, constrainToCellSpots, attach);

		anims = &pTypeExt->LargeFireAnims;
		chances = &pTypeExt->LargeFireChances;
		distances = &pTypeExt->LargeFireDistances;

		LoopAnims(anims, chances, distances, pTypeExt->LargeFireCount, RulesClass::Instance->LargeFire, 0.5, 0.5, 112, 112, constrainToCellSpots, attach);
	}
	else if (pType->Scorch)
	{
		LoopAnims(anims, chances, distances, smallCount, RulesClass::Instance->SmallFire, 1.0, 1.0, 0, 0, constrainToCellSpots, attach);
	}
}

// Changes type of anim in similar fashion to Next.
void AnimExtData::ChangeAnimType(AnimClass* pAnim, AnimTypeClass* pNewType, bool resetLoops, bool restart)
{
	double percentThrough = pAnim->Animation.Value / static_cast<double>(pAnim->Type->End);

	if (pNewType->End == -1)
	{
		pNewType->End = pNewType->GetImage()->Frames;
		if (pNewType->Shadow)
			pNewType->End /= 2;
	}

	if (pNewType->LoopEnd == -1)
	{
		pNewType->LoopEnd = pNewType->End;
	}

	pAnim->Type = pNewType;

	if (resetLoops)
		pAnim->RemainingIterations = static_cast<byte>(pNewType->LoopCount);

	pAnim->Accum = 0;
	pAnim->UnableToContinue = false;
	pAnim->Reverse = pNewType->Reverse;

	int rate = pNewType->Rate;
	if (pNewType->RandomRate.Min || pNewType->RandomRate.Max)
		rate = ScenarioClass::Instance->Random.RandomRanged(pNewType->RandomRate.Min, pNewType->RandomRate.Max);
	if (pNewType->Normalized)
		rate = GameOptionsClass::Instance->GetAnimSpeed(rate);

	pAnim->Animation.Start(rate, pNewType->Reverse ? -1 : 1);

	if (restart)
	{
		pAnim->Animation.Value = pNewType->Reverse ? pNewType->End : pNewType->Start;
		pAnim->Start();
	}
	else
	{
		pAnim->Animation.Value = static_cast<int>(pNewType->End * percentThrough);
	}

	const auto pExt = ((FakeAnimClass*)pAnim)->_GetExtData();
	const auto pTypeExt = AnimTypeExtContainer::Instance.Find(pNewType);

	if (pExt->AttachedSystem && pExt->AttachedSystem->Type != pTypeExt->AttachedSystem.Get())
		pExt->AttachedSystem = nullptr;

	if (!pExt->AttachedSystem && pTypeExt->AttachedSystem)
		pExt->CreateAttachedSystem();
}

// =============================
// load / save

template <typename T>
void AnimExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->BackupCoords)
		.Process(this->DeathUnitFacing)
		.Process(this->DeathUnitTurretFacing)
		.Process(this->Invoker, true)
		.Process(this->OwnerSet)
		.Process(this->AllowCreateUnit)
		.Process(this->WasOnBridge)
		.Process(this->AttachedSystem, true)
		.Process(this->ParentBuilding, true)
		.Process(this->CreateUnitLocation)
		.Process(this->SpawnsStatusData)
		;
}

// =============================
// hooks

#include <Misc/SyncLogging.h>

namespace CTORTemp
{
	CoordStruct coords;
	unsigned int callerAddress;
}

DEFINE_HOOK(0x421EA0, AnimClass_CTOR_SetContext, 0x6)
{
	GET_STACK(CoordStruct*, coords, 0x8);
	GET_STACK(unsigned int, callerAddress, 0x0);

	CTORTemp::coords = *coords;
	CTORTemp::callerAddress = callerAddress;

	return 0;
}


#ifdef TEST
#define GET_REGISTER_STATIC_2(type, dst, reg) static type dst; _asm { mov dst, reg }

[[ noreturn ]] static NOINLINE NAKED void AnimClass_DTOR_Ext() noexcept {
	GET_REGISTER_STATIC_2(AnimClass*, this_ptr, esi);
	if (!this_ptr->Type) {
		goto original_code;
	}

	FakeAnimClass::Remove(this_ptr);

original_code:
	_asm { mov eax, ds:0xA8E9A0 } // GameActive
	JMP_REG(ebx, 0x422912);
}

[[ noreturn ]] static NOINLINE NAKED void AnimClass_CTOR_Ext() noexcept {
	GET_REGISTER_STATIC_2(AnimClass*, this_ptr, esi); // Current "this" pointer.

	if (Phobos::Otamaa::DoingLoadGame || !this_ptr->Type)
		goto original_code;

	// Do this here instead of using a duplicate hook in SyncLogger.cpp
	if (!SyncLogger::HooksDisabled && this_ptr->UniqueID != -2)
		SyncLogger::AddAnimCreationSyncLogEvent(CTORTemp::coords, CTORTemp::callerAddress);

	if (this_ptr->UniqueID == -2)
	{
		Debug::Log("Anim[%s - %x] with some weird ID\n", this_ptr->Type->ID, this_ptr);
	}

	FakeAnimClass::ClearExtAttribute(this_ptr);

	if (AnimExtData* val = FakeAnimClass::AllocateUnchecked(this_ptr))
	{
		FakeAnimClass::SetExtAttribute(this_ptr, val);

		// Something about creating this in constructor messes with debris anims, so it has to be done for them later.
		if (!this_ptr->HasExtras)
			val->CreateAttachedSystem();
	}



original_code:
	/**
	 *  Stolen bytes/code.
	 */
	this_ptr->IsAlive = true;

	/**
	 *  Restore some registers.
	 */
	_asm { mov ecx, this_ptr }
	_asm { mov edx, [ecx + 0xC8] } // this->Class
	_asm { mov eax, edx }

	JMP_REG(edx, 0x4220B7);
}

#undef GET_REGISTER_STATIC_2
DEFINE_JUMP(LJMP, 0x4220AA, MiscTools::to_DWORD(&AnimClass_CTOR_Ext));
DEFINE_JUMP(LJMP, 0x42290B, MiscTools::to_DWORD(&AnimClass_DTOR_Ext));

//DEFINE_HOOK(0x422A52, AnimClass_DTOR, 0x6)
//{
//	GET(AnimClass* const, pItem, ESI);
//	FakeAnimClass::Remove(pItem);
//	return 0;
//}

#else
DEFINE_HOOK(0x422131, AnimClass_CTOR, 0x6)
{
	GET(AnimClass*, pItem, ESI);

	if (Phobos::Otamaa::DoingLoadGame)
		return 0x0;

	// Do this here instead of using a duplicate hook in SyncLogger.cpp
	if (!SyncLogger::HooksDisabled && pItem->UniqueID != -2)
		SyncLogger::AddAnimCreationSyncLogEvent(CTORTemp::coords, CTORTemp::callerAddress);

	if (pItem->UniqueID == -2)
	{
		Debug::Log("Anim[%s - %x] with some weird ID\n", pItem->Type->ID, pItem);
	}

	FakeAnimClass::ClearExtAttribute(pItem);

	if (AnimExtData* val = FakeAnimClass::AllocateUnchecked(pItem))
	{
		FakeAnimClass::SetExtAttribute(pItem, val);

		// Something about creating this in constructor messes with debris anims, so it has to be done for them later.
		if (!pItem->HasExtras)
			val->CreateAttachedSystem();
	}

	return 0;
}

DEFINE_HOOK(0x422A52, AnimClass_DTOR, 0x6)
{
	GET(AnimClass* const, pItem, ESI);
	FakeAnimClass::Remove(pItem);
	return 0;
}

#endif

HRESULT __stdcall FakeAnimClass::_Load(IStream* pStm)
{

	HRESULT res = this->AnimClass::Load(pStm);

	if (SUCCEEDED(res))
	{
		FakeAnimClass::ClearExtAttribute(this);
		auto buffer = FakeAnimClass::AllocateUnchecked(this);
		FakeAnimClass::SetExtAttribute(this, buffer);

		if (!buffer)
			return -1;

		PhobosByteStream loader { 0 };
		if (!loader.ReadBlockFromStream(pStm))
			return -1;

		PhobosStreamReader reader { loader };
		if (!reader.Expect(AnimExtData::Canary))
			return -1;

		reader.RegisterChange(buffer);
		buffer->LoadFromStream(reader);

		if (reader.ExpectEndOfBlock())
		{
			return S_OK;
		}
	}

	return res;
}

HRESULT __stdcall FakeAnimClass::_Save(IStream* pStm, bool clearDirty)
{

	HRESULT res = this->AnimClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
	{
		AnimExtData* const buffer = FakeAnimClass::GetExtAttribute(this);

		// write the current pointer, the size of the block, and the canary
		PhobosByteStream saver { AnimExtData::size_Of() };
		PhobosStreamWriter writer { saver };

		writer.Save(AnimExtData::Canary);
		writer.Save(buffer);

		// save the data
		buffer->SaveToStream(writer);

		// save the block
		if (!saver.WriteBlockToStream(pStm))
		{
			//Debug::Log("[SaveGame] FakeAnimClass fail to write 0x%X block(s) to stream\n", saver.Size());
			return -1;
		}

		//Debug::Log("[SaveGame] FakeAnimClass used up 0x%X bytes\n", saver.Size());
	}

	return res;
}

DEFINE_JUMP(VTABLE, 0x7E3368, MiscTools::to_DWORD(&FakeAnimClass::_Load))
DEFINE_JUMP(VTABLE, 0x7E336C, MiscTools::to_DWORD(&FakeAnimClass::_Save))

DEFINE_HOOK(0x425164, AnimClass_Detach, 0x6)
{
	GET(FakeAnimClass* const, pThis, ESI);
	GET(AbstractClass*, target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	if(auto pExt = pThis->_GetExtData())
		pExt->InvalidatePointer(target, all);

	R->EBX(0);

	if(pThis->OwnerObject == target){

		if(!target)
			return 0x4251A3;

		if (auto const pTechno = flag_cast_to<TechnoClass* , false>(target)) {
			if (TechnoExtContainer::Instance.Find(pTechno)->IsDetachingForCloak)
				return 0x4251A3;
		}

		return 0x425174;
	}

	return 0x4251A3;
}

DEFINE_JUMP(VTABLE, 0x7E3390, MiscTools::to_DWORD(&FakeAnimClass::_GetOwningHouse));