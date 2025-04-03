
// ASMJIT_PATCH(0x6FF394, TechnoClass_FireAt_FeedbackAnim, 0x8)
// {
// 	enum { CreateMuzzleAnim = 0x6FF39C, SkipCreateMuzzleAnim = 0x6FF43F };
//
// 	GET(TechnoClass* const, pThis, ESI);
// 	GET(WeaponTypeClass* const, pWeapon, EBX);
// 	GET(AnimTypeClass* const, pMuzzleAnimType, EDI);
// 	LEA_STACK(CoordStruct*, pFLH, 0x44);
//
// 	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
//
// 	if (const auto pAnimType = pWeaponExt->Feedback_Anim.Get())
// 	{
// 		const auto nCoord = (pWeaponExt->Feedback_Anim_UseFLH ? *pFLH : pThis->GetCoords()) + pWeaponExt->Feedback_Anim_Offset;
// 		{
// 			auto pFeedBackAnim = GameCreate<AnimClass>(pAnimType, nCoord);
// 			AnimExtData::SetAnimOwnerHouseKind(pFeedBackAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);
// 			if (pThis->WhatAmI() != BuildingClass::AbsID)
// 				pFeedBackAnim->SetOwnerObject(pThis);
// 		}
// 	}
//
// 	return pMuzzleAnimType ? CreateMuzzleAnim : SkipCreateMuzzleAnim;
// }
//
// ASMJIT_PATCH(0x6FF3CD, TechnoClass_FireAt_AnimOwner, 0x7)
// {
// 	enum
// 	{
// 		Goto2NdCheck = 0x6FF427, DontSetAnim = 0x6FF43F,
// 		AdjustCoordsForBuilding = 0x6FF3D9, Continue = 0x0
// 	};
//
// 	GET(TechnoClass* const, pThis, ESI);
// 	GET(AnimClass*, pAnim, EDI);
// 	//GET(WeaponTypeClass*, pWeapon, EBX);
// 	GET_STACK(CoordStruct, nFLH, STACK_OFFS(0xB4, 0x6C));
//
// 	if (!pAnim)
// 		return DontSetAnim;
//
// 	AnimExtData::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), pThis->Target ? pThis->Target->GetOwningHouse() : nullptr, pThis, false);
//
// 	return pThis->WhatAmI() == BuildingClass::AbsID ? AdjustCoordsForBuilding : Goto2NdCheck;
// }
// ASMJIT_PATCH(0x703819, TechnoClass_Cloak_Deselect, 0x6)
// {
// 	enum { Skip = 0x70383C, CheckIsSelected = 0x703828 };
//
// 	return R->ESI<TechnoClass*>()->Owner->IsControlledByHuman()
// 		? CheckIsSelected : Skip;
// }

//ASMJIT_PATCH(0x6FE46E, TechnoClass_FireAt_DiskLaser, 0x7) {
//	GET(TechnoClass* const, pThis, ESI);
//	GET(WeaponTypeClass* const, pWeapon, EBX);
//	GET(int, damage, EDI);
//	GET_BASE(int, weapon_idx, 0xC);
//	GET_BASE(AbstractClass*, pTarget, 0x8);
//
//	auto pDiskLaser = GameCreate<DiskLaserClass>();
//
//	++pThis->CurrentBurstIndex;
//	int rearm = pThis->GetROF(weapon_idx);
//	pThis->ROF = rearm;
//	pThis->DiskLaserTimer.Start(rearm);
//	pThis->CurrentBurstIndex %= pWeapon->Burst;
//	pDiskLaser->Fire(pThis, pTarget, pWeapon, damage);
//
//	const auto pWeaponExt = WeaponTypeExtContainer::Instance.Find(pWeapon);
//
//	//this may crash the game , since the object got deleted after killself ,..
//	if (pWeaponExt->RemoveTechnoAfterFiring.Get())
//		TechnoExtData::KillSelf(pThis, KillMethod::Vanish);
//	else if (pWeaponExt->DestroyTechnoAfterFiring.Get())
//		TechnoExtData::KillSelf(pThis, KillMethod::Explode);
//
//	return 0x6FE4E7;
//}
//ASMJIT_PATCH(0x51D43F, InfantryClass_Scatter_Process, 0x6)
//{
//	GET(InfantryClass* const, pThis, ESI);
//
//	if (pThis->Type->JumpJet && pThis->Type->HoverAttack)
//	{
//		pThis->SetDestination(nullptr, 1);
//		return 0x51D47B;
//	}
//
//	return 0x0;
//}
//ASMJIT_PATCH(0x508FCE, HouseClass_SpySat_LimboDeliver, 0x6)
//{
//	GET(BuildingClass*, pBld, ECX);
//
//	return (!pBld->DiscoveredByCurrentPlayer && BuildingExtContainer::Instance.Find(pBld)->LimboID != -1) ?
//		0x508FE1 : 0x0;
//}
// ASMJIT_PATCH(0x6F09C4, TeamTypeClass_CreateOneOf_RemoveLog, 0x5)
// {
// 	GET_STACK(HouseClass* const, pHouse, STACK_OFFS(0x8, -0x4));
// 	R->EDI(pHouse);
// 	return 0x6F09D5;
// }
//
// ASMJIT_PATCH(0x6F0A3F, TeamTypeClass_CreateOneOf_CreateLog, 0x9)
// {
// 	GET(TeamTypeClass* const, pThis, ESI);
// 	GET(HouseClass* const, pHouse, EDI);
// 	const void* ptr = Allocate(sizeof(TeamClass));
// 	Debug::LogInfo("[%s - %x] Creating a new team named [%s - %x].", pHouse ? pHouse->get_ID() : GameStrings::NoneStrb() ,pHouse, pThis->ID, ptr);
// 	R->EAX(ptr);
// 	return 0x6F0A5A;
// }
//ASMJIT_PATCH(0x44DE2F, BuildingClass_MissionUnload_DisableBibLog, 0x5) { return 0x44DE3C; }

