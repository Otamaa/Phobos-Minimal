#include <BuildingClass.h>
#include <CellClass.h>
#include <MapClass.h>
#include <ParticleSystemClass.h>
#include <FootClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Infantry/Body.h>

#include <Misc/Ares/Hooks/Header.h>
#include <Misc/Ares/Hooks/AresTrajectoryHelper.h>

#ifndef PARTONE
// Contains hooks that fix weapon graphical effects like lasers, railguns, electric bolts, beams and waves not interacting
// correctly with obstacles between firer and target, as well as railgun / railgun particles being cut off by elevation.

// Fix techno target coordinates (used for fire angle calculations, target lines etc) to take building target coordinate offsets into accord.
// This, for an example, fixes a vanilla bug where Destroyer has trouble targeting Naval Yards with its cannon weapon from certain angles.
ASMJIT_PATCH(0x70BCDC, TechnoClass_GetTargetCoords_BuildingFix, 0x6)
{
	GET(const AbstractClass* const, pTarget, ECX);
	LEA_STACK(CoordStruct*, nCoord, 0x40 - 0x18);

	if (const auto pBuilding = cast_to<const BuildingClass*, false>(pTarget)) {
		//auto const& nTargetCoord = pBuilding->Type->TargetCoordOffset;
		//Debug::LogInfo("__FUNCTION__ Building  Target [%s] with TargetCoord %d , %d , %d ", pBuilding->get_ID(), nTargetCoord.X , nTargetCoord.Y , nTargetCoord.Z);
		pBuilding->GetTargetCoords(nCoord);

		R->EAX(nCoord);
		return 0x70BCE6u;

	}// else {
	//	pTarget->GetCenterCoords(nCoord);
	//}

	return 0x0;
}

// Fix railgun target coordinates potentially differing from actual target coords.
ASMJIT_PATCH(0x70C6AF, TechnoClass_Railgun_TargetCoords, 0x6)
{
	GET(AbstractClass*, pTarget, EBX);
	GET(CoordStruct*, pBuffer, ECX);

	switch (pTarget->WhatAmI())
	{
	case BuildingClass::AbsID:
	{
		static_cast<BuildingClass*>(pTarget)->GetTargetCoords(pBuffer);
		break;
	}
	case CellClass::AbsID:
	{
		auto const pCell = static_cast<CellClass*>(pTarget);
		pCell->GetCoords(pBuffer);
		if (pCell->ContainsBridge()) {
			pBuffer->Z += Unsorted::BridgeHeight;
		}

		break;
	}
	default:
		pTarget->GetCoords(pBuffer);
		break;
	}

	R->EAX(pBuffer);
	return 0x70C6B5;
}

// Do not adjust map coordinates for railgun particles that are below cell coordinates.
ASMJIT_PATCH(0x62B897, ParticleClass_CTOR_RailgunCoordAdjust, 0x5)
{
	enum { SkipCoordAdjust = 0x62B8CB  ,Continue = 0x0};

	GET(ParticleClass*, pThis, ESI);
	const auto pParticleSys = pThis->ParticleSystem;
	const auto pParticleTypeExt = ParticleTypeExtContainer::Instance.Find(pThis->Type);

	if(pParticleSys
	&& (pParticleSys->Type->BehavesLike == ParticleSystemTypeBehavesLike::Railgun
		|| pParticleSys->Type->BehavesLike == ParticleSystemTypeBehavesLike::Fire)
	){
		GET(CoordStruct*, pCoordBase, EDI);
		LEA_STACK(CoordStruct* , pCoord, 0x10);

		//restore overriden instruction
		if(!pParticleTypeExt->ReadjustZ) {
			pCoord->X = pCoordBase->X;
			pCoord->Y = pCoordBase->Y;
			pCoord->Z = pCoordBase->Z;
		}
		else
		{
			pCoord->X = pCoordBase->X;
			pCoord->Y = pCoordBase->Y;

			auto nZ = MapClass::Instance->GetZPos(pCoordBase);
			if (pCoordBase->Z <= nZ)
				pCoord->Z = nZ;
			else
				pCoord->Z = pCoordBase->Z;
		}

		return SkipCoordAdjust;
	}

	return Continue;
}

