#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Cell/Body.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include "Trajectories/PhobosTrajectory.h"

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>

#include <New/Entity/FlyingStrings.h>

#include <InfantryClass.h>

#include <Phobos.SaveGame.h>

static bool IsAllowedSplitsTarget(TechnoClass* pSource, HouseClass* pOwner, WeaponTypeClass* pWeapon, TechnoClass* pTarget , bool useverses)
{
	auto const pWH = pWeapon->Warhead;
	auto const pType = pTarget->GetTechnoType();
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (!pType->LegalTarget || !pWHExt->CanDealDamage(pTarget,false,!useverses))
		return false;

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pWeaponExt->SkipWeaponPicking)
		return true;

	if (!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, pTarget->Owner)
			|| !EnumFunctions::IsCellEligible(pTarget->GetCell(), pWeaponExt->CanTarget, true, true)
			|| !EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget))
	{
		return false;
	}

	if(!TechnoExtData::ObjectHealthAllowFiring(pTarget, pWeapon))
		return false;

	if (!pWeaponExt->HasRequiredAttachedEffects(pTarget, pSource))
		return false;

	return true;
}

void BulletExtData::ApplyArcingFix(const CoordStruct& sourceCoords, const CoordStruct& targetCoords, VelocityClass& velocity)
{
	const auto distanceCoords = targetCoords - sourceCoords;
	const auto horizontalDistance = Point2D { distanceCoords.X, distanceCoords.Y }.Length();
	const bool lobber = (This()->WeaponType && This()->WeaponType->Lobber) || static_cast<int>(horizontalDistance) < distanceCoords.Z; // 0x70D590
	// The lower the horizontal velocity, the higher the trajectory
	// WW calculates the launch angle (and limits it) before calculating the velocity
	// Here, some magic numbers are used to directly simulate its calculation
	const auto speedMult = (lobber ? 0.45 : (distanceCoords.Z > 0 ? 0.68 : 1.0)); // Simulated 0x48A9D0
	const double gravity = BulletTypeExtData::GetAdjustedGravity(This()->Type);
	const double speed = speedMult * Math::sqrt(horizontalDistance * gravity * 1.2); // 0x48AB90

	if (horizontalDistance < 1e-10 || speed< 1e-10)
	{
		// No solution
		velocity.Z = speed;
	}
	else
	{
		const auto mult = speed / horizontalDistance;
		velocity.X = static_cast<double>(distanceCoords.X) * mult;
		velocity.Y = static_cast<double>(distanceCoords.Y) * mult;
		velocity.Z = (static_cast<double>(distanceCoords.Z) + velocity.Z) * mult + (gravity * horizontalDistance) / (2 * speed);
	}
}

