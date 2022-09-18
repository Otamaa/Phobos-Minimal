#include "Body.h"

#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>

#include <TechnoClass.h>
#include <TacticalClass.h>

#include <Fundamentals.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#endif

#pragma region Otamaa

DEFINE_HOOK(0x46889D, BulletClass_Unlimbo_FlakScatter_SetTargetCoords, 0x8)
{
	GET(BulletClass*, pThis, EBX);
	pThis->TargetCoords = { R->EAX<int>(),R->EDX<int>(),R->ESI<int>() };
	return 0x0;
}

DEFINE_HOOK(0x46B1D6, BulletClass_DrawVXL_Palette, 0x6)
{
	GET_STACK(BulletClass*, pThis, STACK_OFFS(0xF8, 0xE4));
	GET(BulletTypeClass*, pThisType, EDX);
	GET(Point2D*, pPoint, ECX);
	GET(int, nRect_X, EBP);
	GET(int, nRect_Y, ESI);

	R->Stack(STACK_OFFS(0xF8, 0xE4), Point2D { pPoint->X + nRect_X , pPoint->Y + nRect_Y });
	R->EAX(ColorScheme::Array()->Items);

	int nIdx = pThisType->Color;
	if (pThisType->FirersPalette && (pThis->InheritedColor != -1))
		nIdx = pThis->InheritedColor;

	R->ECX(nIdx);

	return 0x46B1F2;
}

DEFINE_HOOK(0x5F5A86, ObjectClass_SpawnParachuted_Animation_Bulet, 0x6)
{
	GET(RulesClass*, pRules, ECX);
	GET(BulletClass*, pBullet, ESI);

	if (auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find<true>(pBullet->Type))
	{
		R->EDX(pBulletTypeExt->Parachute.Get(pRules->BombParachute));
		return 0x5F5A8C;
	}

	return 0x0;
}

#pragma region Otamaa