// Fix fire particle target coordinates potentially differing from actual target coords.
ASMJIT_PATCH(0x62FA20, ParticleSystemClass_FireAI_TargetCoords, 0x6)
{
	enum { SkipGameCode = 0x62FA51, Continue = 0x62FBAF };

	GET(ParticleSystemClass*, pThis, ESI);
	GET(TechnoClass*, pOwner, EBX);

	if (ParticleSystemTypeExtContainer::Instance
			.Find(pThis->Type)->AdjustTargetCoordsOnRotation
		&& pOwner->PrimaryFacing.Is_Rotating())
	{
		auto coords = pThis->TargetCoords;
		R->EAX(&coords);
		return SkipGameCode;
	}

	return Continue;
}

// Fix fire particles being disallowed from going upwards.
ASMJIT_PATCH(0x62D685, ParticleSystemClass_FireAt_Coords, 0x5)
{
	enum { SkipGameCode = 0x62D6B7 };

	// Game checks if MapClass::GetCellFloorHeight() for currentCoords is larger than for previousCoords and sets the flags on ParticleClass to
	// remove it if so. Below is an attempt to create a smarter check that allows upwards movement and does not needlessly collide with elevation
	// but removes particles when colliding with flat ground. It doesn't work perfectly and covering all edge-cases is difficult or impossible so
	// preference was to disable it. Keeping the code here commented out, however.

	/*
	GET(ParticleClass*, pThis, ESI);
	REF_STACK(CoordStruct, currentCoords, STACK_OFFSET(0x24, -0x18));
	REF_STACK(CoordStruct, previousCoords, STACK_OFFSET(0x24, -0xC));
	auto const sourceLocation = pThis->ParticleSystem ? pThis->ParticleSystem->Location : CoordStruct { INT_MAX, INT_MAX, INT_MAX };
	auto const pCell = MapClass::Instance->TryGetCellAt(currentCoords);
	int cellFloor = MapClass::Instance->GetCellFloorHeight(currentCoords);
	bool downwardTrajectory = currentCoords.Z < previousCoords.Z;
	bool isBelowSource = cellFloor < sourceLocation.Z - Unsorted::LevelHeight * 2;
	bool isRamp = pCell ? pCell->SlopeIndex : false;
	if (!isRamp && isBelowSource && downwardTrajectory && currentCoords.Z < cellFloor)
	{
		pThis->unknown_12D = 1;
		pThis->unknown_131 = 1;
	}
	*/

	return SkipGameCode;
}
#endif

#ifndef PERFORMANCE_HEAVY
namespace FireAtTemp
{
	BulletClass* FireBullet = nullptr;
	CoordStruct originalTargetCoords;
	CellClass* pObstacleCell = nullptr;
	AbstractClass* pOriginalTarget = nullptr;
	AbstractClass* pWaveOwnerTarget = nullptr;
}

ASMJIT_PATCH(0x6FF08B, TechnoClass_Fire_RecordBullet, 0x6)
{
	GET(BulletClass*, pBullet, EBX);
	FireAtTemp::FireBullet = pBullet;
	return 0;
}

// https://github.com/Phobos-developers/Phobos/pull/825
// Todo :  Otamaa : massive FPS drops !
// Contains hooks that fix weapon graphical effects like lasers, railguns, electric bolts, beams and waves not interacting
// correctly with obstacles between firer and target, as well as railgun / railgun particles being cut off by elevation.

// Adjust target coordinates for laser drawing.
ASMJIT_PATCH(0x6FD38D, TechnoClass_DrawSth_Coords, 0x7)
{
	GET(CoordStruct*, pTargetCoords, EAX);

	const auto pBullet = FireAtTemp::FireBullet;

	if (pBullet && pBullet->WeaponType)
	{
		// The weapon may not have been set up
		const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pBullet->WeaponType);
		// pBullet->Data.Location (0x4E1130) -> pBullet->Type->Inviso ? pBullet->Location : pBullet->TargetCoords
		CoordStruct _targetCoords= pBullet->GetDestinationCoords();
		if (pWeaponExt && pWeaponExt->VisualScatter)
		{
			const auto pRulesExt = RulesExtData::Instance();
			const auto radius = ScenarioClass::Instance->Random.RandomRanged(pRulesExt->VisualScatter_Min.Get(), pRulesExt->VisualScatter_Max.Get());
			*pTargetCoords = MapClass::GetRandomCoordsNear(_targetCoords, radius, false);
		}
		else
		{
			*pTargetCoords = _targetCoords;
		}
	} else if (FireAtTemp::pObstacleCell)
		*pTargetCoords = FireAtTemp::pObstacleCell->GetCoordsWithBridge();

	R->EAX(pTargetCoords);
	return 0;
}ASMJIT_PATCH_AGAIN(0x6FD70D, TechnoClass_DrawSth_Coords, 0x6) // CreateRBeam
ASMJIT_PATCH_AGAIN(0x6FD514, TechnoClass_DrawSth_Coords, 0x7) // CreateEBolt