//ASMJIT_PATCH(0x4CA00D, FactoryClass_AbandonProduction_Log, 0x9)
//{
//	GET(FactoryClass* const, pThis, ESI);
//	GET(TechnoTypeClass* const, pType, EAX);
//	//Debug::LogInfo("[%x] Factory with Owner '%s' Abandoning production of '%s' ", pThis, pThis->Owner ? pThis->Owner->get_ID() : GameStrings::NoneStrb(), pType->ID);
//	R->ECX(pThis->Object);
//	return 0x4CA021;
//}
// this just an duplicate
//DEFINE_JUMP(LJMP, 0x702765, 0x7027AE);

// ASMJIT_PATCH(0x6FF48D , TechnoClass_FireAt_TargetLaser, 0x5)
// {
// 	GET(TechnoClass* const, pThis, ESI);
// 	//GET(WeaponTypeClass* const, pWeapon, EBX);
// 	GET_BASE(int, weaponIndex, 0xC);
//
// 	auto pType = pThis->GetTechnoType();
// 	if(pType->TargetLaser && pThis->Owner->ControlledByCurrentPlayer()) {
//
// 		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());
//
// 		if (pTypeExt->TargetLaser_WeaponIdx.empty()
// 			|| pTypeExt->TargetLaser_WeaponIdx.Contains(weaponIndex))
// 		{
// 			pThis->TargetLaserTimer.Start(pTypeExt->TargetLaser_Time.Get());
// 		}
// 	}
//
// 	return 0x6FF4CC;
// }
// ASMJIT_PATCH(0x73D909, UnitClass_Mi_Unload_LastPassengerOut, 8)
// {
// 	GET(UnitClass*, pThis, ESI);
//
// 	if (pThis->Passengers.NumPassengers < pThis->NonPassengerCount)
// 	{
// 		pThis->MissionStatus = 4;
// 		pThis->QueueMission(Mission::Guard, false);
// 		pThis->NextMission();
// 		pThis->unknown_bool_B8 = true;
// 	}
//
// 	return 0x0;
// }

// ASMJIT_PATCH(0x711F0F, TechnoTypeClass_GetCost_AICostMult, 0x8)
// {
// 	GET(HouseClass* const, pHouse, EDI);
// 	GET(TechnoTypeClass* const, pType, ESI);
//
// 	double result = int(pType->GetCost() * pHouse->GetHouseCostMult(pType) * pHouse->GetHouseTypeCostMult(pType));
//
// 	if(!pHouse->IsControlledByHuman())
// 		result *= RulesExtData::Instance()->AI_CostMult;
//
// 	R->EAX(result);
// 	return 0x711F46;
// }
// ASMJIT_PATCH(0x447195, BuildingClass_SellBack_Silent, 0x6)
// {
// 	GET(BuildingClass* const, pThis, ESI);
// 	return BuildingExtContainer::Instance.Find(pThis)->Silent ? 0x447203 : 0x0;
// }
// do some pre-validation evenbefore function going to be executed
// save some cpu cycle
//ASMJIT_PATCH(0x486920, CellClass_TriggerVein_Precheck, 0x6)
//{
//	return RulesClass::Instance->VeinAttack ? 0x0 : 0x486A6B;
//}
//
//ASMJIT_PATCH(0x4869AB, CellClass_TriggerVein_Weight, 0x6)
//{
//	GET(TechnoTypeClass*, pTechnoType, EAX);
//	GET(TechnoClass*, pTechno, ESI);
//
//	if (pTechno->WhatAmI() == BuildingClass::AbsID || !RulesExtData::Instance()->VeinsDamagingWeightTreshold.isset())
//		return 0x0;
//
//	if (pTechnoType->Weight < RulesExtData::Instance()->VeinsDamagingWeightTreshold)
//	{
//		return 0x486A55; //skip damaging
//	}
//
//	return 0x0;
//}
#ifdef debug_veinstest
DEFINE_JUMP(LJMP, 0x4869AB, 0x4869CA);
#endif
// ASMJIT_PATCH(0x73E9A0, UnitClass_Mi_Harvest_IncludeWeeder_1, 6)
// {
// 	GET(UnitTypeClass*, pType, EDX);
// 	R->AL(pType->Harvester || pType->Weeder);
// 	return 0x73E9A6;
// }
// 
// ASMJIT_PATCH(0x489671, MapClass_DamageArea_Veinhole, 0x6)
// {
// 	GET(CellClass*, pCell, EBX);
// 	GET(OverlayTypeClass*, pOverlay, EAX);
//
// 	if (pOverlay->IsVeinholeMonster)
// 	{
// 		GET_STACK(int, nDamage, 0x24);
// 		GET(WarheadTypeClass*, pWarhead, ESI);
// 		GET_BASE(TechnoClass*, pSource, 0x8);
// 		GET_BASE(HouseClass*, pHouse, 0x14);
// 		GET(CoordStruct*, pCenter, EDI);
//
// 		if (VeinholeMonsterClass* pMonster = VeinholeMonsterClass::GetVeinholeMonsterFrom(&pCell->MapCoords))
// 		{
// 			if (!pMonster->InLimbo && pMonster->IsAlive && ((int)pMonster->MonsterCell.DistanceFrom(pCell->MapCoords) <= 0))
// 				if (pMonster->ReceiveDamage(&nDamage,
// 					(int)pCenter->DistanceFrom(CellClass::Cell2Coord(pMonster->MonsterCell)),
// 					pWarhead,
// 					pSource,
// 					false,
// 					false,
// 					pSource && !pHouse ? pSource->Owner : pHouse
// 				) == DamageState::NowDead)
// 					Debug::LogInfo("Veinhole at [%d %d] Destroyed!", pMonster->MonsterCell.X, pMonster->MonsterCell.Y);
//
// 		}
//
// 		return 0x4896B2;
// 	}
//
// 	return 0x0;
// }
//ASMJIT_PATCH(0x6E9832, TeamClass_AI_IsThisExecuted, 0x8)
//{
//	Debug::LogInfo(__FUNCTION__" Called ");
//
//	return 0x0;
//}