static DWORD Do_Airburst(BulletClass* pThis)
{
	const auto pType = pThis->Type;
	const auto pExt = BulletTypeExt::ExtMap.Find(pType);

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
			const auto pBulletExt = BulletExt::ExtMap.Find(pThis);
			TechnoClass* pBulletOwner = pThis->Owner ? pThis->Owner : nullptr;
			HouseClass* pBulletHouseOwner = pBulletOwner ? pBulletOwner->GetOwningHouse() : (pBulletExt ? pBulletExt->Owner : nullptr);

			auto& random = ScenarioClass::Instance->Random;

			// some defaults
			int cluster = pType->Cluster;

			// get final target coords and cell
			const CoordStruct crdDest = pExt->AroundTarget.Get(pExt->Splits.Get())
				? pThis->GetTargetCoords() : pThis->GetCoords();

			const CellStruct cellDest = CellClass::Coord2Cell(crdDest);

			// create a list of cluster targets
			std::vector<AbstractClass*> targets;

			if (!pExt->Splits.Get())
			{
				// default hardcoded YR way: hit each cell around the destination once

				// fill target list with cells around the target
				CellRangeIterator<CellClass>{}(cellDest, pExt->AirburstSpread.Get(),
					[&targets](CellClass* const pCell) -> bool
				{
					targets.push_back(pCell);
					return true;
				});

				// we want as many as we get, not more, not less
				cluster = (int)targets.size();

			}
			else
			{
				const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

				// fill with technos in range
				std::for_each(TechnoClass::Array->begin(), TechnoClass::Array->end(), [&](TechnoClass* pTechno)
				{
					if (pTechno->IsInPlayfield && pTechno->IsOnMap && pTechno->Health > 0)
					{
						if ((!pExt->RetargetOwner.Get() && pTechno == pBulletOwner))
							return;

						if (pExt->Splits_TargetingUseVerses.Get() && !pWHExt->CanDealDamage(pTechno))
							return;

						if (pWHExt->CanTargetHouse(pBulletHouseOwner, pTechno))
						{
							const auto nLayer = pTechno->InWhichLayer();
							if (nLayer == Layer::Underground ||
								nLayer == Layer::None)
								return;

							const CoordStruct crdTechno = pTechno->GetCoords();
							if (crdDest.DistanceFrom(crdTechno) < pExt->Splits_Range.Get()
								&& ((!pTechno->IsInAir() && pWeapon->Projectile->AG) || (pTechno->IsInAir() && pWeapon->Projectile->AA))
								)
							{
								targets.push_back(pTechno);
							}
						}
					}
				});

				// fill up the list to cluster count with random cells around destination
				const int nMinRange = pExt->Splits_RandomCellUseHarcodedRange.Get() ? 3 : pWeapon->MinimumRange / Unsorted::LeptonsPerCell;
				const int nMaxRange = pExt->Splits_RandomCellUseHarcodedRange.Get() ? 3 : pWeapon->Range / Unsorted::LeptonsPerCell;

				while ((int)targets.size() < (cluster))
				{
					int x = random.RandomRanged(-nMinRange, nMaxRange);
					int y = random.RandomRanged(-nMinRange, nMaxRange);

					CellStruct cell = { static_cast<short>(cellDest.X + x), static_cast<short>(cellDest.Y + y) };
					targets.push_back(MapClass::Instance->GetCellAt(cell));
				}
			}

			// let it rain warheads
			for (int i = 0; i < cluster; ++i)
			{
				AbstractClass* pTarget = pThis->Target;

				if (!pExt->Splits)
				{
					// simple iteration
					pTarget = targets[i];

				}
				else if (!pTarget || pExt->RetargetAccuracy < random.RandomDouble())
				{
					// select another target randomly
					int index = random.RandomRanged(0, targets.size() - 1);
					pTarget = targets[index];

					// firer would hit itself
					if (pTarget == pThis->Owner)
					{
						if (random.RandomDouble() > 0.5)
						{
							index = random.RandomRanged(0, targets.size() - 1);
							pTarget = targets[index];
						}
					}

					// remove this target from the list
					targets.erase(targets.begin() + index);
				}

				if (pTarget)
				{
#ifdef DEBUG_AIRBURSTSPLITS_TARGETING
					if (const auto pTechno = generic_cast<TechnoClass*>(pTarget))
						Debug::Log("Airburst [%s] targeting Target [%s] \n", pWeapon->get_ID(), pTechno->get_ID());
#endif
					const auto pSplitExt = BulletTypeExt::ExtMap.Find(pWeapon->Projectile);
					if (const auto pBullet = pSplitExt->CreateBullet(pTarget, pThis->Owner, pWeapon))
					{
						pBullet->SetWeaponType(pWeapon);
						DirStruct const dir(5, random.RandomRangedSpecific<short>(0, 31));
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

	auto pExt = BulletExt::ExtMap.Find(pThis);
	HouseClass* const pOWner = pThis->Owner ? pThis->Owner->GetOwningHouse() : (pExt && pExt->Owner ? pExt->Owner : nullptr);
	HouseClass* const Victim = (pThis->Target) ? pThis->Target->GetOwningHouse() : nullptr;
	CoordStruct nCoords { 0,0,0 };

	if (pWarhead->DebrisTypes.Count > 0 && pWarhead->DebrisMaximums.Count > 0)
	{
		nCoords = pThis->GetCoords();
		for (int nCurIdx = 0; nCurIdx < pWarhead->DebrisTypes.Count; ++nCurIdx)
		{
			if (pWarhead->DebrisMaximums[nCurIdx] > 0)
			{
				int nAmountToSpawn = abs(int(ScenarioGlobal->Random.Random())) % pWarhead->DebrisMaximums[nCurIdx] + 1;
				nAmountToSpawn = Math::LessOrEqualTo(nAmountToSpawn, nTotalSpawn);
				nTotalSpawn -= nAmountToSpawn;

				for (; nAmountToSpawn > 0; --nAmountToSpawn)
				{
					if (auto const pVoxelAnimType = pWarhead->DebrisTypes[nCurIdx])
						if (auto pVoxAnim = GameCreate<VoxelAnimClass>(pVoxelAnimType, &nCoords, pOWner))
							if (auto pVoxExt = VoxelAnimExt::ExtMap.Find(pVoxAnim))
								pVoxExt->Invoker = pThis->Owner;
				}
			}

			if (nTotalSpawn <= 0)
			{
				nTotalSpawn = 0;
				break;
			}
		}
	}

	if (!pWarhead->DebrisTypes.Count && (nTotalSpawn > 0))
	{
		const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);
		const auto AnimDebris = pWHExt->DebrisAnimTypes.GetElements(RulesClass::Instance->MetallicDebris);

		if (!AnimDebris.empty())
		{
			nCoords = pThis->GetCoords();
			nCoords.Z += 20;

			for (int i = 0; i < nTotalSpawn; ++i)
			{
				if (auto const pAnimType = AnimDebris[ScenarioClass::Instance->Random(0, AnimDebris.size() - 1)])
				{
					if (auto pAnim = GameCreate<AnimClass>(pAnimType, nCoords))
					{
						AnimExt::SetAnimOwnerHouseKind(pAnim, pOWner, Victim, pThis->Owner, false);
					}
				}
			}
		}
	}

	return Do_Airburst(pThis);
}
#pragma endregion


template<bool Reverse =false>
static __forceinline VelocityClass GenerateVelocity(BulletClass* pThis, AbstractClass* pTarget, const int nSpeed)
{
	VelocityClass velocity { 0.0,0.0,0.0 };
	//inline get Direction from 2 coords
	CoordStruct const nCenter = pTarget->GetCoords();
	//this->radians(Math::atan2(a, b));
	DirStruct const dir_fromXY((double)(pThis->Location.Y - nCenter.Y), (double)(pThis->Location.X - nCenter.X));

	if (velocity.X == 0.0 && velocity.Y == 0.0)
	{
		velocity.X = 100.0;
	}

	double const nFirstMag = velocity.MagnitudeXY();
	double const radians_fromXY = dir_fromXY.radians();
	double const sin_rad = Math::sin(radians_fromXY);
	double const cos_rad = Math::cos(radians_fromXY);

	velocity.X = cos_rad * nFirstMag;
	velocity.Y -= sin_rad * nFirstMag;

	if constexpr (!Reverse)
	{
		double const nSecMag = velocity.MagnitudeXY();
		DirStruct const dir_forZ(velocity.Z, nSecMag);
		double const radians_foZ = dir_forZ.radians();

		double const nThirdMag = velocity.MagnitudeXY();
		if (radians_foZ != 0.0)
		{
			velocity.X /= Math::cos(radians_foZ);
			velocity.Y /= Math::cos(radians_foZ);
		}

		double const nMult_Cos = Math::cos(0.7853262558535721);
		double const nMult_Sin = Math::sin(0.7853262558535721);
		velocity.X *= nMult_Cos;
		velocity.Y *= nMult_Cos;
		velocity.Z *= nMult_Sin * nThirdMag;

		if (velocity.X == 0.0 && velocity.Y == 0.0 && velocity.Z == 0.0)
		{
			velocity.X = 100.0;
		}

		const double nFullMag = velocity.Magnitude();
		const double nDevidedBySpeed = nSpeed / nFullMag;
		velocity *= nDevidedBySpeed;
	}
	else
	{
		const double nFullMag = velocity.Magnitude();
		const double nDevidedBySpeed = nSpeed / nFullMag;
		velocity *= nDevidedBySpeed;

		double const nSecMag = velocity.MagnitudeXY();
		DirStruct const dir_forZ(velocity.Z, nSecMag);
		double const radians_foZ = dir_forZ.radians();

		double const nThirdMag = velocity.MagnitudeXY();
		if (radians_foZ != 0.0)
		{
			velocity.X /= Math::cos(radians_foZ);
			velocity.Y /= Math::cos(radians_foZ);
		}

		double const nMult_Cos = Math::cos(0.7853262558535721);
		double const nMult_Sin = Math::sin(0.7853262558535721);
		velocity.X *= nMult_Cos;
		velocity.Y *= nMult_Cos;
		velocity.Z *= nMult_Sin * nThirdMag;

		if (velocity.X == 0.0 && velocity.Y == 0.0 && velocity.Z == 0.0)
		{
			velocity.X = 100.0;
		}
	}

	return velocity;
}

void ShrapnelsExec(BulletClass* pThis)
{
	auto const pType = pThis->Type;
	auto const pShrapWeapon = pType->ShrapnelWeapon;

	if (!pShrapWeapon)
		return;

	int nCount = pType->ShrapnelCount;


	if (nCount < 0)
	{
		if (auto pOwner = pThis->Owner)
		{
			auto nRes = (-pType->ShrapnelCount) - static_cast<int>(pOwner->GetCoords().DistanceFrom(pThis->Location) / 256.0);
			if (nRes <= 0)
				return;

			nCount = nRes;
		}
		else
		{
			nCount = 3;
		}
	}

	auto const pBulletCell = pThis->GetCell();
	if (auto pFirstOccupy = pBulletCell->FirstObject)
	{
		if (pFirstOccupy->WhatAmI() != AbstractType::Building)
		{
			auto& random = ScenarioClass::Instance->Random;
			auto const nRange = (random.RandomRanged(pShrapWeapon->MinimumRange , pShrapWeapon->Range) / 256);

			if (nRange >= 1)
			{
				int nTotal = 0;
				auto cell = pThis->GetMapCoords();

				for (size_t direction = 0; direction <= 7; direction += 2)
				{
					CellStruct directionOffset = CellSpread::GetNeighbourOffset(direction); // coordinates of the neighboring cell in the given direction relative to the current cell (e.g. 0,1)
					CellStruct cellToCheck = cell;
					for (short distanceFromCenter = 1; distanceFromCenter <= nRange; ++distanceFromCenter)
					{
						cellToCheck += directionOffset; // adjust the cell to check based on current distance, relative to the selected cell
						CellClass* pCell = MapClass::Instance->TryGetCellAt(cellToCheck);
						if (!pCell) { // don't parse this cell if it doesn't exist (duh)
							break;
						}

						auto const pTarget = pCell->FirstObject;
						if (pTarget && pTarget != pThis->Owner && pThis->Owner && !pThis->Owner->Owner->IsAlliedWith(pTarget)
							)
						{
							auto pSplitExt = BulletTypeExt::ExtMap.Find(pShrapWeapon->Projectile);

							if (auto pBullet = pSplitExt->CreateBullet(pTarget, pThis->Owner, pShrapWeapon))
							{
								VelocityClass velocity = GenerateVelocity(pThis, pTarget, pShrapWeapon->Speed);
								pBullet->MoveTo(pThis->Location, velocity);
#ifdef COMPILE_PORTED_DP_FEATURES
								auto sourcePos = pThis->Location;
								auto targetPos = pTarget->GetCoords();

								// Draw bullet effect
								Helpers_DP::DrawBulletEffect(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);
								// Draw particle system
								Helpers_DP::AttachedParticleSystem(pShrapWeapon, sourcePos, pTarget, pThis->Owner, targetPos);
								// Play report sound
								Helpers_DP::PlayReportSound(pShrapWeapon, sourcePos);
								// Draw weapon anim
								Helpers_DP::DrawWeaponAnim(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);
#endif
							}

							//escapes
							if (++nTotal > nCount)
								return;
						}
					}
				}

				// get random coords for last remaining shrapnel if the total still less than ncount
				if (nTotal < nCount)
				{
					for (int nAmountHere = nCount - nTotal; nAmountHere > 0; --nAmountHere)
					{
						const int nX = ScenarioClass::Instance->Random.RandomRanged(-2, 2);
						const int nY = ScenarioClass::Instance->Random.RandomRanged(-2, 2);
						CellStruct nNextCoord = pThis->GetMapCoords();
						nNextCoord.X += nX;
						nNextCoord.Y += nY;
						if (auto pTarget = MapClass::Instance->TryGetCellAt(nNextCoord))
						{
							auto pSplitExt = BulletTypeExt::ExtMap.Find(pShrapWeapon->Projectile);

							if (auto pBullet = pSplitExt->CreateBullet(pTarget, pThis->Owner, pShrapWeapon))
							{
								VelocityClass velocity = GenerateVelocity<true>(pThis, pTarget, pShrapWeapon->Speed);
								pBullet->MoveTo(pThis->Location, velocity);
#ifdef COMPILE_PORTED_DP_FEATURES
								auto sourcePos = pThis->Location;
								auto targetPos = pTarget->GetCoords();

								// Draw bullet effect
								Helpers_DP::DrawBulletEffect(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);
								// Draw particle system
								Helpers_DP::AttachedParticleSystem(pShrapWeapon, sourcePos, pTarget, pThis->Owner, targetPos);
								// Play report sound
								Helpers_DP::PlayReportSound(pShrapWeapon, sourcePos);
								// Draw weapon anim
								Helpers_DP::DrawWeaponAnim(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);
#endif
							}
						}
					}
				}
			}
		}
	}
}
//
//
//DEFINE_HOOK(0x469A4F , BulletClass_Detonate_Shrapnel, 0x7)
//{
//	ShrapnelsExec(R->ESI<BulletClass*>());
//	return 4627030;
//}

#ifdef ENABLE_BULLETADJUSTVELHOOKS
DEFINE_HOOK(0x466D19, BulletClass_Update_AdjustingVelocity, 0x6)
{
	R->ECX(R->EBP());
	return 0;
}

/*
static int __fastcall ProjectileMotion_Exec(
CoordStruct* pCoord,
BulletVelocity* pVel,
CoordStruct* pSecondCoord,
DirStruct* pDir,
bool bInAir,
bool bAirburs,
BulletClass* pBullet, //replaced by hook above
bool bLevel)
{
	return BulletClass::ProjectileMotion(pCoord, pVel, pSecondCoord, pDir, bInAir, bAirburs, pBullet->Type->VeryHigh, bLevel);
}

DEFINE_JUMP(CALL,0x466D31, GET_OFFSET(ProjectileMotion_Exec));

DEFINE_HOOK(0x5B260B, BulletClass_ProjectileMotion_Fix1, 0x7)
{
	GET_BASE(BulletClass*, pBullet, 0x18);
	R->AL(pBullet->Type->VeryHigh);
	R->ECX(R->Stack<DWORD>(0x38));
	return 0x5B2612;
}*/

DEFINE_HOOK(0x5B271A, BulletClass_ProjectileMotion_Fix2, 0x5)
{
	GET_BASE(BulletClass*, pBullet, 0x18);
	return pBullet->Type->VeryHigh ? 0x5B272D : 0x5B2721;
}

DEFINE_HOOK(0x5B2778, BulletClass_ProjectileMotion_AscentAngle, 0x7)
{
	//GET_BASE(BulletClass*, pBullet, 0x18);
	R->Stack(0x18, (Math::clamp((0x4000 - 0x2000), 0, 0x4000)));
	return 0x5B277F;
}

DEFINE_HOOK(0x5B260B, BulletClass_ProjectileMotion_DescentAngle, 0x7)
{
	GET_BASE(BulletClass*, pBullet, 0x18);
	GET_STACK(int, nPointLength, 0x38);

	int DescentAngle = 3;
	return nPointLength <=
		((pBullet->Type->VeryHigh ? 6 : DescentAngle) << 8)
		? 0x5B289C : 0x5B2627;
}

DEFINE_HOOK(0x5B2721, BulletClass_ProjectileMotion_Cruise, 0x5)
{
	//GET_BASE(BulletClass*, pBullet, 0x18);
	GET(int, nLepton, EAX);

	bool bLockedOnTrajectory = false;
	int nCruiseLevel = 5;

	if (bLockedOnTrajectory || nLepton >= nCruiseLevel)
		nLepton = nCruiseLevel;

	R->EAX(nLepton);
	return 0x5B2732;
}

DEFINE_HOOK(0x466BBC, BulletClass_AI_MissileROTVar, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET(RulesClass*, pRules, ECX);

	double dRes = pRules->MissileROTVar;
	if (auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
	{
		dRes = pBulletTypeExt->MissileROTVar.Get(pRules->MissileROTVar);
	}

	R->ESI(&dRes);
	return 0x466BC2;
}

DEFINE_HOOK(0x466E9F, BulletClass_AI_MissileSafetyAltitude, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET(int, comparator, EAX);

	int nAltitude = RulesGlobal->MissileSafetyAltitude;
	if (auto const& pBulletTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
	{
		nAltitude = pBulletTypeExt->MissileSafetyAltitude.Get(RulesGlobal->MissileSafetyAltitude);
	}

	return comparator >= nAltitude
		? 0x466EAD : 0x466EB6;
}

#endif

#ifdef ENABLE_CELLSPREAD_LOCOWH
static void  ManipulateLoco(FootClass* pFirer, AbstractClass* pTarget, BulletClass* pBullet, bool Area)
{
	AbstractClass* pTarget_1 = pTarget;

	if (!Area)
	{
		if (pFirer->LocomotorTarget)
		{
			pFirer->LocomotorImblued(false);
		}

		pTarget_1 = pBullet->Target;
	}

	if (auto pFoot_T = generic_cast<FootClass*>(pTarget_1))
	{
		if (pTarget_1->WhatAmI() == AbstractType::Aircraft || pTarget_1->WhatAmI() == AbstractType::Unit)
		{
			if (auto pExt = WarheadTypeExt::ExtMap.Find(pBullet->WH))
			{
				if (!pExt->CanTargetHouse(pBullet->GetOwningHouse(), pFoot_T))
					return;

				if (!pExt->CanDealDamage(pFoot_T))
					return;
			}

			if (auto pUnit = specific_cast<UnitClass*>(pTarget))
			{

				if (pFoot_T->RadioLinks.IsAllocated &&
					pFoot_T->RadioLinks.IsInitialized &&
					pFoot_T->RadioLinks.Items)
				{
					auto pLink = *pFoot_T->RadioLinks.Items;
					if (pLink && pLink->WhatAmI() == AbstractType::Building && ((BuildingClass*)pLink)->Type->WeaponsFactory)
					{
						if (Map[pFoot_T->Location]->GetBuilding() == pLink)
							return;
					}
				}

				if (pUnit->Deploying || pUnit->Undeploying)
				{
					return;
				}
			}

			if (pFoot_T->InWhichLayer() != Layer::Ground || pFoot_T->IsAttackedByLocomotor)
				return;

			pFirer->FootClass_ImbueLocomotor(pFoot_T, pBullet->WH->Locomotor);
		}
	}
}

DEFINE_HOOK(0x4694CB, BulletClass_Logics_Locomotor, 0x6)
{
	enum
	{
		IsNotLocoRet = 0x469705
		, Return = 0x469AA4
	};

	GET(BulletClass*, pThis, ESI);
	GET(WarheadTypeClass*, pWH, EAX);

	if (!pWH->IsLocomotor)
		return IsNotLocoRet;

	if (!pThis->Owner)
		return Return;

	auto pTarget = pThis->Target;
	if (pThis->Owner->LocomotorTarget == pTarget
	  || pThis->Owner->LocomotorSource
	  || (pThis->Owner->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None && ((FootClass*)pThis->Owner)->unknown_bool_6AC)
	{
		return Return;
	}

	if (pWH->CellSpread == 0.0)
		ManipulateLoco((FootClass*)pThis->Owner, pTarget, pThis, false);
	else
	{
		for (auto pTarget_s : Helpers::Alex::getCellSpreadItems(pTarget->GetCoords(), pWH->CellSpread, true))
		{ ManipulateLoco((FootClass*)pThis->Owner, pTarget_s, pThis, true); }
	}

	return Return;
}

DEFINE_HOOK(0x707B95, TechnoClass_PointerExpired_LocoSource, 0x7)
{
	GET(TechnoClass*, pAttacker, ECX);
	GET(TechnoClass*, pVictim, ESI);

	if (pAttacker->LocomotorTarget == pVictim)
	{
		pAttacker->LocomotorImblued(true);
		pAttacker->LocomotorTarget = nullptr;
		pVictim->LocomotorSource = nullptr;
		return 0x707BB2;
	}

	if (!pVictim || (pVictim->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::None)
	{
		pVictim->LocomotorSource = nullptr;
		return 0x707BB2;
	}

	if (pVictim->GetHeight() > 0)
	{
		pVictim->IsCrashing = 1;
		pVictim->IsBeingManipulated = 1;
		pVictim->BeingManipulatedBy = pVictim;
		pVictim->ChronoWarpedByHouse = pAttacker->Owner;
		pVictim->Stun();
		pVictim->SetDestination(nullptr, true);
	}

	if (auto pUnitV = specific_cast<UnitClass*>(pVictim))
		pUnitV->unknown_int_6D4 = 1;

	pVictim->SetDestination(nullptr, true);
	pVictim->LocomotorSource = nullptr;
	return 0x707BB2;
}

DEFINE_HOOK(0x7102F9, FootClass_ImbueLocomotor_SetDestination, 0x5)
{
	GET(TechnoClass*, pThis, EBX);
	GET(FootClass*, pThat, ESI);

	pThis->LocomotorTarget = pThat;
	pThat->LocomotorSource = generic_cast<FootClass*>(pThis);

	if (!pThis->GetTechnoType()->Insignificant
	  || pThat->WhatAmI() != AbstractType::Aircraft)
	{
		pThat->SetDestination(pThis, true);
		pThat->Deselect();
	}
	else
	{
		pThis->Scatter(CoordStruct::Empty, true, false);
		pThis->Deselect();
	}

	return 0x71033A;
}
#endif

/*
void BulletClass_AI(BulletClass* pThis)
{
	pThis->ObjectClass::Update();
	if (!pThis->IsAlive)
	{
		return;
	}
	CoordStruct v148 = CoordStruct::Empty;

	auto type = pThis->Type;
	if (!pThis->SpawnNextAnim)
	{
		bool v137 = 0;
		bool v136 = 0;
		if (type->Dropping && !pThis->IsFallingDown) {
			v136 = 1;
		}

		if (type->AnimLow || type->AnimHigh)
		{
			if (!--pThis->AnimRateCounter)
			{
				pThis->AnimRateCounter = type->AnimRate;
				if (pThis->AnimFrame++ > type->AnimHigh)
				{
					pThis->AnimFrame = type->AnimLow;
				}
			}
		}
		auto coord = pThis->Location;

		if (type->Trailer)
		{
			if (auto v6 = type->ScaledSpawnDelay)
			{
				if (!(Unsorted::CurrentFrame % v6))
				{
					GameCreate<AnimClass>(type->Trailer, coord, 1, 1, 0x600u, 0, 0);
				}
			}
			else if (!(Unsorted::CurrentFrame % type->SpawnDelay))
			{
				GameCreate<AnimClass>(type->Trailer, coord, 1, 1, 0x600u, 0, 0);
			}
		}

		auto v8 = type;
		int pos = 0;
		if (v8->ROT > 0)
		{
			auto nMaxSpeed = pThis->Speed;
			auto nVelMag = pThis->Velocity.Magnitude();
			auto type_1 = type;
			auto X = nVelMag;

			if (type_1->CourseLockDuration)
			{
				if (pThis->__CourseLockedDuration < type_1->CourseLockDuration)
				{
					if (++pThis->__CourseLockedDuration >= type_1->CourseLockDuration)
					{
						goto LABEL_32;
					}
				}
			}
			else if (pThis->Speed >= 40 || (double)nMaxSpeed <= X + 0.5)
			{
			LABEL_32:
				pThis->__CourseLocked = 0;
				goto LABEL_33;
			}
		LABEL_33:
			auto v17 = pThis->__CourseLocked;
			auto v18 = type_1->Acceleration;
			auto Accel = v18;

			if (v17 && !type_1->CourseLockDuration)
			{
				v18 = (Unsorted::CurrentFrame % 2 == 0);
				Accel = v18;
			}

			auto nMaxSpeed_ = (double)nMaxSpeed;
			if (X >= nMaxSpeed_)
			{
				if (X <= nMaxSpeed_)
				{
					goto LABEL_51;
				}

				nMaxSpeed = (v18 / 2);
				X = X - (double)(v18 / 2);
				if (X <= 0.0)
				{
					X = 0.0;
				}

				if (pThis->Velocity)
				{

				LABEL_50:
					auto v20 = pThis->Velocity.Magnitude();
					auto v21 = X / v20;
					pThis->Velocity *= v21;

				LABEL_51:

					auto v25 = CoordStruct::Empty;

					if (pThis->Target) {
						v25 = pThis->GetAltCoords();
					}

					CoordStruct a3 = v25;
					if (pThis->Target && (pThis->Target->AbstractFlags & AbstractFlags::Object) != AbstractFlags::None) {
						((ObjectClass*)pThis->Target)->GetPosition_0(&a3);
					}

					auto v27 = RulesGlobal->MissileROTVar;
					nMaxSpeed = ((Unsorted::CurrentFrame + pThis->Fetch_ID() ) % 15);
					auto nMaxSpeed__ = ((Math::sin((double)nMaxSpeed * 0.06666666666666667 * 6.283185307179586) * v27 + v27 + 1.0)* (double)pThis->Type->ROT);
					nMaxSpeed = nMaxSpeed__;
					auto v30 = pThis->GetCenterCoord();
					auto v31 = v30 - a3;
					if (v31.Magnitude() < 256) {
						nMaxSpeed__ = (nMaxSpeed * 1.5);
					}

					CoordStruct a2 = coord;
					auto v35 = pThis->Target && pThis->Target->WhatAmI()  == AbstractType::Aircraft;
					auto v36 = pThis->__CourseLocked == 0 ? (unsigned __int8)nMaxSpeed__ : 0;
					auto v37 = pThis->Type;
					DirStruct nDir = DirStruct { v36 };
					nDir = BulletClass::ProjectileMotion(
												 &coord,
												 &pThis->Velocity,
												 &a3,
												 &nDir,
												 v35,
												 v37->Airburst,
												 v37->VeryHigh,
												 v37->Level);

					auto v144 = Map[coord];
					X = (double)nMaxSpeed;
					auto v41 = pThis->Velocity.Magnitude();

					if (v41 * 0.5 >= X || pThis->GetHeight() <= 0)
					{
						v136 = 1;
						pos = 1;
						if (pThis->GetHeight() > 0
						  && !pThis->Type->Airburst
						  && (a3))
						{
							coord = a3;
						}
					}
					if (!a3 && pThis->GetHeight() >= RulesGlobal->MissileSafetyAltitude)
					{
						v136 = 1;
						pos = 1;
					}

					auto v43 = a2 - a3;
					auto nDistance = v43.Magnitude();
					auto v46 =  coord - a3;
					auto nDistance_ = nDistance - v46.Magnitude();

					if (!pThis->__CourseLocked)
					{
						auto v49 = pThis->SomeIntIncrement_118;
						if (v49 >= 60)
						{
							auto v51 = pThis->unknown_120 * 0.9833333333333333 + nDistance_;
							pThis->unknown_120 = v51;
							if (//!v53 && , WTF is this ?
								v51 < 60.0)
							{
								auto v54 = pThis->Type;
								if (!v54->Airburst && !v54->VeryHigh)
								{
									v136 = 1;
									pos = 1;
									goto LABEL_192;
								}
							}
						}
						else
						{
							auto v50 = nDistance_;
							pThis->SomeIntIncrement_118 = v49 + 1;
							pThis->unknown_120 = v50 + pThis->unknown_120;
						}
					}
					if (!pos
					  && ((v144->Flags & CellFlags::Bridge) != CellFlags::Empty
						  || (Map[a2]->Flags & CellFlags::Bridge) != CellFlags::Empty))
					{
						auto v55 = 416 + Map.GetCellFloorHeight(coord);
						if (coord.Z > v55 && a2.Z < v55)
						{
							pos = 1;
							coord.Z = v55;
						LABEL_191:
							v136 = 1;
							goto LABEL_192;
						}
						if (coord.Z < v55 && a2.Z > v55)
						{
							pos = 1;
							coord.Z = v55;
							goto LABEL_191;
						}
					}
				LABEL_192:
					PlacementType n = PlacementType::Redraw;

					if (pos == 2)
					{
						pThis->UpdatePlacement(n);
					}
					else
					{
						pThis->UpdatePlacement(n);
						pThis->SetLocation(coord);
						bool v104 = false;

						if (v136
						  || (a2 = pThis->Location,
							  v104 = pThis->IsForceToExplode(a2),
							  v136 = v104,
							  pThis->SetLocation(a2),
							  v104))
						{
							if (pThis->GetHeight() < 0)
							{
								pThis->SetHeight(0);
							}
						}

						auto v105 = pThis->Type;
						Fuse fuse_result = Fuse::DontIgnite;

						if (v105->ROT > 0 || v105->Ranged)
						{
							fuse_result = pThis->Data.BulletStateCheck(coord);
						}

						auto v107 = pThis->Owner;
						auto v143 = fuse_result;
						if (v107 && v107->GetTechnoType()->JumpJet && fuse_result == Fuse::Ignite_DistaceFactor)
						{
							fuse_result = Fuse::Ignite;
							v143 = Fuse::Ignite;
						}

						if (!v136)
						{
							auto v108 = pThis->Type;
							if (v108->Dropping || fuse_result == Fuse::DontIgnite)
							{
								if (v108->Ranged)
								{
									if (pThis->Health > 5)
									{
										--pThis->Health;
									}
								}
								goto LABEL_241;
							}
						}

						auto v110 = pThis->Target;
						if (v110 && (v143 == Fuse::Ignite || v137))
						{
							auto v111 = pThis->Type;
							if (!v111->Airburst && !v111->Inaccurate)
							{
								auto v112 = coord.X;
								auto v113 = coord.Y;
								auto v114 = coord.Z;
								auto v115 = v110->GetAltCoords();
								v148.X = v115.X;
								v148.Y = v115.Y;
								auto v116 = v115.Z;
								a2.Y = v113 - v115.Y;
								a2.Z = (v116 + v114) / 2 - v116;
								a2.X = v112 - v115.X;
								auto v117 = Math::sqrt(
													(double)(v112 - v115.X) * (double)(v112 - v115.X)
												  + (double)a2.Z * (double)a2.Z
												  + (double)a2.Y * (double)a2.Y);
								nDistance_ = v117;

								if (v137) {
									nDistance_ = v117 / 3;
								}

								auto nMag = pThis->Velocity.Magnitude();
								auto v119 = nMag >= 128.0 ? nMag + nMag : 128.0;

								if (v143 == Fuse::Ignite
								  ||nDistance_ <= v119)
								{
									auto v122 = pThis->Target->GetCoords();
									pThis->SetLocation(v122);
								}
							}
						}
						if (pThis->WH->SameName("NUKE"))
						{
							if (pThis->GetHeight() < 0){
								pThis->SetHeight(0);
							}

							NukeFlash::FadeIn();

							auto v123 = pThis->GetMapCoords();
							RadarEventClass::Create(RadarEventType::SuperweaponActivated, v123);

							auto v124 = AnimTypeClass::FindIndex("NUKEBALL");
							if (v124 != -1)
							{
								if (auto v125 = AnimTypeClass::Array->GetItem(v124))
								{
								    AnimClass* v128 = GameCreate<AnimClass>(v125,pThis->Location,0,1, 0x2600u,-15,0);
									pThis->NextAnim = v128;
									pThis->SpawnNextAnim = 1;
									BulletClass::Array->AddItem(pThis);
									goto LABEL_241;
								}
							}
						}

						pThis->Explode(v136);
					}
					pThis->UnInit();
				LABEL_241:
					pThis->LastMapCoords = coord.TocellStruct();
					return;
				}
			}
			else
			{
				X = v18 + X;
				if (X >= nMaxSpeed_) {
					X = nMaxSpeed_;
				}

				if (pThis->Velocity)
				{
					goto LABEL_50;
				}
			}

			pThis->Velocity.X = 5.333806864e-315;
			goto LABEL_50;
		}

		if (pThis->Velocity.Magnitude() < 8.0) {
			pos = 1;
		}

		auto v59 = pThis->Type;
		VelocityClass CoordToVelX = { (double)coord.X  ,(double)coord.Y ,(double)coord.Z };

		auto v60 = (double)RulesGlobal->Gravity;
		if (v59->Floater) {
			v60 = v60 * 0.5;
		}

		auto v61 = v59;
		if (v61->Vertical)
		{
			auto v62 = pThis->Velocity.Magnitude();
			if ((int)v62 < pThis->Speed)
			{
				auto mnMaxSpeed = (v62 + pThis->Type->Acceleration);
				auto X =mnMaxSpeed;
				if (!pThis->Velocity) {
					pThis->Velocity.X = 5.333806864e-315;
				}

				auto v63 = pThis->Velocity.Magnitude();
				auto v64 = X / v63;
				pThis->Velocity *= v64;
			}

			auto v67 = pThis->Type;
			CoordStruct a2 = coord;
			coord.X += pThis->Velocity.X;
			coord.Y += pThis->Velocity.Y;
			coord.Z += pThis->Velocity.Z;

			if (coord.Z <= v67->DetonationAltitude)
			{
				if (pThis->GetHeight() >= 0)
				{
					auto v68 = Map.GetCellFloorHeight(coord) + 412;
					if ((Map[coord]->Flags & CellFlags::Bridge) != CellFlags::Empty
					  || (Map[a2]->Flags & CellFlags::Bridge) != CellFlags::Empty)
					{
						if (coord.Z < v68)
						{
							if (a2.Z >= v68)
							{
								pos = 1;
								v136 = 1;
							}
						}
						else if (a2.Z < v68)
						{
							pos = 1;
							v136 = 1;
						}
					}
				}
				else
				{
					pos = 1;
					v136 = 1;
				}
			}
			else
			{
				pos = 1;
				v136 = 1;
			}
		LABEL_159:
			auto aPart = coord.TocellStruct();
			auto bPart = pThis->TargetCoords.TocellStruct();
			int BulletHeightLimit = 104;
			CellClass* pCell_aPart = Map[aPart];
			CellClass* pCell_bPart = Map[bPart];
			auto v90 = pCell_aPart->GetBuilding();
			auto v91 = pCell_bPart->GetBuilding();

			if (aPart == bPart
			  && !pThis->Type->Vertical
			  && pThis->GetHeight() < 2 * BulletHeightLimit
			  || (v90)
			  && !pThis->Type->Vertical
			  && v90 == v91
			  && pThis->GetHeight() < 2 * BulletHeightLimit)
			{
				pos = 1;
				v136 = 1;
				v137 = 1;
				goto LABEL_192;
			}

			auto v92 = Map[coord];
			auto v93 = v92->FindTechnoNearestTo({0,0}, 0, nullptr);
			bool v135 = 1;

			if (!pThis->Owner || (v93 != pThis->Owner))
			{
				v135 = 0;
			}

			bool v134 = pThis->Owner ? pThis->Owner->Owner ? pThis->Owner->Owner->IsAlliedWith(v93): false : false;

			bool v98 = 0;
			if (v93)
			{
				if (coord.DistanceFrom(v93->Location) < 128)
				{
					v98 = 1;
				}
			}
			if (v135 || v134 || !v98)
			{
				if (!Map.IsValid(coord))
				{
					pos = 2;
					v136 = 1;
					coord = pThis->Location;
					goto LABEL_192;
				}
				auto v100 = pThis->Type;

				//CoordStruct v145 = coord;

				if (!v100->Vertical) {
					pThis->Velocity = CoordToVelX;
				}

				if (pThis->Velocity.Magnitude() < 10.0
				  && pThis->GetHeight() < 10) {
					pos = 1;
					goto LABEL_191;
				}
			}
			else
			{
				auto v99 = pThis->Type;
				pos = 1;
				v136 = 1;
				if (!v99->Inaccurate)
				{
					coord = v93->Location;
				}
			}
			goto LABEL_192;
		}

		auto v71 = pThis->Owner;
		VelocityClass v142 = { v61->Elasticity ,v61->Elasticity , CoordToVelX.Z - v60 };
		CoordToVelX += v142;
		CoordStruct v145 = { CoordToVelX.X ,CoordToVelX.Y,CoordToVelX.Z };
		auto nCompare = v148 - v145;
		auto nZPosHere = Map.GetCellFloorHeight(v145);
		auto v73 = nZPosHere + 412;
		auto v143 = nZPosHere + 412;
		auto v74 = Map[v145];
		bool v135 = 0;
		bool v134 = 0;

		if ((v74->Flags & CellFlags::Bridge) != CellFlags::Empty ||
			(Map[nCompare]->Flags & CellFlags::Bridge) != CellFlags::Empty)
		{
			if (v145.Z < (int)v73)
			{
				if (v148.Z >= (int)v73)
				{
					v135 = 1;
				}
			}
			else if (v148.Z < (int)v73)
			{
				v134 = 1;
			}
		}

		bool v75 = 0;
		if (!v135 && !v134)
		{
			auto v76 = (double)nZPosHere;
			if (CoordToVelX.Z < v76 || CoordToVelX.Z - 150.0 >= v76)
			{
			LABEL_140:
				auto a3_X = (double)nZPosHere;
				if (CoordToVelX.Z >= a3_X && !v135 && !v134 && !v75)
				{
					goto LABEL_158;
				}
				if (v71)
				{
					double v84 = 0.0;

					if (v135)
					{
						v84 = (double)v143;
					}
					else
					{
						if (!v134)
						{
						LABEL_158:
							coord.X = CoordToVelX.X;
							coord.Y = CoordToVelX.Y;
							coord.Z = CoordToVelX.Z;
							goto LABEL_159;
						}
						nZPosHere = (v73 - 20);
						v84 = (double)(v73 - 20);
					}
					CoordToVelX.Z = v84;
				}
				else
				{
					if (v135)
					{
						CoordToVelX.Z = (double)v143;
					}
					else if (v134)
					{
						v73 = (v73 - 20);
						CoordToVelX.Z = (double)(v73 - 20);
					}
					else
					{
						v73 -= 25;
						if ((double)v73 < CoordToVelX.Z)
						{
							CoordToVelX.Z = (double)nZPosHere;
						}
					}

					auto v79 = TacticalGlobal->GetRamp(&v145);
					Matrix3D v152;
					Game::GetRampMtx(&v152, v79);

					Matrix3D v153;
					Matrix3D::sub_5AFC20(&v153,&v152);
					auto nZ = CoordToVelX.Z;
					auto nY = -CoordToVelX.Y;
					auto nX = CoordToVelX.X;
					Vector3D<float> a3 = { nX , nY , nZ };
					auto v80 = Matrix3D::Rotate_Vector(v153, a3);
					Vector3D<float> a4 = *v80;
					a4 *= a3;
					auto v80 = Matrix3D::Rotate_Vector(v152, a4);
					CoordToVelX.X = v80->X;
					CoordToVelX.Y = -v80->Y;
					CoordToVelX.Z = v80->Z;
				}
				pos = 1;
				v136 = 1;
				goto LABEL_158;
			}

			auto v77 = v74->GetBuilding();
			if (!v77 && !v74->ConnectsToOverlay(-1, -1))
			{
				v73 = v143;
				goto LABEL_140;
			}

			v75 = 1;
			if (v77)
			{
				auto v78 = pThis->Owner;
				if (v77 == pThis->Owner || v77->Type->LaserFence && v77->LaserFenceFrame >= 8)
				{
					v75 = 0;
				}
				if (v77->IsStrange())
				{
					v75 = 0;
				}
				if (v78 && pThis->Owner->Owner && pThis->Owner->Owner->IsAlliedWith(v77))
				{
					v75 = 0;
				}
			}
		}
		v73 = v143;
		goto LABEL_140;
	}

	if (!pThis->NextAnim)
	{
		auto v2 = BulletClass::Array->FindItemIndex(pThis);
		if (v2 != -1 && v2 < BulletClass::Array->Count)
		{
			--BulletClass::Array->Count;
			for (; v2 < BulletClass::Array->Count; BulletClass::Array->Items[v2 - 1] = BulletClass::Array->Items[v2])
			{
				++v2;
			}
		}

		pThis->SpawnNextAnim = 0;
		pThis->Explode(false);
		pThis->UnInit();
	}
}*/