// Cut railgun logic off at obstacle coordinates.
ASMJIT_PATCH(0x70CA64, TechnoClass_Railgun_Obstacles, 0x5)
{
	enum { Continue = 0x70CA79, Stop = 0x70CAD8 };

	REF_STACK(CoordStruct const, coords, STACK_OFFSET(0xC0, -0x80));

	if (MapClass::Instance->GetCellAt(coords) == FireAtTemp::pObstacleCell)
		return Stop;

	return Continue;
}

// WaveClass requires the firer's target and wave's target to match so it needs bit of extra handling here for obstacle cell targets.
ASMJIT_PATCH(0x762AFF, WaveClass_AI_TargetSet, 0x6)
{
	GET(WaveClass*, pThis, ESI);

	if (pThis->Target && pThis->Owner)
	{
		auto const pObstacleCell = TechnoExtContainer::Instance.Find(pThis->Owner)->FiringObstacleCell;

		if (pObstacleCell == pThis->Target && pThis->Owner->Target)
		{
			FireAtTemp::pWaveOwnerTarget = pThis->Owner->Target;
			pThis->Owner->Target = pThis->Target;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x762D57, WaveClass_AI_TargetUnset, 0x6)
{
	GET(WaveClass*, pThis, ESI);

	if (FireAtTemp::pWaveOwnerTarget)
	{
		if (pThis->Owner->Target)
			pThis->Owner->Target = FireAtTemp::pWaveOwnerTarget;

		FireAtTemp::pWaveOwnerTarget = nullptr;
	}

	return 0;
}

#endif

#include <Conversions.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Wave/Body.h>

// Adjust target for bolt / beam / wave drawing.
// same hook with TechnoClass_FireAt_FeedbackWeapon
#ifndef _disable
ASMJIT_PATCH(0x6FF15F, TechnoClass_FireAt_Additionals_Start, 6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(FakeWeaponTypeClass*, pWeapon, EBX);

	REF_STACK(CoordStruct, crdSrc, 0xB0 -0x6C);
	REF_STACK(CoordStruct, crdTgt, 0xB0 -0x28);
	REF_STACK(CoordStruct, railgunCrrd_1, 0xB0  -0x1C);

	GET_BASE(AbstractClass*, pOriginalTarget, 0x8);

	GET_BASE(int, weaponIdx, 0xC);

	auto coords = pOriginalTarget->GetCenterCoords();
	auto pExt = TechnoExtContainer::Instance.Find(pThis);

	if (const auto pBuilding = cast_to<BuildingClass*, false>(pOriginalTarget))
		coords = pBuilding->GetTargetCoords();

	// This is set to a temp variable as well, as accessing it everywhere needed from TechnoExt would be more complicated.
	FireAtTemp::pObstacleCell = TrajectoryHelper::FindFirstObstacle(crdSrc, coords, pWeapon->Projectile, pThis->Owner);
	pExt->FiringObstacleCell = FireAtTemp::pObstacleCell;

	R->Stack(0x10, &crdSrc);

	if (pWeapon->UseFireParticles && !pThis->FireParticleSystem && pWeapon->AttachedParticleSystem) {
		pThis->FireParticleSystem = GameCreate<ParticleSystemClass>(
			pWeapon->AttachedParticleSystem, crdSrc,
			FireAtTemp::pObstacleCell  ? FireAtTemp::pObstacleCell : pOriginalTarget, pThis);
	}

	if (pWeapon->UseSparkParticles && !pThis->SparkParticleSystem && pWeapon->AttachedParticleSystem) {
		pThis->SparkParticleSystem = GameCreate<ParticleSystemClass>(
			pWeapon->AttachedParticleSystem, crdSrc,
			FireAtTemp::pObstacleCell ? FireAtTemp::pObstacleCell : pOriginalTarget, pThis);
	}

	if (pWeapon->AttachedParticleSystem && (pWeapon->_GetExtData()->IsDetachedRailgun || (pWeapon->IsRailgun &&  !pThis->RailgunParticleSystem))) {
		auto coord = pThis->DealthParticleDamage(&railgunCrrd_1, &crdSrc, pOriginalTarget, pWeapon);
		auto pRailgun = GameCreate<ParticleSystemClass>(
			pWeapon->AttachedParticleSystem, &crdSrc,
			nullptr, pThis , coord , nullptr);

		if (!pWeapon->_GetExtData()->IsDetachedRailgun)
			pThis->RailgunParticleSystem = pRailgun;
	}

	++pThis->CurrentBurstIndex;
	int ROF = pThis->GetROF(weaponIdx);

	if (pThis->Berzerk) {
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
		const double multiplier = pExt->BerserkROFMultiplier.Get(RulesExtData::Instance()->BerserkROFMultiplier);
		ROF = static_cast<int>(ROF * multiplier);
	}

	TechnoExtData::SetChargeTurretDelay(pThis, ROF, pWeapon);

	pThis->DiskLaserTimer.Start(ROF);

	// Issue #46: Laser is mirrored relative to FireFLH
	// Author: Starkku
	--pThis->CurrentBurstIndex;

	AnimTypeClass* pFiringAnim = nullptr;
	auto pWeaponExt = pWeapon->_GetExtData();

	if (pThis->CanOccupyFire()) {
		if (pWeaponExt->OccupantAnim_UseMultiple.Get() && !pWeaponExt->OccupantAnims.empty()) {
			if(pWeaponExt->OccupantAnims.size() == 1) {
				pFiringAnim = pWeaponExt->OccupantAnims[0];
			} else {
				pFiringAnim = pWeaponExt->OccupantAnims[ScenarioClass::Instance->Random.RandomFromMax(pWeaponExt->OccupantAnims.size() - 1)];
			}

		} else {
			pFiringAnim = pWeapon->OccupantAnim;
		}

	} else {

		if (pWeapon->Anim.Count > 0)
		{
			int nIdx = -1;

			if (pWeapon->Anim.Count == 1)
				nIdx = 0;
			else {

				DirStruct facing {};
				pThis->GetRealFacing(&facing);

				if (pWeapon->Anim.Count == 8) {
					nIdx = (facing.GetFacing<8>() + 8 / 8) % 8;
				}
				else if (pWeapon->Anim.Count == 16) {
					nIdx = (facing.GetFacing<16>() + 16 / 8) % 16;
				}
				else if (pWeapon->Anim.Count == 32) {
					nIdx = (facing.GetFacing<32>() + 32 / 8) % 32;
				}
				else if (pWeapon->Anim.Count == 64) {
					nIdx = (facing.GetFacing<64>() + 64 / 8) % 64;
				} else {
					//only execute if the anim count is more than 1
					const auto highest = Conversions::Int2Highest(pWeapon->Anim.Count);

					// 2^highest is the frame count, 3 means 8 frames
					if (highest >= 3) {
						nIdx = facing.GetValue(highest, 1u << (highest - 3));
					}

					nIdx %= pWeapon->Anim.Count;
				}
			}

			pFiringAnim = pWeapon->Anim.Items[nIdx];
		}
	}

	if (pWeapon->Report.Count > 0 && !pThis->GetTechnoType()->IsGattling) {
		if (pWeapon->Report.Count == 1) {
			VocClass::PlayAt(pWeapon->Report[0], crdTgt, nullptr);
		} else {
			auto v116 = pThis->weapon_sound_randomnumber_3C8 % pWeapon->Report.Count;
			VocClass::PlayAt(pWeapon->Report[v116], crdTgt, nullptr);
		}
	}

	if (const auto pAnimType = pWeaponExt->Feedback_Anim.Get()) {
			const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? crdSrc : pThis->GetCoords()) + pWeaponExt->Feedback_Anim_Offset; {
			auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord);
			AnimExtData::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);

			if (pThis->WhatAmI() != BuildingClass::AbsID) {
				pFeedBackAnim->SetOwnerObject(pThis);
			}
			else {
				if (pThis->GetOccupantCount() > 0) {
					pFeedBackAnim->ZAdjust = -200;
				} else {
					auto rend = pThis->GetRenderCoords();
					pFeedBackAnim->ZAdjust = (crdSrc.Y - rend.Y) / -4 >= 0 ? 0 : (crdSrc.Y - rend.Y) / -4;
				}
			}
		}
	}

	if (!pThis->IsAlive) {
		return 0x6FF92F;
	}

	if (pFiringAnim)
	{
		auto pFiring = GameCreate<AnimClass>(pFiringAnim, crdSrc, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0);
		AnimExtData::SetAnimOwnerHouseKind(pFiring, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);
		if (pThis->WhatAmI() != BuildingClass::AbsID)
		{
			pFiring->SetOwnerObject(pThis);
		} else {
			if (pThis->GetOccupantCount() > 0) {
				pFiring->ZAdjust = -200;
			} else {
				auto rend = pThis->GetRenderCoords();
				pFiring->ZAdjust = (crdSrc.Y - rend.Y) / -4 >= 0 ? 0 : (crdSrc.Y - rend.Y) / -4;
			}
		}
	}

	if (!pThis->IsAlive) {
		return 0x6FF92F;
	}

