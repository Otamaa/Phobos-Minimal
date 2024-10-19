#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>

#include "Trajectories/PhobosTrajectory.h"

#include <Utilities/Macro.h>
#include <Lib/gcem/gcem.hpp>

#include <New/Entity/FlyingStrings.h>

static bool IsAllowedSplitsTarget(TechnoClass* pSource, HouseClass* pOwner, WeaponTypeClass* pWeapon, TechnoClass* pTarget , bool useverses)
{
	auto const pWH = pWeapon->Warhead;
	auto const pType = pTarget->GetTechnoType();
	const auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);

	if (!pType->LegalTarget || !pWHExt->CanDealDamage(pTarget,false,!useverses))
		return false;

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);

	if (!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pOwner, pTarget->Owner)
			|| !EnumFunctions::IsCellEligible(pTarget->GetCell(), pWeaponExt->CanTarget, true, true)
			|| !EnumFunctions::IsTechnoEligible(pTarget, pWeaponExt->CanTarget))
	{
		return false;
	}

	if (!pWeaponExt->HasRequiredAttachedEffects(pTarget, pSource))
		return false;

	return true;
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

				if (pExt->Airburst_RandomClusters)
				{
					// Do random cells for amount matching Cluster.
					int count = 0;
					int targetCount = targets.size();

					while (count < cluster)
					{
						int index = ScenarioClass::Instance->Random.RandomRanged(0, targetCount);
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
			targets = Helpers::Alex::getCellTechnoRangeItems(crdDest, pExt->Splits_Range, true, [pThis, pWeapon ,pWHExt, pExt , pBulletOwner , pBulletHouseOwner](AbstractClass* pTech){
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
					Debug::Log("Airburst [%s] targeting Target [%s] \n", pWeapon->get_ID(), pTechno->get_ID());
#endif
				if (const auto pBullet = BulletTypeExtContainer::Instance
					.Find(pWeapon->Projectile)->CreateBullet(pTarget, pThis->Owner, pWeapon, true , true))
				{
					DirStruct const dir(5, random.RandomRangedSpecific<short>(0, 32));
					auto const radians = dir.GetRadian();

					auto const sin_rad = Math::sin(radians);
					auto const cos_rad = Math::cos(radians);
					auto const cos_factor = -2.44921270764e-16;
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
				}
			}
		}
	}
}

void BulletExtData::CreateAttachedSystem()
{
	auto pThis = this->AttachedToObject;

	if (!this->AttachedSystem)
	{
		if (auto pAttach = BulletTypeExtContainer::Instance.Find(pThis->Type)->AttachedSystem)
		{
			const auto pOwner = pThis->Owner ? pThis->Owner->GetOwningHouse() : this->Owner;

			this->AttachedSystem = (GameCreate<ParticleSystemClass>(
				pAttach,
				pThis->Location,
				pThis->Owner,
				nullptr,
				CoordStruct::Empty,
				pOwner
			));
		}
	}
}

