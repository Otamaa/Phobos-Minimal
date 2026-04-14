#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <TechnoClass.h>
#include <TacticalClass.h>
#include <AircraftClass.h>

#include <Ext/Bullet/Trajectories/StraightTrajectory.h>


ASMJIT_PATCH(0x469276, BulletClass_Logics_ApplyMindControl , 0xA)
{
	GET(ObjectClass* const, pVictimObject, EDI);
	GET(FakeBulletClass*, pThis, ESI);
	const auto payback = pThis->Owner;

	if(pVictimObject) {
		if(pVictimObject->IsAlive) {
			if(pVictimObject->AttachedTag) {
			  pVictimObject->AttachedTag->SpringEvent(TriggerEvent::AttackedByAnybody,				pVictimObject,
				CellStruct::Empty,
				false,
				payback);
			}

			// #1708: this mofo was raising an event without checking whether
			// there is a valid tag. this is the only faulty call of this kind.
			if(pVictimObject->AttachedTag) {
				pVictimObject->AttachedTag->SpringEvent(TriggerEvent::AttackedByHouse,
				pVictimObject,
				CellStruct::Empty,
				false,
				payback);
			}
		}
	}

	const auto pControlledAnimType = pThis->_GetWarheadTypeExtData()->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	const auto pTechno = flag_cast_to<TechnoClass*>(pThis->Target);
	auto const threatDelay = pThis->_GetWarheadTypeExtData()->MindControl_ThreatDelay.Get(RulesExtData::Instance()->AttackMindControlledDelay);

	R->AL(((FakeCaptureManagerClass*)payback->CaptureManager)->__CaptureUnit(
		pTechno,
		GET_TECHNOTYPEEXT(payback)->MultiMindControl_ReleaseVictim,
		false,
		pControlledAnimType,
		threatDelay));

	return 0x4692D5;
}

// we handle ScreenShake thru warhead
//DEFINE_JUMP(LJMP, 0x4690D4, 0x469130)

#include <Misc/PhobosGlobal.h>
#include <InfantryClass.h>

static FORCEDINLINE void TryDetonateFull(BulletClass* pThis, TechnoClass* pTechno, WarheadTypeExtData* pWHExt, HouseClass* pOwner)
{
	if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner) == FullMapDetonateResult::TargetValid)
	{
		pThis->Target = pTechno;
		pThis->Location = pTechno->GetCoords();
		pThis->Detonate(pTechno->GetCoords());
	}
}

static FORCEDINLINE void TryDetonateDamageArea(BulletClass* pThis, TechnoClass* pTechno, WarheadTypeExtData* pWHExt, HouseClass* pOwner)
{
	if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner) == FullMapDetonateResult::TargetValid)
	{
		int damage = (pThis->Health * pThis->DamageMultiplier) >> 8;
		pWHExt->DamageAreaWithTarget(pTechno->GetCoords(), damage, pThis->Owner, pThis->WH, true, pOwner, pTechno);
	}
}