#ifndef PERFORMANCE_HEAVY
	//TargetSet
	FireAtTemp::originalTargetCoords = crdTgt;
	FireAtTemp::pOriginalTarget = pOriginalTarget;

	if (FireAtTemp::pObstacleCell)
	{
		crdTgt = FireAtTemp::pObstacleCell->GetCoordsWithBridge();
		R->Base(8, FireAtTemp::pObstacleCell); // Replace original target so it gets used by Ares sonic wave stuff etc. as well.
		pOriginalTarget = FireAtTemp::pObstacleCell;
	}
#endif

	//FeedbackWeapon
	if (auto fbWeapon = pWeaponExt->FeedbackWeapon.Get())
	{
		if (!pThis->InOpenToppedTransport || fbWeapon->FireInTransport){
			WeaponTypeExtData::DetonateAt(fbWeapon, pThis, pThis, true, nullptr);
			//pThis techno was die after after getting affect of FeedbackWeapon
			//if the function not bail out , it will crash the game because the vtable is already invalid
			if (!pThis->IsAlive) {
				return 0x6FF92F;
			}
		}
	}

	if (pExt->AE.HasFeedbackWeapon) {
		for (auto const& pAE : pExt->PhobosAE) {

			if(!pAE|| !pAE->IsActive())
				continue;

			if (auto const pWeaponFeedback = pAE->GetType()->FeedbackWeapon)
			{
				if (pThis->InOpenToppedTransport && !pWeaponFeedback->FireInTransport)
					return 0;

				WeaponTypeExtData::DetonateAt(pWeaponFeedback, pThis, pThis, true, nullptr);
			}
		}

			//pThis techno was die after after getting affect of FeedbackWeapon
			//if the function not bail out , it will crash the game because the vtable is already invalid
			if (!pThis->IsAlive) {
				return 0x6FF92F;
			}
	}

	if(pWeapon->IsSonic){
		pThis->Wave = WaveExtData::Create(crdSrc, crdTgt, pThis, WaveType::Sonic, pOriginalTarget, pWeapon);
	}

	return 0x6FF48A;
}
#endif