// ASMJIT_PATCH(0x4686FA, BulletClass_Unlimbo_MissingTargetPointer, 0x6)
// {
// 	GET(BulletClass*, pThis, EBX);
// 	GET_BASE(CoordStruct*, pUnlimboCoords, 0x8);
//
// 	if (!pThis->Target)
// 	{
// 		Debug::LogInfo("Bullet [%s - %x] Missing Target Pointer when Unlimbo! , Fallback To CreationCoord to Prevent Crash",
// 			pThis->get_ID(), pThis);
//
// 		pThis->Target = MapClass::Instance->GetCellAt(pUnlimboCoords);
// 		R->EAX(pUnlimboCoords);
// 		return 0x46870A;
// 	}
//
// 	return 0x0;
// }
//ASMJIT_PATCH(0x456376 , BuildingClass_RemoveSpacingAroundArea, 0x6)
//{
//	GET(BuildingTypeClass*, pThisType, EAX);
   //GET(BuildingClass*, pThis, ESI);
//
	//if (!pThisType->UndeploysInto || (!pThisType->ResourceGatherer && !pThis->IsStrange()))
 //		return 0x456398;
//
 //	return pThisType->Adjacent == 0 ? 0x4563A1 : 0x45638A;
 //}

// ASMJIT_PATCH(0x518607, InfantryClass_TakeDamage_FixOnDestroyedSource, 0xA)
// {
// 	GET(InfantryClass*, pThis, ESI);
// 	GET_STACK(TechnoClass*, pSource, 0xD0 + 0x10);
// 	R->AL(pThis->Crash(pSource));
// 	return 0x518611;
// }
//Patches TechnoClass::Kill_Cargo/KillPassengers (push ESI -> push EBP)
//Fixes recursive passenger kills not being accredited
//to proper techno but to their transports
//ASMJIT_PATCH(0x707CF2, TechnoClass_KillCargo_FixKiller, 0x8)
//{
//	GET(TechnoClass*, pKiller, EBP);
//	GET(TechnoClass*, pCargo, ESI);
//
//	pCargo->KillCargo(pKiller);
//	return 0x707CFA;
//}

//bool ColorInitEd = false;
//ColorScheme* MainColor = nullptr;
//ColorScheme* BackColor = nullptr;
//
//void InitColorDraw()
//{
//	if (!ColorInitEd)
//	{
//		MainColor = ColorScheme::Find("Gold");
//		BackColor = ColorScheme::Find("NeonGreen");
//		ColorInitEd = true;
//	}
//}
//
//bool Draw_Debug_Test()
//{
//	const TextPrintType style = TextPrintType::FullShadow | TextPrintType::Point6Grad;
//	RectangleStruct rect = DSurface::ViewBounds();
//
//	// top left of tactical display.
//	Point2D screen = rect.Top_Left();
//
//	const auto buffer = std::format(L"RulesClass ptr {}", (uintptr_t)RulesClass::Instance());
//	DSurface::Temp()->DrawColorSchemeText(
//		buffer.c_str(),
//		rect,
//		screen,
//		MainColor,
//		BackColor,
//		style);
//
//	return true;
//}
//
//void Debug_Draw_Facings()
//{
//	const auto& objArr = ObjectClass::CurrentObjects;
//
//	if (objArr->Count != 1) {
//		return;
//	}
//
//	const auto pTechno = generic_cast<TechnoClass*>(objArr->Items[0]);
//	if (!pTechno)
//		return;
//
//	RectangleStruct rect = DSurface::ViewBounds();
//	const auto pType = pTechno->GetTechnoType();
//
//	CoordStruct lept {};
//	pType->Dimension2(&lept);
//	Point3D lept_center = Point3D(lept.X / 2, lept.Y / 2, lept.Z / 2);
//
//	Point3D pix {};
//	pType->PixelDimensions(&pix);
//	Point3D pixel_center = Point3D(pix.X / 2, pix.Y / 2, pix.Z / 2);
//
//	Coordinate coord = pTechno->GetCoords();
//
//	Point2D screen {};
//	//func_60F150 tspp
//	TacticalClass::Instance->CoordsToClient(coord, &screen);
//
//	screen.X -= TacticalClass::Instance->TacticalPos.X;
//	screen.Y -= TacticalClass::Instance->TacticalPos.Y;
//
//	screen.X += rect.X;
//	screen.Y += rect.Y;
//
//	DSurface::Temp->Fill_Rect(rect, RectangleStruct(screen.X, screen.Y, 2, 2), DSurface::RGB_To_Pixel(255, 0, 0));
//
//	TextPrintType style = TextPrintType::Center | TextPrintType::FullShadow | TextPrintType::Point6Grad;
//	const auto font = BitFont::BitFontPtr(style);
//
//	screen.Y -= font->GetHeight() / 2;
//
//	const auto buffer1 = std::format(L"{}" , (int)pTechno->PrimaryFacing.Current().GetDir());
//	const auto buffer2 = std::format(L"{}", (int)pTechno->PrimaryFacing.Current().Raw);
//
//	DSurface::Temp()->DrawColorSchemeText(
//		buffer1.c_str(),
//		rect,
//		screen,
//		MainColor,
//		BackColor,
//		style);
//
//	screen.Y += 10;
//	DSurface::Temp()->DrawColorSchemeText(
//		buffer2.c_str(),
//		rect,
//		screen,
//		MainColor,
//		BackColor,
//		style);
//}
#ifdef _Enable
static void __fastcall LaserDrawclassDrawAll()
{
	LaserDrawClass::DrawAll();
	EBolt::DrawAll();
	ElectricBoltManager::Draw_All();
}
DEFINE_FUNCTION_JUMP(CALL, 0x6D4669, LaserDrawclassDrawAll))
#endif

