#include "Body.h"

enum CellFailure
{
	No = -1145,
	NotClear = No,
	InvalidCell = No,
	DifferentLevel = No
};

//Don't know if such thing exists already or not, maybe more efficient (or less?)
//DO NOT USE THIS
static int _FlatAreaLevel(ObjectClass* ignoreMe, CellClass* cell, short spacex, short spacey, int previousLevel)
{
	if (!cell)
		return CellFailure::InvalidCell;
	bool isClear = false;
	if (cell->OverlayTypeIndex == -1)
	{
		if (auto content = cell->FirstObject)
		{
			if (ignoreMe && content->UniqueID == ignoreMe->UniqueID && !content->NextObject)
				isClear = true;
		}
		else isClear = true;
	}
	if (!isClear)
		return CellFailure::NotClear;

	const int level = cell->GetLevel();
	if (level != previousLevel)
		return CellFailure::DifferentLevel;

	if (spacex <= 1 && spacey <= 1)
		return level;

	int Slevel = _FlatAreaLevel(ignoreMe, cell->GetNeighbourCell(Direction::S), 1, spacey - 1, level);

	if (Slevel == CellFailure::No)
		return CellFailure::No;

	if (spacex == 1)
		return Slevel == previousLevel ? Slevel : CellFailure::DifferentLevel;


	int Elevel = _FlatAreaLevel(ignoreMe, cell->GetNeighbourCell(Direction::E), spacex - 1, spacey, level);
	if (Elevel != CellFailure::No
		&& Slevel == Elevel && Elevel == level && level == previousLevel)
		return level;
	else
		return CellFailure::DifferentLevel;
}

static inline bool EnoughSpaceToExpand(ObjectClass* ignoreThis, CellClass* cell, short spacex, short spacey)
{
	return _FlatAreaLevel(ignoreThis, cell, spacex, spacey, cell->GetLevel()) != CellFailure::No;
}

//issue #621: let the mcv in hunt mission deploy asap
void MCVFindBetterPlace(TechnoClass* pThis)
{
	if (pThis->WhatAmI() == AbstractType::Unit &&
	pThis->GetTechnoType()->Category == Category::Support &&
	!pThis->GetOwningHouse()->IsControlledByHuman()
		)
	{// All mcv at the skirmish beginning is hunting
		const auto pFoot = abstract_cast<UnitClass*>(pThis);
		const auto deployType = pFoot->Type->DeploysInto;
		if (pFoot && deployType)
		{
			short XWidth = deployType->GetFoundationWidth();
			short YWidth = deployType->GetFoundationHeight(true);
			if (!pFoot->Destination &&
				(pFoot->GetCurrentMission() == Mission::Hunt || pFoot->GetCurrentMission() == Mission::Unload) &&
				!EnoughSpaceToExpand(pFoot, pFoot->GetCell(), XWidth, YWidth)
				)
			{//for other locomotors they don't have a destination, so give it the nearest location
				CellStruct coord = pFoot->GetCell()->MapCoords;
				coord = MapClass::Instance->NearByLocation(
					coord, pFoot->Type->SpeedType, -1, MovementZone::Normal, false,
					XWidth, YWidth,
					true, false, false, false, CellStruct::Empty, false, true);

				if (const auto tgtCell = MapClass::Instance->TryGetCellAt(coord))
				{
					pFoot->SetDestination(tgtCell, true);
					pFoot->QueueMission(Mission::Guard, false);
				}
			}
		}
	}
}

//Not working for no ore map? whatever
void HarvesterLocoFix(TechnoClass* pThis)
{
	if (pThis->WhatAmI() == AbstractType::Unit)
	{
		const auto pFoot = abstract_cast<UnitClass*>(pThis);
		if (pFoot && !pThis->IsSelected && pFoot->Type->Harvester &&
			(pFoot->Type->Locomotor == LocomotionClass::CLSIDs::Tunnel ||
				pFoot->Type->Locomotor == LocomotionClass::CLSIDs::Jumpjet) &&
			(pFoot->GetCurrentMission() == Mission::Guard ||
				pFoot->GetCurrentMission() == Mission::Area_Guard) &&
			!TechnoExt::IsHarvesting(pThis) && !pFoot->Locomotor->Is_Really_Moving_Now()
		)
		{
			pFoot->QueueMission(Mission::Harvest, true);
		}
	}
}