VelocityClass BulletExtData::GenerateVelocity(BulletClass* pThis, AbstractClass* pTarget, const int nSpeed, bool bCalculateSpeedFirst)
{
	VelocityClass velocity { 100.0 ,0.0,0.0 };
	//inline get Direction from 2 coords
	CoordStruct const nCenter = pTarget->GetCoords();
	DirStruct const dir_fromXY((double)(pThis->Location.Y - nCenter.Y), (double)(pThis->Location.X - nCenter.X));
	double const nFirstMag = velocity.LengthXY();
	double const radians_fromXY = dir_fromXY.GetRadian();
	double const sin_rad = Math::sin(radians_fromXY);
	double const cos_rad = Math::cos(radians_fromXY);
	constexpr double nMult_Cos = gcem::cos(0.7853262558535721);
	constexpr double nMult_Sin = gcem::sin(0.7853262558535721);

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

		velocity.X *= nMult_Cos;
		velocity.Y *= nMult_Cos;
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

		if (const auto pTargetObj = abstract_cast<ObjectClass*>(pTarget))
		{
			auto pTargetType = static_cast<TechnoClass*>(pTargetObj)->GetType();

			switch ((((DWORD*)pTargetObj)[0]))
			{
			case AircraftClass::vtable:
			case InfantryClass::vtable:
			case UnitClass::vtable:
			case BuildingClass::vtable:
			{

				if (!pTargetType->LegalTarget || !pWhExt->CanDealDamage(static_cast<TechnoClass*>(pTargetObj), false, false))
					return false;

				if (!EnumFunctions::IsTechnoEligible(static_cast<TechnoClass*>(pTargetObj), pWeaponExt->CanTarget))
					return false;

				if (!pWeaponExt->HasRequiredAttachedEffects(static_cast<TechnoClass*>(pTargetObj), pThis->Owner))
					return false;
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

			if (!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner ? pThis->Owner->Owner : BulletExtContainer::Instance.Find(pThis)->Owner, pTargetObj->GetOwningHouse()))
				return false;

			if (!EnumFunctions::IsCellEligible(pTargetObj->GetCell(), pWeaponExt->CanTarget, true, true))
				return false;
		}
		else if (pTarget->WhatAmI() == CellClass::AbsID) {
			if (!EnumFunctions::IsCellEligible((CellClass*)pTarget, pWeaponExt->CanTarget, true, true)) {
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

			CellRangeIterator<CellClass> {}(pThis->InlineMapCoords(), nRange,
			[&](CellClass* pCell) -> bool
 {
	 auto const pTarget = pCell->FirstObject;
	 if (BulletExtData::ShrapnelTargetEligible(pThis, pTarget))
	 {
		 const auto pShrapExt = BulletTypeExtContainer::Instance.Find(pShrapWeapon->Projectile);

		 if (auto pBullet = pShrapExt->CreateBullet(pTarget, pThis->Owner, pShrapWeapon))
		 {
			 pBullet->MoveTo(pThis->Location, BulletExtData::GenerateVelocity(pThis, pTarget, pShrapWeapon->Speed));

			 auto sourcePos = pThis->Location;
			 auto targetPos = pTarget->GetCoords();

			 // Draw bullet effect
			 // TODO : Ebolt and Laser Colors
			 Helpers_DP::DrawBulletEffect(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);

			 // Draw particle system
			 Helpers_DP::AttachedParticleSystem(pShrapWeapon, sourcePos, pTarget, pThis->Owner, targetPos);
			 // Play report sound
			 Helpers_DP::PlayReportSound(pShrapWeapon, sourcePos, pThis->Owner);
			 // Draw weapon anim
			 Helpers_DP::DrawWeaponAnim(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pTarget);

		 }

		 //escapes
		 if (++nTotal == nCount)
			 return false; //return false to stop the loop
	 }

	 return true; // return true to continue
			});

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

							auto sourcePos = pThis->Location;
							auto targetPos = pCellTarget->GetCoords();

							// Draw bullet effect
							Helpers_DP::DrawBulletEffect(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pCellTarget);
							// Draw particle system
							Helpers_DP::AttachedParticleSystem(pShrapWeapon, sourcePos, pCellTarget, pThis->Owner, targetPos);
							// Play report sound
							Helpers_DP::PlayReportSound(pShrapWeapon, sourcePos , pThis->Owner);
							// Draw weapon anim
							Helpers_DP::DrawWeaponAnim(pShrapWeapon, sourcePos, targetPos, pThis->Owner, pCellTarget);
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

	const auto pTarget = generic_cast<TechnoClass*>(pThis->Target);

	if(!pTarget || !pTarget->IsAlive)
		return false;

	const auto pTargetType = pTarget->GetTechnoType();
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
		int realDamage = MapClass::GetTotalDamage(altDamage, pAltWarhead, armor_, 0);
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
				false
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

bool BulletExtData::InvalidateIgnorable(AbstractClass* ptr)
{
	switch (VTable::Get(ptr))
	{
	case BuildingClass::vtable:
	case InfantryClass::vtable:
	case UnitClass::vtable:
	case AircraftClass::vtable:
	case HouseClass::vtable:
	case ParticleSystemClass::vtable:
		return false;
	}

	return true;

}

void BulletExtData::InvalidatePointer(AbstractClass* ptr, bool bRemoved) {

	AnnounceInvalidPointer(OriginalTarget, ptr, bRemoved);
	AnnounceInvalidPointer(Owner , ptr);
	AnnounceInvalidPointer(NukeSW, ptr);

	if (auto& pTraj = Trajectory)
		pTraj->InvalidatePointer(ptr, bRemoved);

	if (this->AttachedSystem == ptr)
		this->AttachedSystem= nullptr;
 }

void BulletExtData::ApplyRadiationToCell(CoordStruct const& nCoord, int Spread, int RadLevel)
{
	if (!MapClass::Instance->IsWithinUsableArea(nCoord))
		return;

	const auto pThis = this->AttachedToObject;
	const auto pWeapon = pThis->GetWeaponType();
	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
	const auto pRadType = pWeaponExt->RadType.Get(RadTypeClass::Array[0].get());

		auto const it = RadSiteClass::Array->find_if([=](RadSiteClass* const pSite) {

				auto const pRadExt = RadSiteExtContainer::Instance.Find(pSite);
				if (pRadExt->Type != pRadType)
					return false;

				if (pSite->BaseCell != CellClass::Coord2Cell(nCoord))
					return false;

				if (Spread != pSite->Spread)
					return false;

				if (pWeapon != pRadExt->Weapon)
					return false;

				if (pRadExt->TechOwner && pThis->Owner)
					return pRadExt->TechOwner == pThis->Owner;

				return true;
			});

		if (it != RadSiteClass::Array->end()) {

			const auto nMax = pRadType->GetLevelMax();
			const auto nCurrent = (*it)->GetCurrentRadLevel();

			if (nCurrent + RadLevel > nMax) {
				RadLevel = nMax - nCurrent;
			}

			// Handle It
			RadSiteExtContainer::Instance.Find((*it))->Add(RadLevel);
			return;
		}

	RadSiteExtData::CreateInstance(nCoord, Spread, RadLevel, pWeaponExt, pThis->Owner);
}

void BulletExtData::InitializeLaserTrails()
{
	const auto pThis = this->AttachedToObject;

	if (!LaserTrails.empty())
		return;

	auto const pTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);
	if (!pTypeExt || pTypeExt->LaserTrail_Types.empty())
		return;

	const auto pOwner = pThis->Owner ?
		pThis->Owner->Owner : (this->Owner ? this->Owner : HouseExtData::FindFirstCivilianHouse());

	for (auto const& idxTrail: pTypeExt->LaserTrail_Types) {
		this->LaserTrails.emplace_back(LaserTrailTypeClass::Array[idxTrail].get(), pOwner->LaserColor);
	}
}

void BulletExtData::InterceptBullet(BulletClass* pThis, TechnoClass* pSource, WeaponTypeClass* pWeapon)
{
	auto const pExt = BulletExtContainer::Instance.Find(pThis);
	auto const pThisTypeExt = BulletTypeExtContainer::Instance.Find(pThis->Type);
	bool canAffect = false;
	bool isIntercepted = false;

	if (pThisTypeExt->Armor.isset())
	{
		auto const pWHExt = WarheadTypeExtContainer::Instance.Find(pWeapon->Warhead);
		auto const versus = pWHExt->GetVerses(pThisTypeExt->Armor.Get()).Verses;
		if (((Math::abs(versus) >= 0.001)))
		{
			canAffect = true;
			const int damage = static_cast<int>(pWeapon->Damage * versus * TechnoExtData::GetDamageMult(pSource));
			pExt->CurrentStrength -= damage;

			FlyingStrings::DisplayDamageNumberString(damage, DamageDisplayType::Intercept, pThis->GetRenderCoords(), pExt->DamageNumberOffset);

			if (pExt->CurrentStrength <= 0)
				isIntercepted = true;
			else
				pExt->InterceptedStatus = InterceptedStatus::None;
		}
	}
	else
	{
		canAffect = true;
		isIntercepted = true;
	}

	if (canAffect) {
		auto const pTechnoTypeExt = TechnoTypeExtContainer::Instance.Find(pSource->GetTechnoType());

		if (pSource) {

			pExt->Intercepted_Detonate = !pTechnoTypeExt->Interceptor_DeleteOnIntercept.Get(pThisTypeExt->Interceptable_DeleteOnIntercept);

			if (auto const pWeaponOverride = pTechnoTypeExt->Interceptor_WeaponOverride.Get(pThisTypeExt->Interceptable_WeaponOverride)) {

				pThis->WeaponType = pWeaponOverride;
				pThis->Health = pTechnoTypeExt->Interceptor_WeaponCumulativeDamage ?
					pThis->Health + pWeaponOverride->Damage : pWeaponOverride->Damage;

				pThis->WH = pWeaponOverride->Warhead;
				pThis->Bright = pThis->WeaponType->Bright || pThis->WH->Bright;
				pThis->Speed = pWeaponOverride->Speed;

				if (pTechnoTypeExt->Interceptor_WeaponReplaceProjectile && pWeaponOverride->Projectile != pThis->Type)
				{
					const auto pNewProjTypeExt = BulletTypeExtContainer::Instance.Find(pWeaponOverride->Projectile);

					if (!pNewProjTypeExt)
					{
						//Debug::Log("Failed to find BulletTypeExt For [%s] ! \n", pWeaponOverride->Projectile->get_ID());
						return;
					}

					pThis->Type = pWeaponOverride->Projectile;

					pExt->LaserTrails.clear();
					pExt->InitializeLaserTrails();

					TrailsManager::CleanUp(pExt->AttachedToObject);
					TrailsManager::Construct(pExt->AttachedToObject);

					//LineTrailExt::DeallocateLineTrail(pThis);
					//LineTrailExt::ConstructLineTrails(pThis);

				   // Lose target if the current bullet is no longer interceptable.
					if (!pNewProjTypeExt->Interceptable || (pNewProjTypeExt->Armor.isset() && GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pNewProjTypeExt->Armor.Get()) == 0.0))
						pSource->SetTarget(nullptr);
				}
			}
		}

		if (isIntercepted && !pTechnoTypeExt->Interceptor_KeepIntact.Get()) {
			pExt->InterceptedStatus = InterceptedStatus::Intercepted;
		}
	}
}

constexpr bool TimerIsRunning(CDTimerClass& nTimer)
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

void BulletExtData::DetonateAt(BulletClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, CoordStruct nCoord , HouseClass* pBulletOwner)
{

	if (!nCoord.IsValid() && pTarget)
		nCoord = pTarget->GetCoords();

	if(pBulletOwner && !pOwner) {
		BulletExtContainer::Instance.Find(pThis)->Owner = pBulletOwner;
	}

	pThis->Limbo();
	pThis->SetLocation(nCoord);
	pThis->Explode(true);
	//GameDelete<true,false>(pThis);
	pThis->UnInit();
}

// =============================
// load / save

template <typename T>
void BulletExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->CurrentStrength)
		.Process(this->IsInterceptor)
		.Process(this->InterceptedStatus)
		.Process(this->Intercepted_Detonate)
		.Process(this->LaserTrails)
		.Process(this->SnappedToTarget)
		.Process(this->NukeSW)
		.Process(this->BrightCheckDone)
		.Process(this->Owner)
		.Process(this->Bouncing)
		.Process(this->LastObject)
		.Process(this->BounceAmount)
		.Process(this->InitialBulletDir)
		.Process(this->Trails)
		.Process(this->AttachedSystem)
		.Process(this->OriginalTarget)
		;

	PhobosTrajectory::ProcessFromStream(Stm, this->Trajectory);
}