//ASMJIT_PATCH(0x6D4669, TacticalClass_Render_Addition, 0x5)
//{
//	LaserDrawClass::DrawAll();
//
//	//InitColorDraw();
//
//	EBolt::DrawAll();
//	ElectricBoltManager::Draw_All();
//	return 0x6D4673;
//}
// ASMJIT_PATCH(0x4D54DD, FootClass_Mi_Hunt_NoPath, 6)
// {
// 	GET(FootClass*, pThis, ESI);
//
// 	const auto pOwner = pThis->Owner;
// 	if (!pOwner->IsControlledByHuman()
// 		&& !(Unsorted::CurrentFrame() % 450)
// 		&& pThis->CurrentMapCoords == pThis->LastMapCoords
// 		&& !pThis->GetTechnoType()->ResourceGatherer
// 		)
// 	{
// 		pThis->SetDestination(nullptr, true);
// 		pThis->SetTarget(nullptr);
// 		pThis->TargetAndEstimateDamage(&pThis->Location, ThreatType::Range);
// 	}
//
// 	return 0x0;
// }

//ASMJIT_PATCH(0x4CD747, FlyLocomotionClass_UpdateMoving_OutOfMap, 6)
//{
//	GET(DisplayClass*, pDisplay, ECX);
//	GET(FlyLocomotionClass*, pLoco, ESI);
//	GET_STACK(int, height, 0x58);
//	GET(CellStruct*, pCell, EAX);
//
//	if(pDisplay->CoordinatesLegal(pCell))
//		return 0x4CD751;
//
//	pDisplay->RemoveObject(pLoco->Owner);
//	pLoco->Owner->SetHeight(height);
//	pDisplay->SubmitObject(pLoco->Owner);
//
//	return 0x4CD797;
//}
// ASMJIT_PATCH(0x6E23AD, TActionClass_DoExplosionAt_InvalidCell, 0x8)
// {
// 	GET(CellStruct*, pLoc, EAX);
//
// 	//prevent crash
// 	return !pLoc->IsValid() ? 0x6E2510 : 0x0;
// }
// ASMJIT_PATCH(0x73B0B0, UnitClass_DrawIfVisible, 0xA)
// {
// 	GET(UnitClass*, pThis, ECX);
// 	GET_STACK(RectangleStruct*, pBounds, 0x4);
// 	GET_STACK(bool, ignorecloaked, 0x8);
//
// 	bool result = !pThis->IsTethered;
// 	if (TechnoClass* pContact = pThis->GetNthLink())
// 	{
// 		result |= pContact->WhatAmI() != AbstractType::Building;
// 		result |= pContact->GetCurrentMission() != Mission::Unload && pContact->QueuedMission != Mission::Unload;
// 		result |= !pContact->UnloadTimer.IsOpening()
// 			&& !pContact->UnloadTimer.IsClosing()
// 			&& !pContact->UnloadTimer.IsOpen()
// 			&& !pContact->UnloadTimer.IsClosed();
// 	}
//
// 	result &= pThis->ObjectClass::DrawIfVisible(pBounds, ignorecloaked, 0);
//
// 	R->EAX(result);
//
// 	return 0x73B139;
// }

//ASMJIT_PATCH(0x4824EF, CellClass_CollecCreate_FlyingStrings, 0x8)
//{
//	GET(CellClass*, pThis, ESI);
//	GET(int, amount, EDI);
//	GET_BASE(FootClass*, pPicker, 0x8);
//	CoordStruct loc = CellClass::Cell2Coord(pThis->MapCoords);
//	loc.Z = pThis->GetFloorHeight({ 128 , 128 });
//
//
//	R->Stack(0x84, loc);
//	R->EAX(RulesClass::Instance());
//	return 0x482551;
//}

//#ifndef REMOVE_SOURCE_REUqUIREMENT_FROM_FEAR_CHECK
//DEFINE_JUMP(LJMP, 0x518C45, 0x518C49);
//#endif
//ASMJIT_PATCH(0x44A332, BuildingClass_MI_Deconstruct_ReasonToSpawnCrews, 0x7)
//{
//	GET(BuildingClass*, pThis, EBP);
//
//	if (pThis && IS_SAME_STR_(pThis->get_ID(), "DBEHIM")) {
//		Debug::FatalError("DBEHIM has Undeploys to but still endup here , WTF! HasNoFocuse %s ", pThis->ArchiveTarget ? "Yes" : "No");
//	}
//
//	return 0x0;
//}
//ASMJIT_PATCH(0x449C30, BuildingClass_MI_Deconstruct_FatalIt, 0x6)
//{
//	GET(BuildingClass*, pThis, ECX);
//
//	if (pThis && IS_SAME_STR_(pThis->get_ID(), "DBEHIM"))
//		Debug::LogInfo(__FUNCTION__"Caller [%x]", R->Stack<DWORD>(0x0));
//
//	return 0x0;
//}
// Enable This when needed
#ifdef DEBUG_STUPID_HUMAN_CHECKS

ASMJIT_PATCH(0x50B730, HouseClass_IsControlledByHuman_LogCaller, 0x5)
{
	Debug::LogInfo(__FUNCTION__"Caller [%x]", R->Stack<DWORD>(0x0));
	return 0x0;
}

ASMJIT_PATCH(0x50B6F0, HouseClass_ControlledByCurrentPlayer_LogCaller, 0x5)
{
	Debug::LogInfo(__FUNCTION__"Caller [%x]", R->Stack<DWORD>(0x0));
	return 0x0;
}
#endif