void BulletExtData::ApplyAirburst(BulletClass* pThis)
{
	const auto pType = pThis->Type;
	const auto pExt = BulletTypeExtContainer::Instance.Find(pType);

	auto const GetWeapon = [pExt, pType]()
	{
		if (pExt->AirburstWeapons.empty())
			return pType->AirburstWeapon;

		return pExt->AirburstWeapons[ScenarioClass::Instance->Random.RandomFromMax(pExt->AirburstWeapons.size() - 1)];
	};

	if (WeaponTypeClass* pWeapon = GetWeapon())
	{
		const auto pBulletExt = BulletExtContainer::Instance.Find(pThis);
		TechnoClass* pBulletOwner = pThis->Owner ? pThis->Owner : nullptr;
		HouseClass* pBulletHouseOwner = pBulletOwner ? pBulletOwner->GetOwningHouse() : (pBulletExt ? pBulletExt->Owner : HouseExtData::FindNeutral());

		auto& random = ScenarioClass::Instance->Random;

		// some defaults
		int cluster = pType->Cluster;

		// get final target coords and cell
		const CoordStruct crdDest = pExt->AroundTarget.Get(pExt->Splits.Get())
			? pThis->GetBulletTargetCoords() : pThis->GetCoords();

		const CellStruct cellDest = CellClass::Coord2Cell(crdDest);

		// create a list of cluster targets
		std::vector<AbstractClass*> targets;
		targets.reserve(cluster + 1);

		if (!pExt->Splits.Get())
		{
			// default hardcoded YR way: hit each cell around the destination once

			// fill target list with cells around the target
			CellRangeIterator<CellClass>{}(cellDest, pExt->AirburstSpread.Get(),
			[&targets](CellClass* const pCell) -> bool {
				 targets.push_back(pCell);
				 return true;
			});

			if (pExt->Airburst_UseCluster) {
				HelperedVector<AbstractClass*> newTargets;
				newTargets.reserve(cluster + 1);

				if (pExt->Airburst_RandomClusters)
				{
					// Do random cells for amount matching Cluster.
					int count = 0;
					int targetCount = targets.size();

					while (count < cluster)
					{
						int index = ScenarioClass::Instance->Random.RandomFromMax(targetCount - 1);
						auto const pTarget = targets[index];

						if (count > targetCount || newTargets.find(pTarget)  == newTargets.end()) {
							newTargets.push_back(pTarget);
							count++;
						}
					}
				}
				else
				{
					// Do evenly selected cells for amount matching Cluster.
					double stepSize = (targets.size() - 1.0) / (cluster - 1.0);

					for (int i = 0; i < cluster; i++) {
						newTargets.push_back(targets[(static_cast<int>(round(stepSize * i)))]);
					}
				}

				targets = newTargets;
			}
			else
			{
				// we want as many as we get, not more, not less
				cluster = (int)targets.size();
			}

			// we want as many as we get, not more, not less
			//cluster = (int)targets.size();
		}
		else
		{
			const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);

			// this thing , i hope compiler optimized properly ,..
			// if not i will make another variant of the function that pass the vector referece instead of doing this
			targets = Helpers::Alex::getCellTechnoRangeItems(crdDest, pExt->Splits_Range, true, true, [pThis, pWeapon ,pWHExt, pExt , pBulletOwner , pBulletHouseOwner](AbstractClass* pTech){
				auto pTechno = static_cast<TechnoClass*>(pTech);
					if(!pExt->Splits_UseWeaponTargeting) {
						if(!pWHExt->CanDealDamage(pTechno, false, !pExt->Splits_TargetingUseVerses.Get()))
							return false;

						if (!pTechno->IsInPlayfield || !pTechno->IsOnMap || (!pExt->RetargetOwner.Get() && pTechno == pBulletOwner))
							return false;

						if (pWHExt->CanTargetHouse(pBulletHouseOwner, pTechno))
						{
							const auto nLayer = pTechno->InWhichLayer();

							if (nLayer == Layer::Underground || nLayer == Layer::None)
								return false;

							if (((!pTechno->IsInAir() && pWeapon->Projectile->AG)
								|| (pTechno->IsInAir() && pWeapon->Projectile->AA)))
							{
								return true;
							}
						}
					}
					else
					{
						if (pTechno->IsInPlayfield && pTechno->IsOnMap && pTechno->Health > 0 && (pExt->RetargetOwner || pTechno != pThis->Owner))
						{
							auto const coords = pTechno->GetCoords();

							if ((pThis->Type->AA || !pTechno->IsInAir())
								&& IsAllowedSplitsTarget(pBulletOwner, pBulletHouseOwner, pWeapon, pTechno, pExt->Splits_TargetingUseVerses))
							{
								return true;
							}
						}

					}
				return false;
			});

			if (pExt->Splits_FillRemainingClusterWithRandomcells)
			{
				// fill up the list to cluster count with random cells around destination
				const int nMinRange = pExt->Splits_RandomCellUseHarcodedRange.Get() ? pExt->Splits_TargetCellRange : pWeapon->MinimumRange / Unsorted::LeptonsPerCell;
				const int nMaxRange = pExt->Splits_RandomCellUseHarcodedRange.Get() ? pExt->Splits_TargetCellRange : pWeapon->Range / Unsorted::LeptonsPerCell;

				while ((int)targets.size() < (cluster))
				{
					const int x = random.RandomRanged(-nMinRange, nMaxRange);
					const int y = random.RandomRanged(-nMinRange, nMaxRange);

					const CellStruct cell { static_cast<short>(x + cellDest.X), static_cast<short>(cellDest.Y + y) };
					targets.push_back(MapClass::Instance->GetCellAt(cell));
				}
			}
			else
			{
				cluster = targets.size();
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
				int index = random.RandomFromMax(targets.size() - 1);
				pTarget = targets[index];

				// firer would hit itself
				if (pTarget == pThis->Owner)
				{
					if (random.RandomDouble() > pExt->RetargetSelf_Probability)
					{
						index = random.RandomFromMax(targets.size() - 1);
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
					Debug::LogInfo("Airburst [{}] targeting Target [{}] ", pWeapon->get_ID(), pTechno->get_ID());
#endif
				if (const auto pBullet = BulletTypeExtContainer::Instance
					.Find(pWeapon->Projectile)->CreateBullet(pTarget, pThis->Owner, pBulletHouseOwner,  pWeapon, pExt->AirburstWeapon_ApplyFirepowerMult, true))
				{

#ifdef _OLD_AIRBURSt
					DirStruct const dir(5, random.RandomRangedSpecific<short>(0, 32));
					auto const radians = dir.GetRadian();

					auto const sin_rad = Math::sin(radians);
					auto const cos_rad = Math::cos(radians);
					auto const cos_factor = Math::APPROX_CORRECTION_TERM_1;
					auto const flatSpeed = cos_factor * pBullet->Speed;

					pBullet->MoveTo(pThis->Location,
						{ cos_rad * flatSpeed ,sin_rad * flatSpeed , static_cast<double>(-pBullet->Speed) });

					auto sourcePos = pThis->Location;
					auto targetPos = pTarget->GetCoords();

					// Draw bullet effect
					Helpers_DP::DrawBulletEffect(pWeapon, sourcePos, targetPos, pBulletOwner, pTarget);
					// Draw particle system
					Helpers_DP::AttachedParticleSystem(pWeapon, sourcePos, pTarget, pBulletOwner, targetPos);
					// Play report sound
					Helpers_DP::PlayReportSound(pWeapon, sourcePos , pThis->Owner);
					// Draw weapon anim
					Helpers_DP::DrawWeaponAnim(pWeapon, sourcePos, targetPos, pBulletOwner, pTarget);
#else
					auto coords = pThis->Location;
					int scatterMin = pExt->AirburstWeapon_SourceScatterMin.Get();
					int scatterMax = pExt->AirburstWeapon_SourceScatterMax.Get();

					if (pType->Airburst && pExt->Airburst_TargetAsSource)
					{
						coords = pTarget->GetCoords();

						if (pExt->Airburst_TargetAsSource_SkipHeight)
							coords.Z = pThis->Location.Z;
					}

					if (scatterMin > 0 || scatterMax > 0)
					{
						int distance = ScenarioClass::Instance->Random.RandomRanged(scatterMin, scatterMax);
						coords = MapClass::GetRandomCoordsNear(coords, distance, false);
					}

					BulletExtData::SimulatedFiringUnlimbo(pBullet, pBulletHouseOwner, pWeapon, coords, true);
					BulletExtData::SimulatedFiringEffects(pBullet, pBulletHouseOwner, nullptr, false, true);
#endif
				}
			}
		}
	}
}

void BulletExtData::CreateAttachedSystem()
{
	auto pThis = This();

	if (!this->AttachedSystem)
	{
		if (auto pAttach = BulletTypeExtContainer::Instance.Find(pThis->Type)->AttachedSystem)
		{
			const auto pOwner = pThis->Owner ? pThis->Owner->GetOwningHouse() : this->Owner;

			this->AttachedSystem.reset(GameCreate<ParticleSystemClass>(
				pAttach,
				pThis->Location,
				pThis->Owner,
				pThis,
				CoordStruct::Empty,
				pOwner
			));
		}
	}
}

VelocityClass BulletExtData::GenerateVelocity(BulletClass* pThis, AbstractClass* pTarget, const int nSpeed, bool bCalculateSpeedFirst)
{
	VelocityClass velocity { 100.0 ,0.0,0.0 };
	//OPTIONALINLINE get Direction from 2 coords
	CoordStruct const nCenter = pTarget->GetCoords();
	DirStruct const dir_fromXY((double)(pThis->Location.Y - nCenter.Y), (double)(pThis->Location.X - nCenter.X));
	double const nFirstMag = velocity.LengthXY();
	double const radians_fromXY = dir_fromXY.GetRadian();
	double const sin_rad = Math::sin(radians_fromXY);
	double const cos_rad = Math::cos(radians_fromXY);

	double nMult_Cos = Math::COS_PI_BY_FOUR_ACCURATE;
	double nMult_Sin = Math::SIN_PI_BY_FOUR_ACCURATE;
	velocity.X = cos_rad * nFirstMag;
	velocity.Y -= sin_rad * nFirstMag;

	if (!bCalculateSpeedFirst)
	{
		double const nSecMag = velocity.LengthXY();
		DirStruct const dir_forZ(velocity.Z, nSecMag);
		double const radians_foZ = dir_forZ.GetRadian();
		double const nThirdMag = velocity.LengthXY();

		if (radians_foZ != 0.0)
		{
			velocity.X /= Math::cos(radians_foZ);
			velocity.Y /= Math::cos(radians_foZ);
		}

		velocity.X *= (double)nMult_Cos;
		velocity.Y *= (double)nMult_Cos;
		velocity.Z *= nMult_Sin * nThirdMag;
		velocity.SetIfZeroXYZ();

		const double nFullMag = velocity.Length();
		const double nDevidedBySpeed = nSpeed / nFullMag;
		velocity *= nDevidedBySpeed;
	}
	else
	{
		const double nFullMag = velocity.Length();
		const double nDevidedBySpeed = nSpeed / nFullMag;
		velocity *= nDevidedBySpeed;
		double const nSecMag = velocity.LengthXY();
		DirStruct const dir_forZ(velocity.Z, nSecMag);
		double const radians_foZ = dir_forZ.GetRadian();
		double const nThirdMag = velocity.LengthXY();

		if (radians_foZ != 0.0)
		{
			velocity.X /= Math::cos(radians_foZ);
			velocity.Y /= Math::cos(radians_foZ);
		}

		velocity.X *= nMult_Cos;
		velocity.Y *= nMult_Cos;
		velocity.Z *= nMult_Sin * nThirdMag;

		velocity.SetIfZeroXYZ();
	}

	return velocity;
}

int BulletExtData::GetShrapAmount(BulletClass* pThis)
{
	if (pThis->Type->ShrapnelCount < 0)
	{
		if (const auto pOwner = pThis->Owner)
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

bool BulletExtData::AllowShrapnel(BulletClass* pThis, CellClass* pCell)
{
	auto const pData = BulletTypeExtContainer::Instance.Find(pThis->Type);

	if (pData->Shrapnel_Chance.isset()
		&& ScenarioClass::Instance->Random.RandomDouble() < Math::abs(pData->Shrapnel_Chance.Get())) {
			return false;
	}

	if (const auto pObject = pCell->FirstObject)
	{
		if ((((DWORD*)pObject)[0]) != BuildingClass::vtable || pData->Shrapnel_AffectsBuildings)
			return true;
	}
	else if (pData->Shrapnel_AffectsGround)
		return true;

	return false;
}

bool BulletExtData::ShrapnelTargetEligible(BulletClass* pThis, AbstractClass* pTarget, bool checkOwner)
{
	if (!pTarget || pThis->Target == pTarget)
		return false;

	const auto pWH = pThis->Type->ShrapnelWeapon->Warhead;
	const auto pBulletExt = BulletTypeExtContainer::Instance.Find(pThis->Type);

	if (pBulletExt->Shrapnel_UseWeaponTargeting)
	{
		const auto pWhExt = WarheadTypeExtContainer::Instance.Find(pWH);
		const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pThis->Type->ShrapnelWeapon);

		if (const auto pTargetObj = flag_cast_to<ObjectClass* , false>(pTarget))
		{
			if(!pWeaponExt->SkipWeaponPicking && !TechnoExtData::ObjectHealthAllowFiring(pTargetObj, pThis->Type->ShrapnelWeapon))
				return false;

			auto pTargetType = pTargetObj->GetType();

			switch ((((DWORD*)pTargetObj)[0]))
			{
			case AircraftClass::vtable:
			case InfantryClass::vtable:
			case UnitClass::vtable:
			case BuildingClass::vtable:
			{

				if (!pTargetType->LegalTarget || !pWhExt->CanDealDamage(static_cast<TechnoClass*>(pTargetObj), false, false))
					return false;

				if(!pWeaponExt->SkipWeaponPicking){
					if (!EnumFunctions::IsTechnoEligible(static_cast<TechnoClass*>(pTargetObj), pWeaponExt->CanTarget))
						return false;

					if (!pWeaponExt->HasRequiredAttachedEffects(static_cast<TechnoClass*>(pTargetObj), pThis->Owner))
						return false;
				}
			}
			break;
			default:
			{
				if (const auto pType = pTargetObj->GetType()) {
					if (GeneralUtils::GetWarheadVersusArmor(pWH, pType->Armor) < 0.001) {
						return false;
					}
				}
			}
			break;
			}

			if(!pWeaponExt->SkipWeaponPicking) {
				if (!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner, pTargetObj->GetOwningHouse()))
					return false;

				if (!EnumFunctions::IsCellEligible(pTargetObj->GetCell(), pWeaponExt->CanTarget, true, true))
					return false;
			}
		}
		else if (pTarget->WhatAmI() == CellClass::AbsID) {
			if (!pWeaponExt->SkipWeaponPicking && !EnumFunctions::IsCellEligible((CellClass*)pTarget, pWeaponExt->CanTarget, true, true)) {
				return false;
			}
		}
	}
	else {
		if (pThis->Owner && checkOwner) {
			if (pThis->Owner->Owner->IsAlliedWith(pTarget)) {
				return false;
			}
		}
	}

	return true;
}

void BulletExtData::ApplyShrapnel(BulletClass* pThis)
{
	auto const pType = pThis->Type;
	auto const pShrapWeapon = pType->ShrapnelWeapon;

	if (!pShrapWeapon)
		return;

	const int nCount = BulletExtData::GetShrapAmount(pThis);
	if (nCount <= 0)
		return;

	auto const pBulletCell = pThis->GetCell();
	if (BulletExtData::AllowShrapnel(pThis, pBulletCell))
	{
		auto const nRange = pShrapWeapon->Range.ToCell();

		if (nRange >= 1)
		{
			int nTotal = 0;

			for (CellSpreadEnumerator it((short)nRange); it; ++it)
			{
				auto cellhere = (pBulletCell->MapCoords + *it);
				auto pCurCell = MapClass::Instance->GetCellAt(cellhere);
				auto const pTarget = pCurCell->FirstObject;
				if (BulletExtData::ShrapnelTargetEligible(pThis, pTarget))
				{
					const auto pShrapExt = BulletTypeExtContainer::Instance.Find(pShrapWeapon->Projectile);

					if (auto pBullet = pShrapExt->CreateBullet(pTarget, pThis->Owner, pShrapWeapon))
					{
						pBullet->MoveTo(pThis->Location, BulletExtData::GenerateVelocity(pThis, pTarget, pShrapWeapon->Speed));
						BulletExtData::SimulatedFiringEffects(pBullet, pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pBullet)->Owner, pThis->Owner, true, true);
					}

					//escapes
					if (++nTotal == nCount)
						break; //stop the loop
				}

			}

			auto const nRemaining = Math::abs(nCount - nTotal);

			// get random coords for last remaining shrapnel if the total still less than ncount
			if (nRemaining)
			{
				for (int nAmountHere = nRemaining; nAmountHere > 0; --nAmountHere)
				{
					const short nX = ScenarioClass::Instance->Random.RandomRangedSpecific<short>(-2, 2);
					const short nY = ScenarioClass::Instance->Random.RandomRangedSpecific<short>(-2, 2);
					CellStruct nNextCoord = pThis->InlineMapCoords();
					nNextCoord.X += nX;
					nNextCoord.Y += nY;
					if (const auto pCellTarget = MapClass::Instance->GetCellAt(nNextCoord))
					{
						const auto pShrapExt = BulletTypeExtContainer::Instance.Find(pShrapWeapon->Projectile);

						if (const auto pBullet = pShrapExt->CreateBullet(pCellTarget, pThis->Owner, pShrapWeapon))
						{
							pBullet->MoveTo(pThis->Location, BulletExtData::GenerateVelocity(pThis, pCellTarget, pShrapWeapon->Speed, true));
							BulletExtData::SimulatedFiringEffects(pBullet, pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pBullet)->Owner, pThis->Owner, true, true);
						}
					}
				}
			}
		}
	}
}