ASMJIT_PATCH(0x4690D4, BulletClass_Logics_ApplyAdditionals, 0x6)
{
	enum { SkipShaking = 0x469130, GoToExtras = 0x469AA4  , ReturnFromFunction = 0x46A2FB };

	GET(FakeBulletClass*, pBullet, ESI);
	GET(FakeWarheadTypeClass*, pWarhead, EAX);
	//GET_BASE(CoordStruct*, pCoords, 0x8);

	if (pBullet->WH)
	{
		const auto pWHExt = pBullet->_GetWarheadTypeExtData();

		if (pWHExt->DetonateOnAllMapObjects &&
			!pWHExt->WasDetonatedOnAllMapObjects &&
			pWHExt->DetonateOnAllMapObjects_AffectTargets != AffectedTarget::None &&
			pWHExt->DetonateOnAllMapObjects_AffectHouses != AffectedHouse::None)
		{
			pWHExt->WasDetonatedOnAllMapObjects = true;
			auto const originalLocation = pBullet->Location;
			const auto pHouse = BulletExtData::GetHouse(pBullet);
			BulletExtContainer::Instance.Find(pBullet)->OriginalTarget = pBullet->Target;

			COMPILETIMEEVAL auto copy_dvc = []<typename T>(std::vector<T>&dest, const DynamicVectorClass<T>&dvc)
			{
				dest.resize(dvc.Count);
				std::ranges::copy(dvc, dest.begin());
				return &dest;
			};

			void (*tryDetonate)(BulletClass*, TechnoClass*, WarheadTypeExtData*, HouseClass*)
				= pWHExt->DetonateOnAllMapObjects_Full ? TryDetonateFull : TryDetonateDamageArea;

			auto& CurCopyArray = PhobosGlobal::Instance()->CurCopyArray[pBullet->WH];

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Aircraft) != AffectedTarget::None)
			{
				auto const aircraft = copy_dvc(CurCopyArray.Aircraft, *AircraftClass::Array);

				for (auto pAircraft : *aircraft)
					tryDetonate(pBullet, pAircraft, pWHExt, pHouse);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Building) != AffectedTarget::None)
			{
				auto const buildings = copy_dvc(CurCopyArray.Building, *BuildingClass::Array);

				for (auto pBuilding : *buildings)
					tryDetonate(pBullet, pBuilding, pWHExt, pHouse);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Infantry) != AffectedTarget::None)
			{
				auto const infantry = copy_dvc(CurCopyArray.Infantry, *InfantryClass::Array);

				for (auto pInf : *infantry)
					tryDetonate(pBullet, pInf, pWHExt, pHouse);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Unit) != AffectedTarget::None)
			{
				auto const units = copy_dvc(CurCopyArray.Unit, *UnitClass::Array);

				for (auto const pUnit : *units)
					tryDetonate(pBullet, pUnit, pWHExt, pHouse);
			}

			pBullet->Target = pBullet->_GetExtData()->OriginalTarget;
			pBullet->Location = originalLocation;
			pWHExt->WasDetonatedOnAllMapObjects = false;
			return ReturnFromFunction;
		}
	}


	if (auto pTarget = flag_cast_to<ObjectClass*>(pBullet->Target)) {
		// Check if the WH should affect the techno target or skip it
		if (!pWarhead->_GetExtData()->IsHealthInThreshold(pTarget))
			return GoToExtras;

		auto const pTargetTech = flag_cast_to<TechnoClass*>(pBullet->Target);

		if (pTargetTech && !pWarhead->_GetExtData()->IsVeterancyInThreshold(pTargetTech))
			return GoToExtras;

		const auto pBulletExt = pBullet->_GetExtData();
		TechnoClass* pBulletOwner = pBullet->Owner ? pBullet->Owner : nullptr;
		HouseClass* pBulletHouseOwner = pBulletOwner ? pBulletOwner->GetOwningHouse() : (pBulletExt ? pBulletExt->Owner : HouseExtData::FindNeutral());

		if(!pWarhead->_GetExtData()->CanAffectHouse(pBulletHouseOwner, pTarget->GetOwningHouse())){
			return GoToExtras;
		}
	}

	return SkipShaking;
}


ASMJIT_PATCH(0x469B44, BulletClass_Logics_LandTypeCheck, 0x6)
{
	enum { SkipChecks = 0x469BA2 };

	GET(FakeBulletClass*, pThis, ESI);

	return pThis->_GetWarheadTypeExtData()->Conventional_IgnoreUnits ? SkipChecks : 0;
}

ASMJIT_PATCH(0x468B72, BulletClass_MoveTo_End, 0x5)
{
	GET(FakeBulletClass*, pThis, EBX);
	GET_STACK(CoordStruct*, pCoord, STACK_OFFS(0x54, -0x4));
	GET_STACK(VelocityClass*, pOriginalVelocity, STACK_OFFS(0x54, -0x8));
	
	auto pType = pThis->Type;
	auto pTypeExt = pThis->_GetTypeExtData();

	PhobosTrajectory::CreateInstance(pThis, pCoord, pOriginalVelocity);

	// Parasite=yes will make the bullet MoveTo twice, and may cause some issue.
	// Before we know how to deal with it, just exclude it.
	if ((!pThis->WeaponType || !pThis->WeaponType->Warhead->Parasite)
		&& !pThis->WH->Parasite
		&& pTypeExt->UpdateImmediately.Get(pType->Inviso && RulesExtData::Instance()->UpdateInvisoImmediately))
	{
		pThis->Update();
	}

	return 0;
}