// =============================
// container
BulletExtContainer BulletExtContainer::Instance;
std::vector<BulletExtData*> BulletExtContainer::Pool;

// =============================
// container hooks

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);
	BulletExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);
	BulletExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46AFB0, BulletClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46AE70, BulletClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

//Before :
//DEFINE_HOOK_AGAIN(0x46AF97, BulletClass_Load_Suffix, 0x7)
//DEFINE_HOOK(0x46AF9E, BulletClass_Load_Suffix, 0x7)

DEFINE_HOOK(0x46AF97, BulletClass_Load_Suffix, 0x7)
{
	BulletExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46AFC4, BulletClass_Save_Suffix, 0x3)
{
	GET(const HRESULT, nRes, EAX);

	if(SUCCEEDED(nRes))
		BulletExtContainer::Instance.SaveStatic();

	return 0;
}

// DEFINE_HOOK(	, BulletClass_Detach, 0x6)
// {
// 	GET(BulletClass*, pThis, ESI);
// 	GET(void*, target, EDI);
// 	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));
//
// 	BulletExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
//
// 	return pThis->NextAnim == target ? 0x4685C6 :0x4685CC;
// }

#include <Misc/Hooks.Otamaa.h>

void FakeBulletClass::_Detach(AbstractClass* target , bool all)
{
	BulletExtContainer::Instance.InvalidatePointerFor(this, target, all);
	this->BulletClass::PointerExpired(target , all);
}

DEFINE_JUMP(VTABLE, 0x7E470C, MiscTools::to_DWORD(&FakeBulletClass::_Detach))
DEFINE_JUMP(VTABLE, 0x7E4744, MiscTools::to_DWORD(&FakeBulletClass::_AnimPointerExpired))