#ifdef ENABLE_HOMING_MISSILE
if (pTypeExt->MissileHoming
	&& pThis->Spawned
	&& pThis->Type->MissileSpawn)
{
	const auto pLoco = static_cast<LocomotionClass*>(pThis->Locomotor.get());
	CLSID nID { };
	pLoco->GetClassID(&nID);

	if (nID == LocomotionClass::CLSIDs::Rocket())
	{
		if (auto const pTracker = pExt->MissileTargetTracker)
		{
			pTracker->AI();
			auto const pRocket = static_cast<RocketLocomotionClass*>(pLoco);

			//check if the coord is actually valid
			// if not , just move on
			if (pRocket->MissionState > 2 && pTracker->Coord && (Map.GetCellAt(pTracker->Coord)))
				pRocket->MovingDestination = pTracker->Coord;
		}
	}
}
#endif


/*
DEFINE_HOOK(0x4F8FE1, Houseclass_AI_Add, 0x5)
{
	return 0x0;
}
*/


//DEFINE_HOOK(0x55DC99, LoGicClassUpdate_probe, 0x5)
//{
//	return 0x0;
//}
//
//DEFINE_HOOK(0x55B4EB, KamikazeClass_AI_Probe, 0x5)
//{
//   return 0x0;
//}
//
//DEFINE_HOOK(0x55AFB3, LogicClass_Update, 0x6) {
//	//VerticalLaserClass::OnUpdateAll();
//	return 0x0;
//}
//
//DEFINE_HOOK(0x55B719, LogicClass_Update_Late, 0x5)
//{
//	//VerticalLaserClass::OnUpdateAll();
//#ifdef ENABLE_HOMING_MISSILE
//	HomingMissileTargetTracker::Update_All();
//#endif
//	return 0x0;
//}
//
//// in progress: Initializing Tactical display
//DEFINE_HOOK(0x6875F3 , Scenario_Start1, 0x6) {
//
//	return 0;
//}


//DEFINE_HOOK(0x6F858F, TechnoClass_CanAutoTarget_BuildingOut1, 0x6)
//{
//	GET(TechnoClass*, pThis, EDI);
//	GET(TechnoClass*, pTarget, ESI);
//
//	enum { CannotTarget = 0x6F85F8, NextIf = 0x6F860C, FarIf = 0x6F866D };
//
//	if (FootClass* pFoot = abstract_cast<FootClass*>(pThis))
//	{
//		if (pFoot->Team != nullptr
//			|| !pFoot->Owner->IsControlledByHuman()
//			|| pTarget->IsStrange()
//			|| pTarget->WhatAmI() != AbstractType::Building
//			|| pTarget->GetTurretWeapon() && pTarget->GetTurretWeapon()->WeaponType != nullptr && pTarget->GetThreatValue())
//		{
//			//game code
//			if (!pThis->IsEngineer())
//				return FarIf;
//
//			return NextIf;
//		}
//		else
//		{
//			if (!pThis->IsEngineer())
//			{
//				//dehardcode
//				if (pTarget->WhatAmI() == AbstractType::Building && pTarget->GetThreatValue())
//				{
//					return FarIf;
//				}
//
//				return CannotTarget;
//			}
//
//			return NextIf;
//		}
//	}
//
//	return NextIf;
//}

//
//DEFINE_HOOK(0x6F889B, TechnoClass_CanAutoTarget_BuildingOut2, 0xA)
//{
//	GET(TechnoClass*, pTarget, ESI);
//
//	enum { CannotTarget = 0x6F894F, GameCode = 0x6F88BF };
//
//	if (pTarget->WhatAmI() != AbstractType::Building)
//		return CannotTarget;
//
//	WeaponStruct* pWeapon = pTarget->GetTurretWeapon();
//
//	if (pWeapon == nullptr || pWeapon->WeaponType == nullptr)
//	{
//		if (pTarget->GetThreatValue())
//			return GameCode;
//
//		return CannotTarget;
//	}
//
//	return GameCode;
//}