//ASMJIT_PATCH(0x448260, Debug_ChangeOwnership_Building, 0x8)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::LogInfo("%s ChangeOwnership For[%s] Caller[%x]", __FUNCTION__, pThis->get_ID() , caller);
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x4DBED0 , Debug_ChangeOwnership_Foot , 0x5)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::LogInfo("%s ChangeOwnership For[%s] Caller[%x]", __FUNCTION__, pThis->get_ID(), caller);
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x7463A0, Debug_ChangeOwnership_Unit, 0x5)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::LogInfo("%s ChangeOwnership For[%s] Caller[%x]", __FUNCTION__, pThis->get_ID(), caller);
//	return 0x0;
//}
// ASMJIT_PATCH(0x5657A0, MapClass_OpBracket_CellStructPtr, 0x5)
// {
// 	GET_STACK(CellStruct*, pCell, 0x4);
// 	GET_STACK(DWORD, callr, 0x0);
//
// 	if (!pCell) {
// 		Debug::FatalErrorAndExit("addr [0x%x] calling MapClass_OpBracket_CellStruct with nullptr cell!", callr);
// 	}
//
// 	return 0x0;
// }
//DEFINE_PATCH(0x6443E2, 0xBA, 0x01, 0x00, 0x00, 0x00, 0x90);
//
//ASMJIT_PATCH(0x5AE610, Matrix_OPMultiply, 0x5)
//{
//	GET(Matrix3D*, pThis, ECX);
//	GET_STACK(Matrix3D*, pThat, 0x4);
//	GET_STACK(DWORD, caller, 0x0);
//
//	if (!pThis || !pThat)
//		Debug::FatalErrorAndExit(__FUNCTION__" Called from(0x%x) with Invalid args ptr!", caller);
//
//	return 0x0;
//}

//static FootClass* LastAccessThisFunc;
//
//ASMJIT_PATCH(0x42C2BF, AstarClass_FindPath_SaveArgs, 0x6)
//{
//	GET(FootClass*, pFoot, ESI);
//	LastAccessThisFunc = pFoot;
//	return 0x0;
//}

#ifdef OLD_
ASMJIT_PATCH(0x48248D, CellClass_CrateBeingCollected_MoneyRandom, 6)
{
	GET(int, nCur, EAX);

	const auto nAdd = RulesExtData::Instance()->RandomCrateMoney;

	if (nAdd > 0)
		nCur += ScenarioClass::Instance->Random.RandomFromMax(nAdd);

	R->EDI(nCur);
	return 0x4824A7;
}

ASMJIT_PATCH(0x481C6C, CellClass_CrateBeingCollected_Armor1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (TechnoExtContainer::Instance.Find(Unit)->AE_ArmorMult == 1.0) ? 0x481D52 : 0x481C86;
}

ASMJIT_PATCH(0x481CE1, CellClass_CrateBeingCollected_Speed1, 6)
{
	GET(FootClass*, Unit, EDI);
	return (TechnoExtContainer::Instance.Find(Unit)->AE_SpeedMult == 1.0) ? 0x481D52 : 0x481C86;
}

ASMJIT_PATCH(0x481D0E, CellClass_CrateBeingCollected_Firepower1, 6)
{
	GET(TechnoClass*, Unit, EDI);
	return (TechnoExtContainer::Instance.Find(Unit)->AE.FirepowerMultiplier52 : 0x481C86;
}

ASMJIT_PATCH(0x481D3D, CellClass_CrateBeingCollected_Cloak1, 6)
{
	GET(TechnoClass*, Unit, EDI);

	if (Unit->CanICloakByDefault() || TechnoExtContainer::Instance.Find(Unit)->AE.Cloakable)
	{
		return 0x481C86;
	}

	// cloaking forbidden for type
	return  (!TechnoTypeExtContainer::Instance.Find(Unit->GetTechnoType())->CloakAllowed)
		? 0x481C86 : 0x481D52;
}

//overrides on actual crate effect applications
ASMJIT_PATCH(0x48294F, CellClass_CrateBeingCollected_Cloak2, 7)
{
	GET(TechnoClass*, Unit, EDX);
	TechnoExtContainer::Instance.Find(Unit)->AE.Cloakable = true;
	AEProperties::Recalculate(Unit);
	return 0x482956;
}

ASMJIT_PATCH(0x482E57, CellClass_CrateBeingCollected_Armor2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_ArmorMultiplier, 0x20);

	if (TechnoExtContainer::Instance.Find(Unit)->AE.ArmorMultiplier == 1.0)
	{
		TechnoExtContainer::Instance.Find(Unit)->AE.ArmorMultiplier = Pow_ArmorMultiplier;
		AEProperties::Recalculate(Unit);
		R->AL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x482E89;
	}
	return 0x482E92;
}

ASMJIT_PATCH(0x48303A, CellClass_CrateBeingCollected_Speed2, 6)
{
	GET(FootClass*, Unit, EDI);
	GET_STACK(double, Pow_SpeedMultiplier, 0x20);

	// removed aircraft check
	// these originally not allow those to gain speed mult

	if (TechnoExtContainer::Instance.Find(Unit)->AE_SpeedMult == 1.0)
	{
		TechnoExtContainer::Instance.Find(Unit)->AE_SpeedMult = Pow_SpeedMultiplier;
		AEProperties::Recalculate(Unit);
		R->CL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483078;
	}
	return 0x483081;
}

ASMJIT_PATCH(0x483226, CellClass_CrateBeingCollected_Firepower2, 6)
{
	GET(TechnoClass*, Unit, ECX);
	GET_STACK(double, Pow_FirepowerMultiplier, 0x20);

	if (TechnoExtContainer::Instance.Find(Unit)->AE_FirePowerMult == 1.0)
	{
		TechnoExtContainer::Instance.Find(Unit)->AE_FirePowerMult = Pow_FirepowerMultiplier;
		AEProperties::Recalculate(Unit);
		R->AL(Unit->GetOwningHouse()->IsInPlayerControl);
		return 0x483258;
	}
	return 0x483261;
}
#endif

