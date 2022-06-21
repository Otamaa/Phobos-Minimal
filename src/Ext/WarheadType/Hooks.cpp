#include "Body.h"

#include <BulletClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#pragma region DETONATION

bool DetonationInDamageArea = true;

DEFINE_HOOK(0x46920B, BulletClass_Logics, 0x6)
{
	GET(BulletClass* const, pThis, ESI);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH))
	{
		GET_BASE(const CoordStruct*, pCoords, 0x8);

		auto const pExt = BulletExt::GetExtData(pThis);
		auto const pTechno = pThis ? pThis->Owner : nullptr;
		auto const pHouse = pTechno ? pTechno->Owner : pExt && pExt->Owner ? pExt->Owner :nullptr;

		pWHExt->Detonate(pTechno, pHouse, pThis, *pCoords);
	}

	DetonationInDamageArea = false;

	return 0;
}

DEFINE_HOOK(0x46A290, BulletClass_Logics_Return, 0x5)
{
	DetonationInDamageArea = true;
	return 0;
}

DEFINE_HOOK(0x489286, MapClass_DamageArea, 0x6)
{
	GET_BASE(const WarheadTypeClass*, pWH, 0x0C);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
	{
		// GET(const int, Damage, EDX);
		// GET_BASE(const bool, AffectsTiberium, 0x10);
		GET(const CoordStruct*, pCoords, ECX);
		GET_BASE(TechnoClass*, pOwner, 0x08);
		GET_BASE(HouseClass*, pHouse, 0x14);

		Point2D screenCoords;
		bool const ShakeAllow = pWHExt->ShakeIsLocal ? TacticalClass::Instance->CoordsToClient(pCoords, &screenCoords) : true;

		if (ShakeAllow)
		{
			if (pWH->ShakeXhi || pWH->ShakeXlo)
				Map.ScreenShakeX = abs(Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeXhi, pWH->ShakeXlo));

			if (pWH->ShakeYhi || pWH->ShakeYlo)
				Map.ScreenShakeY = abs(Random2Class::NonCriticalRandomNumber->RandomRanged(pWH->ShakeYhi, pWH->ShakeYlo));
		}

		auto const pDecidedOwner = !pHouse && pOwner ? pOwner->Owner : pHouse;

		if (!pWHExt->Launchs.empty())
		{
			for (size_t i = 0; i < pWHExt->Launchs.size(); i++)
			{
				Debug::Log("MapClass_DetonateArea_Executed [%s] SW[%d] ! \n", pWH->get_ID() ,i);
				LauchSWData const Lauch = pWHExt->Launchs[i];

				if (Lauch.LaunchWhat)
				{
					Helpers::Otamaa::LauchSW(Lauch.LaunchWhat,
						pDecidedOwner, *pCoords, Lauch.LaunchWaitcharge,
						Lauch.LaunchResetCharge,
						Lauch.LaunchGrant,
						Lauch.LaunchGrant_RepaintSidebar,
						Lauch.LaunchGrant_OneTime,
						Lauch.LaunchGrant_OnHold,
						Lauch.LaunchSW_Manual,
						Lauch.LaunchSW_IgnoreInhibitors,
						Lauch.LauchSW_IgnoreMoney
					);
				}
			}
		}

		if (DetonationInDamageArea)
			pWHExt->Detonate(pOwner, pDecidedOwner, nullptr, *pCoords);
	}

	return 0;
}

#pragma endregion

DEFINE_HOOK(0x48A551, WarheadTypeClass_AnimList_SplashList, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt && pWHExt->SplashList.size())
	{
		GET(int, nDamage, ECX);
		int idx = pWHExt->SplashList_PickRandom ?
			ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->SplashList.size() - 1) :
			std::min(pWHExt->SplashList.size() * 35 - 1, (size_t)nDamage) / 35;
		R->EAX(pWHExt->SplashList[idx]);
		return 0x48A5AD;
	}

	return 0;
}

DEFINE_HOOK(0x48A5BD, WarheadTypeClass_AnimList_PickRandom, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	return pWHExt && pWHExt->AnimList_PickRandom ? 0x48A5C7 : 0;
}