bool BulletExtData::HandleBulletRemove(BulletClass* pThis, bool bDetonate, bool bRemove)
{
	if (bDetonate)
		pThis->Detonate(pThis->GetCoords());

	if (bRemove)
	{
		if (!pThis->InLimbo)
			pThis->Limbo();

		const auto pTechno = pThis->Owner;
		const bool isLimbo =
			pTechno &&
			pTechno->InLimbo &&
			pThis->WeaponType &&
			pThis->WeaponType->LimboLaunch;

		if (isLimbo)
		{
			pThis->SetTarget(nullptr);
			auto damage = pTechno->GetType()->Strength;
			pTechno->SetLocation(pThis->GetCoords());
			pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}

		GameDelete<true,false>(pThis);
		return true;
	}

	return false;
}

bool BulletExtData::ApplyMCAlternative(BulletClass* pThis)
{
	if (!pThis->WH->MindControl || !pThis->Owner)
		return false;

	auto const pWarheadExt = WarheadTypeExtContainer::Instance.Find(pThis->WH);
	//if(!pWarheadExt->MindControl_UseTreshold)
	//	return false;

	const auto pTarget = flag_cast_to<TechnoClass*>(pThis->Target);

	if(!pTarget || !pTarget->IsAlive)
		return false;

	//const auto pTargetType = pTarget->GetTechnoType();
	const double currentHealthPerc = pTarget->GetHealthPercentage();
	const bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;
	double nTreshold = pWarheadExt->MindControl_Threshold;

	// If inversed threshold: mind control works until the target health is lower than the threshold percentage.
	// If not inversed: mind control only works when the target health is lower than the threshold percentage.
	if (nTreshold < 0.0 || nTreshold > 1.0)
		nTreshold = flipComparations ? 0.0 : 1.0;

	// Targets health threshold calculations
	const bool skipMindControl = flipComparations ? (nTreshold > 0.0) : (nTreshold < 1.0);
	const bool healthComparation = flipComparations ? (currentHealthPerc <= nTreshold) : (currentHealthPerc >= nTreshold);

	if (skipMindControl
		&& healthComparation
		&& pWarheadExt->MindControl_AlternateDamage.isset()
		&& pWarheadExt->MindControl_AlternateWarhead.isset())
	{
		// Alternate damage
		const int altDamage = pWarheadExt->MindControl_AlternateDamage.Get();
		// Alternate warhead
		WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead.Get();
		// Get the damage the alternate warhead can produce against the target
		auto const armor_ = TechnoExtData::GetArmor(pTarget);
		int realDamage = FakeWarheadTypeClass::ModifyDamage(altDamage, pAltWarhead, armor_, 0);
		const int animDamage = realDamage;

		const auto pAttacker = pThis->Owner;
		const auto pAttackingHouse = pThis->Owner->Owner;

		// Keep the target alive if necessary
		if (!pWarheadExt->MindControl_CanKill.Get() && realDamage >= pTarget->Health)
			realDamage = pTarget->Health - 1;

		pTarget->ReceiveDamage(&realDamage, 0, pAltWarhead, pAttacker, true, false, pAttackingHouse);

		// Alternate Warhead's animation from AnimList
		const auto nLandType = MapClass::Instance()->GetCellAt(pTarget->Location)->LandType;

		if (const auto pAnimType = MapClass::SelectDamageAnimation(animDamage, pAltWarhead, nLandType, pTarget->Location))
		{
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, pTarget->Location),
				pThis->Owner->Owner,
				pTarget->Owner,
				pThis->Owner,
				false, false
			);
		}

		return true;
	}

	// Run mind control code
	return false;
}