#ifndef ENABLE_THESE_THINGS
#include <Ext/WeaponType/Body.h>

ASMJIT_PATCH(0x70C862, TechnoClass_Railgun_AmbientDamageIgnoreTarget1, 0x5)
{
	enum { IgnoreTarget = 0x70CA59, Continue = 0x0 };

	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);

	return WeaponTypeExtContainer::Instance.Find(pWeapon)->AmbientDamage_IgnoreTarget ?
		IgnoreTarget : Continue;
}

ASMJIT_PATCH(0x70CA8B, TechnoClass_Railgun_AmbientDamageIgnoreTarget2, 0x6)
{
	enum { IgnoreTarget = 0x70CBB0 };

	GET_BASE(WeaponTypeClass*, pWeapon, 0x14);
	REF_STACK(DynamicVectorClass<ObjectClass*>, objects, 0xC0 - 0xAC);

	if (WeaponTypeExtContainer::Instance.Find(pWeapon)->AmbientDamage_IgnoreTarget)
	{
		R->EAX(objects.Count);
		return IgnoreTarget;
	}

	return 0;
}

ASMJIT_PATCH(0x70CBDA, TechnoClass_Railgun_AmbientDamageWarhead, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(TechnoClass*, pSource, EDX);

	R->Stack<HouseClass*>(0xC, pSource->Owner);
	R->EDX(WeaponTypeExtContainer::Instance.Find(pWeapon)->AmbientDamage_Warhead.Get(pWeapon->Warhead));
	return 0x70CBE0;
}
#endif

