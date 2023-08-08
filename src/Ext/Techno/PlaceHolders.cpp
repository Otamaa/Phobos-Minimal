#include "Body.h"


/*
DEFINE_HOOK(0x4B387A, DriveLocoClass_ClearNavCom2_Empty_WTF, 0x4)
{
	GET(UnitClass*, pLinked, ECX);

	if (pLinked->Destination)
	{
		if (auto pDest = specific_cast<AircraftClass*>(pLinked->Destination))
		{
			return 0x4B3607;
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x4B05EE, DriveLocoClass_InfCheck_Extend , 0x5)
{
	GET(AbstractClass*, pDest, ECX);

	return pDest->WhatAmI() == AbstractType::Infantry || pDest->WhatAmI() == AbstractType::Aircraft ? 0x4B05F8 : 0x4B063D;
}*/

/* Dont Enable ! , broke targeting !
DEFINE_HOOK(0x6F7891, TechnoClass_TriggersCellInset_IgnoreVertical, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(CoordStruct, nCoord, STACK_OFFS(0x30, 0xC));
	GET(TechnoClass*, pTarget, EBX);

	bool bRangeIgnoreVertical = false;
	if (auto const pExt = WeaponTypeExt::ExtMap.Find(pWeapon)) {
		bRangeIgnoreVertical = pExt->Range_IgnoreVertical.Get();
	}

	if (pThis->IsInAir() && !bRangeIgnoreVertical) {
		nCoord.Z = pTarget->GetCoords().Z;
	}

	R->EAX(pThis->InRange(nCoord, pTarget, pWeapon));
	return 0x6F78BD;
}

DEFINE_HOOK(0x6F7893, TechnoClass_TriggersCellInset_IgnoreVertical, 0x5)
{
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET(TechnoClass*, pThis, ESI);

	bool bRangeVertical = pThis->IsInAir();
	if (auto const pExt = WeaponTypeExt::ExtMap.Find(pWeapon)) {
		bRangeVertical = bRangeVertical && !pExt->Range_IgnoreVertical.Get();
	}

	R->AL(bRangeVertical);
	return 0x6F7898;
}*/


/*
DEFINE_HOOK(0x6F3B2E, TechnoClass_Transform_FLH, 0x6)
{
	GET(WeaponStruct*, nWeaponStruct, EAX);
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, idxWeapon, 0x8);

	CoordStruct nRet = nWeaponStruct->FLH;

	if (auto const pInf = specific_cast<InfantryClass*>(pThis))
	{
		if (pInf->Crawling)
		{
			if (auto const pExt = TechnoTypeExt::ExtMap.Find(pInf->Type))
			{
				if (!pThis->Veterancy.IsElite())
				{
					if (idxWeapon == 0)
						nRet = pExt->PrimaryCrawlFLH.Get(nWeaponStruct->FLH);
					else
						nRet = pExt->SecondaryCrawlFLH.Get(nWeaponStruct->FLH);
				}
				else
				{
					if (idxWeapon == 0)
						nRet = pExt->Elite_PrimaryCrawlFLH.Get(nWeaponStruct->FLH);
					else
						nRet = pExt->Elite_SecondaryCrawlFLH.Get(nWeaponStruct->FLH);
				}
			}
		}
	}

	R->ECX(nRet.X);
	R->EBP(nRet.Y);
	R->EAX(nRet.Z);

	return 0x6F3B37;
}*/

// DEFINE_HOOK(0x702E9D, TechnoClass_RegisterDestruction, 0x6)
// {
// 	GET(TechnoClass*, pVictim, ESI);
// 	GET(TechnoClass*, pKiller, EDI);
// 	GET(int, cost, EBP);

// 	const auto pVictimTypeExt = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType());
// 	const auto pKillerTypeExt = TechnoTypeExt::ExtMap.Find(pKiller->GetTechnoType());
// 	const double giveExpMultiple = pVictimTypeExt->Experience_VictimMultiple.Get();
// 	const double gainExpMultiple = pKillerTypeExt->Experience_KillerMultiple.Get();