//DEFINE_HOOK(0x6FDD93, TechnoClass_FireAt_DelayedFire, 0x6) // or 0x6FDD99  , 0x6
//{
//	GET(WeaponTypeClass*, pWeaponType, EBX);
//	GET(TechnoClass*, pThis, ESI);
//	//GET_STACK(FootClass*, pTarget, 0xD4);//8);
//
//	enum { skipDelayedFire = 0, skipFireAt = 0x6FDE03 };
//
//	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeaponType);
//	auto pExt = TechnoExt::ExtMap.Find(pThis);
//
//	if (pWeaponTypeExt->DelayedFire_Anim_LoopCount <= 0 || !pWeaponTypeExt->DelayedFire_Anim.isset())
//		return skipDelayedFire;
//
//	if (pWeaponTypeExt->DelayedFire_DurationTimer.Get() > 0 && pExt->DelayedFire_DurationTimer < 0)
//		pExt->DelayedFire_DurationTimer = pWeaponTypeExt->DelayedFire_DurationTimer.Get();
//
//	AnimTypeClass* pDelayedFireAnimType = pWeaponTypeExt->DelayedFire_Anim.isset() ? pWeaponTypeExt->DelayedFire_Anim.Get() : nullptr;
//	bool hasValidDelayedFireAnimType = pDelayedFireAnimType ? true : false;
//
//	if (!hasValidDelayedFireAnimType)
//	{
//		pExt->DelayedFire_Anim = nullptr;
//		pExt->DelayedFire_Anim_LoopCount = 0;
//		pExt->DelayedFire_DurationTimer = -1;
//
//		return skipDelayedFire;
//	}
//
//	bool isDelayedFireAnimPlaying = pExt->DelayedFire_Anim ? true : false;
//	bool hasDeployAnimFinished = (isDelayedFireAnimPlaying && (pExt->DelayedFire_Anim->Animation.Value >= pDelayedFireAnimType->End + pDelayedFireAnimType->Start - 1)) ? true : false;
//
//	if (!isDelayedFireAnimPlaying)
//	{
//		// Create the DelayedFire animation & stop the Fire process
//		TechnoTypeClass* pThisType = pThis->GetTechnoType();
//		int weaponIndex = pThis->CurrentWeaponNumber;
//		CoordStruct animLocation = pThis->Location;
//
//		if (pWeaponTypeExt->DelayedFire_Anim_UseFLH)
//			animLocation = TechnoExt::GetFLHAbsoluteCoords(pThis, pThisType->GetWeapon(weaponIndex)->FLH, pThis->HasTurret());//pThisType->Weapon[weaponIndex].FLH;
//
//		if (auto pAnim = GameCreate<AnimClass>(pDelayedFireAnimType, animLocation))//pThis->Location))//animLocation))
//		{
//			pExt->DelayedFire_Anim = pAnim;
//			AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, pThis, false);
//			pExt->DelayedFire_Anim->SetOwnerObject(pThis);
//			pExt->DelayedFire_Anim_LoopCount++;
//		}
//		else
//		{
//			Debug::Log("ERROR! DelayedFire animation [%s] -> %s can't be created.\n", pThis->GetTechnoType()->ID, pDelayedFireAnimType->ID);
//			TechnoExt::ResetDelayFireAnim(pThis);
//			return skipDelayedFire;
//		}
//	}
//	else
//	{
//		if (pWeaponTypeExt->DelayedFire_DurationTimer.Get() > 0)
//		{
//			pExt->DelayedFire_DurationTimer--;
//
//			if (pExt->DelayedFire_DurationTimer <= 0)
//			{
//				pExt->DelayedFire_Anim = nullptr;
//				pExt->DelayedFire_Anim_LoopCount = 0;
//				pExt->DelayedFire_DurationTimer = -1;
//
//				return skipDelayedFire;
//			}
//		}
//
//		if (hasDeployAnimFinished)
//		{
//			// DelayedFire animation finished but it can repeat more times, if set
//			pExt->DelayedFire_Anim = nullptr;
//
//			if (pExt->DelayedFire_Anim_LoopCount >= pWeaponTypeExt->DelayedFire_Anim_LoopCount && pWeaponTypeExt->DelayedFire_Anim_LoopCount > 0)
//			{
//				pExt->DelayedFire_Anim_LoopCount = 0;
//				pExt->DelayedFire_DurationTimer = -1;
//
//				return skipDelayedFire;
//			}
//		}
//	}
//
//
//	return skipFireAt;
//}


//
//DEFINE_HOOK(0x6F42F7, TechnoClass_Init_NewEntities, 0x2)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//
//	//if (auto pShieldObj = ShieldObject::CreateMe())
//	//	Debug::Log("[%x] Created ! \n", pShieldObj);
//
//	return 0;
//}

#ifdef ENABLE_HOMING_MISSILE
if (const auto pFoot = specific_cast<AircraftClass*>(pThis))
{
	if (auto const pLoco = static_cast<LocomotionClass*>(pFoot->Locomotor.get()))
	{
		CLSID nID { };
		pLoco->GetClassID(&nID);

		if (nID == LocomotionClass::CLSIDs::Rocket && pTypeExt->MissileHoming)
		{
			pExt->MissileTargetTracker = GameCreate<HomingMissileTargetTracker>();
		}
	}
}
#endif

#ifdef ENABLE_HOMING_MISSILE
pExt->IsMissileHoming = pTypeExt->MissileHoming.Get();
#endif

//if (pExt && pTypeExt) {
//	if (pExt->ID != pThis->get_ID()) {
//		pExt->ID = pThis->get_ID();
//	}
//}
//
//if (pThis->TemporalTargetingMe)
//{
//	if (auto const pCell = pThis->GetCell())
//	{
//		if (auto const pBld = pCell->GetBuilding())
//		{
//			if (pBld->Type->BridgeRepairHut)
//			{
//				pThis->TemporalTargetingMe->Detach();
//			}
//		}
//	}
//}
