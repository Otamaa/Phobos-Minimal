#include <AbstractClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <HoverLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <Conversions.h>

#include <RocketLocomotionClass.h>
#include <SpawnManagerClass.h>

#pragma region RocketLocoHooks
//TODO : Cmisl Hardcoded shit 
// 662496
// 66235D

//static DamageAreaResult __fastcall _RocketLocomotionClass_DamageArea(
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
	GET(AircraftClass*, pThis, ECX);

	auto pExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pExt->IsCustomMissile) {
		R->EAX(pExt->CustomMissileData.GetEx());
		return 0x66230A;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x66238A, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff1, 5)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	auto pOwner = static_cast<AircraftClass*>(pLocomotor->LinkedTo);
	auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type);

	if (AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
		if (auto pAnim = GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner->Owner, nullptr, pOwner, true);
		}
		return 0x6623F3;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x662512, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff2, 5)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	auto pOwner = static_cast<AircraftClass*>(pLocomotor->LinkedTo);
	auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type);

	if (AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
		if (auto pAnim = GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner->Owner, nullptr, pOwner, true);
		}
		return 0x66257B;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x6627E5, RocketLocomotionClass_ILocomotion_Process_CustomMissileTakeoff3, 5)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	auto pOwner = static_cast<AircraftClass*>(pLocomotor->LinkedTo);
	auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type);

	if (AnimTypeClass* pType = pExt->CustomMissileTakeoffAnim) {
		if (auto pAnim = GameCreate<AnimClass>(pType, pOwner->Location, 2, 1, 0x600, -10, false)) {
			AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner->Owner, nullptr, pOwner, true);
		}
		return 0x662849;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x662D85, RocketLocomotionClass_ILocomotion_Process_CustomMissileTrailer, 6)
{
	GET(ILocomotion*, pThis, ESI);

	auto pLocomotor = static_cast<RocketLocomotionClass*>(pThis);
	auto pOwner = static_cast<AircraftClass*>(pLocomotor->LinkedTo);
	auto pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type);

	if (pLocomotor->TrailerTimer.Expired())
	{
		pLocomotor->TrailerTimer.Start(pExt->CustomMissileTrailerSeparation);

		if (AnimTypeClass* pType = pExt->CustomMissileTrailerAnim) {
			if (auto pAnim = GameCreate<AnimClass>(pType, pOwner->Location)) {
				AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner->Owner, nullptr, pOwner, true);
			}
		}

		return 0x662E16;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x66305A, RocketLocomotionClass_Explode_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, ECX);
	GET(RocketLocomotionClass*, pLocomotor, ESI);

	LEA_STACK(WarheadTypeClass**, ppWarhead, 0x10);
	LEA_STACK(RocketStruct**, ppRocketData, 0x14);

	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->IsCustomMissile)
	{
		*ppRocketData = pExt->CustomMissileData.GetEx();

		bool isElite = pLocomotor->SpawnerIsElite;
		*ppWarhead = (isElite ? pExt->CustomMissileEliteWarhead : pExt->CustomMissileWarhead);

		return 0x6630DD;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x663218, RocketLocomotionClass_Explode_CustomMissile2, 5)
{
	GET(RocketLocomotionClass* const, pThis, ESI);
	REF_STACK(CoordStruct const, coords, STACK_OFFS(0x60, 0x18));

	auto const pOwner = static_cast<AircraftClass*>(pThis->LinkedTo);
	auto const pExt = TechnoTypeExt::ExtMap.Find(pOwner->Type);

	if (pExt->IsCustomMissile) {
		if (auto const& pWeapon = pThis->SpawnerIsElite
			? pExt->CustomMissileEliteWeapon : pExt->CustomMissileWeapon) {
			WeaponTypeExt::DetonateAt(pWeapon, coords, pOwner);
			return 0x6632CC;
		}
	}

	GET(int, nDamage, EDI);
	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFS(0x60, 0x50));
	LEA_STACK(CellStruct* const, pCellStr, STACK_OFFS(0x60, 0x38));
	auto const pCell = MapClass::Instance->GetCellAt(pCellStr);

	if (auto pAnimType = MapClass::SelectDamageAnimation(nDamage, pWH, pCell->LandType, coords)) {
		if (auto pAnim = GameCreate<AnimClass>(pAnimType, coords, 0, 1, 0x2600, -15)) {
			auto const pTargetOwner = pOwner->Target ? pOwner->Target->GetOwningHouse() : nullptr;
			AnimExt::SetAnimOwnerHouseKind(pAnim, pOwner->Owner, pTargetOwner, pOwner, true);
		}
	}

	MapClass::FlashbangWarheadAt(nDamage, pWH, coords, false);
	MapClass::DamageArea(coords, nDamage, pOwner, pWH, pWH->Tiberium, pOwner->Owner);

	return 0x6632CC;
	//return 0;
}