// 	R->EBP(int(cost * giveExpMultiple * gainExpMultiple));

// 	return 0;
// }

//DropPod able to move Limboed passengers outside
// if it failed to move , the locomotor is not relesed
//Teleport loco cant move limboed passengers , passengers will stuck after ejected with Chrono locomotor still intact
bool CreateWithDroppod(FootClass* Object, const CoordStruct& XYZ, const CLSID& nID = LocomotionClass::CLSIDs::DropPod)
{
	auto MyCell = MapClass::Instance->GetCellAt(XYZ);
	if (Object->IsCellOccupied(MyCell, -1, -1, nullptr, false) != Move::OK)
	{
		Debug::Log("Cell occupied... poof!\n");
		return false;
	}
	else
	{
		Debug::Log("Destinating %s @ {%d, %d, %d}\n", Object->get_ID(), XYZ.X, XYZ.Y, XYZ.Z);
		LocomotionClass::ChangeLocomotorTo(Object, nID);
		CoordStruct xyz = XYZ;
		xyz.Z = 0;
		Object->SetLocation(xyz);
		Object->SetDestination(MyCell, 1);
		Object->Locomotor.get()->Move_To(XYZ);
		Object->PrimaryFacing.Set_Desired(DirStruct());
		if (!Object->InLimbo)
		{
			Object->See(0, 0);
			return true;
		}

		Debug::Log("InLimbo... failed?\n");
		return false;
	}
}

int DrawHealthBar_Pip(TechnoClass* pThis, bool isBuilding, const Point3D& Pip)
{
	const auto strength = pThis->GetTechnoType()->Strength;

	if (pThis->Health > RulesClass::Instance->ConditionYellow * strength && Pip.X != -1)
		return Pip.X;
	else if (pThis->Health > RulesClass::Instance->ConditionRed * strength && (Pip.Y != -1 || Pip.X != -1))
		return Pip.Y == -1 ? Pip.X : Pip.Y;
	else if (Pip.Z != -1 || Pip.X != -1)
		return Pip.Z == -1 ? Pip.X : Pip.Z;

	return isBuilding ? 5 : 16;
}

int DrawHealthBar_PipAmount(TechnoClass* pThis, int iLength)
{
	return pThis->Health > 0
		? std::clamp(static_cast<int>(std::round(pThis->GetHealthPercentage() * iLength)), 0, iLength)
		: 0;
}

void DrawGroupID_Building(TechnoClass* pThis, Point2D* pLocation, const Point2D& GroupID_Offs)
{
	auto const pHouse = pThis->GetOwningHouse() ? pThis->GetOwningHouse() : HouseExt::FindCivilianSide();

	CoordStruct vCoords = { 0, 0, 0 };
	pThis->GetTechnoType()->Dimension2(&vCoords);

	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };

	Point2D vPos = { 0, 0 };
	TacticalClass::Instance->CoordsToScreen(&vPos, &vCoords2);

	Point2D vLoc = *pLocation;
	vLoc.X += vPos.X;
	vLoc.Y += vPos.Y;

	Point2D vOffset = GroupID_Offs;

	vLoc.X += vOffset.X;
	vLoc.Y += vOffset.Y;

	if (pThis->Group >= 0)
	{
		const COLORREF GroupIDColor = Drawing::RGB2DWORD(pHouse->Color);

		RectangleStruct rect
		{
			vLoc.X - 7,
			vLoc.Y + 26,
			12,13
		};

		DSurface::Temp->Fill_Rect(rect, COLOR_BLACK);
		DSurface::Temp->Draw_Rect(rect, GroupIDColor);

		const int groupid = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		Point2D vGroupPos
		{
			vLoc.X - 4,
			vLoc.Y + 25
		};

		rect = DSurface::Temp->Get_Rect_WithoutBottomBar();
		Point2D nTemp { };
		Fancy_Text_Print_Wide(&nTemp, L"%d", DSurface::Temp(), &rect, &vGroupPos, GroupIDColor, 0, TextPrintType::NoShadow, groupid);
	}
}