DEFINE_HOOK(0x48A5B3, WarheadTypeClass_AnimList_CritAnim, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt && pWHExt->HasCrit && pWHExt->Crit_AnimList.size() && !pWHExt->Crit_AnimOnAffectedTargets)
	{
		GET(int, nDamage, ECX);
		int idx = pThis->EMEffect || pWHExt->Crit_AnimList_PickRandom.Get(pWHExt->AnimList_PickRandom) ?
			ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->Crit_AnimList.size() - 1) :
			std::min(pWHExt->Crit_AnimList.size() * 25 - 1, (size_t)nDamage) / 25;
		R->EAX(pWHExt->Crit_AnimList[idx]);
		return 0x48A5AD;
	}

	return 0;
}

DEFINE_HOOK(0x4896EC, Explosion_Damage_DamageSelf, 0x6)
{
	enum { SkipCheck = 0x489702 };

	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead)) {
		if (pWHExt->AllowDamageOnSelf.isset() && pWHExt->AllowDamageOnSelf.Get())
			return SkipCheck;
	}

	return 0;
}

DEFINE_HOOK(0x469008, BulletClass_Explode_Cluster, 0x8)
{
	enum { SkipGameCode = 0x469091 };

	GET(BulletClass*, pThis, ESI);
	GET_STACK(CoordStruct, origCoords, STACK_OFFS(0x3C, 0x30));

	if (pThis->Type->Cluster > 0)
	{
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
		{
			int min = pTypeExt->Cluster_Scatter_Min.Get(Leptons(256));
			int max = pTypeExt->Cluster_Scatter_Max.Get(Leptons(512));
			auto coords = origCoords;

			for (int i = 0; i < pThis->Type->Cluster; i++)
			{
				pThis->Detonate(coords);

				if (!pThis->IsAlive)
					break;

				int distance = ScenarioClass::Instance->Random.RandomRanged(min, max);
				coords = MapClass::GetRandomCoordsNear(origCoords, distance, false);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x4687F8, BulletClass_Unlimbo_FlakScatter, 0x6)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(float, mult, STACK_OFFS(0x5C, 0x44));

	if (pThis->WeaponType)
	{
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
		{
			int defaultmax = RulesClass::Instance->BallisticScatter;
			int min = pTypeExt->BallisticScatter_Min.Get(Leptons(0));
			int max = pTypeExt->BallisticScatter_Max.Get(Leptons(defaultmax));

			int result = static_cast<int>((mult * ScenarioClass::Instance->Random.RandomRanged(2 * min, 2 * max)) / pThis->WeaponType->Range);
			R->EAX(result);
		}
	}

	return 0;
}

DEFINE_HOOK(0x469D1A, BulletClass_Logics_Debris_Checks, 0x6)
{
	enum { SkipGameCode = 0x469EBA, SetDebrisCount = 0x469D36 };

	GET(BulletClass*, pThis, ESI);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	bool isLand = pThis->GetCell()->LandType != LandType::Water || pThis->GetCell()->ContainsBridge();

	if (pWHExt && !isLand && pWHExt->Debris_Conventional.Get())
		return SkipGameCode;

	// Fix the debris count to be in range of Min, Max instead of Min, Max-1.
	R->EBX(ScenarioClass::Instance->Random.RandomRanged(pThis->WH->MinDebris, pThis->WH->MaxDebris));

	return SetDebrisCount;
}

#pragma region Otamaa

static DWORD Do_Airburst(BulletClass* pThis)
{
	auto pType = pThis->Type;

	auto pExt = BulletTypeExt::ExtMap.Find(pType);

	if (pExt->HasSplitBehavior())
	{
		auto const GetWeapon = [pExt, pType]()
		{
			if (pExt->AirburstWeapons.empty())
				return pType->AirburstWeapon;

			return pExt->AirburstWeapons[ScenarioGlobal->Random.RandomRanged(0, pExt->AirburstWeapons.size() - 1)];
		};

		if (WeaponTypeClass* pWeapon = GetWeapon())
		{
			auto pBulletExt = BulletExt::GetExtData(pThis);
			TechnoClass* pBulletOwner = pThis->Owner ? pThis->Owner : nullptr;
			HouseClass* pBulletHouseOwner = pBulletOwner ? pBulletOwner->GetOwningHouse() : (pBulletExt ? pBulletExt->Owner : nullptr);

			auto& random = ScenarioClass::Instance->Random;

			// some defaults
			int cluster = pType->Cluster;

			// get final target coords and cell
			CoordStruct crdDest = pExt->AroundTarget.Get(pExt->Splits.Get())
				? pThis->GetTargetCoords() : pThis->GetCoords();

			CellStruct cellDest = CellClass::Coord2Cell(crdDest);

			// create a list of cluster targets
			DynamicVectorClass<AbstractClass*> targets;

			if (!pExt->Splits.Get())
			{
				// default hardcoded YR way: hit each cell around the destination once

				// fill target list with cells around the target
				CellRangeIterator<CellClass>{}(cellDest, pExt->AirburstSpread.Get(),
					[&targets](CellClass* const pCell) -> bool
				{
					targets.AddItem(pCell);
					return true;
				});

				// we want as many as we get, not more, not less
				cluster = targets.Size();

			}
			else
			{
				auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

				// fill with technos in range
				for (auto pTechno : *TechnoClass::Array)
				{
					if (pTechno->IsInPlayfield && pTechno->IsOnMap && pTechno->Health > 0)
					{
						if ((!pExt->RetargetOwner.Get() && pTechno == pBulletOwner))
							continue;

						if (pWHExt->CanDealDamage(pTechno) && pWHExt->CanTargetHouse(pBulletHouseOwner, pTechno))
						{
							if (pTechno->InWhichLayer() == Layer::Underground ||
								pTechno->InWhichLayer() == Layer::None)
								continue;

							CoordStruct crdTechno = pTechno->GetCoords();
							if (crdDest.DistanceFrom(crdTechno) < pExt->Splits_Range.Get()
								&& ((!pTechno->IsInAir() && pWeapon->Projectile->AG) || (pTechno->IsInAir() && pWeapon->Projectile->AA))
								)
							{
								//if (targets.FindItemIndex(pTechno) != -1 || pThis->Target == pTechno)
								//	continue;
								//else
								targets.AddItem(pTechno);
							}
						}
					}
				}

				// fill up the list to cluster count with random cells around destination
				const int nMinRange = pExt->Splits_RandomCellUseHarcodedRange.Get() ? 3 : pWeapon->MinimumRange / Unsorted::LeptonsPerCell;
				const int nMaxRange = pExt->Splits_RandomCellUseHarcodedRange.Get() ? 3 : pWeapon->Range / Unsorted::LeptonsPerCell;

				while (targets.Count < (cluster)) {
					int x = random.RandomRanged(-nMinRange, nMaxRange);
					int y = random.RandomRanged(-nMinRange, nMaxRange);

					CellStruct cell = { static_cast<short>(cellDest.X + x), static_cast<short>(cellDest.Y + y) };
					CellClass* pCell = MapClass::Instance->GetCellAt(cell);

					targets.AddItem(pCell);
				}
			}

			// let it rain warheads
			for (int i = 0; i < cluster; ++i) {
				AbstractClass* pTarget = pThis->Target;

				if (!pExt->Splits) {
					// simple iteration
					pTarget = targets.GetItem(i);

				} else if (!pTarget || pExt->RetargetAccuracy < random.RandomDouble()) {
					// select another target randomly
					int index = random.RandomRanged(0, targets.Count - 1);
					pTarget = targets.GetItem(index);

					// firer would hit itself
					if (pTarget == pThis->Owner) {
						if (random.RandomDouble() > 0.5) {
							index = random.RandomRanged(0, targets.Count - 1);
							pTarget = targets.GetItem(index);
						}
					}

					// remove this target from the list
					targets.RemoveItem(index);
				}

				if (pTarget) {
					auto pSplitExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);

					if (auto pBullet = pSplitExt->CreateBullet(pTarget, pThis->Owner, pWeapon)) {
						pBullet->SetWeaponType(pWeapon);
						DirStruct const dir(5, static_cast<short>(random.RandomRanged(0, 31)));
						auto const radians = dir.radians();

						auto const sin_rad = Math::sin(radians);
						auto const cos_rad = Math::cos(radians);
						auto const cos_factor = -2.44921270764e-16;
						auto const flatSpeed = cos_factor * pBullet->Speed;

						VelocityClass velocity;
						velocity.X = cos_rad * flatSpeed;
						velocity.Y = sin_rad * flatSpeed;
						velocity.Z = -pBullet->Speed;

						pBullet->MoveTo(pThis->Location, velocity);

#ifdef COMPILE_PORTED_DP_FEATURES
						auto sourcePos = pThis->Location;
						auto targetPos = pTarget->GetCoords();

						// Draw bullet effect
						Helpers_DP::DrawBulletEffect(pWeapon, sourcePos, targetPos, pBulletOwner, pTarget);
						// Draw particle system
						Helpers_DP::AttachedParticleSystem(pWeapon, sourcePos, pTarget, pBulletOwner, targetPos);
						// Play report sound
						Helpers_DP::PlayReportSound(pWeapon, sourcePos);
						// Draw weapon anim
						Helpers_DP::DrawWeaponAnim(pWeapon, sourcePos, targetPos, pBulletOwner, pTarget);
#endif
					}
				}
			}
		}
	}

	return 0x46A290;
}

DEFINE_HOOK(0x469D12, BulletClass_Logics_CheckDoAirburst_MaxDebris, 0x8)
{
	GET(BulletClass*, pThis, ESI);
	GET(int, nMaxCount, EAX);

	return (nMaxCount > 0) ? 0x469D1A : Do_Airburst(pThis);
}

DEFINE_HOOK(0x469D3C, BulletClass_Logics_Debris, 0xA)
{
	GET(BulletClass*, pThis, ESI);
	GET(int, nTotalSpawn, EBX);
	GET(WarheadTypeClass*, pWarhead, EAX);

	auto pExt = BulletExt::GetExtData(pThis);
	HouseClass* const pOWner = pThis->Owner ? pThis->Owner->GetOwningHouse() : (pExt && pExt->Owner ? pExt->Owner :HouseExt::FindCivilianSide());
	HouseClass* const Victim = (pThis->Target) ? pThis->Target->GetOwningHouse() : nullptr;
	CoordStruct nCoords { 0,0,0 };

	if (pWarhead->DebrisTypes.Count > 0 && pWarhead->DebrisMaximums.Count > 0) {
		nCoords = pThis->GetCoords();
		for (int nCurIdx = 0; nCurIdx < pWarhead->DebrisTypes.Count; ++nCurIdx) {
			if (pWarhead->DebrisMaximums[nCurIdx] > 0) {
				int nAmountToSpawn = abs(int(ScenarioGlobal->Random.Random())) % pWarhead->DebrisMaximums[nCurIdx] + 1;
				nAmountToSpawn = Math::LessOrEqualTo(nAmountToSpawn, nTotalSpawn);
				nTotalSpawn -= nAmountToSpawn;

				for (; nAmountToSpawn > 0; --nAmountToSpawn) {
					if (auto const pVoxelAnimType = pWarhead->DebrisTypes[nCurIdx])
						if (auto pVoxAnim = GameCreate<VoxelAnimClass>(pVoxelAnimType, &nCoords, pOWner))
							VoxelAnimExt::Invokers[pVoxAnim] = pThis->Owner;
				}
			}

			if (nTotalSpawn <= 0) {
				nTotalSpawn = 0;
				break;
			}
		}
	}

	if (!pWarhead->DebrisTypes.Count && (nTotalSpawn > 0)) {
		const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
		const auto AnimDebris = pWHExt->DebrisAnimTypes.GetElements(RulesClass::Instance->MetallicDebris);

		if (!AnimDebris.empty()) {
			nCoords = pThis->GetCoords();
			nCoords.Z += 20;

			for (int i = 0; i < nTotalSpawn; ++i) {
				if (auto const pAnimType = AnimDebris[ScenarioClass::Instance->Random(0, AnimDebris.size() - 1)]) {
					if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoords)) {
						AnimExt::SetAnimOwnerHouseKind(pAnim, pOWner, Victim, false);
						if (auto const pAnimExt = AnimExt::GetExtData(pAnim))
							pAnimExt->Invoker = pThis->Owner;
					}
				}
			}
		}
	}

	return Do_Airburst(pThis);
}
#pragma endregion