ASMJIT_PATCH(0x6FF656, TechnoClass_FireAt_Additionals_End, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_BASE(AbstractClass*, pTarget, 0x8);
	GET(WeaponTypeClass* const, pWeaponType, EBX);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFS(0xB0, 0x74));
	GET_BASE(int, weaponIndex, 0xC);
	LEA_STACK(CoordStruct*, pTargetCoords, STACK_OFFSET(0xB0, -0x28));

	//remove ammo rounds depending on weapon
	TechnoExt_ExtData::DecreaseAmmo(pThis, pWeaponType);
	auto const pExt = TechnoExtContainer::Instance.Find(pThis);

#ifndef PERFORMANCE_HEAVY
	// Restore original target & coords
	*pTargetCoords = FireAtTemp::originalTargetCoords;
	R->Base(8, FireAtTemp::pOriginalTarget);
	pTarget = FireAtTemp::pOriginalTarget;
	R->EDI(FireAtTemp::pOriginalTarget);

	// Reset temp values
	FireAtTemp::originalTargetCoords = CoordStruct::Empty;
	FireAtTemp::FireBullet = nullptr;
	FireAtTemp::pObstacleCell = nullptr;
	FireAtTemp::pOriginalTarget = nullptr;
#endif

	//TechnoClass_FireAt_ToggleLaserWeaponIndex
	if (pThis->WhatAmI() == BuildingClass::AbsID && pWeaponType->IsLaser) {
		if (pExt->CurrentLaserWeaponIndex.empty())
			pExt->CurrentLaserWeaponIndex = weaponIndex;
		else
			pExt->CurrentLaserWeaponIndex.clear();
	}

	//TechnoClass_FireAt_BurstOffsetFix_2
	++pThis->CurrentBurstIndex;
	pThis->CurrentBurstIndex %= pWeaponType->Burst;

	if (pThis->WhatAmI() == AbstractType::Infantry){
		auto pInf = ((FakeInfantryClass*)(pThis));
		if (pInf->_GetExtData()->ForceFullRearmDelay)
		{
			pInf->_GetExtData()->ForceFullRearmDelay = false;
			pThis->CurrentBurstIndex = 0;
		}
	}

	if (auto const pTargetObject = cast_to<BulletClass* const, false>(pTarget))
	{
		if (TechnoExtContainer::Instance.Find(pThis)->IsInterceptor())
		{
			BulletExtContainer::Instance.Find(pBullet)->IsInterceptor = true;
			BulletExtContainer::Instance.Find(pTargetObject)->InterceptedStatus = InterceptedStatus::Targeted;

			// If using Inviso projectile, can intercept bullets right after firing.
			// if (pTargetObject->IsAlive && pWeaponType->Projectile->Inviso)
			// {
			// 	WarheadTypeExtContainer::Instance.Find(pWeaponType->Warhead)->InterceptBullets(pThis, pWeaponType, pTargetObject->Location);
			// }
		}
	}

	auto const pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeaponType);

	if (!pWeaponExt->ShakeLocal.Get() || pThis->IsOnMyView())
	{
		if (pWeaponExt->Xhi || pWeaponExt->Xlo)
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, ScenarioClass::Instance->Random(pWeaponExt->Xlo, pWeaponExt->Xhi));

		if (pWeaponExt->Yhi || pWeaponExt->Ylo)
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, ScenarioClass::Instance->Random(pWeaponExt->Ylo, pWeaponExt->Yhi));
	}

	return 0x6FF660;
}