//ASMJIT_PATCH(0x42CC48, AStarClass_RegularFindpathError, 0x5)
//{
//	GET_STACK(CellStruct, from, 0x30 - 0x1C);
//	GET_STACK(CellStruct, to, 0x30 - 0x20);
//
//	Debug::LogInfo("Regular findpath failure: (%d,%d) -> (%d, %d)", from.X, from.Y, to.X, to.Y);
//	return 0x42CC6D;
//}

//TechnoClass_CTOR_TiberiumStorage
//DEFINE_JUMP(LJMP, 0x6F2ECE , 0x6F2ED3)
//HouseClass_CTOR_TiberiumStorages
//DEFINE_JUMP(LJMP, 0x4F58CD , 0x4F58D2)
//ASMJIT_PATCH(0x6C96B0, StorageClass_DecreaseAmount_caller, 0x7)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::LogInfo(__FUNCTION__" Caller[0x%x]");
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x6C9680, StorageClass_GetAmount_caller, 0x7)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::LogInfo(__FUNCTION__" Caller[0x%x]");
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x6C9650, StorageClass_GetTotalAmount_caller, 0xB)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::LogInfo(__FUNCTION__" Caller[0x%x]");
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x6C9690, StorageClass_IncreaseAmount_caller, 0x9)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::LogInfo(__FUNCTION__" Caller[0x%x]");
//	return 0x0;
//}
//ASMJIT_PATCH(0x6C9820, StorageClass_FirstUsedSlot_caller, 0xA)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::LogInfo(__FUNCTION__" Caller[0x%x]");
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x6C9600, StorageClass_GetTotalValue_caller, 0xA)
//{
//	GET_STACK(DWORD, caller, 0x0);
//	Debug::LogInfo(__FUNCTION__" Caller[0x%x]");
//	return 0x0;
//}
//WW skip shadow for visceroids
//DEFINE_JUMP(LJMP, 0x705FED, 0x70600C);

//ASMJIT_PATCH(0x6D4A35, TacticalClass_Render_HandleSWTextPrint, 0x6)
//{
//	GET(SuperClass*, pSuper, ECX);
//	GET(int, value, EBX);
//	GET(int, time_left, ESI);
//
//	//double percent = ((double)time_left / (double)pSuper->Type->RechargeTime)* 100;
//
//	DrawSWTimers(value++,
//		ColorScheme::Array->Items[pSuper->Owner->ColorSchemeIndex],
//		time_left / 15,
//		pSuper->Type->UIName,
//		&pSuper->BlinkTimer,
//		&pSuper->BlinkState);
//
//	return 0x6D4A71;
//}

//ASMJIT_PATCH(0x4FD500, HouseClass_ExpertAI_Add, 0x6)
//{
//	GET(HouseClass*, pThis, ECX);
//
//
//	return 0x0;
//}

//static int retvalue;
//
//void NAKED _BuildingClass_ExitObject_Add_RET()
//{
//	POP_REG(edi);
//	POP_REG(esi);
//	POP_REG(ebp);
//	SET_REG32(eax, retvalue);
//	POP_REG(ebx);
//	ADD_ESP(0x130);
//	JMP(0x4440D4);
//}

//ASMJIT_PATCH(0x444F39, BuildingClass_ExitObject_Add, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	if (pThis->Type->PowersUpBuilding[0] != '\0')  {
//		return 0x0;
//	}
//
//	retvalue = BuildingClass_Exit_Object_Custom_Position(pThis);
//	return (int)_BuildingClass_ExitObject_Add_RET;
//}

//ASMJIT_PATCH(0x530792, Game_InitSecondaryMixes_Maps, 0x5)
//{
//	if (Phobos::Otamaa::NoCD) {
//		return  0x530B76;
//	}
//
//	return 0x530792;
//}
//void NAKED NOINLINE ret___()
//{
//	POP_REG(edi);
//	POP_REG(esi);
//	POP_REG(ebp);
//	POP_REG(ebx);
//	__asm mov eax, 0;
//	__asm setnz al;
//	__asm add esp, 0x198;
//	JMP(0x531292);
//}
//
//DEFINE_JUMP(LJMP, 0x530B61, 0x530B76);
//DEFINE_FUNCTION_JUMP(LJMP, 0x530D05, GET_OFFSET(ret___));
//ASMJIT_PATCH(0x6E5380  ,TagClass_IsTags_Trigger_Validate , 0x)

// the rules data not yet instansiated
//ASMJIT_PATCH(0x688338, AssignHouse_SpecialHouse, 0x5)
//{
//	R->EAX(RulesExtData::Instance()->SpecialCountryIndex);
//	return 0x688342;
//}

// the rules data not yet instansiated
//ASMJIT_PATCH(0x6869AB, split_from_Read_Scenario_INI_Special, 0x5)
//{
//	R->EAX(HouseExtData::FindSpecial());
//	return 0x6869BC;
//}
// 
//#pragma optimize("", off )
//PhobosMap<TemporalClass*, DWORD> __iDent {};
//
//ASMJIT_PATCH(0x680CB0, TemporalClass_Save_WhyCrashes, 0x6) {
//	GET(TemporalClass*, pTemp, EAX);
//	GET_STACK(int, _ArrayCount, 0x10);
//	GET(int, _CurLoop, ESI);
//	auto pArr = TemporalClass::Array->Items;
//
//	if (!pTemp || VTable::Get(pTemp) != TemporalClass::vtable) {
//		Debug::FatalError("nullptr !");
//	}
//
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x71A4E0, TemporalClass_CTOR_Source, 0x5)
//{
//	GET(TemporalClass*, pItem, ESI);
//	GET_STACK(DWORD, caller, 0x0);
//
//	__iDent[pItem] = caller;
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x71B1B0, TemporalClass_SDDTOR, 0x8)
//ASMJIT_PATCH(0x71A5D0, TemporalClass_SDDTOR, 0x8)
//{
//	GET(TemporalClass*, pItem, ESI);
//	__iDent.erase(pItem);
//	return 0;
//}
//#pragma optimize("", on )

