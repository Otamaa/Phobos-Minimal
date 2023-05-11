#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#endif

#include "Trajectories/PhobosTrajectory.h"

#include <Utilities/Macro.h>

TechnoClass* BulletExt::InRangeTempFirer;

DWORD BulletExt::ApplyAirburst(BulletClass* pThis)
{
	const auto pType = pThis->Type;
	const auto pExt = BulletTypeExt::ExtMap.Find(pType);

	if (!pExt->HasSplitBehavior())
		return 0x46A290;

	auto const GetWeapon = [pExt, pType]()
	{
		if (pExt->AirburstWeapons.empty())
			return pType->AirburstWeapon;

		return pExt->AirburstWeapons[ScenarioClass::Instance->Random.RandomFromMax(pExt->AirburstWeapons.size() - 1)];
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
		targets.reserve(cluster);

		if (!pExt->Splits.Get())
		{
			// default hardcoded YR way: hit each cell around the destination once

			// fill target list with cells around the target
			CellRangeIterator<CellClass>{}(cellDest, pExt->AirburstSpread.Get(),
			[&targets](CellClass* const pCell) -> bool {
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
			std::for_each(TechnoClass::Array->begin(), TechnoClass::Array->end(), [&](TechnoClass* pTechno) {
				if (pWHExt->CanDealDamage(pTechno, false, !pExt->Splits_TargetingUseVerses.Get())) {
					 if ((!pExt->RetargetOwner.Get() && pTechno == pBulletOwner))
						 return;

					 //if (!EnumFunctions::IsCellEligible(pTarget->GetCell(), pExt->Splits_Affects))
						 //	return;
		
					 //if (!EnumFunctions::IsTechnoEligible(pTarget, this->Splits_Affects))
						 //	return;

					 if (pWHExt->CanTargetHouse(pBulletHouseOwner, pTechno)) {
							 const auto nLayer = pTechno->InWhichLayer();

						if (nLayer == Layer::Underground || nLayer == Layer::None)
							 return;

						const CoordStruct crdTechno = pTechno->GetCoords();
						if (crdDest.DistanceFrom(crdTechno) < pExt->Splits_Range.Get()
						 && ((!pTechno->IsInAir() && pWeapon->Projectile->AG) 
							 || (pTechno->IsInAir() && pWeapon->Projectile->AA))) {
							 targets.push_back(pTechno);
						}
					 }
				}
			});

			if (pExt->Splits_FillRemainingClusterWithRandomcells)
			{
				// fill up the list to cluster count with random cells around destination
				const int nMinRange = pExt->Splits_RandomCellUseHarcodedRange.Get() ? 3 : pWeapon->MinimumRange / Unsorted::LeptonsPerCell;
				const int nMaxRange = pExt->Splits_RandomCellUseHarcodedRange.Get() ? 3 : pWeapon->Range / Unsorted::LeptonsPerCell;

				while ((int)targets.size() < (cluster))
				{
					int x = random.RandomRanged(-nMinRange, nMaxRange);
					int y = random.RandomRanged(-nMinRange, nMaxRange);

					CellStruct cell { static_cast<short>(cellDest.X + x), static_cast<short>(cellDest.Y + y) };
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
					if (random.RandomDouble() > 0.5)
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
				if (const auto pBullet = BulletTypeExt::ExtMap
					.Find(pWeapon->Projectile)->CreateBullet(pTarget, pThis->Owner, pWeapon))
				{
					DirStruct const dir(5, random.RandomRangedSpecific<short>(0, 32));
					auto const radians = dir.GetRadian();

					auto const sin_rad = std::sin(radians);
					auto const cos_rad = std::cos(radians);
					auto const cos_factor = -2.44921270764e-16;
					auto const flatSpeed = cos_factor * pBullet->Speed;

					pBullet->MoveTo(pThis->Location,
						{ cos_rad * flatSpeed,sin_rad * flatSpeed, static_cast<double>(-pBullet->Speed) });

#ifdef COMPILE_PORTED_DP_FEATURES
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
#endif
				}
			}
		}
	}

	return 0x46A290;
}

VelocityClass BulletExt::GenerateVelocity(BulletClass* pThis, AbstractClass* pTarget, const int nSpeed, bool bCalculateSpeedFirst)
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
	double const sin_rad = std::sin(radians_fromXY);
	double const cos_rad = std::cos(radians_fromXY);

	velocity.X = cos_rad * nFirstMag;
	velocity.Y -= sin_rad * nFirstMag;

	if (!bCalculateSpeedFirst)
	{
		double const nSecMag = velocity.MagnitudeXY();
		DirStruct const dir_forZ(velocity.Z, nSecMag);
		double const radians_foZ = dir_forZ.GetRadian();
		double const nThirdMag = velocity.MagnitudeXY();

		if (radians_foZ != 0.0)
		{
			velocity.X /= std::cos(radians_foZ);
			velocity.Y /= std::cos(radians_foZ);
		}

		double const nMult_Cos = std::cos(0.7853262558535721);
		double const nMult_Sin = std::sin(0.7853262558535721);
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
		double const radians_foZ = dir_forZ.GetRadian();
		double const nThirdMag = velocity.MagnitudeXY();

		if (radians_foZ != 0.0)
		{
			velocity.X /= std::cos(radians_foZ);
			velocity.Y /= std::cos(radians_foZ);
		}

		double const nMult_Cos = std::cos(0.7853262558535721);
		double const nMult_Sin = std::sin(0.7853262558535721);
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

int BulletExt::GetShrapAmount(BulletClass* pThis)
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

bool BulletExt::AllowShrapnel(BulletClass* pThis, CellClass* pCell)
{
	auto const pData = BulletTypeExt::ExtMap.Find(pThis->Type);

	if (auto const pObject = pCell->FirstObject)
	{
		if ((((DWORD*)pObject)[0]) != BuildingClass::vtable || pData->Shrapnel_AffectsBuildings)
			return true;
	}
	else if (pData->Shrapnel_AffectsGround)
		return true;

	return false;
}

bool BulletExt::ShrapnelTargetEligible(BulletClass* pThis, AbstractClass* pTarget, bool checkOwner)
{
	if (!pTarget)
		return false;

	const auto pWH = pThis->Type->ShrapnelWeapon->Warhead;
	const auto pWhExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (const auto pTargetObj = abstract_cast<ObjectClass*>(pTarget))
	{
		switch ((((DWORD*)pTargetObj)[0]))
		{
		case AircraftClass::vtable:
		case InfantryClass::vtable:
		case UnitClass::vtable:
		case BuildingClass::vtable:
		{
			if (!pWhExt->CanDealDamage(static_cast<TechnoClass*>(pTargetObj), false, false)) {
				return false;
			}
		}
		break;
		default: {
			if (auto pType = pTargetObj->GetType()) {
				//if (std::abs(pWhExt->GetVerses(pType->Armor).Verses) < 0.001)
				if(GeneralUtils::GetWarheadVersusArmor(pWH , pType->Armor) < 0.001) {

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

void BulletExt::ApplyShrapnel(BulletClass* pThis)
{
	auto const pType = pThis->Type;
	auto const pShrapWeapon = pType->ShrapnelWeapon;

	if (!pShrapWeapon)
		return;

	const int nCount = BulletExt::GetShrapAmount(pThis);
	if (nCount <= 0)
		return;

	auto const pBulletCell = pThis->GetCell();
	if (BulletExt::AllowShrapnel(pThis, pBulletCell))
	{
		auto const nRange = pShrapWeapon->Range.ToCell();

		if (nRange >= 1)
		{
			int nTotal = 0;

			CellRangeIterator<CellClass> {}(pThis->InlineMapCoords(), nRange,
			[&](CellClass* pCell) -> bool
 {
	 auto const pTarget = pCell->FirstObject;
	 if (BulletExt::ShrapnelTargetEligible(pThis, pTarget))
	 {
		 const auto pShrapExt = BulletTypeExt::ExtMap.Find(pShrapWeapon->Projectile);

		 if (auto pBullet = pShrapExt->CreateBullet(pTarget, pThis->Owner, pShrapWeapon))
		 {
			 pBullet->MoveTo(pThis->Location, BulletExt::GenerateVelocity(pThis, pTarget, pShrapWeapon->Speed));


#ifdef COMPILE_PORTED_DP_FEATURES
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
#endif
		 }

		 //escapes
		 if (++nTotal == nCount)
			 return false; //return false to stop the loop
	 }

	 return true; // return true to continue
			});

			auto const nRemaining = abs(nCount - nTotal);

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
						const auto pShrapExt = BulletTypeExt::ExtMap.Find(pShrapWeapon->Projectile);

						if (const auto pBullet = pShrapExt->CreateBullet(pCellTarget, pThis->Owner, pShrapWeapon))
						{
							pBullet->MoveTo(pThis->Location, BulletExt::GenerateVelocity(pThis, pCellTarget, pShrapWeapon->Speed, true));

#ifdef COMPILE_PORTED_DP_FEATURES
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
#endif
						}
					}
				}
			}
		}
	}
}

void BulletExt::HandleBulletRemove(BulletClass* pThis, bool bDetonate, bool bRemove)
{
	if (bDetonate)
		pThis->Detonate(pThis->GetCoords());

	if (bRemove)
	{
		if (!pThis->InLimbo)
			pThis->Limbo();

		pThis->UnInit();

		const auto pTechno = pThis->Owner;
		const bool isLimbo =
			pTechno &&
			pTechno->InLimbo &&
			pThis->WeaponType &&
			pThis->WeaponType->LimboLaunch;

		if (isLimbo)
		{
			pThis->SetTarget(nullptr);
			auto damage = pTechno->Health;
			pTechno->SetLocation(pThis->GetCoords());
			pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}
}

bool BulletExt::ApplyMCAlternative(BulletClass* pThis)
{
	const auto pTarget = generic_cast<TechnoClass*>(pThis->Target);

	if (!pTarget || !pThis->WH->MindControl || !pThis->Owner)
		return false;

	const auto pTargetType = pTarget->GetTechnoType();
	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	double currentHealthPerc = pTarget->GetHealthPercentage();
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
		int realDamage = MapClass::GetTotalDamage(altDamage, pAltWarhead, pTargetType->Armor, 0);
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
			if (auto pAnim = GameCreate<AnimClass>(pAnimType, pTarget->Location))
			{
				AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->Owner->Owner, pTarget->Owner, pThis->Owner, false);
			}
		}

		return true;
	}

	// Run mind control code
	return false;
}

bool BulletExt::IsReallyAlive(BulletClass* pThis)
{
	return pThis && pThis->IsAlive;
}

HouseClass* BulletExt::GetHouse(BulletClass* const pThis)
{
	if (pThis->Owner) {
		return pThis->Owner->GetOwningHouse();
	}

	return BulletExt::ExtMap.Find(pThis)->Owner;
}

bool BulletExt::ExtData::InvalidateIgnorable(void* ptr) const
{
	switch (VTable::Get(ptr))
	{
	case BuildingClass::vtable:
	case InfantryClass::vtable:
	case UnitClass::vtable:
	case AircraftClass::vtable:
	case HouseClass::vtable:
		return false;
	}

	return true;

}

void BulletExt::ExtData::InvalidatePointer(void* ptr, bool bRemoved) {
	AnnounceInvalidPointer(Owner , ptr);

	if (auto& pTraj = Trajectory)
		pTraj->InvalidatePointer(ptr, bRemoved);
 }

void BulletExt::ExtData::ApplyRadiationToCell(CoordStruct const& nCoord, int Spread, int RadLevel)
{
	if (!MapClass::Instance->IsWithinUsableArea(nCoord))
		return;

	const auto pThis = this->Get();
	const auto pWeapon = pThis->GetWeaponType();
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const auto pRadType = pWeaponExt->RadType.Get(RadTypeClass::Array[0].get());

		auto const it = std::find_if(RadSiteClass::Array->begin(), RadSiteClass::Array->end(),
			[=](auto const pSite) {

				auto const pRadExt = RadSiteExt::ExtMap.Find(pSite);
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
			RadSiteExt::ExtMap.Find((*it))->Add(RadLevel);
			return;
		}

	RadSiteExt::CreateInstance(nCoord, Spread, RadLevel, pWeaponExt, pThis->Owner);
}

void BulletExt::ExtData::InitializeLaserTrails()
{
	const auto pThis = Get();

	if (!LaserTrails.empty() || pThis->Type->Inviso)
		return;

	auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
	if (!pTypeExt || pTypeExt->LaserTrail_Types.empty())
		return;

	const auto pOwner = pThis->Owner ?
		pThis->Owner->Owner : (this->Owner ? this->Owner : HouseExt::FindCivilianSide());

	for (auto const& idxTrail: pTypeExt->LaserTrail_Types) {
		this->LaserTrails.emplace_back(LaserTrailTypeClass::Array[idxTrail].get(), pOwner->LaserColor);
	}
}

void BulletExt::InterceptBullet(BulletClass* pThis, TechnoClass* pSource, WeaponTypeClass* pWeapon)
{
	auto const pExt = BulletExt::ExtMap.Find(pThis);
	auto const pThisTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type);
	bool canAffect = false;
	bool isIntercepted = false;

	if (pThisTypeExt->Armor.isset())
	{
		auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
		auto const versus = pWHExt->GetVerses(pThisTypeExt->Armor.Get()).Verses;
		if (((std::abs(versus) >= 0.001)))
		{
			canAffect = true;
			pExt->CurrentStrength -= static_cast<int>(pWeapon->Damage * versus * TechnoExt::GetDamageMult(pSource));

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
		auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());

		if (pSource) {

			pExt->Intercepted_Detonate = !pTechnoTypeExt->Interceptor_DeleteOnIntercept.Get(pThisTypeExt->Interceptable_DeleteOnIntercept);

			if (auto const pWeaponOverride = pTechnoTypeExt->Interceptor_WeaponOverride.Get(pThisTypeExt->Interceptable_WeaponOverride.Get(nullptr))) {
				
				pThis->WeaponType = pWeaponOverride;
				pThis->Health = pTechnoTypeExt->Interceptor_WeaponCumulativeDamage ?
					pThis->Health + pWeaponOverride->Damage : pWeaponOverride->Damage;

				pThis->WH = pWeaponOverride->Warhead;
				pThis->Bright = pThis->WeaponType->Bright || pThis->WH->Bright;
				pThis->Speed = pWeaponOverride->Speed;

				if (pTechnoTypeExt->Interceptor_WeaponReplaceProjectile && pWeaponOverride->Projectile != pThis->Type)
				{
					const auto pNewProjTypeExt = BulletTypeExt::ExtMap.Find(pWeaponOverride->Projectile);

					if (!pNewProjTypeExt)
					{
						//Debug::Log("Failed to find BulletTypeExt For [%s] ! \n", pWeaponOverride->Projectile->get_ID());
						return;
					}

					pThis->Type = pWeaponOverride->Projectile;

					if (!pExt->LaserTrails.empty()) {
						pExt->LaserTrails.clear();
						pExt->InitializeLaserTrails();
					}

#ifdef COMPILE_PORTED_DP_FEATURES
					TrailsManager::CleanUp(pExt->Get());
					TrailsManager::Construct(pExt->Get());
#endif

					//LineTrailExt::DeallocateLineTrail(pThis);
					//LineTrailExt::ConstructLineTrails(pThis);
				}
			}
		}

		if (isIntercepted && !pTechnoTypeExt->Interceptor_KeepIntact.Get()) {
			pExt->InterceptedStatus = InterceptedStatus::Intercepted;
		}
	}
}

bool TimerIsRunning(CDTimerClass& nTimer)
{
	auto nStart = nTimer.StartTime;
	auto nTimerLeft = nTimer.TimeLeft;

	if (nStart == -1)
	{
	CheckTimeLeft:
		if (nTimerLeft) {
			return true;
		}

		return false;
	}
	if (Unsorted::CurrentFrame() - nStart < nTimerLeft)
	{
		nTimerLeft -= Unsorted::CurrentFrame() - nStart;
		goto CheckTimeLeft;
	}

	return false;
}

Fuse BulletExt::FuseCheckup(BulletClass* pBullet, CoordStruct* newlocation)
{
	auto& nFuse = pBullet->Data;

	if (TimerIsRunning(nFuse.ArmTimer))
		return Fuse::DontIgnite;

	const int proximity = (int)(newlocation->DistanceFrom(nFuse.Location)) / 2;

	int nProx = 32;
	const auto pExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
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

void BulletExt::DetonateAt(BulletClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, CoordStruct nCoord , HouseClass* pBulletOwner)
{

	if (!nCoord && pTarget)
		nCoord = pTarget->GetCoords();

	if(pBulletOwner && !pOwner) {
		BulletExt::ExtMap.Find(pThis)->Owner = pBulletOwner;
	}

	pThis->Limbo();
	pThis->SetLocation(nCoord);
	pThis->Explode(false);
	pThis->UnInit();
}

// =============================
// load / save

template <typename T>
void BulletExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->CurrentStrength)
		.Process(this->IsInterceptor)
		.Process(this->InterceptedStatus)
		.Process(this->Intercepted_Detonate)
		.Process(this->LaserTrails)
		.Process(this->SnappedToTarget)
		.Process(this->BrightCheckDone)
		.Process(this->Owner)
		.Process(this->Bouncing)
		.Process(this->LastObject)
		.Process(this->BounceAmount)
		.Process(this->InitialBulletDir)
#ifdef COMPILE_PORTED_DP_FEATURES
		.Process(this->Trails)
#endif
		;

	PhobosTrajectory::ProcessFromStream(Stm, this->Trajectory);
}

// =============================
// container
BulletExt::ExtContainer BulletExt::ExtMap;
BulletExt::ExtContainer::ExtContainer() : Container("BulletClass") { }
BulletExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);
	BulletExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);
	BulletExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46AFB0, BulletClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46AE70, BulletClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

// Before : 
//DEFINE_HOOK_AGAIN(0x46AF97, BulletClass_Load_Suffix, 0x7)
//DEFINE_HOOK(0x46AF9E, BulletClass_Load_Suffix, 0x7)

DEFINE_HOOK(0x46AF97, BulletClass_Load_Suffix, 0x7)
{
	BulletExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46AFC4, BulletClass_Save_Suffix, 0x3)
{
	GET(const HRESULT, nRes, EAX);

	if(SUCCEEDED(nRes))
		BulletExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x4685BE, BulletClass_Detach, 0x6)
{
	GET(BulletClass*, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));

	BulletExt::ExtMap.InvalidatePointerFor(pThis, target, all);

	return pThis->NextAnim == target ? 0x4685C6 :0x4685CC;
}

static void __fastcall BulletClass_AnimPointerExpired(BulletClass* pThis, void* _, AnimClass* pTarget)
{
	pThis->ObjectClass::AnimPointerExpired(pTarget);
}

DEFINE_JUMP(VTABLE, 0x7E4744, GET_OFFSET(BulletClass_AnimPointerExpired))