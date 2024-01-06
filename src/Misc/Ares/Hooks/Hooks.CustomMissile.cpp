#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Conversions.h>

#include <Locomotor/Cast.h>
#include <SpawnManagerClass.h>

#pragma region RocketLocoHooks
//TODO : Cmisl Hardcoded shit
// 662496
// 66235D

//static DamageAreaResult FC _RocketLocomotionClass_DamageArea(
//	CoordStruct* pCoord,
//	int Damage,
//	TechnoClass* Source,
//	WarheadTypeClass* Warhead,
//	bool AffectTiberium,
//	HouseClass* SourceHouse //nullptr
//
//)
//{
//	HouseClass* pHouseOwner = Source ? Source->Owner : SourceHouse;
//	return MapClass::DamageArea
//	(pCoord, Damage, Source, Warhead, Warhead->Tiberium, pHouseOwner);
//}

//DEFINE_JUMP(CALL, 0x6632C7, GET_OFFSET(_RocketLocomotionClass_DamageArea));

DEFINE_OVERRIDE_HOOK(0x6622E0, RocketLocomotionClass_ILocomotion_Process_CustomMissile, 6)
{
	GET(AircraftClass* const, pThis, ECX);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pThis->Type);

	if (pExt->IsCustomMissile) {
		R->EAX(pExt->CustomMissileData.GetEx());
		return 0x66230A;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x66238A, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff1, 5)
{
	GET(ILocomotion* const, pThis, ESI);

	const auto pLocomotor = static_cast<RocketLocomotionClass* const>(pThis);
	const auto pOwner = static_cast<AircraftClass* const>(pLocomotor->LinkedTo);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

	if (AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {

		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false),
			pOwner->Owner,
			nullptr,
			pOwner,
			true
		);

		return 0x6623F3;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x662512, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff2, 5)
{
	GET(ILocomotion* const, pThis, ESI);

	const auto pLocomotor = static_cast<RocketLocomotionClass* const>(pThis);
	const auto pOwner = static_cast<AircraftClass* const>(pLocomotor->LinkedTo);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

	if (AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false),
			pOwner->Owner,
			nullptr,
			pOwner,
			true
		);

		return 0x66257B;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x6627E5, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff3, 5)
{
	GET(ILocomotion* const, pThis, ESI);

	const auto pLocomotor = static_cast<RocketLocomotionClass* const>(pThis);
	const auto pOwner = static_cast<AircraftClass* const>(pLocomotor->LinkedTo);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

	if (AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false),
			pOwner->Owner,
			nullptr,
			pOwner,
			true
		);

		return 0x662849;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x662D85, RocketLocomotionClass_ILocomotion_Process_CustomMissileTrailer, 6)
{
	GET(ILocomotion* const, pThis, ESI);

	const auto pLocomotor = static_cast<RocketLocomotionClass* const>(pThis);

	if (pLocomotor->TrailerTimer.Expired())
	{
		const auto pOwner = static_cast<AircraftClass* const>(pLocomotor->LinkedTo);
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

		pLocomotor->TrailerTimer.Start(pExt->CustomMissileTrailerSeparation);

		if (AnimTypeClass* pType = pExt->CustomMissileTrailerAnim) {
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pOwner->Location),
				pOwner->Owner,
				nullptr,
				pOwner,
				true
			);
		}

		return 0x662E16;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x66305A, RocketLocomotionClass_Explode_CustomMissile, 6)
{
	GET(AircraftTypeClass* const, pType, ECX);
	GET(RocketLocomotionClass* const, pLocomotor, ESI);

	LEA_STACK(WarheadTypeClass**, ppWarhead, 0x10);
	LEA_STACK(RocketStruct**, ppRocketData, 0x14);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->IsCustomMissile)
	{
		*ppRocketData = pExt->CustomMissileData.GetEx();

		const bool isElite = pLocomotor->SpawnerIsElite;
		*ppWarhead = (isElite ? pExt->CustomMissileEliteWarhead : pExt->CustomMissileWarhead);

		return 0x6630DD;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x663218, RocketLocomotionClass_Explode_CustomMissile2, 5)
{
	GET(RocketLocomotionClass* const, pThis, ESI);
	REF_STACK(CoordStruct const, coords, STACK_OFFS(0x60, 0x18));

	const auto pOwner = static_cast<AircraftClass* const>(pThis->LinkedTo);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pOwner->Type);

	if (pExt->IsCustomMissile) {
		if (auto const& pWeapon = pThis->SpawnerIsElite
			? pExt->CustomMissileEliteWeapon : pExt->CustomMissileWeapon) {
			WeaponTypeExtData::DetonateAt(pWeapon, coords, pOwner , true , pOwner ? pOwner->Owner : nullptr);
			pOwner->Limbo();
			pOwner->UnInit();
			return 0x6632D9;
		}
	}

	GET(int, nDamage, EDI);
	GET_STACK(WarheadTypeClass* const, pWH, STACK_OFFS(0x60, 0x50));
	LEA_STACK(CellStruct* const, pCellStr, STACK_OFFS(0x60, 0x38));
	const auto pCell = MapClass::Instance->GetCellAt(pCellStr);

	if (auto pAnimType = MapClass::SelectDamageAnimation(nDamage, pWH, pCell->LandType, coords)) {
		AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pAnimType, coords, 0, 1, 0x2600, -15),
			pOwner->Owner,
			pOwner->Target ? pOwner->Target->GetOwningHouse() : nullptr,
			pOwner,
			true
		);
	}

	MapClass::FlashbangWarheadAt(nDamage, pWH, coords, false);
	MapClass::DamageArea(coords, nDamage, pOwner, pWH, pWH->Tiberium, pOwner->Owner);
	pOwner->Limbo();
	pOwner->UnInit();
	return 0x6632D9;
}