void DrawGroupID_Other(TechnoClass* pThis, Point2D* pLocation, const Point2D& GroupID_Offs)
{
	auto const pHouse = pThis->GetOwningHouse() ? pThis->GetOwningHouse() : HouseExt::FindCivilianSide();

	Point2D vLoc = *pLocation;
	Point2D vOffset = GroupID_Offs;

	vLoc.X += vOffset.X;
	vLoc.Y += vOffset.Y;

	if (pThis->Group >= 0)
	{
		if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			vLoc.X -= 20;
			vLoc.Y -= 25;
		}
		else
		{
			vLoc.X -= 30;
			vLoc.Y -= 23;
		}

		const COLORREF GroupIDColor = Drawing::RGB2DWORD(pHouse->Color);

		RectangleStruct rect
		{
			vLoc.X,
			vLoc.Y,
			12,13
		};

		DSurface::Temp->Fill_Rect(rect, COLOR_BLACK);
		DSurface::Temp->Draw_Rect(rect, GroupIDColor);

		const int groupid = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		rect = DSurface::Temp->Get_Rect_WithoutBottomBar();

		Point2D vGroupPos
		{
			vLoc.X + 2,
			vLoc.Y - 1
		};

		Point2D nTemp { };
		Fancy_Text_Print_Wide(&nTemp, L"%d", DSurface::Temp(), &rect, &vGroupPos, GroupIDColor, 0, TextPrintType::NoShadow, groupid);

	}
}

void DrawHealthBar_Other(TechnoClass* pThis, int iLength, Point2D* pLocation, RectangleStruct* pBound, ConvertClass* PipsPAL, SHPStruct* PipSHP, SHPStruct* PipBrdSHP, ConvertClass* PipBrdPAL, const Point3D& pip, const Point2D& DrawOffs, int nXOffset)
{
	if (PipSHP == nullptr) return;
	if (PipsPAL == nullptr) return;
	if (PipBrdSHP == nullptr) return;
	if (PipBrdPAL == nullptr) return;

	Point2D vPos = { 0,0 };
	Point2D vLoc = *pLocation;

	int frame, XOffset, YOffset;
	YOffset = pThis->GetTechnoType()->PixelSelectionBracketDelta;
	vLoc.Y -= 5;

	vLoc.X += nXOffset;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 11;
		vPos.Y = vLoc.Y - 20 + YOffset;
		frame = 1;
		XOffset = -5;
		YOffset -= 19;
	}
	else
	{
		vPos.X = vLoc.X + 1;
		vPos.Y = vLoc.Y - 21 + YOffset;
		frame = 0;
		XOffset = -15;
		YOffset -= 20;
	}

	if (pThis->IsSelected)
	{
		DSurface::Temp->DrawSHP(PipBrdPAL, PipBrdSHP, frame, &vPos, pBound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	const int iTotal = DrawHealthBar_PipAmount(pThis, iLength);

	frame = DrawHealthBar_Pip(pThis, false, pip);

	Point2D DrawOffset = DrawOffs ? DrawOffs : Point2D { 2,0 };

	for (int i = 0; i < iTotal; ++i)
	{
		vPos.X = vLoc.X + XOffset + DrawOffset.X * i;
		vPos.Y = vLoc.Y + YOffset + DrawOffset.Y * i;

		DSurface::Temp->DrawSHP(PipsPAL, PipSHP,
			frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}


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
			(pFoot->Type->Locomotor.get() == LocomotionClass::CLSIDs::Tunnel ||
				pFoot->Type->Locomotor.get() == LocomotionClass::CLSIDs::Jumpjet) &&
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