//ASMJIT_PATCH_AGAIN(0x42A432, AStarClass_FindPathRegular_Exit, 0x5)
//ASMJIT_PATCH(0x42A451, AStarClass_FindPathRegular_Exit, 0x5)
//{
//	PhobosGlobal::Instance()->PathfindTechno.Clear();
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x429A90, AStarClass_FindPathRegular_Entry, 0x5)
//{
//	GET_STACK(TechnoClass*, pTech, 0x6A);
//	GET_STACK(CellStruct*, from, 0x62);
//	GET_STACK(CellStruct*, to, 0x66);
//
//	if (pTech)
//		PhobosGlobal::Instance()->PathfindTechno = { pTech  ,*from , *to };
//
//	return 0x0;
//}
//ASMJIT_PATCH(0x468992, BulletClass_Unlimbo_Obstacle_ZeroVel, 0x6)
//{
//	GET(BulletClass*, pThis, EBX);
//	pThis->Velocity = {};
//	return 0x468A3F;
//}

//void UpdateStrip(StripClass* pStrip, int* pKey, Point2D* pDraw) {
//
//	bool redraw = false;
//
//	/*
//	**	Reflect the scroll desired direction/value into the scroll
//	**	logic handler. This might result in up or down scrolling.
//	*/
//	if (!pStrip->IsScrolling && pStrip->Scroller)
//	{
//		if (pStrip->BuildableCount <= SidebarClassExtension::Max_Visible())
//		{
//			pStrip->Scroller = 0;
//		}
//		else
//		{
//			/*
//			**	Top of list is moving toward lower ordered entries in the object list. It looks like
//			**	the "window" to the object list is moving up even though the actual object images are
//			**	scrolling downward.
//			*/
//			if (pStrip->Scroller < 0)
//			{
//				if (pStrip->TopRowIndex <= 0)
//				{
//					pStrip->TopRowIndex = 0;
//					pStrip->Scroller = 0;
//				}
//				else
//				{
//					pStrip->Scroller++;
//					pStrip->IsScrollingDown = false;
//					pStrip->IsScrolling = true;
//					pStrip->TopRowIndex -= 2;
//					pStrip->Slid = 0;
//				}
//
//			}
//			else
//			{
//				if (pStrip->TopRowIndex + SidebarClassExtension::Max_Visible() > pStrip->BuildableCount)
//				{
//					pStrip->Scroller = 0;
//				}
//				else
//				{
//					pStrip->Scroller--;
//					pStrip->Slid = OBJECT_HEIGHT;
//					pStrip->IsScrollingDown = true;
//					pStrip->IsScrolling = true;
//				}
//			}
//		}
//	}
//
//	/*
//	**	Scroll logic is handled here.
//	*/
//	if (pStrip->IsScrolling)
//	{
//		if (pStrip->IsScrollingDown)
//		{
//			pStrip->Slid -= SCROLL_RATE;
//			if (pStrip->Slid <= 0)
//			{
//				pStrip->IsScrolling = false;
//				pStrip->Slid = 0;
//				pStrip->TopRowIndex += 2;
//			}
//		}
//		else
//		{
//			pStrip->Slid += SCROLL_RATE;
//			if (pStrip->Slid >= OBJECT_HEIGHT)
//			{
//				pStrip->IsScrolling = false;
//				pStrip->Slid = 0;
//			}
//		}
//		redraw = true;
//	}
//
//	/*
//	**	Handle any flashing logic. Flashing occurs when the player selects an object
//	**	and provides the visual feedback of a recognized and legal selection.
//	*/
//	if (pStrip->Flasher != -1)
//	{
//		if (Graphic_Logic())
//		{
//			redraw = true;
//			if (Fetch_Stage() >= 7)
//			{
//				Set_Rate(0);
//				Set_Stage(0);
//				pStrip->Flasher = -1;
//			}
//		}
//	}
//
//	/*
//	**	If any of the logic determined that this side strip needs to be redrawn, then
//	**	set the redraw flag for this side strip.
//	*/
//	static COMPILETIMEEVAL reference<bool, 0x884B8E> tootip_something {};
//	static COMPILETIMEEVAL reference<bool, 0x884B8Fu> const tootip_Bound {};
//	static COMPILETIMEEVAL reference<bool, 0xB0B518> const SidebarBlitRequested_FullRedraw {};
//	static COMPILETIMEEVAL constant_ptr<StripClass, 0x880D2C> const Collum_begin {};
//	static COMPILETIMEEVAL reference<int, 0x884B84> const something_884B84 {};
//
//	if (redraw) {
//		tootip_something = 1;
//		Collum_begin[something_884B84].NeedsRedraw = true;
//		GScreenClass::Instance->MarkNeedsRedraw(false);
//		pStrip->NeedsRedraw = true;
//		GScreenClass::Instance->MarkNeedsRedraw(false);
//		tootip_Bound = 1;
//		SidebarBlitRequested_FullRedraw = true;
//	}
//}