bool BulletExtData::IsReallyAlive(BulletClass* pThis)
{
	return pThis && pThis->IsAlive;
}

HouseClass* BulletExtData::GetHouse(BulletClass* const pThis)
{
	if (pThis->Owner) {
		return pThis->Owner->Owner;
	}

	return BulletExtContainer::Instance.Find(pThis)->Owner;
}

void BulletExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved) {

	this->ObjectExtData::InvalidatePointer(ptr, bRemoved);

	AnnounceInvalidPointer(OriginalTarget, ptr, bRemoved);
	AnnounceInvalidPointer(Owner , ptr);
	AnnounceInvalidPointer(NukeSW, ptr);

	if (auto& pTraj = Trajectory)
		pTraj->InvalidatePointer(ptr, bRemoved);

	if (this->AttachedSystem == ptr)
		this->AttachedSystem.detachptr();
 }

void BulletExtData::ApplyRadiationToCell(CellClass* pCell, int Spread, int RadLevel)
{
	const auto pThis = This();
	const auto pWeapon = pThis->GetWeaponType();
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	const auto pRadType = pWeaponExt->RadType.Get(RadTypeClass::FindOrAllocate(GameStrings::Radiation()));
	auto const pCellExt = CellExtContainer::Instance.Find(pCell);

	auto const it = pCellExt->RadSites.find_if([=](RadSiteClass* const pSite) {
		if (pSite->RadTimeLeft <= 0)
			return false;

		auto const pRadExt = RadSiteExtContainer::Instance.Find(pSite);
		if (pRadExt->Type != pRadType)
			return false;

		if (Spread != pSite->Spread)
			return false;

		if (pWeapon != pRadExt->Weapon)
			return false;

		if (pRadExt->TechOwner && pThis->Owner)
			return pRadExt->TechOwner == pThis->Owner;

		return true;
	});

	if (it != pCellExt->RadSites.end()) {

		const auto nMax = pRadType->GetLevelMax();
		const auto nCurrent = (*it)->GetCurrentRadLevel();

		if (nCurrent + RadLevel > nMax) {
			RadLevel = nMax - nCurrent;
		}

		// Handle It
		RadSiteExtContainer::Instance.Find((*it))->Add(RadLevel);
		return;
	}

	RadSiteExtData::CreateInstance(pCell, Spread, RadLevel, pWeaponExt, pThis->Owner);
}

