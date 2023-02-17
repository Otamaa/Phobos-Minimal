#include "Body.h"

#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#endif

//DEFINE_HOOK(0x46A328, BulletClass_Shrapnel_CheckWeapon, 0x6)
//{
//	GET(BulletTypeClass*, pType, EAX);
//	return pType->ShrapnelWeapon ? 0x0 : 0x46ADCD;
//}

//DEFINE_HOOK(0x46A3D6, BulletClass_Shrapnel_Forced, 0xA)
//{
//	enum { Shrapnel = 0x46A40C, Skip = 0x46ADCD };
//
//	GET(BulletClass*, pBullet, EDI);
//
//	auto const pData = BulletTypeExt::ExtMap.Find(pBullet->Type);
//
//	if (auto const pObject = pBullet->GetCell()->FirstObject)
//	{
//		if (pObject->WhatAmI() != AbstractType::Building || pData->Shrapnel_AffectsBuildings)
//			return Shrapnel;
//	}
//	else if (pData->Shrapnel_AffectsGround)
//		return Shrapnel;
//
//	return Skip;
//}

DEFINE_HOOK(0x46A4ED, BulletClass_Shrapnel_CheckVerses, 0x5)
{
	enum { Continue = 0x46A4F3, Skip = 0x46A8EA };

	GET(BulletClass*, pThis, EDI);
	GET(AbstractClass*, pTarget, EBP);

	WarheadTypeClass* pWH = pThis->Type->ShrapnelWeapon->Warhead;

	if (const auto pTargetObj = abstract_cast<ObjectClass*>(pTarget))
	{
		auto const pWhat = pTargetObj->WhatAmI();
		switch (pWhat)
		{
		case AbstractType::Aircraft:
		case AbstractType::Infantry:
		case AbstractType::Unit:
		case AbstractType::Building:
		{
			if (!WarheadTypeExt::ExtMap.Find(pWH)->CanDealDamage(static_cast<TechnoClass*>(pTargetObj), false, false))
			{
				return Skip;
			}
		}
		break;
		default:
		{
			if (auto pType = pTargetObj->GetType())
			{
				if (GeneralUtils::GetWarheadVersusArmor(pWH, pType->Armor) == 0.0)
				{
					return Skip;
				}
			}
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

NOINLINE VelocityClass GenerateVelocity(BulletClass* pThis, AbstractClass* pTarget, const int nSpeed)
{
	VelocityClass velocity { 0.0,0.0,0.0 };
	//inline get Direction from 2 coords
	CoordStruct const nCenter = pTarget->GetCoords();
	DirStruct const dir_fromXY((double)(pThis->Location.Y - nCenter.Y), (double)(pThis->Location.X - nCenter.X));

	if (velocity.X == 0.0 && velocity.Y == 0.0)
	{
		velocity.X = 100.0;
	}

	double const nFirstMag = velocity.MagnitudeXY();
	double const radians_fromXY = dir_fromXY.GetRadian();
	double const sin_rad = Math::sin(radians_fromXY);
	double const cos_rad = Math::cos(radians_fromXY);

	velocity.X = cos_rad * nFirstMag;
	velocity.Y -= sin_rad * nFirstMag;

	double const nSecMag = velocity.MagnitudeXY();
	DirStruct const dir_forZ(velocity.Z, nSecMag);
	double const radians_foZ = dir_forZ.GetRadian();

	//double const nThirdMag = velocity.MagnitudeXY();
	if (radians_foZ != 0.0)
	{
		velocity.X /= Math::cos(radians_foZ);
		velocity.Y /= Math::cos(radians_foZ);
	}

	double const nMult_Cos = Math::cos(0.7853262558535721);
	double const nMult_Sin = Math::sin(0.7853262558535721);
	velocity.X *= nMult_Cos;
	velocity.Y *= nMult_Cos;
	velocity.Z *= nMult_Sin;

	if (velocity.X == 0.0 && velocity.Y == 0.0 && velocity.Z == 0.0)
	{
		velocity.X = 100.0;
	}

	const double nFullMag = velocity.Magnitude();
	const double nDevidedBySpeed = nSpeed / nFullMag;
	velocity *= nDevidedBySpeed;
	return velocity;
}
NOINLINE int GetShrapAmount(BulletClass* pThis)
{
	if (pThis->Type->ShrapnelCount < 0)
	{
		if (auto pOwner = pThis->Owner)
		{
			return (-pThis->Type->ShrapnelCount) -
				static_cast<int>(pOwner->GetCoords().
				DistanceFrom(pThis->Location) / Unsorted::d_LeptonsPerCell);
		}
		else
		{
			return 3;
		}
	}

	return pThis->Type->ShrapnelCount;
};
NOINLINE bool AllowShrapnel(BulletClass* pThis, CellClass* pCell)
{
	auto const pData = BulletTypeExt::ExtMap.Find(pThis->Type);

	if (auto const pObject = pCell->FirstObject)
	{
		if (pObject->WhatAmI() != AbstractType::Building || pData->Shrapnel_AffectsBuildings)
			return true;
	}
	else if (pData->Shrapnel_AffectsGround)
		return true;

	return false;
}
NOINLINE bool ShrapnelTargetEligible(BulletClass* pThis, AbstractClass* pTarget, bool checkOwner = true)
{
	if (!pTarget)
		return false;

	WarheadTypeClass* pWH = pThis->Type->ShrapnelWeapon->Warhead;

	if (const auto pTargetObj = abstract_cast<ObjectClass*>(pTarget))
	{
		auto const pWhat = pTargetObj->WhatAmI();
		switch (pWhat)
		{
		case AbstractType::Aircraft:
		case AbstractType::Infantry:
		case AbstractType::Unit:
		case AbstractType::Building:
		{
			if (!WarheadTypeExt::ExtMap.Find(pWH)->CanDealDamage(static_cast<TechnoClass*>(pTargetObj), false, false))
			{
				return false;
			}
		}
		break;
		default:
		{
			if (auto pType = pTargetObj->GetType())
			{
				if (GeneralUtils::GetWarheadVersusArmor(pWH, pType->Armor) == 0.0)
				{
					return false;
				}
			}
		}
		break;
		}

		if (pThis->Target == pTarget)
			return false;

		if (pThis->Owner && checkOwner)
		{
			if (pThis->Owner->Owner->IsAlliedWith(pTarget))
				return false;
		}
	}

	return true;
}
NOINLINE void Shrapnel_Exec(BulletClass* pThis)
{
	auto const pType = pThis->Type;
	auto const pShrapWeapon = pType->ShrapnelWeapon;

	if (!pShrapWeapon)
		return;

	const int nCount = GetShrapAmount(pThis);
	if (nCount <= 0)
		return;

	auto const pBulletCell = pThis->GetCell();
	if (AllowShrapnel(pThis, pBulletCell)) {
		auto const nRange = pShrapWeapon->Range / Unsorted::LeptonsPerCell;

		if (nRange >= 1) {
			int nTotal = 0;

			CellRangeIterator<CellClass> {}(pThis->GetMapCoords(), nRange,
			[&](CellClass* pCell) -> bool {
				 auto const pTarget = pCell->FirstObject;
				if (ShrapnelTargetEligible(pThis, pTarget))
				{
					const auto pShrapExt = BulletTypeExt::ExtMap.Find(pShrapWeapon->Projectile);

					if (auto pBullet = pShrapExt->CreateBullet(pTarget, pThis->Owner, pShrapWeapon))
					{
						pBullet->MoveTo(pThis->Location, GenerateVelocity(pThis, pTarget, pShrapWeapon->Speed));


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
					if (++nTotal == nCount)
						return false;
				}
				return true;
			});

			auto const nRemaining = abs(nCount - nTotal);

			// get random coords for last remaining shrapnel if the total still less than ncount
			if (nRemaining)
			{
				for (int nAmountHere = nRemaining; nAmountHere > 0; --nAmountHere)
				{
					const short nX = ScenarioClass::Instance->Random.RandomRangedSpecific<short>(-2, 2);
					const short nY = ScenarioClass::Instance->Random.RandomRangedSpecific<short>(-2, 2);
					CellStruct nNextCoord = pThis->GetMapCoords();
					nNextCoord.X += nX;
					nNextCoord.Y += nY;
					if (const auto pCellTarget = Map[nNextCoord])
					{
						const auto pShrapExt = BulletTypeExt::ExtMap.Find(pShrapWeapon->Projectile);

						if (const auto pBullet = pShrapExt->CreateBullet(pCellTarget, pThis->Owner, pShrapWeapon))
						{
							pBullet->MoveTo(pThis->Location, GenerateVelocity(pThis, pCellTarget, pShrapWeapon->Speed));

#ifdef COMPILE_PORTED_DP_FEATURES
							auto sourcePos = pThis->Location;
							auto targetPos = pCellTarget->GetCoords();

							// Draw bullet effect
							Helpers_DP::DrawBulletEffect(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pCellTarget);
							// Draw particle system
							Helpers_DP::AttachedParticleSystem(pShrapWeapon, sourcePos, pCellTarget, pThis->Owner, targetPos);
							// Play report sound
							Helpers_DP::PlayReportSound(pShrapWeapon, sourcePos);
							// Draw weapon anim
							Helpers_DP::DrawWeaponAnim(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pCellTarget);
#endif
						}
					}
				}
			}
		}
	}
}

DEFINE_HOOK(0x46A310, BulletClass_Shrapnel_Replace, 0x6)
{
	GET(BulletClass*, pThis, ECX);
	Shrapnel_Exec(pThis);
	return 0x46ADD4;
}