//static std::vector<bool> ShakeScreenTibsunStyle {};
//ASMJIT_PATCH(0x6F1FAF, TeamTypeClass_6F1FA0_CheckTaskforce, 0x7)
//{
//	GET(TeamTypeClass*, pTeam, ESI);
//
//	if (!pTeam->TaskForce)
//		Debug::FatalError("Team[%s] missing TaskForce Pointer !", pTeam->ID);
//
//	return 0x0;
//}
//
//ASMJIT_PATCH(0x6E5FA3, TagTypeClass_SwizzleTheID, 0x8)
//{
//	GET(char*, ID, EDI);
//	GET(TagTypeClass*, pCreated, ESI);
//
//	Debug::LogInfo("TagType[%s] Allocated as [%p]!", ID, pCreated);
//
//	return 0x6E5FB6;
//}
//
//ASMJIT_PATCH(0x6E8300, TaskForceClass_SwizzleTheID, 0x5)
//{
//	LEA_STACK(char*, ID, 0x2C - 0x18);
//	GET(TaskForceClass*, pCreated, ESI);
//
//	Debug::LogInfo("TaskForce[%s] Allocated as [%p]", ID, pCreated);
//
//	return 0x6E8315;
//}

COMPILETIMEEVAL int __fastcall charToID(char* string)
{
	char* v1 = string;
	int v2 = 0;
	while (v1)
	{
		if (!isxdigit(*v1))
		{
			break;
		}
		char v3 = *v1;
		int v4 = 16 * v2;
		++v1;
		if (v3 < '0' || v3 > '9')
		{
			v2 = v4 + toupper(v3) - '7';
		}
		else
		{
			v2 = v4 + v3 - '0';
		}
	}
	return v2;
}

//ASMJIT_PATCH(0x72593E, DetachFromAll_FixCrash, 0x5) {
//	GET(AbstractClass*, pTarget, ESI);
//	GET(bool, bRemoved, EDI);

//	auto it = std::remove_if(PointerExpiredNotification::NotifyInvalidObject->Array.begin(),
//		PointerExpiredNotification::NotifyInvalidObject->Array.end(), [pTarget , bRemoved](AbstractClass* pItem) {
//			if (!pItem) {
//				Debug::LogInfo("NotifyInvalidObject Attempt to PointerExpired nullptr pointer");
//				return true;
//			} else {
//				pItem->PointerExpired(pTarget, bRemoved);
//			}

//			return false;
//	});

//	PointerExpiredNotification::NotifyInvalidObject->Array.Reset(
//		std::distance(PointerExpiredNotification::NotifyInvalidObject->Array.begin(), it));

//	return 0x725961;
//}
// 
//ASMJIT_PATCH(0x43FE27, BuildingClass_AfterAnimAI_Check, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	if (!pThis->IsAlive)
//		return 0x440573;
//
//	return 0x0;
//}

//ASMJIT_PATCH(0x4D3920, FootClass_FindPath_Speed_Zero, 0x5)
//{
//	GET(FootClass* , pThis , ECX);
//
//	if(pThis->GetTechnoType()->Speed == 0){
//		R->EAX(false);
//		return 0x4D399C;
//	}
//
//	return 0x0;
//}

 // Fix 0x007BAEA1 crash
 //DEFINE_PATCH_TYPED(BYTE, 0x6B78EA, 0x89 , 0x45 , 0x00 ,0x90, 0x90 );


//ASMJIT_PATCH_AGAIN(0x7BBAF0, XSurface_Func_check, 0x5)
//ASMJIT_PATCH(0x7BB350, XSurface_Func_check, 0x6) {
   // GET(XSurface*, pThis, ECX);
   // GET_STACK(uintptr_t, caller, 0x0);

   // if (!pThis || VTable::Get(pThis) != XSurface::vtable){
   //	 Debug::LogInfo("XSurface Invalid caller [0x{0:x}]!!", caller);
   // }

   // return 0x0;
//}

//ASMJIT_PATCH(0x6B770D, SpawnManagerClass_AI_doSomething_crashAtRandomAddr, 0x7) {
   // GET(int, pIndex, EBX);
   // GET(SpawnManagerClass*, pThis, ESI);

   // auto& con = pThis->SpawnedNodes;

   // con[pIndex]->Unit->SetTarget(pThis->Target);
   // con[pIndex]->Unit->QueueMission(Mission::Attack, 0);

   // return 0x6B795A;
//}

//ASMJIT_PATCH(0x6B7793, SpawnManagerClass_AI_doSomething_crashAtRandomAddrB, 0x7)
//{
   // GET(SpawnManagerClass*, pThis, ESI);
   // GET(TechnoClass*, pSpawnee, EDI);
   // LEA_STACK(CellStruct*, OwnerCellBuffer, 0x24);
   // LEA_STACK(CellStruct*, SpawnerCellBuffer, 0x28);

   // pThis->Owner->GetMapCoords(OwnerCellBuffer);

   // if (!pSpawnee->IsAlive)
   //	 Debug::LogError("SpawManager[{}] Trying to use dead techno !", (void*)pThis);

   // pSpawnee->GetMapCoords(SpawnerCellBuffer);

   // R->EAX(OwnerCellBuffer);
   // R->EBP(SpawnerCellBuffer);
   // return 0x6B77B4;

//}

/*ASMJIT_PATCH(0x4F671D, HouseClass_CanAffordBase_GetBuildingEmpty, 0x5) {
	GET(HouseClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, EAX);

	if (!pBldType) {
		Debug::FatalErrorAndExit("Cannot Find Any suitable building from BuildWeapons [Count %d] for House[%x - %s]", RulesClass::Instance->BuildWeapons.Count, pThis, pThis->get_ID());
	}

	return 0x0;
}*/

//ASMJIT_PATCH(0x6D32DB, TacticalClass_RenderOverlay_missingPointer, 0x6)
//{
   // GET(TacticalClass*, pThis, EDI);

   // if (!pThis){
   //	 Debug::LogError("EDI register is nullptr ??");
   //	 R->EDI(TacticalClass::Instance());
   // }

   // return 0x0;
//}