void BulletExtData::InitializeLaserTrails()
{
	const auto pThis = This();

	if (!LaserTrails.empty())
		return;

	auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);
	if (!pTypeExt || pTypeExt->LaserTrail_Types.empty())
		return;

	const auto pOwner = pThis->Owner ?
		pThis->Owner->Owner : (this->Owner ? this->Owner : HouseExtData::FindFirstCivilianHouse());

	this->LaserTrails.reserve(pTypeExt->LaserTrail_Types.size());

	for (auto const& idxTrail: pTypeExt->LaserTrail_Types) {
		this->LaserTrails.emplace_back(
			std::move(std::make_unique<LaserTrailClass>(
			LaserTrailTypeClass::Array[idxTrail].get(), pOwner->LaserColor)));
	}
}

void BulletExtData::InterceptBullet(BulletClass* pThis, TechnoClass* pSource, BulletClass* pInterceptor)
{
	if (pInterceptor)
	{

		auto const pExt = BulletExtContainer::Instance.Find(pThis);
		auto pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);
		const auto pInterceptorType = TechnoTypeExtContainer::Instance.Find(BulletExtContainer::Instance.Find(pInterceptor)->InterceptorTechnoType);

		if (!pTypeExt->Armor.isset())
		{
			if (!pInterceptorType->Interceptor_KeepIntact)
				pExt->InterceptedStatus |= InterceptedStatus::Intercepted;
		}
		else
		{
			auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pInterceptor->WH);
			const bool canAffect = Math::abs(pWHExt->GetVerses(pTypeExt->Armor.Get()).Verses) >= 0.001;

			if (canAffect)
			{

				const int damage = static_cast<int>(pInterceptor->Health * pWHExt->GetVerses(pTypeExt->Armor.Get()).Verses);
				pExt->CurrentStrength -= damage;

				FlyingStrings::Instance.DisplayDamageNumberString(damage, DamageDisplayType::Intercept, pThis->GetRenderCoords(), pExt->DamageNumberOffset , Phobos::Debug_DisplayDamageNumbers);

				if (pExt->CurrentStrength <= 0)
				{
					pExt->CurrentStrength = 0;

					if (!pInterceptorType->Interceptor_KeepIntact)
						pExt->InterceptedStatus |= InterceptedStatus::Intercepted;
				}
			}
		}

		pExt->DetonateOnInterception = !pInterceptorType->Interceptor_DeleteOnIntercept.Get(pTypeExt->Interceptable_DeleteOnIntercept);

		if (const auto pWeaponOverride = pInterceptorType->Interceptor_WeaponOverride.Get(pTypeExt->Interceptable_WeaponOverride))
		{

			pThis->WeaponType = pWeaponOverride;
			pThis->Health = pInterceptorType->Interceptor_WeaponCumulativeDamage.Get() ? pThis->Health + pWeaponOverride->Damage : pWeaponOverride->Damage;
			pThis->WH = pWeaponOverride->Warhead;
			pThis->Bright = pWeaponOverride->Bright;

			if (pInterceptorType->Interceptor_WeaponReplaceProjectile
				&& pWeaponOverride->Projectile
				&& pWeaponOverride->Projectile != pThis->Type)
			{
				pThis->Speed = pWeaponOverride->Speed;
				pThis->Type = pWeaponOverride->Projectile;
				pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);

				pExt->LaserTrails.clear();
				pExt->InitializeLaserTrails();

				TrailsManager::CleanUp(pExt->This());
				TrailsManager::Construct(pExt->This());

				// Lose target if the current bullet is no longer interceptable.

				if (pSource && (!pTypeExt->Interceptable || (pTypeExt->Armor.isset() &&
							Math::abs(WarheadTypeExtContainer::Instance.Find(pInterceptor->WH)->GetVerses(pTypeExt->Armor.Get()).Verses) < 0.001)))
					pSource->SetTarget(nullptr);
			}
		}
	}
}