DEFINE_OVERRIDE_HOOK(0x6632F2, RocketLocomotionClass_ILocomotion_MoveTo_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, EDX);
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->IsCustomMissile) {
		R->EDX(pExt->CustomMissileData.GetEx());
		return 0x66331E;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x6634F6, RocketLocomotionClass_ILocomotion_DrawMatrix_CustomMissile, 6)
{
	GET(AircraftTypeClass*, pType, ECX);
	auto pExt = TechnoTypeExt::ExtMap.Find(pType);

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

	if (const auto pAir = static_cast<AircraftClass*>(pThis->Owner)) {
		const auto pExt = TechnoTypeExt::ExtMap.Find(pAir->Type);
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
	GET(SpawnManagerClass*, pSpawnManager, ESI);
	return TechnoTypeExt::ExtMap.Find(pSpawnManager->SpawnType)->IsCustomMissile ? 0x6B6D86 : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6B78F8, SpawnManagerClass_Update_CustomMissile, 6)
{
	GET(TechnoTypeClass*, pSpawnType, EAX);
	return TechnoTypeExt::ExtMap.Find(pSpawnType)->IsCustomMissile ? 0x6B791F : 0x0;
}

DEFINE_OVERRIDE_HOOK(0x6B7A72, SpawnManagerClass_Update_CustomMissile2, 6)
{
	//GET(SpawnManagerClass*, pSpawnManager, ESI);
	//GET(int, idxSpawn, EDI);
	GET(TechnoTypeClass*, pSpawnType, EDX);

	const auto pExt = TechnoTypeExt::ExtMap.Find(pSpawnType);

	if (pExt->IsCustomMissile) {
		R->EDX(Unsorted::CurrentFrame());
		//pSpawnManager->SpawnedNodes.Items[idxSpawn]->SpawnTimer.Start();
		R->ECX(pExt->CustomMissileData->PauseFrames + pExt->CustomMissileData->TiltFrames);
		//return 0x6B7B03;
		return 0x6B7AB1;
	}

	return 0;
}

//new
DEFINE_HOOK(0x6B750B, SpawnManagerClass_Update_CustomMissilePreLauchAnim, 0x6)
{
	GET(AircraftClass*, pSpawned, EDI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSpawned->Type);
	
	if (pSpawned->Type == RulesClass::Instance->CMisl.Type) {
		return 0x0;
	} else if (pTypeExt->IsCustomMissile) {
		if(AnimTypeClass* pType = pTypeExt->CustomMissilePreLauchAnim)
			if (auto pAnim = GameCreate<AnimClass>(pType, pSpawned->Location, 2, 1, 0x600, -10, false))
				AnimExt::SetAnimOwnerHouseKind(pAnim, pSpawned->Owner, nullptr, pSpawned, true);
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
	GET(AircraftTypeClass*, pMissile, EAX);

	const auto pExt = TechnoTypeExt::ExtMap.Find(pMissile);

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
	for (auto const& pNode : pThis->SpawnedNodes)
	{
		const auto  nStatus = pNode->Status;
		const auto  nEligible =
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

	R->EAX(nCur);
	return 0x6B7D73;
}
#pragma endregion
