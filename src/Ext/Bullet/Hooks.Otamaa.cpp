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

	R->EDX(BulletTypeExt::ExtMap.Find(pBullet->Type)->Parachute.Get(pRules->BombParachute));
	return 0x5F5A8C;
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
				? pThis->GetBulletTargetCoords() : pThis->GetCoords();

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
					if (pWHExt->CanDealDamage(pTechno, false, !pExt->Splits_TargetingUseVerses.Get()))
					{
						if ((!pExt->RetargetOwner.Get() && pTechno == pBulletOwner))
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
						auto const radians = dir.GetRadian();

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
	HouseClass* const pOWner = pThis->Owner ? pThis->Owner->GetOwningHouse() : (pExt->Owner ? pExt->Owner : nullptr);
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
								VoxelAnimExt::ExtMap.Find(pVoxAnim)->Invoker = pThis->Owner;
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


//template<bool Reverse =false>
//static __forceinline VelocityClass GenerateVelocity(BulletClass* pThis, AbstractClass* pTarget, const int nSpeed)
//{
//	VelocityClass velocity { 0.0,0.0,0.0 };
//	//inline get Direction from 2 coords
//	CoordStruct const nCenter = pTarget->GetCoords();
//	//this->radians(Math::atan2(a, b));
//	DirStruct const dir_fromXY((double)(pThis->Location.Y - nCenter.Y), (double)(pThis->Location.X - nCenter.X));
//
//	if (velocity.X == 0.0 && velocity.Y == 0.0)
//	{
//		velocity.X = 100.0;
//	}
//
//	double const nFirstMag = velocity.MagnitudeXY();
//	double const radians_fromXY = dir_fromXY.GetRadian();
//	double const sin_rad = Math::sin(radians_fromXY);
//	double const cos_rad = Math::cos(radians_fromXY);
//
//	velocity.X = cos_rad * nFirstMag;
//	velocity.Y -= sin_rad * nFirstMag;
//
//	if constexpr (!Reverse)
//	{
//		double const nSecMag = velocity.MagnitudeXY();
//		DirStruct const dir_forZ(velocity.Z, nSecMag);
//		double const radians_foZ = dir_forZ.GetRadian();
//
//		double const nThirdMag = velocity.MagnitudeXY();
//		if (radians_foZ != 0.0)
//		{
//			velocity.X /= Math::cos(radians_foZ);
//			velocity.Y /= Math::cos(radians_foZ);
//		}
//
//		double const nMult_Cos = Math::cos(0.7853262558535721);
//		double const nMult_Sin = Math::sin(0.7853262558535721);
//		velocity.X *= nMult_Cos;
//		velocity.Y *= nMult_Cos;
//		velocity.Z *= nMult_Sin * nThirdMag;
//
//		if (velocity.X == 0.0 && velocity.Y == 0.0 && velocity.Z == 0.0)
//		{
//			velocity.X = 100.0;
//		}
//
//		const double nFullMag = velocity.Magnitude();
//		const double nDevidedBySpeed = nSpeed / nFullMag;
//		velocity *= nDevidedBySpeed;
//	}
//	else
//	{
//		const double nFullMag = velocity.Magnitude();
//		const double nDevidedBySpeed = nSpeed / nFullMag;
//		velocity *= nDevidedBySpeed;
//
//		double const nSecMag = velocity.MagnitudeXY();
//		DirStruct const dir_forZ(velocity.Z, nSecMag);
//		double const radians_foZ = dir_forZ.GetRadian();
//
//		double const nThirdMag = velocity.MagnitudeXY();
//		if (radians_foZ != 0.0)
//		{
//			velocity.X /= Math::cos(radians_foZ);
//			velocity.Y /= Math::cos(radians_foZ);
//		}
//
//		double const nMult_Cos = Math::cos(0.7853262558535721);
//		double const nMult_Sin = Math::sin(0.7853262558535721);
//		velocity.X *= nMult_Cos;
//		velocity.Y *= nMult_Cos;
//		velocity.Z *= nMult_Sin * nThirdMag;
//
//		if (velocity.X == 0.0 && velocity.Y == 0.0 && velocity.Z == 0.0)
//		{
//			velocity.X = 100.0;
//		}
//	}
//
//	return velocity;
//}

//void ShrapnelsExec(BulletClass* pThis)
//{
//	auto const pType = pThis->Type;
//	auto const pShrapWeapon = pType->ShrapnelWeapon;
//
//	if (!pShrapWeapon)
//		return;
//
//	int nCount = pType->ShrapnelCount;
//
//
//	if (nCount < 0)
//	{
//		if (auto pOwner = pThis->Owner)
//		{
//			auto nRes = (-pType->ShrapnelCount) - static_cast<int>(pOwner->GetCoords().DistanceFrom(pThis->Location) / 256.0);
//			if (nRes <= 0)
//				return;
//
//			nCount = nRes;
//		}
//		else
//		{
//			nCount = 3;
//		}
//	}
//
//	auto const pBulletCell = pThis->GetCell();
//	if (auto pFirstOccupy = pBulletCell->FirstObject)
//	{
//		if (pFirstOccupy->WhatAmI() != AbstractType::Building)
//		{
//			auto& random = ScenarioClass::Instance->Random;
//			auto const nRange = (random.RandomRanged(pShrapWeapon->MinimumRange , pShrapWeapon->Range) / 256);
//
//			if (nRange >= 1)
//			{
//				int nTotal = 0;
//				auto cell = pThis->GetMapCoords();
//
//				for (size_t direction = 0; direction <= 7; direction += 2)
//				{
//					CellStruct directionOffset = CellSpread::GetNeighbourOffset(direction); // coordinates of the neighboring cell in the given direction relative to the current cell (e.g. 0,1)
//					CellStruct cellToCheck = cell;
//					for (short distanceFromCenter = 1; distanceFromCenter <= nRange; ++distanceFromCenter)
//					{
//						cellToCheck += directionOffset; // adjust the cell to check based on current distance, relative to the selected cell
//						CellClass* pCell = MapClass::Instance->TryGetCellAt(cellToCheck);
//						if (!pCell) { // don't parse this cell if it doesn't exist (duh)
//							break;
//						}
//
//						auto const pTarget = pCell->FirstObject;
//						if (pTarget && pTarget != pThis->Owner && pThis->Owner && !pThis->Owner->Owner->IsAlliedWith(pTarget)
//							)
//						{
//							auto pSplitExt = BulletTypeExt::ExtMap.Find(pShrapWeapon->Projectile);
//
//							if (auto pBullet = pSplitExt->CreateBullet(pTarget, pThis->Owner, pShrapWeapon))
//							{
//								VelocityClass velocity = GenerateVelocity(pThis, pTarget, pShrapWeapon->Speed);
//								pBullet->MoveTo(pThis->Location, velocity);
//#ifdef COMPILE_PORTED_DP_FEATURES
//								auto sourcePos = pThis->Location;
//								auto targetPos = pTarget->GetCoords();
//
//								// Draw bullet effect
//								Helpers_DP::DrawBulletEffect(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);
//								// Draw particle system
//								Helpers_DP::AttachedParticleSystem(pShrapWeapon, sourcePos, pTarget, pThis->Owner, targetPos);
//								// Play report sound
//								Helpers_DP::PlayReportSound(pShrapWeapon, sourcePos);
//								// Draw weapon anim
//								Helpers_DP::DrawWeaponAnim(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);
//#endif
//							}
//
//							//escapes
//							if (++nTotal > nCount)
//								return;
//						}
//					}
//				}
//
//				// get random coords for last remaining shrapnel if the total still less than ncount
//				if (nTotal < nCount)
//				{
//					for (int nAmountHere = nCount - nTotal; nAmountHere > 0; --nAmountHere)
//					{
//						const int nX = ScenarioClass::Instance->Random.RandomRanged(-2, 2);
//						const int nY = ScenarioClass::Instance->Random.RandomRanged(-2, 2);
//						CellStruct nNextCoord = pThis->GetMapCoords();
//						nNextCoord.X += nX;
//						nNextCoord.Y += nY;
//						if (auto pTarget = MapClass::Instance->TryGetCellAt(nNextCoord))
//						{
//							auto pSplitExt = BulletTypeExt::ExtMap.Find(pShrapWeapon->Projectile);
//
//							if (auto pBullet = pSplitExt->CreateBullet(pTarget, pThis->Owner, pShrapWeapon))
//							{
//								VelocityClass velocity = GenerateVelocity<true>(pThis, pTarget, pShrapWeapon->Speed);
//								pBullet->MoveTo(pThis->Location, velocity);
//#ifdef COMPILE_PORTED_DP_FEATURES
//								auto sourcePos = pThis->Location;
//								auto targetPos = pTarget->GetCoords();
//
//								// Draw bullet effect
//								Helpers_DP::DrawBulletEffect(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);
//								// Draw particle system
//								Helpers_DP::AttachedParticleSystem(pShrapWeapon, sourcePos, pTarget, pThis->Owner, targetPos);
//								// Play report sound
//								Helpers_DP::PlayReportSound(pShrapWeapon, sourcePos);
//								// Draw weapon anim
//								Helpers_DP::DrawWeaponAnim(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);
//#endif
//							}
//						}
//					}
//				}
//			}
//		}
//	}
//}

//
//DEFINE_HOOK(0x469A4F , BulletClass_Detonate_Shrapnel, 0x7)
//{
//	ShrapnelsExec(R->ESI<BulletClass*>());
//	return 4627030;
//}

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

DEFINE_HOOK(0x466D19, BulletClass_Update_AdjustingVelocity, 0x6)
{
	R->ECX(R->EBP());
	return 0;
}

DEFINE_HOOK(0x5B271A, BulletClass_ProjectileMotion_Fix2, 0x5)
{
	GET_BASE(BulletClass*, pBullet, 0x18);
	return pBullet->Type->VeryHigh ? 0x5B272D : 0x5B2721;
}

DEFINE_HOOK(0x5B260B, BulletClass_ProjectileMotion_DescentAngle, 0x7)
{
	GET_BASE(BulletClass*, pBullet, 0x18);
	GET_STACK(int, nData, 0x38);

	return nData <= ((pBullet->Type->VeryHigh ? 6 : 3) << 8)
		? 0x5B289C : 0x5B2627;
}

DEFINE_HOOK(0x5B2778, BulletClass_ProjectileMotion_AscentAngle, 0x7)
{
	//GET_BASE(BulletClass*, pBullet, 0x18);
	//R->Stack(0x18, (Math::clamp((0x4000 - 0x2000), 0, 0x4000)));
	R->Stack<WORD>(0x18, 0x2000);
	return 0x5B277F;
}

DEFINE_HOOK(0x5B2721, BulletClass_ProjectileMotion_Cruise, 0x5)
{
	//(BulletClass*, pBullet, 0x18);
	GET(int, nLepton, EAX);

	//bool bLockedOnTrajectory = false;
	//int nCruiseLevel = 5;
	//if (bLockedOnTrajectory || nLepton >= nCruiseLevel)
	//	nLepton = nCruiseLevel;

	R->EAX(nLepton >= 5 ? 5 : nLepton);
	return 0x5B2732;
}

// to fuck off optimization
//static double NOINLINE GetMissileRotVar(const BulletClass* const pThis, const RulesClass* const pRules)
//{
//	const auto pExt = BulletTypeExt::ExtMap.Find(pThis->Type);
//	return pExt->GetMissileROTVar(pRules);
//}

// Optimization fuckup the code again ,..
//#pragma optimize( "", off )
//DEFINE_HOOK(0x466BBC, BulletClass_AI_MissileROTVar, 0x6)
//{
//	GET(BulletClass*, pThis, EBP);
//	GET(RulesClass*, pRules, ECX);
//
//	double dRes = GetMissileRotVar(pThis, pRules);
//	R->ESI(&dRes);
//	return 0x466BC2;
//}
//#pragma optimize( "", on )

DEFINE_HOOK(0x466BCB,BulletClass_AI_MissileROTVar, 0x8 )
{
	GET(const double* const , pOriginal , ESI);
	GET(BulletClass*, pThis, EBP);
	GET(int, nCurFrame, ECX);

	auto nFrame = (nCurFrame + pThis->Fetch_ID()) % 15;
	double nMissileROTVar = BulletTypeExt::ExtMap.Find(pThis->Type)->MissileROTVar.Get(*pOriginal);

	R->EAX(Game::F2I(Math::sin(static_cast<double>(nFrame) * 0.06666666666666667 * 6.283185307179586) * nMissileROTVar + nMissileROTVar + 1.0) * static_cast<double>(pThis->Type->ROT));
	return 0x466C14;
}

DEFINE_HOOK(0x466E9F, BulletClass_AI_MissileSafetyAltitude, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	GET(int, comparator, EAX);
	auto const pBulletTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
	return comparator >= pBulletTypeExt->GetMissileSaveAltitude(RulesGlobal)
		? 0x466EAD : 0x466EB6;
}

DEFINE_HOOK(0x46A4ED, BulletClass_Shrapnel_CheckVerses, 0x5)
{
	enum { Continue = 0x46A4F3, Skip = 0x46A8EA };

	GET(BulletClass*, pThis, EDI);
	GET(AbstractClass*, pTarget, EBP);

	WarheadTypeClass* pWH = pThis->Type->ShrapnelWeapon->Warhead;

	if (const auto pTargetObj = abstract_cast<ObjectClass*>(pTarget))
	{
		auto const pWhat = pTargetObj->What_Am_I();
		switch (pWhat)
		{
		case AbstractType::Aircraft:
		case AbstractType::Infantry:
		case AbstractType::Unit:
		case AbstractType::Building:
		{
			if (!WarheadTypeExt::ExtMap.Find(pWH)->CanDealDamage(static_cast<TechnoClass*>(pTargetObj),false,false))
				return Skip;
		}
		 break;
		default:
		{
			if (GeneralUtils::GetWarheadVersusArmor(pWH, pTargetObj->GetType()->Armor) == 0.0)
				return Skip;
		}
		 break;
		}

		if (pThis->Target == pTarget)
			return Skip;
	}


	return Continue;
}

//ToDO : complete this maybe ?
//DEFINE_HOOK(0x46A951, BulletClass_Shrapnel_RandomFragmentsRange, 0x6)
//{
//	GET(BulletClass*, pThis, ESI);
//
//	R->Stack(0x34, R->EDX());
//
//
//}