static COMPILETIMEEVAL bool TimerIsRunning(CDTimerClass& nTimer)
{
	const auto nStart = nTimer.StartTime;

	if (nStart == -1 && nTimer.TimeLeft) {
		return true;
	}

	auto nTimerLeft = nTimer.TimeLeft;

	if (Unsorted::CurrentFrame() - nStart < nTimerLeft) {
		nTimerLeft -= Unsorted::CurrentFrame() - nStart;

		if (nTimerLeft) {
			return true;
		}
	}

	return false;
}

Fuse BulletExtData::FuseCheckup(BulletClass* pBullet, CoordStruct* newlocation)
{
	auto& nFuse = pBullet->Data;

	if (TimerIsRunning(nFuse.ArmTimer))
		return Fuse::DontIgnite;

	const int proximity = (int)(newlocation->DistanceFrom(nFuse.Location)) / 2;

	int nProx = 32;
	const auto pExt = BulletTypeExtContainer::Instance.Find(pBullet->Type);
	if (pExt->Proximity_Range.isset())
		nProx = pExt->Proximity_Range.Get() * 256;

	if (proximity < nProx)
	{
		return Fuse::Ignite;
	}

	if (proximity < 256 && proximity > nFuse.Distance)
	{
		return Fuse::Ignite_DistaceFactor;
	}

	nFuse.Distance = proximity;

	return Fuse::DontIgnite;
}

// void BulletExtData::DetonateAt(BulletClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, CoordStruct nCoord , HouseClass* pBulletOwner)
// {
//
// 	if (!nCoord.IsValid() && pTarget)
// 		nCoord = pTarget->GetCoords();
//
// 	if(pBulletOwner && !pOwner) {
// 		BulletExtContainer::Instance.Find(pThis)->Owner = pBulletOwner;
// 	}
//
// 	pThis->Limbo();
// 	pThis->SetLocation(nCoord);
// 	pThis->Explode(true);
// 	pThis->UnInit();
//
// }


#pragma region BulletAffects
//TODO : fuckton more stuffs from the `Fire` function
//		 Use proper function call for some of these shits

static inline int SetBuildingFireAnimZAdjust(BuildingClass* pBuilding, int animY)
{
	if (pBuilding->GetOccupantCount() > 0)
		return -200;

	const auto renderCoords = pBuilding->GetRenderCoords();
	const auto zAdj = (animY - renderCoords.Y) / -4;
	return (zAdj >= 0) ? 0 : zAdj;
}

#include <Ext/Anim/Body.h>