DEFINE_OVERRIDE_HOOK(0x6632F2, RocketLocomotionClass_ILocomotion_MoveTo_CustomMissile, 6)
{
	GET(AircraftTypeClass* const, pType, EDX);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->IsCustomMissile) {
		R->EDX(pExt->CustomMissileData.GetEx());
		return 0x66331E;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x6634F6, RocketLocomotionClass_ILocomotion_DrawMatrix_CustomMissile, 6)
{
	GET(AircraftTypeClass* const, pType, ECX);
	const auto pExt = TechnoTypeExtContainer::Instance.Find(pType);

	if (pExt->IsCustomMissile) {
		R->EAX(pExt->CustomMissileData.GetEx());
		return 0x66351B;
	}

	return 0;
}

// new
// SpawnerOwner may die when this processed , should store the data at SpawnManagerExt or something
DEFINE_HOOK(0x662720, RocketLocomotionClass_ILocomotion_Process_Raise, 0x6)
{
	enum { Handled = 0x6624C8, Continue = 0x0 };

	GET(RocketLocomotionClass* const, pThis, ESI);

	if (const auto pAir = static_cast<AircraftClass* const>(pThis->Owner)) {
		const auto pExt = TechnoTypeExtContainer::Instance.Find(pAir->Type);
		if (pExt->IsCustomMissile.Get() && pAir->SpawnOwner) {
			if(!pExt->CustomMissileRaise.Get(pAir->SpawnOwner))
			return Handled;
		}
	}

	return Continue;
}

#pragma endregion
#pragma region SpawnManagerHooks
DEFINE_OVERRIDE_HOOK(0x6B6D60, SpawnManagerClass_CTOR_CustomMissile, 6)
{
	GET(SpawnManagerClass* const, pSpawnManager, ESI);
	return TechnoTypeExtContainer::Instance.Find(pSpawnManager->SpawnType)->IsCustomMissile ? 0x6B6D86 : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6B78F8, SpawnManagerClass_Update_CustomMissile, 6)
{
	GET(TechnoTypeClass* const, pSpawnType, EAX);
	return TechnoTypeExtContainer::Instance.Find(pSpawnType)->IsCustomMissile ? 0x6B791F : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6B7A72, SpawnManagerClass_Update_CustomMissile2, 6)
{
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	GET(int, idxSpawn, EDI);
	GET(TechnoTypeClass* const, pSpawnType, EDX);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pSpawnType);

	if (pExt->IsCustomMissile) {
		auto node = &pSpawnManager->SpawnedNodes.Items[idxSpawn]->NodeSpawnTimer;
		node->StartTime = Unsorted::CurrentFrame();
		node->TimeLeft = pExt->CustomMissileData->PauseFrames + pExt->CustomMissileData->TiltFrames;
		return 0x6B7B03;
	}

	return 0;
}

//new
DEFINE_HOOK(0x6B750B, SpawnManagerClass_Update_CustomMissilePreLauchAnim, 0x5)
{
	GET(AircraftClass*, pSpawned, EDI);

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pSpawned->Type);

	if (pSpawned->Type == RulesClass::Instance->CMisl.Type) {
		return 0x0;
	} else if (pTypeExt->IsCustomMissile) {
		if(AnimTypeClass* pType = pTypeExt->CustomMissilePreLauchAnim) {
			AnimExtData::SetAnimOwnerHouseKind(GameCreate<AnimClass>(pType, pSpawned->Location, 2, 1, 0x600, -10, false),
				pSpawned->Owner,
				nullptr,
				pSpawned,
				true
			);
		}
	}

	return 0x6B757A;
}

// new
DEFINE_HOOK(0x6B74BC, SpawnManagerClass_Update_MissileCoordOffset, 0x6)
{
	enum
	{
		OffsetBy28 = 0x6B74C4,
		GetPrimaryFacing = 0x6B74DB
	};

	//GET(SpawnManagerClass*, pThis, ESI);
	//GET(AircraftClass*, pSpawned, EDI);
	GET(AircraftTypeClass* const, pMissile, EAX);

	const auto pExt = TechnoTypeExtContainer::Instance.Find(pMissile);

	if (pExt->IsCustomMissile && pExt->CustomMissileOffset.isset()) {
		R->Stack(0x2C, R->ECX<int>() - pExt->CustomMissileOffset->X);
		R->Stack(0x30, R->EDX<int>() - pExt->CustomMissileOffset->Y);
	}
	else if(pMissile == RulesClass::Instance->CMisl.Type) {
		return OffsetBy28;
	}

	return GetPrimaryFacing;
}

DEFINE_OVERRIDE_HOOK(0x6B7D50, SpawnManagerClass_CountDockedSpawns, 0x6)
{
	GET(SpawnManagerClass*, pThis, ECX);

	int nCur = 0;
	if(pThis) { // some function call this without checking , so here it is the check
		for (auto const& pNode : pThis->SpawnedNodes)
		{
			const auto nStatus = pNode->Status;
			const auto nEligible =
				nStatus == SpawnNodeStatus::Idle
				|| nStatus == SpawnNodeStatus::Reloading
				|| nStatus == SpawnNodeStatus::Dead
				// spawn timer should be updated somewhere ?
				&& pNode->NodeSpawnTimer.IsNotActive();

			if (nEligible)
			{
				++nCur;
			}

		}
	}

	R->EAX(nCur);
	return 0x6B7D73;
}
#pragma endregion