// Make sure pBullet and pBullet->WeaponType is not empty before call
void BulletExtData::SimulatedFiringAnim(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach)
{
	const auto pWeapon = pBullet->WeaponType;
	const auto animCounts = pWeapon->Anim.Count;

	if (animCounts <= 0)
		return;

	const auto pFirer = pBullet->Owner;
	const auto pAnimType = pWeapon->Anim[(animCounts % 8 == 0) // Have direction
		? (static_cast<int>((std::atan2(pBullet->Velocity.Y, pBullet->Velocity.X) / Math::GAME_TWOPI + 1.5) * animCounts - (animCounts / 8) + 0.5) % animCounts) // Calculate direction
		: ScenarioClass::Instance->Random.RandomRanged(0, animCounts - 1)]; // Simple random;
	/*
		const auto velocityRadian = std::atan2(pBullet->Velocity.Y , pBullet->Velocity.X);
		const auto ratioOfRotateAngle = velocityRadian / Math::GAME_TWOPI;
		const auto correctRatioOfRotateAngle = ratioOfRotateAngle + 1.5; // Correct the Y-axis in reverse and ensure that the ratio is a positive number
		const auto animIndex = correctRatioOfRotateAngle * animCounts;
		const auto correctAnimIndex = animIndex - (animCounts / 8); // A multiple of 8 greater than 8 will have an additional offset
		const auto trueAnimIndex = static_cast<int>(correctAnimIndex + 0.5) % animCounts; // Round down and prevent exceeding the scope
	*/

	if (!pAnimType)
		return;

	const auto pAnim = GameCreate<AnimClass>(pAnimType, pBullet->SourceCoords);

	AnimExtData::SetAnimOwnerHouseKind(pAnim, pHouse, nullptr, pFirer,false, false);

	if (pAttach)
	{
		if (pAttach->WhatAmI() == AbstractType::Building)
			pAnim->ZAdjust = SetBuildingFireAnimZAdjust(static_cast<BuildingClass*>(pAttach), pBullet->SourceCoords.Y);
		else
			pAnim->SetOwnerObject(pAttach);
	}
	else if (const auto pBuilding = cast_to<BuildingClass*>(pFirer))
	{
		pAnim->ZAdjust = SetBuildingFireAnimZAdjust(pBuilding, pBullet->SourceCoords.Y);
	}
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
void BulletExtData::SimulatedFiringReport(BulletClass* pBullet)
{
	const auto pWeapon = pBullet->WeaponType;

	if (pWeapon->Report.Count <= 0)
		return;

	const auto pFirer = pBullet->Owner;
	const auto reportIndex = pWeapon->Report[(pFirer ? pFirer->weapon_sound_randomnumber_3C8 : ScenarioClass::Instance->Random.Random()) % pWeapon->Report.Count];
	VocClass::SafeImmedietelyPlayAt(reportIndex, & pBullet->Location, nullptr);
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
void BulletExtData::SimulatedFiringLaser(BulletClass* pBullet, HouseClass* pHouse)
{
	// Can not use 0x6FD210 because the firer may die
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon->IsLaser)
		return;

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (pWeapon->IsHouseColor || pWeaponExt->Laser_IsSingleColor)
	{
		const auto black = ColorStruct { 0, 0, 0 };
		const auto pLaser = GameCreate<LaserDrawClass>(pBullet->SourceCoords, (pBullet->Type->Inviso ? pBullet->Location : pBullet->TargetCoords),
			((pWeapon->IsHouseColor && pHouse) ? pHouse->LaserColor : pWeapon->LaserInnerColor), black, black, pWeapon->LaserDuration);

		pLaser->IsHouseColor = true;
		pLaser->Thickness = pWeaponExt->Laser_Thickness;
		pLaser->IsSupported = (pLaser->Thickness > 3);
	}
	else
	{
		const auto pLaser = GameCreate<LaserDrawClass>(pBullet->SourceCoords, (pBullet->Type->Inviso ? pBullet->Location : pBullet->TargetCoords),
			pWeapon->LaserInnerColor, pWeapon->LaserOuterColor, pWeapon->LaserOuterSpread, pWeapon->LaserDuration);

		pLaser->IsHouseColor = false;
		pLaser->Thickness = 3;
		pLaser->IsSupported = false;
	}
}

#include <Ext/Ebolt/Body.h>

// Make sure pBullet and pBullet->WeaponType is not empty before call
void BulletExtData::SimulatedFiringElectricBolt(BulletClass* pBullet)
{
	// Can not use 0x6FD460 because the firer may die
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon->IsElectricBolt)
		return;

	const auto pEBolt = EboltExtData::_CreateOneOf(pWeapon , nullptr);
	pEBolt->Fire(pBullet->SourceCoords, (pBullet->Type->Inviso ? pBullet->Location : pBullet->TargetCoords), 0);
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
void BulletExtData::SimulatedFiringRadBeam(BulletClass* pBullet, HouseClass* pHouse)
{
	const auto pWeapon = pBullet->WeaponType;

	if (!pWeapon->IsRadBeam)
		return;

	const auto pWH = pWeapon->Warhead;
	const bool isTemporal = pWH && pWH->Temporal;
	const auto pRadBeam = RadBeam::Allocate(isTemporal ? RadBeamType::Temporal : RadBeamType::RadBeam);

	pRadBeam->SetCoordsSource(pBullet->SourceCoords);
	pRadBeam->SetCoordsTarget((pBullet->Type->Inviso ? pBullet->Location : pBullet->TargetCoords));

	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	pRadBeam->Color = (pWeaponExt->Beam_IsHouseColor && pHouse) ? pHouse->LaserColor
		: pWeaponExt->Beam_Color.Get(isTemporal ? RulesClass::Instance->ChronoBeamColor : RulesClass::Instance->RadColor);

	pRadBeam->Period = pWeaponExt->Beam_Duration;
	pRadBeam->Amplitude = pWeaponExt->Beam_Amplitude;
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
void BulletExtData::SimulatedFiringParticleSystem(BulletClass* pBullet, HouseClass* pHouse)
{
	if (const auto pPSType = pBullet->WeaponType->AttachedParticleSystem)
	{
		GameCreate<ParticleSystemClass>(pPSType, pBullet->SourceCoords, pBullet->Target, pBullet->Owner,
			(pBullet->Type->Inviso ? pBullet->Location : pBullet->TargetCoords), pHouse);
	}
}

// Make sure pBullet is not empty before call
void BulletExtData::SimulatedFiringUnlimbo(BulletClass* pBullet, HouseClass* pHouse, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords, bool randomVelocity)
{
	// Velocity
	auto velocity = VelocityClass::Empty;
	if (pBullet->Type->FirersPalette)
			pBullet->InheritedColor = pHouse->ColorSchemeIndex;

	const auto gravity = BulletTypeExtData::GetAdjustedGravity(pBullet->Type);
	const auto targetCoords = pBullet->Target->GetCenterCoords();
	const auto distanceCoords = targetCoords - sourceCoords;
	const auto horizontalDistance = Point2D{distanceCoords.X, distanceCoords.Y}.Length();
	const bool lobber = pWeapon->Lobber || static_cast<int>(horizontalDistance) < distanceCoords.Z; // 0x70D590
	// The lower the horizontal velocity, the higher the trajectory
	// WW calculates the launch angle (and limits it) before calculating the velocity
	// Here, some magic numbers are used to directly simulate its calculation
	const auto speedMult = (lobber ? 0.45 : (distanceCoords.Z > 0 ? 0.68 : 1.0)); // Simulated 0x48A9D0
	const auto speed = static_cast<int>(speedMult * Math::sqrt(horizontalDistance * gravity * 1.2)); // 0x48AB90

	// Simulate firing Arcing bullet
	if (horizontalDistance < 1e-10 || speed < 1e-10)
	{
		// No solution
		velocity.Z = speed;
	}
	else
	{
		const auto mult = speed / horizontalDistance;

		velocity.X = static_cast<double>(distanceCoords.X) * mult;
		velocity.Y = static_cast<double>(distanceCoords.Y) * mult;
		velocity.Z = static_cast<double>(distanceCoords.Z) * mult + (gravity * horizontalDistance) / (2 * speed);
	}

	// Unlimbo
	pBullet->MoveTo(sourceCoords, velocity);
}

// Make sure pBullet and pBullet->WeaponType is not empty before call
void BulletExtData::SimulatedFiringEffects(BulletClass* pBullet, HouseClass* pHouse, ObjectClass* pAttach, bool firingEffect, bool visualEffect)
{
	if (firingEffect)
	{
		BulletExtData::SimulatedFiringAnim(pBullet, pHouse, pAttach);
		BulletExtData::SimulatedFiringReport(pBullet);
	}

	if (visualEffect)
	{
		BulletExtData::SimulatedFiringLaser(pBullet, pHouse);
		BulletExtData::SimulatedFiringElectricBolt(pBullet);
		BulletExtData::SimulatedFiringRadBeam(pBullet, pHouse);
		BulletExtData::SimulatedFiringParticleSystem(pBullet, pHouse);
	}
}

#pragma endregion
// =============================
// load / save

template <typename T>
void BulletExtData::Serialize(T& Stm)
{
	// Define the debug wrapper
	auto debugProcess = [&Stm](auto& field, const char* fieldName) -> auto&
		{
			if constexpr (std::is_same_v<T, PhobosStreamWriter>)
			{
				size_t beforeSize = Stm.Getstream()->Size();
				auto& result = Stm.Process(field);
				size_t afterSize = Stm.Getstream()->Size();
				GameDebugLog::Log("[BulletExtData] SAVE %s: size %zu -> %zu (+%zu)\n",
					fieldName, beforeSize, afterSize, afterSize - beforeSize);
				return result;
			}
			else
			{
				size_t beforeOffset = Stm.Getstream()->Offset();
				bool beforeSuccess = Stm.Success();
				auto& result = Stm.Process(field);
				size_t afterOffset = Stm.Getstream()->Offset();
				bool afterSuccess = Stm.Success();

				GameDebugLog::Log("[BulletExtData] LOAD %s: offset %zu -> %zu (+%zu), success: %s -> %s\n",
					fieldName, beforeOffset, afterOffset, afterOffset - beforeOffset,
					beforeSuccess ? "true" : "false", afterSuccess ? "true" : "false");

				if (!afterSuccess && beforeSuccess)
				{
					GameDebugLog::Log("[BulletExtData] ERROR: %s caused stream failure!\n", fieldName);
				}
				return result;
			}
		};

	// Use the debug wrapper for each field
	debugProcess(this->CurrentStrength, "CurrentStrength");
	debugProcess(this->InterceptorTechnoType, "InterceptorTechnoType");
	debugProcess(this->InterceptedStatus, "InterceptedStatus");
	debugProcess(this->DetonateOnInterception, "DetonateOnInterception");
	debugProcess(this->LaserTrails, "LaserTrails");
	debugProcess(this->SnappedToTarget, "SnappedToTarget");
	debugProcess(this->NukeSW, "NukeSW");
	debugProcess(this->BrightCheckDone, "BrightCheckDone");
	debugProcess(this->Owner, "Owner");
	debugProcess(this->Trails, "Trails");
	debugProcess(this->AttachedSystem, "AttachedSystem");
	debugProcess(this->DamageNumberOffset, "DamageNumberOffset");
	debugProcess(this->OriginalTarget, "OriginalTarget");
	debugProcess(this->ParabombFallRate, "ParabombFallRate");
	debugProcess(this->IsInstantDetonation, "IsInstantDetonation");
	PhobosTrajectory::ProcessFromStream(Stm, this->Trajectory);
}

// =============================
// container
BulletExtContainer BulletExtContainer::Instance;

bool BulletExtContainer::LoadAll(const json& root)
{
	this->Clear();

	if (root.contains(BulletExtContainer::ClassName))
	{
		auto& container = root[BulletExtContainer::ClassName];

		for (auto& entry : container[BulletExtData::ClassName])
		{

			uint32_t oldPtr = 0;
			if (!ExtensionSaveJson::ReadHex(entry, "OldPtr", oldPtr))
				return false;

			size_t dataSize = entry["datasize"].get<size_t>();
			std::string encoded = entry["data"].get<std::string>();
			auto buffer = this->AllocateNoInit();

			PhobosByteStream loader(dataSize);
			loader.data = std::move(Base64Handler::decodeBase64(encoded, dataSize));
			PhobosStreamReader reader(loader);

			PHOBOS_SWIZZLE_REGISTER_POINTER(oldPtr, buffer, BulletExtData::ClassName);

			buffer->LoadFromStream(reader);

			if (!reader.ExpectEndOfBlock())
				return false;
		}

		return true;
	}

	return false;
}

bool BulletExtContainer::SaveAll(json& root)
{
	auto& first_layer = root[BulletExtContainer::ClassName];

	json _extRoot = json::array();
	for (auto& _extData : BulletExtContainer::Array)
	{
		PhobosByteStream saver(sizeof(*_extData));
		PhobosStreamWriter writer(saver);

		_extData->SaveToStream(writer); // write all data to stream

		json entry;
		ExtensionSaveJson::WriteHex(entry, "OldPtr", (uint32_t)_extData);
		entry["datasize"] = saver.data.size();
		entry["data"] = Base64Handler::encodeBase64(saver.data);
		_extRoot.push_back(std::move(entry));
	}

	first_layer[BulletExtData::ClassName] = std::move(_extRoot);
	return true;
}

// =============================
// container hooks

//dont have noint
ASMJIT_PATCH(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);
	BulletExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x466501, BulletClass_InitSomeStuffs, 0x6) {
	GET(FakeBulletClass*, pItem, ECX);
	pItem->_GetExtData()->Name = pItem->Type->ID;
	return 0x0;
}

ASMJIT_PATCH(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);
	BulletExtContainer::Instance.Remove(pItem);
	return 0;
}

void FakeBulletClass::_Detach(AbstractClass* target , bool all)
{
	if(auto pExt = this->_GetExtData())
		pExt->InvalidatePointer(target, all);
	this->BulletClass::PointerExpired(target , all);
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E470C, FakeBulletClass::_Detach)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4744, FakeBulletClass::_AnimPointerExpired)