
//fcking heap pointer is deleted after some time
//who the fuck doing that ?
//DEFINE_HOOK(0x74D847, VeinholeMonsterClass_AI_HeapIsZero, 0x6)
//{
//	GET(VeinholeMonsterClass*, pThis, ESI);
//
//	pThis->RegisterAffectedCells();
//
//	return pThis->GrowthLogic.Heap->Count ? 0x74D84D : 0x74D877;
//}

//DEFINE_HOOK(0x4CF3CB, FlyLocomotionClass_4CEFB0 , 5)
//{
// GET(DirStruct*, pDir, EAX);
// GET(DirStruct*, pDirB, EDX);
// GET(void**, pPtr , ESI);
//
// TechnoClass* pTechno = (TechnoClass*)(*pPtr);
//
// if (pTechno->IsInAir() &&
//	(pTechno->GetTechnoType()->Spawned || pTechno->CurrentMission != Mission::Enter))
// {
//	 if (pDir)
//	 {
//		 pDir->Raw = (pTechno->TurretFacing().GetValue(), 0u);
//	 }
//	 else if (pDirB)
//	 {
//		 pDirB->Raw = (pTechno->TurretFacing().GetValue(), 0u);
//	 }
// }
//
// return 0;
//}

//  DEFINE_HOOK(0x6F3B5C, UnitClassClass_GetFLH_UnbindTurret , 6)
// {
// 	enum { retNotTechno = 0x6F3C1A , retContinue = 0x6F3B62, retSetMtx = 0x6F3C52};
// 	GET(TechnoClass*, pThis, EBX);
// 	GET_STACK(int , weaponIdx , 0xE0);
// 	LEA_STACK(Matrix3D* , pResult , 0x48);
//
//     if (pThis)
//     {
// 		Matrix3D nDummy {};
// 		std::memcpy(pResult ,&nDummy, sizeof(Matrix3D) );
//         return retSetMtx;
//
// 		return retContinue;
//     }
//
//     return retNotTechno;
// }


// better put it at 0x6F3D22 ,..
// replacing getFLH so it safe some execution time
// DEFINE_HOOK(0x6F3D2F, UnitClassClass_GetFLH_OnTarget , 5)
// {
//     GET(TechnoClass*, pTarget, EBX);
// 	R->EAX<CoordStruct*>();
//     return 0;
// }

//TODO Garrison/Ungarrison eva and sound
// NPExt Stuffs
//DEFINE_HOOK(0x6FC96E, TechnoClass_GetFireError_BurstAsRofDelay, 0x5)
//{
//	GET(TechnoClass*, pThis, ESI);
//	enum { retFireErrRearm = 0x6FC940 , retContinueCheck = 0x6FC981 };
//
//	const auto pType = pThis->GetTechnoType();
//	const auto pExt = TechnoTypeExt::ExtMap.Find(pType);
//
//	if (R->EAX<int>())
//	{
//		if (!pExt->UseROFAsBurstDelays && pThis->CurrentBurstIndex)
//		{
//			auto& nTime = pThis->DiskLaserTimer;
//
//			if (nTime.StartTime != -1)
//			{
//				const auto nSpend = nTime.CurrentTime - nTime.StartTime;
//				if (nSpend >= nTime.TimeLeft)
//					return retContinueCheck;
//
//				nTime.TimeLeft -= nSpend;
//			}
//
//			if (!nTime.TimeLeft)
//				return retContinueCheck;
//		}
//
//		return retFireErrRearm;
//	}
//
//	return retContinueCheck;
//}

//UseAlternateFLH 0x5A0
// 0x6F3C82 ....
// there is some change here , need to check for other for compatibility

// PrismSupportDelay , promotable , deglobalized !

//DEFINE_HOOK(0x423FF6, AnimClass_AI_SpawnTib_Probe, 0x6)
//{
//	Debug::Log("HEreIam ! \n");
//	return 0x0;
//}
//
//DEFINE_HOOK(0x71AAFD, TemporalClass_Update_OwnerEnterIdleMode_vtablecheck, 0x5)
//{
//	GET(TechnoClass*, pOwner,ECX);
//	GET(TemporalClass*, pTemp, ESI);
//
//	if (!Is_Techno(pOwner)){
//		pTemp->Owner = nullptr;
//		return 0x71AB08;
//	}
//
//	return 0x0;
//}
//DEFINE_HOOK(0x672458, RulesClass_ReadSides_checkAmount, 0x5)
//{
//	GET(int, keyCount, EDI);
//	Debug::Log("Processing %d Sides.\n", keyCount);
//	R->Stack(0x18, keyCount);
//	return 0x672469;
//}
//bool NOINLINE IsLaserFence(BuildingClass* pNeighbour, BuildingClass* pThis, short nFacing)
//{
//	if (!pNeighbour->Owner || !pThis->Owner || pNeighbour->Owner != pThis->Owner)
//		return false;
//
//	const auto pThisExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
//	const auto& nFence = pThisExt->LaserFencePostLinks;
//
//	if (nFence.empty() || !nFence.Contains(pNeighbour->Type))
//		return false;
//
//	const auto pThatExt = BuildingTypeExt::ExtMap.Find(pNeighbour->Type);
//
//	const auto nFacing_ = (nFacing & 3);
//
//	if (pNeighbour->Type->LaserFencePost
//		|| (pThisExt->LaserFenceWEType.Get(pThisExt->LaserFenceType) == pThatExt->LaserFenceWEType.Get(pNeighbour->Type))
//		|| nFacing_ == pThatExt->LaserFenceDirection && pThisExt->LaserFenceType == pNeighbour->Type)
//	{
//		return true;
//	}
//
//	return false;
//}

//DEFINE_HOOK(0x452F60, BuildingClass_CreateLaserPost_Construction, 5)
//{
//	enum { Failed = 0x452F7A, Succeeded = 0x452EAD };
//	GET(BuildingClass*, pThis, ESI);
//	LEA_STACK(CoordStruct*, pCoord, 0x24);
//	GET(DirType, nValue, ECX);
//
//	const bool IsLasefence = pThis->Type->LaserFence;
//
//	if (!IsLasefence)
//		pThis->QueueMission(Mission::Construction, false);
//
//	if (!pThis->Unlimbo(*pCoord, nValue))
//		return Failed;
//
//	if (!IsLasefence)
//	{
//		pThis->DiscoveredBy(pThis->Owner);
//		pThis->IsReadyToCommence = true;
//	}
//
//	return Succeeded;
//}
//
//DEFINE_SKIP_HOOK(0x452E2C, BuildingClass_CreateLaserPost_SkipTypeSearch, 5, 452E58)
//
//DEFINE_HOOK(0x452EFA, BuildingClass_CreateLaserPost_Type, 5)
//{
//	GET(BuildingClass*, pThis, EDI);
//	GET_STACK(short, nFacing, 0x34);
//
//	auto const bTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);
//
//	if (!bTypeExt->LaserFenceType)
//		return 0x45304A;
//
//	BuildingTypeClass* pDecided = nullptr;
//
//	if ((nFacing & 3) != 0) {
//		pDecided = bTypeExt->LaserFenceWEType.Get(bTypeExt->LaserFenceType);
//	}
//
//	R->EAX((BuildingClass*)pDecided->CreateObject(pThis->GetOwningHouse()));
//	return 0x452F0F;
//}
//
//DEFINE_HOOK(0x452BB0, BuildingClass_GetNearbyLaserFence, 7)
//{
//	GET(BuildingClass*, pThis, ECX);
//	GET_STACK(short, nFacing, 0x4);
//	GET_STACK(bool, bOnlyCheckPost, 0x8);
//	GET_STACK(int, nThread, 0xC);
//
//	auto const pType = pThis->Type;
//	BuildingClass* pResult = nullptr;
//
//	if (nThread < 0)
//	{
//		if (pType->LaserFencePost)
//		{
//			nThread = 1;
//		}
//
//		auto const nThreadPosed = pType->ThreatPosed >> 8;
//
//		if (nThreadPosed > 1)
//			nThread = nThreadPosed;
//	}
//
//	if (!nThread)
//	{
//		R->EAX(pResult);
//		return 0x452D37;
//	}
//
//	auto nCell = CellClass::Coord2Cell(pThis->GetCoords());
//
//	for (int i = 0; i < nThread; ++i)
//	{
//		nCell += CellSpread::AdjacentCell[nFacing & 7];
//
//		if (auto const pBuilding = MapClass::Instance->GetCellAt(nCell)->GetBuilding())
//		{
//			if (IsLaserFence(pBuilding, pThis, nFacing))
//				pResult = pBuilding;
//
//			const auto nFacing_ = (nFacing & 3);
//
//			if (!bOnlyCheckPost || !pType->LaserFencePost || (((((pThis->PrimaryFacing.Current().Raw) >> 12) + 1) >> 1) & 3) != nFacing_)
//				break;
//		}
//	}
//
//	R->EAX(pResult);
//	return 0x452D37;
//}
//
//DEFINE_HOOK(0x6D5801, TacticalClass_DrawLaserFencePlaceLink, 6)
//{
//	GET(BuildingClass*, pThat, EAX);
//	GET_STACK(short, nFacing, 0x28);
//	return IsLaserFence(pThat, TacticalClass::DisplayPendingObject(), nFacing) ? 0x6D5828 : 0x6D59A6;
//}

//DEFINE_HOOK(0x6F6D0E, TechnoClass_Unlimbo_LastStep, 7)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	auto const pExt = TechnoExt::ExtMap.Find(pThis);
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pExt->Type);
//
//	if (pThis->Owner && !pThis->Owner->RecheckTechTree && !pTypeExt->Linked_SW.empty())
//		pThis->Owner->UpdateSuperWeaponsUnavailable();
//
//	return 0x0;
//}

DWORD ReloadAircraft(REGISTERS* R)
{
	GET(TechnoClass*, pThis, ESI);

	const auto pType = pThis->GetTechnoType();
	const auto ammo = pType->Ammo;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	int realod = 1;

	if (pThis->Ammo > 0)
	{
		pThis->Ammo = pThis->Ammo + pTypeExt->ReloadAmount;
		R->EAX(true);
		return 0x6F4CCE;
	}

	if (!pThis->Ammo)
	{
		pThis->Ammo = pThis->Ammo + pTypeExt->EmptyReloadAmount.Get(pTypeExt->ReloadAmount);
		R->EAX(true);
		return 0x6F4CCE;
	}

	R->EAX(true);
	return 0x6F4CCE;
}

//DEFINE_HOOK(0x41AA87 , AircraftClass_SetDestination_BugFix, 0xA)
//{
//	GET(AircraftClass*, pThis, ECX);
//	GET(AbstractClass*, pTarget, EDI);
//	R->ESI(pThis);
//
//	if (!pThis->Type->AirportBound)
//	{
//		return pTarget ? 0x41AA91 : 0x41ADAC;
//	}
//
//	auto CheckLoco = [&]() {
//		if (!pThis->IsAttackedByLocomotor)
//		{
//			auto pLoco = pThis->Locomotor.GetInterfacePtr();
//			if (!pLoco->Is_Moving_Now())
//			{
//				auto pCell = pThis->GetCell();
//				if (auto pCBld = pCell->GetBuilding())
//				{
//					if (pCBld == pTarget)
//					{
//						if (pThis->ContainsLink(pCBld))
//						{
//							pThis->SendCommand(RadioCommand::RequestLink, pCBld);
//							R->EAX(false);
//							return 0x41AAAB;
//						}
//					}
//				}
//			}
//		}
//
//		return pTarget ? 0x41AA91 : 0x41ADAC;
//	};
//
//	if (!pTarget) {
//		return CheckLoco();
//	}
//
//	const auto whatTech = pTarget->WhatAmI();
//	if (whatTech == AbstractType::Building)
//	{
//		if (pThis->SendCommand(RadioCommand::QueryCanEnter, (BuildingClass*)pTarget) != RadioCommand::AnswerPositive)
//		{
//			R->EDI(((BuildingClass*)pTarget)->GetCell());
//			return 0x41AA91;
//		}
//
//		return CheckLoco();
//	}
//
//	if (whatTech != AbstractType::Cell) {
//		return CheckLoco();
//	}
//
//	auto pCell = (CellClass*)pTarget;
//	auto pCBldHere = pCell->GetBuilding();
//
//	if(!pCBldHere || !pCBldHere->Type->Helipad)
//		return CheckLoco();
//
//	if (pThis->SendCommand(RadioCommand::QueryCanEnter, pCBldHere) != RadioCommand::AnswerPositive)
//		return 0x41AA9C;
//
//	if (pThis->ContainsLink(pCBldHere)) {
//		pThis->SendCommand(RadioCommand::RequestLink, pCBldHere);
//	}
//
//	R->EDI(pCBldHere);
//	return 0x41AA91;
//}

//DEFINE_HOOK(0x4179F7, AircraftClass_AssumeTaskComplete_DontCrash, 0x6)
//{
//	GET(AircraftClass*, pThis, ESI);
//
//	if (pThis->Type->Spawned || pThis->Type->Carryall)
//		return 0;
//
//	pThis->SetDestination(nullptr, true);
//	return 0x417B69;
//}

//DEFINE_HOOK(0x44B1E8, BuildingClass_Mi_Attack_Facing, 7)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	if (pThis->Type->TurretAnimIsVoxel || pThis->Anims[9])
//		return 0;
//
//	R->EAX(10);
//	return 0x44B22D;
//}

//DEFINE_HOOK(446593, BuildingClass_Place_Turret, 6)
//{
//	GET(BuildingClass*, pThis, EBP);
//
//	return pThis->Type->IsAnimDelayedFire ?
//		0x4466D5 : 0x0;
//}

//DEFINE_HOOK(0x44EA37, BuildingClass_PointerExpired_Anim, 8)
//{
//	GET(AnimClass*, pAnim, EBP);
//	GET(BuildingClass*, pThis, ESI);
//	int result = 0;
//
//	if (pThis->IsAlive)
//	{
//		int nIdx = 0;
//		for (auto i = pThis->Anims; pAnim != *i; ++i)
//		{
//			if (++nIdx >= 21)
//				return 0x44EA3F;
//		}
//
//		pThis->Anims[nIdx] = 0;
//		auto const pThisType = pThis->Type;
//		switch (nIdx)
//		{
//		case 10:
//			if (pThisType->UnitRepair)
//			{
//				if (pThis->HasAnyLink()
//					&& pThis->GetMission() == Mission::Repair)
//				{
//					pThis->PlayNthAnim(BuildingAnimSlot::SpecialTwo, 0);
//					result = 0x44EA3F;
//				}
//				else
//				{
//					pThis->PlayNthAnim(BuildingAnimSlot::Idle, 0);
//					result = 0x44EA3F;
//				}
//				return result;
//			}
//
//			if (pThisType->IsAnimDelayedFire)
//			{
//				if (pAnim->__ToDelete_197)
//				{
//					pThis->PlayNthAnim(BuildingAnimSlot::Active, 0);
//					if (pThisType->Turret)
//					{
//						pThis->PlayNthAnim(BuildingAnimSlot::Turret, 0);
//						auto const nDeployDir = DirStruct(pThisType->DeployFacing << 8);
//						pThis->PrimaryFacing.Set_Desired(nDeployDir);
//
//						if (auto v9 = pThis->Anims[9])
//						{
//							auto ReverseFacing32 = *reinterpret_cast<int(*)[8]>(0x7F4890);
//							auto facing = ReverseFacing32[nDeployDir.Getvalue32()];
//
//							v9->Animation.Value = facing;
//							v9->Animation.Step = 0;
//
//							return 0x44EA3F;
//						}
//					}
//				}
//			}
//			break;
//		case 12:
//			if (!pThisType->UnitRepair || !pAnim->__ToDelete_197)
//				return 0x44EA3F;
//
//			pThis->PlayNthAnim(BuildingAnimSlot::Idle, 0);
//			result = 0x44EA3F;
//		case 15:
//			if (!pAnim->__ToDelete_197)
//				return 0x44EA3F;
//
//			pThis->PlayNthAnim(BuildingAnimSlot::SuperThree, 0);
//			return 0x44EA3F;
//		case 17:
//			if (pAnim->__ToDelete_197)
//			{
//				pThis->PlayNthAnim(BuildingAnimSlot::Super, 0);
//				if (pThisType->IsAnimDelayedFire)
//					pThis->PlayNthAnim(BuildingAnimSlot::Active, 0);
//			}
//			return 0x44EA3F;
//		default:
//			return 0x44EA3F;
//		}
//	}
//	return 0x44EA3F;
//}

//DEFINE_HOOK(0x43C630 , BuildingClass_ReceiveCommand_Helipad, 6)
//{
//	GET(TechnoClass*, pFrom, EDI);
//	GET(BuildingClass*, pThis, ESI);
//
//	const auto pFromwhat = pFrom->WhatAmI();
//	const RadioCommand answer = pFromwhat == AbstractType::Aircraft ? RadioCommand::AnswerPositive : RadioCommand::AnswerNegative;
//
//	if (pFromwhat == AbstractType::Aircraft)
//	{
//		auto pAir = static_cast<AircraftClass*>(pFrom);
//		auto pAirType = pAir->Type;
//		if (pAirType->Dock.FindItemIndex(pThis->Type) == -1)
//		{
//			R->EAX(RadioCommand::AnswerNegative);
//			return 0x43CE55;
//		}
//
//		auto nDockCount = pThis->Type->NumberOfDocks;
//		if (pThis->RadioLinks.Capacity > 0)
//		{
//			for (int i = 0; i < pThis->RadioLinks.Capacity; ++i)
//			{
//				auto pRadio = pThis->RadioLinks[i];
//				if (pRadio && pRadio != pAir)
//				{
//					if (pRadio->WhatAmI() == AbstractType::Aircraft)
//					{
//						nDockCount -= 1;
//					}
//				}
//			}
//		}
//
//		if (nDockCount < 1){
//			R->EAX(RadioCommand::AnswerNegative);
//			return 0x43CE55;
//		}
//	}
//
//	R->EAX(answer);
//	return 0x43CE55;
//}



//std::string hex(uint32_t n, uint8_t d)
//{
//	std::string s(d, '0');
//	const char arr[] = ("0123456789ABCDEF");
//	for (int i = d - 1; i >= 0; i--, n >> 4)
//		s[i] = arr[(n & 0xFF) < sizeof(arr) ? (n & 0xFF) : sizeof(arr)];
//	return s;
//}

//DEFINE_HOOK(0x4179AA, AircraftClass_EnterIdleMode_AlreadiHasDestination, 0x6)
//{
//	GET(AircraftClass*, pThis, ESI);
//	GET(BuildingClass*, pDest, EAX);
//	R->EDI(pDest);
//	return pThis->Destination == pDest ? 0x4179DD : 0x0;
//}

//DEFINE_HOOK(0x419CC1, AircraftClass_Mi_Enter_AiportBound, 0x6)
//{
//	GET(AircraftClass*, pThis, ESI);
//	GET(AbstractClass*, pNavCom, EDI);
//	GET(BuildingClass*, pCellBuilding, EAX);
//
//	if (pNavCom != pCellBuilding)
//	{
//		if (auto pNavBuilding = specific_cast<BuildingClass*>(pNavCom))
//		{
//			pThis->DockedTo = pNavBuilding;
//			pThis->SendToFirstLink(RadioCommand::NotifyUnlink);
//			pThis->SendCommand(RadioCommand::RequestLink, pNavBuilding);
//			pThis->SetDestination(pNavBuilding, true);
//			return 0x419CFF;
//		}
//
//		if (pThis->SendCommand(RadioCommand::QueryCanEnter, pCellBuilding) == RadioCommand::AnswerPositive)
//		{
//			pThis->SendToFirstLink(RadioCommand::NotifyUnlink);
//			pThis->SendToFirstLink(RadioCommand::RequestUntether);
//			pThis->SendCommand(RadioCommand::RequestLink, pCellBuilding);
//			pThis->SetDestination(pCellBuilding, true);
//		}
//	}
//
//	return 0x419D0B;
//}

//DEFINE_HOOK(0x416748, AircraftClass_AirportBound_SkipValidatingLZ, 0x5)
//{
//	GET(AircraftClass*, pThis, ESI);
//	return pThis->Type->AirportBound ? 0x41675D : 0x0;
//}



//stop EMPulseCannon building finding target altho it cant fire automatically
//DEFINE_HOOK(0x445F00, BuildingClass_GreatestThreat_EMPulseCannon, 0x6)
//{
//	GET(BuildingClass*, pThis, ECX);
//
//	if (pThis->Type->EMPulseCannon && !TechnoExt::ExtMap.Find(pThis)->LinkedSW) {
//		R->EAX(0);
//		return 0x445F6F;
//	}
//
//	return 0x0;
//}
//
//DEFINE_HOOK(0x44D584, BuildingClass_MI_Missile_ClearLinked, 0x6)
//{
//	GET(BuildingClass*, pThis, ECX);
//
//	if (TechnoExt::ExtMap.Find(pThis)->LinkedSW)
//		TechnoExt::ExtMap.Find(pThis)->LinkedSW = nullptr;
//
//	return 0x0;
//}


// DEFINE_HOOK(0x70F837, TechnoClass_GetOriginalOwner_PermaMCed, 0x6)
// {
// 	GET(TechnoClass*, pThis, ECX);
//
// 	if (pThis->MindControlledByAUnit)
// 		return 0x70F841;
//
// 	return 0x0;
// }
//DEFINE_HOOK(0x442282, BuilngClass_TakeDamage_LATIme_SourceHouseptrIsMissing, 0xA)
//{
//	GET(TechnoClass*, pSource, EBP);
//	GET(BuildingClass*, pThis, ESI);
//
//	if (!Is_Techno(pSource)) {
//		Debug::Log("Building[%s] Taking damage from unknown source [%x] ! , skipping this part", pThis->get_ID(), pSource);
//		return 0x4422C1;
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x701E0E, TechnoClass_TakeDamage_UpdateAnger_nullptrHouse, 0xA)
//{
//	GET(TechnoClass*, pSource, EAX);
//	GET(TechnoClass*, pThis, ESI);
//
//	if (!Is_Techno(pSource)) {
//		Debug::Log("Techno[%s] Taking damage from unknown source [%x] ! , skipping this part",
//			pThis->get_ID(), pSource);
//		return 0x701E71;
//	}
//
//	return 0x0;
//}


//DEFINE_HOOK(0x5FC668, OverlayTypeClass_Mark_Veinholedummies, 0x7)
//{
//	GET(CellStruct*, pPos, EBX);
//
//	++Unsorted::ScenarioInit();
//	const bool Allow = VeinholeMonsterClass::IsCellEligibleForVeinHole(pPos);
//	--Unsorted::ScenarioInit();
//
//	if (Allow)
//	{
//		auto pCell = MapClass::Instance->GetCellAt(pPos);
//
//		for (int i = 0 ; i < 8; ++i)
//		{
//			auto v11 = pCell->GetAdjacentCell((FacingType)i);
//			v11->OverlayTypeIndex = 126;
//			v11->OverlayData = 48u;
//			v11->RedrawForVeins();
//		}
//
//		//pCell->OverlayTypeIndex = 167; //VeiholeDummy -> used to place veinhole monster
//		//pCell->OverlayData = 0;
//
//		//GameCreate<VeinholeMonsterClass>(pPos);
//	}
//
//	return 0x5FD1FA;
//}

// this thing do some placement check twice
// this can be bad because the `GrowthLogic` data inside not inited properly !
//DEFINE_HOOK(0x74C688, VeinholeMonsterClass_CTOR_SkipPlacementCheck, 0x0)
//{
//	return 0x74C697;
//}

// DEFINE_HOOK(0x746B6B, UnitClass_Disguise_FullName , 7)
//{
//	GET(UnitClass*, pThis, ESI);
//
//    if (pThis->IsDisguised())
//    {
//		if(!pThis->Owner)
//			return 0x746B48;
//
//		const auto pPlayer = HouseClass::CurrentPlayer();
//		if(!pPlayer || (pThis->Owner != pPlayer))
//			return 0x746B48;
//
//		if(!pThis->Owner->IsAlliedWith_(pPlayer))
//			return 0x746B48;
//    }
//
//    return 0;
//}

// it only work when first created , when captured or change owner
// it wont change ,.. need more stuffs
//DEFINE_HOOK(0x45197B, BuildingClass_AnimLogic_SetOwner, 0x6)
//{
//	GET(AnimClass*, pAnim, EBP);
//	GET(BuildingClass*, pThis, ESI);
//
//	if (pAnim && pAnim->Owner != pThis->Owner)
//		pAnim->Owner = pThis->Owner;
//
//	return 0x0;
//}



//DEFINE_HOOK(0x4899FE, MapClass_DamageArea_DamageGroup_BeforeTechnoGetDamaged, 0x9)
//{
//	GET_BASE(WarheadTypeClass*, pWH, 0xC);
//	GET(TechnoClass*, pTarget, ESI);
//
//	const auto pType = pTarget->GetTechnoType();
//
//	if (IS_SAME_STR_("HuskWH", pWH->ID) && pTarget->OnBridge)
//	{
//		Debug::Log("[%s] Here !" , pType->ID);
//	}
//
//	return 0x0;
//}

//skip vanilla TurretOffset read
//DEFINE_SKIP_HOOK(0x715876,TechnoTypeClass_ReadINI_SkipTurretOffs, 0x6 , 71589A);
//DEFINE_JUMP(LJMP, 0x715876, 0x71589A);


bool __fastcall InfantryClass_SetOwningHouse(InfantryClass* const pThis, DWORD, HouseClass* pNewOwner, bool bAnnounce)
{
	return pThis->FootClass::SetOwningHouse(pNewOwner, bAnnounce);
}

bool __fastcall AircraftClass_SetOwningHouse(AircraftClass* const pThis, DWORD, HouseClass* pNewOwner, bool bAnnounce)
{
	return pThis->FootClass::SetOwningHouse(pNewOwner, bAnnounce);
}

//DEFINE_JUMP(LJMP, 0x7E2678, GET_OFFSET(InfantryClass_SetOwningHouse));
//DEFINE_JUMP(LJMP, 0x7EB42C, GET_OFFSET(AircraftClass_SetOwningHouse));

//DEFINE_HOOK(0x440333, BuildingClass_AI_C4TimerRanOut_ApplyDamage, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//
//	if (pExt->C4Damage.isset())
//	{
//		int nDamage = pExt->C4Damage.get();
//		const auto nResult = pThis->ReceiveDamage(&nDamage, 0, pExt->C4Warhead, pThis->C4AppliedBy, false, false, pExt->C4Owner);
//		Debug::Log("C4Damage Result [%s] ! \n", DamageState_to_srings[(int)nResult]);
//		return 0x44035E;
//	}
//
//	pExt->C4Damage.clear();
//	return 0x0;
//}

//basically this C4 thing will always one hit kill regardless,
// because of fcki g weird ass ww code desing ,..
//DEFINE_HOOK(0x442696, BuildingClass_ReceiveDamage_C4, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI)
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//
//	if (pExt->C4Damage.isset())
//	{
//		pExt->C4Damage.clear();
//		return 0x4426A7;
//	}
//
//	return 0;
//}

//DEFINE_HOOK(0x701F60, TechnoClass_TakeDamage_IsGoingToBlow, 0x6)
//{
//	Debug::Log("Exec ! \n");
//	return 0x0;
//}

//DEFINE_HOOK(0x4400C1, BuildingClass_AI_C4TimerRanOut_ApplyDamage_B, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//	if (pExt->C4Damage.isset())
//	{
//		int nDamage = pExt->C4Damage.get();
//		pThis->ReceiveDamage(&nDamage, 0, RulesClass::Instance->C4Warhead, pThis->C4AppliedBy, false, false,
//			pExt->C4Owner);
//
//		pExt->C4Damage.clear();
//		return pThis->IsAlive ? 0x4400F2 : 0x4400EA;
//	}
//
//	pExt->C4Damage.clear();
//	return 0x0;
//}

//DEFINE_HOOK(0x440327, BuildingClass_AI_C4DataClear, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI);
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//	pExt->C4Damage.clear();
//	pExt->C4Owner = nullptr;
//	return 0x0;
//}

//DEFINE_HOOK(0x457CA0, BuildingClass_ApplyIC_C4DataClear, 0x6)
//{
//	GET(BuildingClass*, pThis, ECX);
//	const auto pExt = BuildingExt::ExtMap.Find(pThis);
//	pExt->C4Damage.clear();
//	pExt->C4Owner = nullptr;
//	return 0x0;
//}


//double FC HouseClass_GetTypeCostMult(HouseClass* pThis, DWORD, TechnoTypeClass* pType)
//{
   // const double mult = !pThis->ControlledByPlayer() ? RulesExt::Global()->AI_CostMult : 1.0;
   // return pThis->GetHouseTypeCostMult(pType) * mult;
//}

//DEFINE_JUMP(CALL,0x711F12, GET_OFFSET(HouseClass_GetTypeCostMult));

//DEFINE_HOOK(0x7818D4, UnitClass_CrushCell_CrushAnim, 0x6)
//{
//	GET(FootClass*, pVictim, ESI);
//
//	if()
//}

// idk , the ext stuffs is set somewhere ?
//DEFINE_HOOK(0x7009D4 , TechnoClass_GetActionOnCell_Wall, 6)
//{
//	GET(OverlayTypeClass*, pOvl, EDX);
//	GET(WarheadTypeClass*, pWeapon, EDI);
//	int result; // eax
//
//	return !pOvl->Immune || pWeapon->Wall
//		? 0x7009F3 : 0x7009DE;
//}

//DEFINE_HOOK(70BD34 , TechnoClass_EstimateCannonCoords, 6)
//{
//	// [COLLAPSED LOCAL DECLARATIONS. PRESS KEYPAD CTRL-"+" TO EXPAND]
//
//	v1 = R;
//	v2 = R->_EBX.data;
//	v3 = v2->t.r.m.o.a.vftable->t.r.m.o.Class_Of(v2);
//	v14 = v2->Locomotion;
//	v4 = v3->JumpJet == 0;
//	v5 = *v14;
//	if (v4)
//		v6 = (v5->Is_Moving)(v14);
//	else
//		v6 = (v5->Is_Moving_Now)(v14);
//	if (v6)
//	{
//		v7 = v1->_ESI.data;
//		pStack = v1->_ESP.data;
//		Debug::Log(
//		  "\nTechnoClass_EstimateCannonCoords : Adjusted_X = %d, Adjusted_Y = %d\n",
//		  *(pStack + 0x34),
//		  *(pStack + 0x38));
//		v19 = v2->t.r.m.o.a.vftable->Get_Movement_Speed(v2);
//		v8 = ObjectClass::Distance(v7, v2);
//		v9 = &v7->vftable->t;
//		R = v8;
//		v10 = (v9->Get_Primary_Weapon(v7))->WeaponType;
//		if (v10)
//		{
//			v11 = WeaponTypeClass::Get_Speed(v10, R);
//			Debug::Log(
//			  "TechnoClass_EstimateCannonCoords : BulletSpeed = %d ,  TargetSpeed = %d , TargetDistance = %d\n",
//			  v11,
//			  v19,
//			  R);
//			v18 = R / (v11 * 0.9) * v19;
//			v16 = (*FacingClass::Current(&v2->t.PrimaryFacing, &R) - 0x3FFF) * -0.00009587672516830327;
//			v12 = (FastMath::Sin)(LODWORD(v16), HIDWORD(v16));
//			*(pStack + 0x38) -= MEMORY[0x7C5F00](v12* v18);
//			v15 = (FastMath::Cos)(LODWORD(v16), HIDWORD(v16)) * v18;
//			*(pStack + 0x34) += MEMORY[0x7C5F00](v15);
//			Debug::Log(
//			  "TechnoClass_EstimateCannonCoords : shift = %d ,  direction = %lf , Adjusted_X = %d, Adjusted_Y = %d\n",
//			  LODWORD(v18),
//			  COERCE_LONG_DOUBLE(__PAIR64__(LODWORD(v16), HIDWORD(v18))),
//			  HIDWORD(v16),
//			  *(pStack + 0x34));
//			v1->_EBP.data = *(pStack + 0x38);
//		}
//	}
//	return 0x70BE29;
//}

// UnitClass_Unload_NoManualEject , 0x0 false , 0x73DCD3 true , Typeptr Eax
//  , 0x0 false , 0x7400F0 true , Typeptr Eax

// DEFINE_HOOK(0x70A3E5, TechnoClass_DrawPipScale_Ammo_Idx, 7)
// {
// 	GET(TechnoClass* const, pThis, EBP);
// 	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
// 	R->ESP(pTypeExt->PipScaleIndex.Get(13));
// 	return 0x0;
// }

// DEFINE_HOOK(0x70A35D, TechnoClass_DrawPipScale_Ammo, 5)
// {
// 	GET(TechnoClass* const, pThis, EBP);
// 	GET_STACK(RectangleStruct*, pRect, 0x80);
// 	GET_STACK(int const, nX, 0x50);
// 	GET_STACK(int const, nY, 0x54);
//
// 	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
// 	if (pTypeExt->AmmoPip.isset())
// 	{
// 		const auto pSHApe = pTypeExt->AmmoPip;
// 		const int nFrame = int(((1.0 - pThis->Ammo) / pTypeExt->Get()->Ammo) * pSHApe->Frames);
// 		Point2D offs { nX, nY };
// 		offs += pTypeExt->AmmoPip_Offset.Get();
// 		ConvertClass* pConvert = FileSystem::PALETTE_PAL();
// 		if (const auto pConvertData = pTypeExt->AmmoPip_Palette)
// 		{
// 			pConvert = pConvertData->GetConvert<PaletteManager::Mode::Default>();
// 		}
//
// 		DSurface::Temp->DrawSHP(pConvert, pSHApe,
// 			nFrame, &offs, pRect, (BlitterFlags)0x600u, 0, 0, 1000, 0, 0, nullptr, 0, 0, 0);
//
// 		return 0x70A4EC;
// 	}
//
// 	return 0x0;
// }


//DEFINE_HOOK(0x714522, TechnoTypeClass_LoadFromINI_RequiredHouses, 9)
//{
//	GET(CCINIClass*, pCCINI, ESI);
//	GET(TechnoTypeClass*, pThis, ESI);
//	GET(const char*, pSection, EBX);
//
//	if (pCCINI->ReadString(pSection, (const char*)0x843BB4, Phobos::readDefval, Phobos::readBuffer)) {
//		if (GameStrings::IsBlank(Phobos::readBuffer)) {
//			pThis->RequiredHouses = -1;
//			return 0x71453C;
//		}
//
//		char* context = nullptr;
//
//		int i = 0;
//		for (auto cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
//			cur;
//			cur = strtok_s(nullptr, Phobos::readDelims, &context)) {
//			pThis->RequiredHouses |= HouseTypeClass::FindIndexOfNameShiftToTheRightOnce(cur);
//			Debug::Log("TechnoType[%s] Reading Owner BitField of %s \n", pSection, cur);
//			++i;
//		}
//
//		Debug::Log("TechnoType[%s] Have %d Owner Bitfield! \n", pSection , i);
//	}
//
//	return 0x71453C;
//}

//DEFINE_HOOK(0x73B02D, UnitClass_UpdatePos_CrushMovementZone, 0x6)
//{
//	GET(MovementZone, nZone, EAX);
//
//	return nZone == MovementZone::CrusherAll || nZone == MovementZone::Subterrannean ?
//		0x73B036 : 0x73B074;
//}

//DEFINE_HOOK(0x73AFE3 , UnitClass_UpdatePosition_Crush, 6)
//{
//	GET(UnitClass*, pUnit, EBP);
//	GET(CellClass*, pCell, EDI);
//
//	const auto pType = pUnit->Type;
//	const auto nMovementZone = pType->MovementZone;
//
//	if (pType->Crusher || pUnit->HasAbility(AbilityType::Crusher))
//	{
//		if (auto pTerrain = pCell->GetTerrain(false))
//		{
//			if (pTerrain->IsAlive)
//			{
//				const auto pTType = pTerrain->Type;
//				if (!pTType->SpawnsTiberium &&
//					!pTType->Immune &&
//					!TerrainTypeExt::ExtMap.Find(pTType)->IsPassable &&
//					pTType->Crushable
//					)
//				{
//					if (TechnoTypeExt::ExtMap.Find(pType)->CrushLevel.Get(pUnit) >
//						TerrainTypeExt::ExtMap.Find(pTType)->CrushableLevel)
//					{
//						VocClass::PlayAt(pType->CrushSound, pUnit->Location);
//						pTerrain->ReceiveDamage(&pTerrain->Health, 0, RulesClass::Instance->C4Warhead, pUnit, true, true, pUnit->Owner);
//
//						if (pType->TiltsWhenCrushes)
//							pUnit->RockingForwardsPerFrame += 0.02f;
//
//						pUnit->iscrusher_6B5 = false;
//					}
//				}
//			}
//		}
//
//		if (pCell->OverlayTypeIndex != -1)
//		{
//			const auto pOverlayType = OverlayTypeClass::Array->GetItem(pCell->OverlayTypeIndex);
//			if (pOverlayType->Crushable
//			  || pOverlayType->Wall
//			  && (nMovementZone == MovementZone::CrusherAll) || nMovementZone == MovementZone::Subterrannean)
//			{
//				VocClass::PlayIndexAtPos(pOverlayType->CrushSound, pUnit->Location, 0);
//				pCell->ReduceWall();
//				pUnit->iscrusher_6B5 = 0;
//				pUnit->RockingForwardsPerFrame += 0.02f;
//			}
//		}
//	}
//
//	return 0x73B074;
//}

// the unit seems dont like it
// something missing
//bool IsAllowToTurn(UnitClass* pThis, AbstractClass* pTarget, int nMax, DirStruct* pTargetDir)
//{
//	if (!pTargetDir && !pTarget)
//		return true;
//
//	auto nPriFacing = (short)pThis->PrimaryFacing.Current().Raw;
//	auto nTargetDir = pTargetDir ? (short)pTargetDir->Raw : (short)pThis->GetDirectionOverObject(pTarget).Raw;
//
//	int nFrom = (((nPriFacing >> 7) + 1) >> 1);
//	int nTo = (((nTargetDir >> 7) + 1) >> 1);
//
//	if (abs(nFrom - nTo) <= nMax)
//		return true;
//
//	short n_To_s = (nTo <= 127) ? nTo : (nTo - 256);
//	short n_From_s = (nFrom <= 127) ? nFrom : (nFrom - 256);
//
//	if (abs(n_From_s - n_To_s) > nMax)
//		return false;
//
//	return true;
//}

//DEFINE_HOOK(0x741229, UnitClass_GetFireError_Facing, 6)
//{
//	GET(WeaponTypeClass*, pWeapon, EBX);
//	GET(UnitClass*, pThis, ESI);
//	GET_STACK(AbstractClass*, pTarget, 0x20);
//
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
//
//	if (pThis->Type->DeployFire
//		&& !pThis->Type->IsSimpleDeployer
//		&& !pThis->Deployed)
//	{
//		if (!pTypeExt->DeployFire_UpdateFacing)
//		{
//			//R->EAX(FireError::OK); //yes , dont return facing error
//			//return 0x74132B;
//			return 0x741327; //fireOK
//		}
//	}
//
//	if (pWeapon->OmniFire)
//		return 0x741314;
//
//	if (!pThis->IsFiring && pThis->IsRotating && !pWeapon->Projectile->ROT)
//		return 0x74124D;
//
//	const auto pType = pThis->Type;
//
//	if (IsAllowToTurn(pThis, pTarget , 10 , nullptr))
//		return 0x74125C;
//
//	//0x7412F1 FireError::Facing
//
//	return pType->Turret ? 0x74101E : 0x74124D;
//}

//DEFINE_HOOK(0x7369F2, UnitClass_UpdateFacing_TurretLimit, 8)
//{
//	GET(UnitClass*, pThis, ESI);
//	GET_STACK(DirStruct, nDir, 0x8);
//
//	if (IsAllowToTurn(pThis, nullptr, 10, &nDir))
//		return 0x0;
//
//	if (!pThis->IsRotating)
//	{
//		pThis->IsRotating = true;
//		pThis->PrimaryFacing.Set_Desired(nDir);
//	}
//
//	pThis->SecondaryFacing.Set_Desired(nDir);
//
//	return 0x736A8E;
//}


// DEFINE_HOOK(0x73B002, UnitClass_UpdatePosition_CrusherTerrain, 0x6)
// {
// 	GET(UnitClass*, pThis, EBP);
// 	GET(CellClass* const, pCell, EDI);
//
// 	if (const auto pTerrain = pCell->GetTerrain(false))
// 	{
// 		if (pTerrain->IsAlive)
// 		{
// 			const auto pType = pTerrain->Type;
// 			if (!pType->SpawnsTiberium &&
// 				!pType->Immune &&
// 				!TerrainTypeExt::ExtMap.Find(pType)->IsPassable &&
// 				pTerrain->Type->Crushable
// 				)
// 			{
// 				if (TechnoTypeExt::ExtMap.Find(pThis->Type)->CrushLevel.Get(pThis) >
// 					TerrainTypeExt::ExtMap.Find(pType)->CrushableLevel)
// 				{
// 					VocClass::PlayIndexAtPos(pType->CrushSound, pThis->Location);
// 					pTerrain->ReceiveDamage(&pTerrain->Health, 0, RulesClass::Instance->C4Warhead, pThis, true, true, pThis->Owner);
//
// 					if (pThis->Type->TiltsWhenCrushes)
// 						pThis->RockingForwardsPerFrame += 0.02f;
//
// 					pThis->iscrusher_6B5 = false;
// 				}
// 			}
// 		}
// 	}
//
// 	R->EAX(pCell->OverlayTypeIndex);
// 	return pCell->OverlayTypeIndex != -1 ? 0x73B00A : 0x73B074;
// }

// 5B1020 , for mechLoco
// 5B1404 , for mechLoco
// 6A1025 , for shipLoco
// DEFINE_HOOK(0x4B1999, DriveLocomotionClass_4B0F20_CrusherTerrain, 0x6)
// {
// 	GET(DriveLocomotionClass*, pLoco, EBP);
// 	GET(CellClass* const, pCell, EBX);
//
// 	const auto pLinkedTo = pLoco->LinkedTo;
// 	if (const auto pTerrain = pCell->GetTerrain(false))
// 	{
// 		if (pTerrain->IsAlive)
// 		{
// 			const auto pType = pTerrain->Type;
// 			if (!pType->SpawnsTiberium &&
// 				!pType->Immune &&
// 				!TerrainTypeExt::ExtMap.Find(pType)->IsPassable &&
// 				pTerrain->Type->Crushable
// 				)
// 			{
// 				if (TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType())->CrushLevel.Get(pLinkedTo) >
// 					TerrainTypeExt::ExtMap.Find(pType)->CrushableLevel)
// 				{
// 					VocClass::PlayIndexAtPos(pType->CrushSound, pLinkedTo->Location);
// 					pTerrain->ReceiveDamage(&pTerrain->Health, 0, RulesClass::Instance->C4Warhead, pLinkedTo, true, true, pLinkedTo->Owner);
//
// 					if (pLinkedTo->GetTechnoType()->TiltsWhenCrushes)
// 						pLinkedTo->RockingForwardsPerFrame = -0.05f;
// 				}
// 			}
// 		}
// 	}
//
// 	R->EAX(pCell->OverlayTypeIndex);
// 	return pCell->OverlayTypeIndex != -1 ? 0x4B19A1 : 0x4B1A04;
// }

// this shit is something fuckup check
// it check if not unit then check if itself is not infantry is building
// what event shit is this
//DEFINE_HOOK(0x6FA697, TechnoClass_UpdateTarget_ShouldReTarget , 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (!Is_Unit(pThis))
//		return 0x0;
//
//	bool bConditionMet = false;
//	switch (pThis->CurrentMission)
//	{
//	case Mission::Move:
//	case Mission::Guard:
//	case Mission::Harvest:
//	case Mission::Return:
//		return 0x6FA6AC;
//	case Mission::Sabotage:
//	{
//		if (!Is_Infantry(pThis))
//			return 0x6FA6AC;
//
//		break;
//	}
//	case Mission::Missile:
//		bConditionMet = !Is_Building(pThis);
//		break;
//	}
//
//	return bConditionMet ? 0x6FA6AC : 0x6FA6F5;
//}

//void PlayReloadEffects(TechnoClass* pThis, int report, int sound)
//{
//	VocClass::PlayIndexAtPos(report, pThis->Location);
//	VocClass::PlayIndexAtPos(sound, pThis->Location);
//}

//bool CheckDone = false;
//
//DEFINE_HOOK(0x55B582, LogicClass_Update_AfterTeamClass, 0x6)
//{
//	// Uninited techno still playing `EMPulseSparkle` anim ,..
//
//	//if (!CheckDone)
//	//{
//	//	if (SessionClass::Instance->GameMode != GameMode::Campaign && SessionClass::Instance->GameMode != GameMode::Skirmish)
//	//	{
//	//		HouseExt::ExtMap.Find(HouseClass::CurrentPlayer)->Seed = Random2Class::Seed();
//	//
//	//		Debug::Log("Scenario Name [%s] , Map Name [%s] \n", ScenarioClass::Instance->FileName, SessionClass::Instance->ScenarioFilename);
//	//		for (auto const& it : *HouseClass::Array)
//	//		{
//	//			auto pExt = HouseExt::ExtMap.TryFind(it);
//	//
//	//			Debug::Log("Player Name: %s IsCurrentPlayer: %u; ColorScheme: %s; ID: %d; HouseType: %s; Edge: %d; StartingAllies: %u; Startspot: %d,%d; Visionary: %d; MapIsClear: %u; Money: %d Seed: %d\n",
//	//			it->PlainName ? it->PlainName : NONE_STR,
//	//			it->IsHumanPlayer,
//	//			ColorScheme::Array->GetItem(it->ColorSchemeIndex)->ID,
//	//			it->ArrayIndex,
//	//			HouseTypeClass::Array->GetItem(it->Type->ArrayIndex)->Name,
//	//			it->Edge,
//	//			it->StartingAllies.data,
//	//			it->StartingCell.X,
//	//			it->StartingCell.Y,
//	//			it->Visionary,
//	//			it->MapIsClear,
//	//			it->Available_Money(),
//	//			pExt->Seed
//	//			);
//	//
//	//			if (it != HouseClass::CurrentPlayer && pExt->Seed != -1 && pExt->Seed != Random2Class::Seed())
//	//			{
//	//				Debug::FatalError("Player %s with currentPlayer , have different random2 seeds ! \n", it->PlainName, HouseClass::CurrentPlayer->PlainName);
//	//			}
//	//		}
//	//
//	//		CheckDone = true;
//	//	}
//	//}
//
//	for (auto pAnim : *AnimClass::Array)
//	{
//		if (pAnim->IsAlive)
//		{
//			if (pAnim->OwnerObject && !pAnim->OwnerObject->IsAlive)
//				pAnim->TimeToDie = true;
//			else if (pAnim->InLimbo)
//				pAnim->TimeToDie = true;
//		}
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x518077, InfantryClass_ReceiveDamage_ResultDestroyed, 0x6)
//{
//	return 0x0;
//}

//#include <SpotlightClass.h>
//
//struct TechnoTypeExt_Gscript
//{
//	Valueable<bool> TroopCrawler {};
//	Promotable<bool> Promoted_PalaySpotlight { };
//	Promotable<SpotlightFlags> Promoted_PalaySpotlight_bit
//	{ SpotlightFlags::NoGreen | SpotlightFlags::NoRed,
//		SpotlightFlags::NoRed,
//		SpotlightFlags::NoGreen | SpotlightFlags::NoBlue
//	};
//
//	Promotable<AnimTypeClass*> Promoted_PlayAnim {};
//
//	ValueableIdx<VoxClass> DiscoverEVA { -1 };
//
//	static TechnoTypeExt_Gscript* Get(TechnoTypeClass* pThis)
//	{
//		return (TechnoTypeExt_Gscript*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset));
//	}
//
//	static void PlayPromoteAffects(TechnoClass* pThis)
//	{
//		auto const pTypeExt = pThis->GetTechnoType();
//		auto const pExt = TechnoTypeExt_Gscript::Get(pTypeExt);
//
//		if (pExt->Promoted_PalaySpotlight.Get(pThis))
//		{
//			if (auto pSpot = GameCreate<SpotlightClass>(pThis->Location, 50))
//			{
//				pSpot->DisableFlags = pExt->Promoted_PalaySpotlight_bit.Get(pThis);
//			}
//		}
//
//
//		if (auto pAnimType = pExt->Promoted_PlayAnim.Get(pThis))
//		{
//			GameCreate<AnimClass>(pAnimType, pThis->Location, 0, 1, 0x600u, 0, 0);
//		}
//	}
//};

//DEFINE_HOOK_AGAIN(0x736A22,UnitClass_UpdateTurret_ApplyTarget_ClearFlags, 0xA)
//DEFINE_HOOK(0x736A09, UnitClass_UpdateTurret_ApplyTarget_ClearFlags, 0x5)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	auto const pTechnoExt = TechnoExt_Gscript::Get(pThis);
//	pTechnoExt->TargetingDelayTimer.Start(ScenarioClass::Instance->Random.RandomRanged(30, 50));
//
//	return 0x0;
//}

//DEFINE_HOOK(0x7394C4, UnitClass_TryToDeploy_EmptyToPlace_Crawler, 0x7)
//{
//	GET(UnitTypeClass*, pType, EAX);
//
//	return TechnoTypeExt_Gscript::Get(pType)->TroopCrawler.Get() ?
//		0x73958A : 0x0;
//}

//DEFINE_HOOK(0x73F015, UnitClass_Mi_Hunt_MCVFindSpotReworked, 0x6)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	const auto nMissionStatus = pThis->MissionStatus;
//	if (!nMissionStatus)
//	{
//		if (!pThis->GetHeight())
//		{
//			if (pThis->GotoClearSpot() && pThis->TryToDeploy())
//			{
//				pThis->MissionStatus = 1;
//				return 0x73F059;
//			}
//
//			pThis->Scatter(CoordStruct::Empty, true, false);
//		}
//
//		return 0x73F059;
//	}
//
//	if (nMissionStatus != 1)
//		return 0x73F059;
//
//	if (!pThis->Deploying)
//		pThis->MissionStatus = 0;
//
//	return 0x73F059;
//}

//DEFINE_HOOK(0x6F4974, TechnoClass_UpdateDiscovered_ByPlayer, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(HouseClass*, pDiscoverer, EDI);
//
//	const auto pHouseExt = HouseExt::ExtMap.Find(pDiscoverer);
//
//	if (!pHouseExt->DiscoverEvaDelay.IsTicking() || !pHouseExt->DiscoverEvaDelay.GetTimeLeft()){
//		pHouseExt->DiscoverEvaDelay.Start(200);
//
//		if (auto pTypeExt = TechnoTypeExt_Gscript::Get(pThis->GetTechnoType())) {
//			const auto nIdx = pTypeExt->DiscoverEVA.Get();
//
//			if (nIdx != -1)
//				VoxClass::PlayIndex(nIdx);
//		}
//	}
//
//	return 0x0;
//}

// DEFINE_HOOK(0x708F5E, TechnoClass_ResponseToSelect_PlaySound, 0xA)
// {
// 	GET(TechnoClass*, pThis, ESI);
//
// 	return 0x0;
// }

//struct TechnoExt_Gscript
//{
//	CDTimerClass TargetingDelayTimer {};
//
//	static TechnoExt_Gscript* Get(TechnoClass* pThis)
//	{
//		return (TechnoExt_Gscript*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset));
//	}
//};


//#pragma optimize("", off )
//DEFINE_HOOK(0x722FC2, TiberiumClass_Grow_Validate, 0x5)
//{
//	GET(const TPriorityQueueClass<MapSurfaceData>*, pHeap, ECX);
//	Debug::Log("__FUNCTION__ , Tiberium Logic with HeapSize(%d) \n" , pHeap->HeapSize);
//	return 0x0;
//}
//#pragma optimize("", on)


//DEFINE_HOOK(0x5D4E3B, DispatchingMessage_ReloadResources, 0x5)
//{
//	LEA_STACK(tagMSG*, pMsg, 0x10);
//	GET_STACK(DWORD, nDW, 0x14);
//
//	if ((nDW == 0x10 || nDW == 0x2 || nDW == 0x112) && pMsg->wParam == (WPARAM)0xF060)
//		ExitProcess(1u);
//
//	//if ((nDW == 0x104 || nDW == 0x100)
//	//	&& pMsg.wParam == (WPARAM)0xD && ((pMsg.lParam & 0x20000000) != 0))
//	//{
//	//	set critical section here ?
//	//}
//
//	Imports::TranslateMessage.get()(pMsg);
//	Imports::DispatchMessageA.get()(pMsg);
//
//	return 0x5D4E4D;
//}

//DEFINE_HOOK(0x6AB64F, SidebarClass_ClickedAction_Focus, 0x6)
//{
//	GET(TechnoTypeClass*, pItem, EAX);
//
//	const HouseClass* pHouse = HouseClass::CurrentPlayer;
//	if (!pHouse || !pItem)
//		return 0x0;
//
//	const CanBuildResult canBuild = pHouse->CanBuild(pItem, true, false);
//
//	if (canBuild == CanBuildResult::TemporarilyUnbuildable) {
//		for (auto pTechno : *TechnoClass::Array) {
//
//			if (!pTechno->IsAlive || pTechno->IsCrashing || pTechno->IsSinking)
//				continue;
//
//			if (pTechno->Owner == pHouse && pTechno->GetTechnoType() == pItem)
//			{
//				CoordStruct coords = pTechno->GetCoords();
//
//				if (!coords)
//					continue;
//
//				TacticalClass::Instance->SetTacticalPosition(&coords);
//				pTechno->Flash(60);
//				if (pItem->VoiceSelect.Items)
//					pTechno->QueueVoice(pItem->VoiceSelect[0]);
//
//				MapClass::Instance->MarkNeedsRedraw(1);
//				break;
//			}
//		}
//	}
//
//	return 0;
//}

void ObjectClass_ReceiveDamage_NPEXT_EMPulseSparkles(ObjectClass* pTarget)
{
	int nLoopCount = ScenarioClass::Instance->Random.RandomFromMax(25);
	if (auto pSparkel = GameCreate<AnimClass>(RulesClass::Instance->EMPulseSparkles, pTarget->GetCoords(), 0, nLoopCount))
		pSparkel->SetOwnerObject(pTarget);
}

enum class KickOutProductionType : int
{
	Normal = 0,
	DropPod,
	Paradrop,
	Anim,
};

enum class FunctionreturnType : int
{
	Succeeded = 0,
	Failed,
	Nothing,
};

FunctionreturnType KickoutTechnoType(BuildingClass* pProduction, KickOutProductionType nDecided)
{
	bool UnlimboSucceeded = false;
	switch (nDecided)
	{
	case KickOutProductionType::DropPod:
	{
		return UnlimboSucceeded ? FunctionreturnType::Succeeded : FunctionreturnType::Failed;
	}
	case KickOutProductionType::Paradrop:
	{
		return UnlimboSucceeded ? FunctionreturnType::Succeeded : FunctionreturnType::Failed;
	}
	case KickOutProductionType::Anim:
	{
		return UnlimboSucceeded ? FunctionreturnType::Succeeded : FunctionreturnType::Failed;
	}
	}

	return FunctionreturnType::Nothing;
}
//
//DEFINE_HOOK(0x70F8D8, TechnoClass_GoBerzerkFor_SetMission, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	const Mission nMission = !pThis->IsArmed() ? Mission::Sleep : Mission::Hunt;
//	pThis->QueueMission(nMission, false);
//
//	return 0x70F8E6;
//}

// tester says these make team completely stop protecting ToProtect
// all of them instead of the skipped one , weird
// Infantry
// continue 0x708239 , skip 0x7083BC
//DEFINE_HOOK(0x70822B, TechnoClass_ToProtectAttacked_Ignore_Infantry, 0x6)
//{
//	GET(InfantryClass*, pInf, ESI);
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pInf->Type);
//	return pTypeExt->IgnoreToProtect || pTypeExt->IsDummy
//		? 0x7083BC : 0x0;
//}

// Unit 0x7086F5
// recuit chance
// 0x708461
// continue 0x708461 , skip 0x708622
//DEFINE_HOOK(0x708461, TechnoClass_ToProtectAttacked_Ignore_Unit, 0x6)
//{
//	GET(UnitClass*, pUnit, ESI);
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pUnit->Type);
//	return pTypeExt->IgnoreToProtect || pTypeExt->IsDummy
//		? 0x708622 : 0x0;
//}

//DEFINE_HOOK(0x4445FB, BuildingClass_KickOut_FactoryType_NotAWeaponFactory, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//	switch(KickoutTechnoType(pThis, KickOutProductionType::Normal))
//	{
//	case FunctionreturnType::Nothing:
//		return 0x0; // nothing
//	case FunctionreturnType::Succeeded:
//		return 0x4448CE; // set mission
//	case FunctionreturnType::Failed :
//		return 0x444EDE; // decrease mutex
//	}
//}


//DEFINE_HOOK(0x51BCA4, InfantryClass_AI_ReloadInTransporterFix, 0x6)
//{
//	enum { RetFunct = 0x51BF80, CheckLayer = 0x51BDCF, CheckMission = 0x51BCC0 };
//
//	GET(InfantryClass*, pThis, ESI);
//
//	if (!pThis->IsAlive)
//		return RetFunct;
//
//	if (!pThis->InLimbo || pThis->Transporter)
//		pThis->Reload();
//
//	if (pThis->InLimbo)
//		return CheckLayer;
//
//	return CheckMission;
//}


// this using enum , not boolean afaik
//  so far i know
//	-1 None
//	2 FootMethod
//  1 BuildingMethod
//	0 prohibited
//TODO : Add PerTechnoOverride
//DEFINE_JUMP(LJMP, 0x639DE5, 0x639DF9);
//DEFINE_HOOK(0x639DD8, TechnoClass_PlanningManager_DecideEligible, 0x5)
//{
//	enum { CanUse =   , ContinueCheck = 0x639E03 };
//	GET(TechnoClass* const, pThis, ESI);
//	return ((pThis->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
//		? CanUse : ContinueCheck;
//}

// DEFINE_HOOK(0x518B98, InfantryClass_ReceiveDamage_UnInit, 0x8)
// {
// 	GET(InfantryClass*, pThis, ESI);
// 	// REF_STACK(args_ReceiveDamage const, args, STACK_OFFS(0xD0, -0x4));
//
// 	// if (!InfantryExt::ExtMap.Find(pThis)->IsUsingDeathSequence && !pThis->Type->JumpJet) {
// 	// 	auto pWHExt = WarheadTypeExt::ExtMap.Find(args.WH);
// 	// 	if (!pWHExt->DeadBodies.empty()) {
// 	// 		if (AnimTypeClass* pSelected = pWHExt->DeadBodies.at(
// 	// 			ScenarioClass::Instance->Random.RandomFromMax(pWHExt->DeadBodies.size() - 1)))
// 	// 		{
// 	// 			if (const auto pAnim = GameCreate<AnimClass>(pSelected, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0)) {
// 	// 				AnimExt::SetAnimOwnerHouseKind(pAnim, args.Attacker ? args.Attacker->GetOwningHouse() : args.SourceHouse, pThis->GetOwningHouse(), true);
// 	// 			}
// 	// 		}
// 	// 	}
// 	// }
//
// 	R->ECX(pThis);
// 	pThis->UnInit();
// 	return 0x518BA0;
// }

//DEFINE_HOOK(0x4775F4, CCINIClass_ReadVHPScan_new, 0x5)
//{
//	GET(const char* const, cur, ESI);
//
//	int vHp = 0;
//	for (int i = 0; i < (int)NewVHPScanToString.size(); ++i)
//	{
//		if (IS_SAME_STR_(cur, NewVHPScanToString[i]))
//		{
//			vHp = i;
//			break;
//		}
//	}
//
//	R->EAX(vHp);
//	return 0x4775E9;
//}
//
//DEFINE_HOOK(0x4775B0, CCINIClass_ReadVHPScan_ReplaceArray, 0x7)
//{
//	int nIdx = R->EDI<int>();
//	nIdx = (nIdx > (int)NewVHPScanToString.size() ? (int)NewVHPScanToString.size() : nIdx);
//	R->EDX(NewVHPScanToString[nIdx]);
//	return 0x4775B7;
//}

//DEFINE_HOOK(0x4870D0, CellClass_SensedByHouses_ObserverAlwaysSensed, 0x6)
//{
//	GET_STACK(int, nHouseIdx, 0x4);
//
//	const auto pHouse = HouseClass::Array->GetItemOrDefault(nHouseIdx);
//	if (HouseExt::IsObserverPlayer(pHouse))
//	{
//		R->AL(1);
//		return 0x4870DE;
//	}
//
//	return 0;
//}

//DEFINE_HOOK(0x70DA6D, TechnoClass_SensorAI_ObserverSkipWarn, 0x6)
//{
//	return HouseExt::IsObserverPlayer() ? 0x70DADC : 0x0;
//}

//DEFINE_HOOK(0x6FF1FB, TechnoClass_FireAt_Shield, 0x6)
//{
//	GET_BASE(AbstractClass*, pTarget, 0x8);
//	GET_STACK(CoordStruct, nCoord, 0x44);
//
//	if (auto const pTechno = abstract_cast<TechnoClass*>(pTarget))
//	{
//		auto const pExt = TechnoExt::ExtMap.Find(pTechno);
//		if (auto const pShield = pExt->GetShield())
//		{
//			if (!pShield->IsActive())
//				return 0x0;
//
//			if(auto pHitAnim = pShield->Anim)
//		}
//	}
//	return 0x0;
//}



//DEFINE_JUMP(LJMP, 0x517FF5, 0x518016);

static int AnimClass_Expired_SpawnsParticle(REGISTERS* R)
{
	GET(AnimClass*, pThis, ESI);
	GET(AnimTypeClass* const, pAnimType, EAX);
	GET(int, nNumParticles, ECX);

	const auto pType = ParticleTypeClass::Array->Items[pAnimType->SpawnsParticle];
	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pAnimType);
	//should be lepton ?
	const auto nMin = static_cast<int>(pTypeExt->ParticleRangeMin);
	const auto nMax = static_cast<int>(pTypeExt->ParticleRangeMax);

	if (nMin || nMax)
	{
		const auto nCoord = pThis->GetCoords();
		const auto v8 = nCoord.Z - MapClass::Instance->GetCellFloorHeight(nCoord);
		const auto v17 = 6.283185307179586 / nNumParticles;
		double v16 = 0.0;

		if (nNumParticles > 0)
		{
			for (; nNumParticles; --nNumParticles)
			{
				const auto v13 = abs(ScenarioClass::Instance->Random.RandomRanged(nMin, nMax));
				const auto v10 = ScenarioClass::Instance->Random.RandomDouble() * v17 + v16;
				const auto v18 = Math::cos(v10);
				const auto v9 = Math::sin(v10);
				CoordStruct nCoordB { nCoord.X + static_cast<int>(v13 * v18),nCoord.Y - static_cast<int>(v9 * v13), nCoord.Z };
				nCoordB.Z = v8 + MapClass::Instance->GetCellFloorHeight(nCoordB);
				ParticleSystemClass::Instance->SpawnParticle(pType, &nCoordB);
				v16 += v17;
			}
		}
	}

	return 0x42504D;
}

//DEFINE_HOOK(0x6D9466, TacticalClass_Render_BuildingInLimboDeliveryC, 0x9)
//{
//	enum { Draw = 0x0, DoNotDraw = 0x6D9587 };
//
//	GET(BuildingClass* const, pBuilding, EBX);
//	return BuildingExt::ExtMap.Find(pBuilding)->LimboID != -1 ? DoNotDraw : Draw;
//}

//DEFINE_HOOK(0x6FDDC0, TechnoClass_FireAt_RememberAttacker, 0x6)
//{
//	GET(TechnoClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0xB4,0x4));
//
//	if (auto pTechnoTarget = generic_cast<TechnoClass*>(pTarget))
//	{
//		auto pTargetTExt = TechnoExt::ExtMap.Find(pTechnoTarget);
//		pTargetTExt->LastAttacker = pThis;
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x73C6F5, UnitClass_DrawAsSHP_StandFrame, 0x9)
//{
//	GET(UnitTypeClass*, pType, ECX);
//	GET(UnitClass*, pThis, EBP);
//	GET(int, nFrame, EBX);
//
//	auto nStand = pType->StandingFrames;
//	auto nStandStartFrame = pType->StartFiringFrame + nStand * nFrame;
//	if (pType->IdleRate > 0)
//		nStandStartFrame += pThis->WalkedFramesSoFar;
//
//	R->EBX(nStandStartFrame);
//
//	return 0x73C725;
//}

#ifdef ENABLE_NEWHOOKS

#ifdef Eng_captureDelay
// TODO : more hooks

DEFINE_HOOK(0x4D69F5, FootClass_ApproachTarget_EngineerCaptureDelay, 0x6)
{
	GET(FootClass*, pFoot, EBX);

	if (auto pInf = specific_cast<InfantryClass*>(pFoot))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pInf->Type);
		const auto pExt = TechnoExt::ExtMap.Find(pInf);

		if (pExt->EngineerCaptureDelay.InProgress())
		{
			return 0x4D6A01;
		}
	}

	return 0x0;
}

//set timer here ?
DEFINE_HOOK(0x5206B7, InfantryClass_FiringUpdate_EngCapture, 0x6)
{
	GET(InfantryClass*, pFoot, EBP);

	if (pTypeExt->Engineer_CaptureDelay)
	{
		if (pFoot->Destination == pExt->LastBuilding)
		{
			if (pExt->EngineerCaptureDelay.GetTimeLeft() < 0)
				pFoot->ForceMission(Mission::Capture);
		}
		else if (pFoot->Destionation
			   && pFoot->Destination->WhatIam() == BuildingClass::AbsID
			   && pFoot->Type->Engineer)
		{
			pExt->LastBuilding = pFoot->Destionation;
		}
	}

	return 0;
}

DEFINE_HOOK(0x51EEED, InfantryClass_GetCursorOverObject_Infiltrate, 0x7)
{
	GET(InfantryClass*, pFoot, ESI);
	GET(BuildingClass*, pTarget, EDI);

	//BuildingType -> SpyEffect.Times
	// if pTarget -> is on -> BuildingTypeList of pFoot Agent.Allowed = allow ret nothing
	enum
	{
		DontAllow = 0x51F04E,
		Nothing = 0x0
	};

	return Nothing;
}

DEFINE_HOOK(0x51EE97, InfantryClass_WhatAction_Capturable, 0x6)
{
	enum
	{
		Skip = 0x51F04E,
		Nothing = 0x0
	};

	GET(InfantryClass*, pFoot, EDI);

	if (pExt->EngineerCaptureDelay.InProgress())
	{
		return Skip;
	}

	return Nothing;
}

DEFINE_HOOK(0x51E5AB, InfantryClass_WhatAction_Capturable2, 0x6)
{
	enum
	{
		Skip = 0x51E668,
		Nothing = 0x0
	};

	GET(InfantryClass*, pFoot, EDI);

	if (pExt->EngineerCaptureDelay.InProgress())
	{
		return Skip;
	}

	return Nothing;
}

DEFINE_HOOK(0x51E6BA, InfantryClass_WhatAction_EngAttack, 0x6)
{
	enum
	{
		Skip = 0x51EB15,
		Nothing = 0x0
	};

	GET(InfantryClass*, pFoot, EDI);

	if (pFoot->Type->Engineer && pExt->EngineerCaptureDelay.InProgress())
	{
		return Skip;
	}

	return Nothing;
}
#endif

//ToDo : building check ?
// What : Attacked -> got 2 states -> spawn OnFireAnim from randomized idx -> Scatter coord -> create anim
/*struct nTempBelow
{

		ValueableVector<AnimTypeClass*> Rules_OnFire_Aircraft;
		ValueableVector<AnimTypeClass*> Rules_OnFire_Infantry;
		ValueableVector<AnimTypeClass*> Rules_OnFire_Unit;
		ValueableVector<AnimTypeClass*> Rules_OnFire_Building;

	ValueableVector<AnimTypeClass*> OnFire_Aircraft;
	ValueableVector<AnimTypeClass*> OnFire_Infantry;
	ValueableVector<AnimTypeClass*> OnFire_Unit;
	ValueableVector<AnimTypeClass*> OnFire_Building;


};
DEFINE_HOOK(0x4D7431, FootClass_ReceiveDamage_OnFire, 0x5)
{
	GET(DamageState, nState, EAX);

	if (nState == DamageState::NowYellow || nState == DamageState::NowRed)
	{
		GET_STACK(TechnoClass*, pAttacker, STACK_OFFS(0x1C, -0x10));
		GET_STACK(HouseClass*, pHouse, STACK_OFFS(0x1C, -0x18));
		GET(FootClass*, pThis, ESI);
		GET(WarheadTypeClass*, pWH, EBP);

		if (pWH->Sparky)
		{
			Iterator<AnimTypeClass*> nVec;
			switch (pThis->WhatAmI())
			{
			default:
				break;
			}

		}

	}
	return 0x0;
}*/
//DEFINE_HOOK(0x4FD8F7, HouseClass_FireSale_UnitCheck, 0x6)
//{
//	GET(HouseClass*, pThis, EBX);
//
//	if (!Unsorted::ShortGame || pThis->OwnedUnits - pThis->OwnedInfantry <= 10)
//		return 0x0;
//
//	pThis->All_To_Hunt();
//	return 0x4FD907;
//}
//

//DEFINE_HOOK(0x519B58, InfantryClass_PerCellProcess_AutoSellCheck, 0x6)
//{
//	enum
//	{
//		Nothing = 0x0,
//		Skip = 0x51A03E
//	};
//
//	GET(InfantryClass*, pThis, ESI);
//	GET(TechnoClass*, pTarget, EDI);
//
//	return Nothing;
//}
//
//DEFINE_HOOK(0x51A002, InfantryClass_PerCellProcess_EventCheck, 0x6)
//{
//	GET(InfantryClass*, pThis, ESI);
//	GET(TechnoClass*, pTarget, EDI);
//
//	return 0x0;
//}

//struct DummyBtypeExt
//{
//	static ValueableVector<BuildingTypeClass*> BuildingAdjentBaseOn;
//};
//
//DEFINE_HOOK(0x4A8FEC , MouseClass_CanPlaceHere_SpecifiedBase , 0x7)
//{
//	GET(BuildingTypeClass*, pBuildingType, EDX);
//
//	auto Display_PendingObj = Make_Global<BuildingTypeClass*>(0x880990);
//
//	if (!Display_PendingObj || Display_PendingObj->WhatAmI()  != AbstractType::BuildingType)
//		return 0x0;
//
//	return DummyBtypeExt::BuildingAdjentBaseOn.empty() || DummyBtypeExt::BuildingAdjentBaseOn.Contains(pBuildingType) ? 0x0 : 0x4A8FFA;
//}


DEFINE_HOOK(0x452831, BuildingClass_Overpowerer_AddUnique, 0x6)
{
	enum
	{
		AddItem = 0x45283C,
		Skip = 0x45289C
	};

	GET(const DynamicVectorClass<TechnoClass*>*, pVec, ECX);
	GET(TechnoClass* const, pOverpowerer, ESI);

	return pVec->FindItemIndex(pOverpowerer) == -1 ? AddItem : Skip;
}

/*
DEFINE_HOOK(0x452831, BuildingClass_UpdateOverpowerState, 0x6)
{
	GET(const BuildingClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, ECX);

	return pThis->SelectWeapon(pTarget) == -1 ?
		0x45283C : 0x45289C;
}*/

DEFINE_HOOK(0x746CD0, UnitClass_SelectWeapon_Replacements, 0x6)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	const auto pType = pThis->Type;
	R->EAX((pThis->Deployed && pType->DeployFire && pType->DeployFireWeapon != -1) ?
		pType->DeployFireWeapon : pThis->TechnoClass::SelectWeapon(pTarget));

	return 0x746CFD;
}

//struct WeaponWeight
//{
//	short index;
//	bool InRange;
//	float DPF;
//	bool operator < (const WeaponWeight& RHS) const
//	{
//		return (this->InRange < RHS.InRange&& this->DPF < RHS.DPF);
//	}
//};
//
//static float EvalVersesAgainst(TechnoClass* pThis, ObjectClass* pTarget, WeaponTypeClass* W)
//{
//	Armor nArmor = pTarget->GetType()->Armor;
//	if (const auto pTargetTech = generic_cast<TechnoClass*>(pTarget))
//		if (auto const pExt = TechnoExt::ExtMap.Find(pTargetTech))
//			if (auto const pShield = pExt->GetShield())
//				if (pShield->IsActive() && pExt->CurrentShieldType)
//					nArmor = pShield->GetArmor();
//
//	const double Verses = GeneralUtils::GetWarheadVersusArmor(W->Warhead, nArmor);
//	return (W->Damage * TechnoExt::GetDamageMult(pThis)) * Verses / (W->ROF * Helpers_DP::GetROFMult(pThis));
//}
//
//static bool EvalWeaponAgainst(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* W)
//{
//	if (!W || W->NeverUse || !pTarget) { return 0; }
//
//	WarheadTypeClass* WH = W->Warhead;
//	if (!WH) { return 0; }
//
//	if (!pTarget->AsTechno())
//		return 0;
//
//	TechnoTypeClass* pTargetT = ((TechnoClass*)pTarget)->GetTechnoType();
//
//	if (WH->Airstrike)
//	{
//		if (pTarget->WhatAmI() != AbstractType::Building) {
//			return 0;
//		}
//
//		BuildingTypeClass* pBT = ((BuildingClass*)pTarget)->Type;
//		// not my design, leave me alone
//		return pBT->CanC4 && (!pBT->ResourceDestination || !pBT->ResourceGatherer);
//	}
//
//	if (WH->IsLocomotor) {
//		return (pTarget->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None;
//	}
//
//	if (W->DrainWeapon) {
//		return pTargetT->Drainable && !pThis->DrainTarget && !pThis->Owner->IsAlliedWith_(pTarget);
//	}
//
//	if (W->AreaFire) {
//		return pThis->GetCurrentMission() == Mission::Unload;
//	}
//
//	if (pTarget->WhatAmI() == AbstractType::Building && ((BuildingClass*)pTarget)->Type->Overpowerable) {
//		return WH->ElectricAssault && pThis->Owner->CanOverpower(((TechnoClass*)pTarget));
//	}
//
//	if (pTarget->IsInAir() && !W->Projectile->AA) {
//		return 0;
//	}
//
//	if (pTarget->IsOnFloor() && !W->Projectile->AG) {
//		return 0;
//	}
//
//	return 1;
//}
//
//static int EvalDistanceAndVerses(TechnoClass* pThis, ObjectClass* pTarget)
//{
//	auto const pType = pThis->GetTechnoType();
//
//	int WCount = 2;
//	if (pType->WeaponCount > 0) {
//		WCount = pType->WeaponCount;
//	}
//
//	std::vector<WeaponWeight> Weights(WCount);
//
//	for (short i = 0; i < WCount; ++i)
//	{
//		WeaponTypeClass* W = pThis->GetWeapon(i)->WeaponType;
//		Weights[i].index = i;
//		if (W)
//		{
//			CoordStruct xyz1 = pThis->GetCoords();
//			CoordStruct xyz2 = pTarget->GetCoords();
//			float distance = abs(xyz1.DistanceFrom(xyz2));
//			bool CloseEnough = distance <= W->Range && distance >= W->MinimumRange;
//			Weights[i].DPF = EvalVersesAgainst(pThis, pTarget, W);
//			Weights[i].InRange = CloseEnough;
//		}
//		else
//		{
//			Weights[i].DPF = 0.0;
//			Weights[i].InRange = 0;
//		}
//	}
//	std::stable_sort(Weights.begin(), Weights.end());
//	std::reverse(Weights.begin(), Weights.end());
//	return Weights[0].index;
//}
//
//static int SelectWeaponAgainst(TechnoClass* pThis, AbstractClass* pTarget)
//{
//	int Index = 0;
//	if (!pTarget)
//		return 0;
//
//	const TechnoTypeClass* pThisT = pThis->GetTechnoType();
//
//	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThisT)) {
//		if (pTypeExt->Interceptor.Get() && pTarget->WhatAmI() == AbstractType::Bullet) {
//			return(pTypeExt->Interceptor_Weapon.Get() == -1 ? 0 : pTypeExt->Interceptor_Weapon.Get());
//		}
//	}
//
//	//WeaponStruct* W1 = pThis->GetWeapon(0);
//	//WeaponTypeClass* W1T = W1->WeaponType;
//	WeaponStruct* W2 = pThis->GetWeapon(1);
//	WeaponTypeClass* W2T = W2->WeaponType;
//
//	if (pThisT->HasMultipleTurrets() && !pThisT->IsGattling) {
//		return pThis->CurrentWeaponNumber;
//	}
//
//	if (pThis->CanOccupyFire()) {
//		return 0;
//	}
//
//	if (pThis->InOpenToppedTransport) {
//		Index = pThisT->OpenTransportWeapon;
//		if (Index != -1) {
//			return Index;
//		}
//	}
//
//	if (pThisT->IsGattling) {
//		int CurrentStage = pThis->CurrentGattlingStage * 2;
//		if (pTarget->AbstractFlags & AbstractFlags::Techno && pTarget->IsInAir()) {
//			if (W2T && W2T->Projectile->AA) {
//				return CurrentStage + 1;
//			}
//		}
//		return CurrentStage;
//	}
//
//	if (pThis->WhatAmI() == AbstractType::Building && ((BuildingClass*)pThis)->IsOverpowered) {
//		return 1;
//	}
//
//	if (pTarget->WhatAmI() == AbstractType::Aircraft && ((AircraftClass*)pTarget)->IsCrashing) {
//		return 1;
//	}
//
//	// haaaaaaaate
//	if (pTarget->WhatAmI() == AbstractType::Cell) {
//		CellClass* pTargetCell = (CellClass*)pTarget;
//		if (
//
//			(pTargetCell->LandType != LandType::Water && pTargetCell->IsOnFloor())
//			|| ((pTargetCell->ContainsBridge()) && pThisT->Naval)
//
//			&& (!pTargetCell->IsInAir() && pThisT->LandTargeting == LandTargetingType::Land_secondary)
//
//		) {
//			return 1;
//		}
//	}
//
//	auto const pTechnShit = generic_cast<ObjectClass*>(pTarget);
//	const LandType ltTgt = pTechnShit->GetCell()->LandType;
//	bool InWater = !pTechnShit->OnBridge && !pTarget->IsInAir() && (ltTgt == LandType::Water || ltTgt == LandType::Beach);
//
//	if (InWater)
//	{
//		Index = (int)pThis->SelectNavalTargeting(pTarget);
//		if (Index != -1) {
//			return Index;
//		}
//		else {
//			return 0; // what?
//		}
//	}
//
//	if (!pTarget->IsInAir() && pThisT->LandTargeting == LandTargetingType::Land_secondary) {
//		return 1;
//	}
//
//	return EvalDistanceAndVerses(pThis, pTechnShit);
//}
//
//static int HandleSelectWeapon(TechnoClass* pThis, AbstractClass* pTarget)
//{
//	//const TechnoTypeClass* pThisT = pThis->GetTechnoType();
//	//if(pThisT->Spawns)
//	return SelectWeaponAgainst(pThis,pTarget);
//}

/*
//complete replacement ?
DEFINE_HOOK(0x6F3330, TechnoClass_SelectWeapon, 5)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	const int v5 = HandleSelectWeapon(pThis, pTarget);
	if (pTarget == pThis->Target)
		pThis->CurrentWeaponNumber = v5;


	R->EAX(v5);
	return 0x6F3813;
}
*/


#pragma region ES_Stuffs



//#include <CCToolTip.h>
//
//DEFINE_HOOK(0x77778D, YRWindoProc_FixToolTip1, 0x5)
//{
//	GET(HWND, nHw, EDI);
//
//	//CCToolTip::Instance
//	GameConstruct<CCToolTip>(CCToolTip::Instance, nHw);
//	CCToolTip::Instance->Delay = 7;
//
//	return 0x7779B5;
//}
//
//DEFINE_HOOK(0x7777F3, YRWindoProc_FixToolTip2, 6) {
//	GameConstruct<ToolTipManager>(CCToolTip::Instance, nullptr);
//	return 0x777809;
//}

//
//DEFINE_HOOK(0x6276A6, LoadPaletteFiles_LoadFile_Debug, 0x5) {
//	GET(void*, pFile, EAX);
//	LEA_STACK(char*, pFilename, STACK_OFFS(0x424, 0x400));
//
//	if (pFile == R->EBX<void*>()) {
//		Debug::Log("Could not find palette file %s \n", pFilename);
//		return 0x6276AA;
//	}
//
//	return 0x6276B9;
//}

#ifdef ES_ExpDamageHook
static void Detonate(TechnoClass* pTarget, HouseClass* pOwner, CoordStruct const& nCoord)
{

}

//TODO , check stack , make this working

#endif


#endif
#pragma region TSL_Test
//struct SHP_AlphaData
//{
//	int Width;
//	BYTE* CurPixel;
//	BYTE* CurData2;
//};
//
//void SetTSLData(DWORD TlsIdx, int nFrame, SHPReference* pAlplhaFile)
//{
//	auto nFrameBounds = pAlplhaFile->GetFrameBounds(nFrame);
//	auto nPixel = pAlplhaFile->GetPixels(nFrame);
//	SHP_AlphaData nData;
//	nData.Width = pAlplhaFile->Width;
//	nData.CurPixel = &nPixel[nFrameBounds.X + nFrameBounds.Y * pAlplhaFile->Width];
//	TlsSetValue(TlsIdx, &nData);
//}
//
//void GetTSLData_1(DWORD TlsIdx, void* pSomePtr, int nVal_y)
//{
//	if (TlsGetValue(TlsIdx))
//	{
//		if (pSomePtr)
//		{
//			if (int nX = (*(int*)((char*)pSomePtr + 0x14)))
//			{
//				if (int nY = (*(int*)((char*)pSomePtr + 0x18)))
//				{
//					int nWidth = 0;
//					if (nVal_y > 0)
//						nWidth = nVal_y;
//
//					(*(int*)((char*)pSomePtr + 0x10)) = nX + nY * nWidth;
//				}
//			}
//		}
//	}
//}
//
//void GetTSLData_2(DWORD TlsIdx, int nVala, int nValb)
//{
//	if (SHP_AlphaData* pData = (SHP_AlphaData*)TlsGetValue(TlsIdx))
//	{
//		if (auto nWidth = pData->Width)
//		{
//			if (auto nData = pData->CurPixel)
//				pData->CurData2 = &nData[nVala * nWidth + nValb];
//		}
//	}
//}
//
//void GetTSLData_3(DWORD TlsIdx, int nVal)
//{
//	if (SHP_AlphaData* pData = (SHP_AlphaData*)TlsGetValue(TlsIdx))
//	{
//		if (auto nWidth = pData->Width)
//		{
//			if (auto nData2 = pData->CurData2)
//				pData->CurData2 = &nData2[nVal * nWidth];
//		}
//	}
//}
//
//void GetTSLData_4(DWORD TlsIdx, void* pSomePtr)
//{
//	if (TlsGetValue(TlsIdx))
//	{
//		if (pSomePtr)
//		{
//			if (int nX = (*(int*)((char*)pSomePtr + 0x10)))
//			{
//				if (int nY = (*(int*)((char*)pSomePtr + 0x18)))
//				{
//					(*(int*)((char*)pSomePtr + 0x10)) = nX + nY;
//				}
//			}
//		}
//	}
//}
//
//void GetTSLData_5(DWORD TlsIdx, int nVal)
//{
//	if (SHP_AlphaData* pData = (SHP_AlphaData*)TlsGetValue(TlsIdx))
//	{
//		if (auto nWidth = pData->Width)
//		{
//			if (auto nData2 = pData->CurData2)
//				pData->CurData2 = &nData2[nWidth];
//		}
//	}
//}
//
//void ClearTSLData(DWORD TlsIdx)
//{
//	TlsSetValue(TlsIdx, nullptr);
//}
//
//void InitTSLData(DWORD& TslIdx)
//{
//	TslIdx = TlsAlloc();
//	if (TslIdx == -1)
//		Debug::FatalErrorAndExit("Failed to Allocate TSL Index ! \n");
//}
#pragma endregion

//DEFINE_HOOK(0x73B5B5, UnitClass_DrawVoxel_AttachmentAdjust, 0x6)
//{
//	enum { Skip = 0x73B5CE };
//
//	GET(UnitClass*, pThis, EBP);
//	LEA_STACK(VoxelIndexKey *, pKey, STACK_OFFS(0x1C4, 0x1B0));
//	LEA_STACK(Matrix3D* , pMtx ,STACK_OFFS(0x1C4, 0xC0));
//	R->EAX(pThis->Locomotor.get()->Draw_Matrix(pMtx,pKey));
//	return Skip;
//}

//DEFINE_HOOK(0x73C864, UnitClass_drawcode_AttachmentAdjust, 0x6)
//{
//	enum { Skip = 0x73C87D };
//
//	GET(UnitClass*, pThis, EBP);
//	LEA_STACK(VoxelIndexKey *, pKey, STACK_OFFS(0x128, 0xC8));
//	LEA_STACK(Matrix3D* , pMtx ,STACK_OFFS(0x128, 0x30));
//	R->EAX(pThis->Locomotor.get()->Draw_Matrix(pMtx,pKey));
//	return Skip;
//}

//DEFINE_HOOK(0x663225, RocketLocomotionClass_DetonateOnTarget_Anim, 0x6)
//{
//	GET(AnimClass*, pMem, EAX);
//	GET(RocketLocomotionClass* const, pThis, ESI);
//	REF_STACK(CellStruct const, nCell, STACK_OFFS(0x60, 0x38));
//	REF_STACK(CoordStruct const, nCoord, STACK_OFFS(0x60, 0x18));
//	GET_STACK(WarheadTypeClass* const, pWarhead, STACK_OFFS(0x60, 0x50));
//
//	GET(int, nDamage, EDI);
//
//	const auto pCell = MapClass::Instance->GetCellAt(nCell);
//	if (auto pAnimType = MapClass::SelectDamageAnimation(nDamage, pWarhead, pCell ? pCell->LandType : LandType::Clear, nCoord))
//	{
//		GameConstruct(pMem, pAnimType, nCoord, 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200 | AnimFlag::AnimFlag_2000, -15, false);
//		AnimExt::SetAnimOwnerHouseKind(pMem, pThis->LinkedTo->GetOwningHouse(), pThis->LinkedTo->Target ? pThis->LinkedTo->Target->GetOwningHouse() : nullptr, nullptr, false);
//	}
//	else
//	{
//		//no constructor called , so it is safe to delete the allocated memory
//		GameDelete<false, false>(pMem);
//	}
//
//	return 0x66328C;
//}

//DEFINE_HOOK(0x70166E, TechnoClass_Captured, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	pThis->UpdatePlacement(PlacementType::Remove);
//	return 0x70167A;
//}

//DEFINE_HOOK(0x5B3614, MissionClass_AssignMission_CheckBuilding, 0x6)
//{
//	GET(MissionClass*, pThis, ESI);
//	GET(Mission, nMission, EAX);
//
//	if (pThis->WhatAmI() == AbstractType::Building && nMission == Mission::Hunt)
//		pThis->QueuedMission = Mission::Guard;
//	else
//		pThis->QueuedMission = nMission;
//
//	return 0x5B361A;
//}

//DEFINE_HOOK(0x6B721F, SpawnManagerClass_Manage_Clear, 0x6)
//{
//	GET(SpawnManagerClass*, pThis, ESI);
//	pThis->Target = nullptr;
//	pThis->NewTarget = nullptr;
//	pThis->Status = SpawnManagerStatus::Idle;
//	return 0x0;
//}

//
//DEFINE_HOOK(0x4FD635, HouseClass_AI_UpdatePlanOnEnemy_FixDistance, 0x5)
//{
//	GET(HouseClass*, pThis, ESI);
//	GET(HouseClass*, pEnemy, EBX);
//
//	if (pThis->IsAlliedWith_(pEnemy))
//		R->EAX(INT_MAX);
//	else
//		R->EAX(pThis->BaseCenter ? pThis->BaseCenter.X : pThis->BaseSpawnCell.X);
//
//	return 0x4FD657;
//}

//DEFINE_HOOK(0x73DDC0, UnitClass_Mi_Unload_DeployIntoSpeed, 0x6)
//{
//	GET(UnitClass* const, pThis, ESI);
//
//	//Loco->Is_Moving
//	if (R->AL())
//	{
//		if (pThis->Type->Speed > 0)
//		{
//			return 0x73DE20;
//		}
//		else
//		{
//			pThis->StopMoving();
//			pThis->QueueMission(Mission::Unload, false);
//			pThis->NextMission();
//			return 0x73DE3A;
//		}
//	}
//
//	return 0x73DDC4;
//}

//DEFINE_HOOK(0x7397E4, UnitClass_DeployIntoBuilding_DesyncFix, 0x5)
//{
//	GET(HouseClass*, pHouse, ECX);
//
//	bool CurPlayer = false;
//	if (SessionGlobal.GameMode != GameMode::Campaign)
//	{
//		CurPlayer = (pHouse->CurrentPlayer);
//	}
//	else
//	{
//		CurPlayer = (pHouse->CurrentPlayer || pHouse->IsInPlayerControl);
//	}
//
//	R->AL(CurPlayer);
//	return 0x7397E9;
//}

//DEFINE_HOOK(0x73B4F4, UnitClass_DrawAsVXL_FiringAnim, 0x8)
//{
//	R->ECX(R->EBX<UnitTypeClass*>()->TurretVoxel.HVA);
//	R->ESI(0);
//	return 0x73B4FC;
//}
//
//DEFINE_HOOK(0x67E875, LoadGame_Start_AfterMouse, 0x6)
//{
//	Unsorted::CursorSize = nullptr;
//	return 0;
//}

//DEFINE_HOOK(0x4CA0F8, FactoryClass_AbandonProduction_RemoveProduct, 0x7)
//{
//	GET(TechnoClass*, pProduct, ECX);
//	pProduct->UnInit();
//	return 0x4CA0FF;
//}

//DEFINE_HOOK_AGAIN(0x534F4E, ScoreClass_LoadMix, 0x5)
//DEFINE_HOOK(0x6D97BF , ScoreClass_LoadMix, 0x5)
//{
//	Debug::Log("%s\n", R->ESP<char*>());
//	return R->Origin() + 5;
//}

//DEFINE_HOOK(0x6A9791, StripClass_DrawIt_BuildingFix, 0x6)
//{
//	GET(BuildingTypeClass*, pTech, EBX);
//	auto const pHouse = HouseClass::CurrentPlayer();
//
//	const auto pFac = pHouse->GetPrimaryFactory(pTech->WhatAmI(), pTech->Naval, pTech->BuildCat);
//	if (pFac && pFac->Object->GetTechnoType() != pTech)
//		R->Stack(0x17, 1);
//
//
//	return 0;
//}

//DEFINE_HOOK_AGAIN(0x6A91E5, SidebarClass_Update_MultiMoney, 0x6)
//DEFINE_HOOK(0x6A9137, SidebarClass_Update_MultiMoney, 0x6)
//{
//	GET(int, nMoney, EAX);
//	GET(HouseClass*, pHouse, ESI);
//
//	if (!pHouse->IsControlledByCurrentPlayer())
//	{
//		nMoney = static_cast<int>(nMoney * 100.0 / RulesGlobal->MultiplayerAICM.GetItem(static_cast<int>(pHouse->AIDifficulty)));
//	}
//
//	R->EAX(nMoney);
//	return R->Origin() + 6;
//}

static void ClearShit(TechnoTypeClass* a1)
{
	const auto pObjectToSelect = MapClass::Instance->NextObject(
		ObjectClass::CurrentObjects->Count ? ObjectClass::CurrentObjects->GetItem(0) : nullptr);

	auto pNext = pObjectToSelect;
	while (!pNext || pNext->GetTechnoType() != a1)
	{
		pNext = MapClass::Instance->NextObject(pNext);
		if (!pNext || pNext == pObjectToSelect)
			return;
	}

	MapClass::Instance->SetTogglePowerMode(0);
	MapClass::Instance->SetWaypointMode(0, false);
	MapClass::Instance->SetRepairMode(0);
	MapClass::Instance->SetSellMode(0);

	MapClass::UnselectAll();
	pObjectToSelect->Select();
	MapClass::Instance->CenterMap();
	MapClass::Instance->MarkNeedsRedraw(1);
}

//DEFINE_HOOK(0x6AB63B, SelectClass_Action_UnableToBuild, 0x6)
//{
//	GET(TechnoTypeClass*, pTech, EAX);
//
//	if (HouseClass::CurrentPlayer()->CanBuild(pTech, 0, 0) == CanBuildResult::Buildable)
//		return 0;
//
//	ClearShit(pTech);
//	return 0x6AB95A;
//}

/*
DEFINE_HOOK(0x4F9AF0, HouseClass_IsAlly_AbstractClass, 0x7)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);

	bool res = false;
	if (pTarget)
	{
		auto const pWhat = pTarget->WhatAmI();
		if ((pTarget->AbstractFlags & AbstractFlags::Object) != AbstractFlags::None)
		{
			res = pThis->IsAlliedWith_(pTarget->GetOwningHouse());
		}
		else if (pWhat == AbstractType::House)
		{
			switch (pWhat)
			{
			case AbstractType::House:
				res = pThis->IsAlliedWith_(static_cast<HouseClass*>(pTarget));
			break;
			case AbstractType::Bomb:
				res = pThis->IsAlliedWith_(pTarget->GetOwningHouse());
			break;
			}
		}
	}

	R->AL(res);
	return 0x4F9B11;
}*/

//DEFINE_HOOK(0x50CA30, HouseClass_Center_50C920, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (pThis->BelongsToATeam())
//		return 0x50CAB4;
//
//	R->CL(pThis->GetTechnoType()->DeploysInto->ConstructionYard);
//	return 0x50CA3C;
//}

//DEFINE_HOOK(0x4A9004, MouseClass_CanPlaceHere_SkipSelf, 0x6)
//{
//	if (auto const pHouse = HouseClass::Array->GetItem(R->EAX<int>()))
//	{
//		if (pHouse == R->ECX<HouseClass*>())
//			return 0x4A902C;
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x737E66, UnitClass_ReceiveDamage_Debug, 0x8)
//{
//	GET(UnitClass*, pThis, ESI);
//	GET_STACK(WarheadTypeClass*, pWH, STACK_OFFS(0x48, -0xC));
//	Debug::Log("[%d] %s Warhead Destroying %s ! \n ", pWH, pWH->ID, pThis->get_ID());
//	return 0x0;
//}

//DEFINE_HOOK(0x4F9A50, HouseClass_IsAlly_HouseClass, 0x6)
//{
//	GET(HouseClass*, pThis, ECX);
//	GET_STACK(HouseClass*, pTarget, 0x4);
//
//	bool ret = false;
//	if (pTarget) {
//		if (pThis == pTarget)
//			ret = true;
//		else
//			ret = pThis->IsAlliedWith_(pTarget->ArrayIndex);
//	}
//	R->AL(ret);
//	return 0x4F9A60;
//}

//DEFINE_HOOK(0x4F9A90 ,HouseClass_IsAlly_ObjectClass , 0x7)
//{
//	GET(HouseClass*, pThis, ECX);
//	GET_STACK(ObjectClass*, pTarget, 0x4);
//	R->AL(pThis->IsAlliedWith_(pTarget->GetOwningHouse()));
//	return 0x4F9AAB;
//}

//DEFINE_HOOK(0x722FFA, TiberiumClass_Grow_CheckMapCoords, 0x6)
//{
//	enum
//	{
//		increment = 0x72312F,
//		SetCell = 0x723005
//	};
//
//	GET(const MapSurfaceData*, pSurfaceData, EBX);
//	R->EBX(pSurfaceData);
//	const auto nCell = pSurfaceData->MapCoord;
//
//	if (!MapClass::Instance->IsValidCell(nCell))
//	{
//		Debug::Log("Tiberium Growth With Invalid Cell ,Skipping !\n");
//		return increment;
//	}
//
//	R->EAX(MapClass::Instance->GetCellAt(nCell));
//	return SetCell;
//}

//TaskForces_LoadFromINIList_WhySwizzle , 0x5
//DEFINE_JUMP(LJMP, 0x6E8300, 0x6E8315)
//static BulletClass* Fuse_Bullet = nullptr;
//DEFINE_HOOK(0x467C2A, BulletClass_AI_Fuse_FetchBullet, 0x5)
//{
//	Fuse_Bullet = R->EBP<BulletClass*>();
//	return 0x0;
//}
//
//DEFINE_HOOK(0x4E1278, FuseClass_BulletProximity, 0x5)
//{
//	GET(int, nRange, EAX);
//	auto const pBullet = Fuse_Bullet;
//
//	int nProx = 32;
//	auto pExt = BulletExt::ExtMap.Find(pBullet);
//	if (pExt->TypeExt->Proximity_Range.isset())
//		nProx = pExt->TypeExt->Proximity_Range.Get() * 256;
//
//	Fuse_Bullet = nullptr;
//	return (nProx) <= nRange ? 0x4E1289 : 0x4E127D;
//}
//Get_Join_Responses_DuplicateSerianNumber 0x5
//DEFINE_JUMP(LJMP, 0x5E0C24, 0x5E0C4E)

//HouseClass_AllyAIHouses 0x5
//DEFINE_JUMP(LJMP, 0x501640, 0x50174E)
//
//DEFINE_HOOK(0x54DCD2, JumpetLocomotionClass_DrawMatrix, 0x8)
//{
//	GET(FootClass*, pFoot, ECX);
//
//	bool Allow = true;
//	if (pFoot->GetTechnoType()->TiltCrashJumpjet)
//	{
//		Allow = LocomotionClass::End_Piggyback(pFoot->Locomotor);
//	}
//
//	return Allow ? 0x54DCE8 : 0x54DF13;
//}
//
//DEFINE_HOOK(0x5D6BE0, MPGameModeClass_StartingPositionsToHouseBaseCells_Debug, 0x7)
//{
//	Debug::Log("House count = %d", HouseClass::Array->Count);
//	Debug::Log("\n");
//	for (auto pHouse : *HouseClass::Array)
//	{
//		Debug::Log("House start cell = [%d] { %d, %d }",
//		(DWORD)pHouse,
//		pHouse->StartingCell.X,
//		pHouse->StartingCell.Y);
//		Debug::Log("\n");
//	}
//	return 0;
//}

// TODO : this breaking deploy fire thing
//DEFINE_HOOK(0x6FA467, TechnoClass_AI_AttackAllies, 0x5)
//{
//	enum { ClearTarget = 0x0, ExtendChecks = 0x6FA472 };
//	GET(const TechnoClass*, pThis, ESI);
//
//	//if (pThis->GetTechnoType()->AttackFriendlies && pThis->Target)
//	//{
//	//	const int nWeapon = pThis->SelectWeapon(pThis->Target);
//	//	const auto nFireErr = pThis->GetFireError(pThis->Target, nWeapon, false);
//
//	//	if (nFireErr == FireError::CANT || nFireErr == FireError::ILLEGAL)
//	//	{
//	//		return ClearTarget;
//	//	}
//
//	//	return ExtendChecks;
//	//}
//	const auto pType = pThis->GetTechnoType();
//	if (pType->AttackFriendlies && !pType->DeployFire)
//	{
//		return ExtendChecks;
//	}
//
//	return ClearTarget;
//}
//DEFINE_HOOK(0x6EE606, TeamClass_TMission_Move_To_Own_Building_index, 0x7)
//{
//	GET(TeamClass*, pThis, EBP);
//	GET(int, nRawData, EAX);
//
//	const auto nBuildingIdx = nRawData & 0xFFFF;
//
//	if (nBuildingIdx < BuildingTypeClass::Array()->Count)
//		return 0x0;
//
//	const auto nTypeIdx = nRawData >> 16 & 0xFFFF;
//	const auto nScript = pThis->CurrentScript;
//
//	Debug::Log("[%x]Team script [%s]=[%d] , Failed to find type[%d] building at idx[%d] ! \n", pThis, nScript->Type->get_ID(), nScript->CurrentMission, nTypeIdx, nBuildingIdx);
//	return 0x6EE7D0;
//}

// thse were removed to completely disable vein
//DEFINE_HOOK(0x74D376, VeinholeMonsterClass_AI_TSRandomRate_1, 0x6)
//{
//	GET(RulesClass* const, pRules, EAX);
//
//	const auto nRand = pRules->VeinholeShrinkRate > 0 ?
//		ScenarioClass::Instance->Random.RandomFromMax(pRules->VeinholeShrinkRate / 2) : 0;
//
//	R->EAX(pRules->VeinholeShrinkRate + nRand);
//	return 0x74D37C;
//}
//
//DEFINE_HOOK(0x74C5E1, VeinholeMonsterClass_CTOR_TSRandomRate, 0x6)
//{
//	GET(RulesClass* const, pRules, EAX);
//	const auto nRand = pRules->VeinholeGrowthRate > 0 ?
//		ScenarioClass::Instance->Random.RandomFromMax(pRules->VeinholeGrowthRate / 2) : 0;
//
//	R->EAX(pRules->VeinholeGrowthRate + nRand);
//	return 0x74C5E7;
//}
//
//DEFINE_HOOK(0x74D2A4, VeinholeMonsterClass_AI_TSRandomRate_2, 0x6)
//{
//	GET(RulesClass* const, pRules, ECX);
//
//	const auto nRand = pRules->VeinholeGrowthRate > 0 ?
//		ScenarioClass::Instance->Random.RandomFromMax(pRules->VeinholeGrowthRate / 2) : 0;
//
//	R->EAX(pRules->VeinholeGrowthRate + nRand);
//	return 0x74D2AA;
//}
//
//Lunar limitation
// DEFINE_JUMP(LJMP, 0x546C8B, 0x546CBF);
//
//DEFINE_HOOK(0x730F1C, ObjectClass_StopCommand, 0x5)
//{
//	GET(ObjectClass*, pObject, ESI);
//
//	if (auto pTechno = generic_cast<TechnoClass*>(pObject)) {
//		if (auto pManager = pTechno->SpawnManager) {
//			if (!pManager->SpawnType->MissileSpawn && pManager->Status != SpawnManagerStatus::Idle && pManager->Target)
//				pManager->ResetTarget();
//		}
//	}
//
//	return 0;
//}

#ifdef ENABLE_NEWHOOKS
//TODO : retest for desync
DEFINE_HOOK(0x442A2A, BuildingClass_ReceiveDamage_RotateVsAircraft, 0x6)
{
	GET(BuildingClass* const, pThis, ESI);
	GET(RulesClass*, pRules, ECX);

	if (pThis && pThis->Type)
	{
		if (auto const pStructureExt = BuildingTypeExt::ExtMap.Find(pThis->Type))
		{
			R->AL(pStructureExt->PlayerReturnFire.Get(pRules->PlayerReturnFire));
			return 0x442A30;
		}
	}

	return 0x0;
}
#endif

//size
//#ifdef ENABLE_NEWHOOKS
//Crash
//DEFINE_HOOK(0x444014, AircraftClass_ExitObject_DisableRadioContact, 0x5)
//{
//	enum { SkipAllSet = 0x444053, Nothing = 0x0 };
//
//	//GET(BuildingClass*, pBuilding, ESI);
//	GET(AircraftClass*, pProduct, EBP);
//
//	if (pProduct) {
//		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pProduct->Type);
//		if (pTypeExt && !pProduct->Type->AirportBound && pTypeExt->NoAirportBound_DisableRadioContact.Get()) {
//			return SkipAllSet;
//		}
//	}
//
//	return Nothing;
//}

//#endif
//


//DEFINE_HOOK(0x4CD8C9, FlyLocomotionClass_Movement_AI_DisableTSExp, 0x9)
//{
//	GET(FootClass*, pFoot, EDX);
//	auto const& pTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType());
//	return pTypeExt->Disable_C4WarheadExp.Get() ? 0x4CD9C0 : 0x0;
//}

//#ifndef ENABLE_NEWHOOKS
//template<bool CheckNotHuman = true>
//static Iterator<AnimTypeClass*> GetDeathBodies(InfantryTypeClass* pThisType, const ValueableVector<AnimTypeClass*>& nOverrider)
//{
//	if (!nOverrider.empty())
//		return nOverrider;
//
//	if (pThisType->DeadBodies.Count > 0)
//	{
//		return pThisType->DeadBodies;
//	}
//
//	if constexpr (CheckNotHuman)
//	{
//		if (!pThisType->NotHuman)
//		{
//			return RulesGlobal->DeadBodies;
//		}
//	}
//
//	return { };
//}
//
//DEFINE_HOOK(0x520BCC, InfantryClass_DoingAI_FetchDoType, 0x6)
//{
//	GET(InfantryClass*, pThis, ESI);
//	GET(int, nDoType, EAX);
//	InfantryExt::ExtMap.Find(pThis)->CurrentDoType = nDoType;
//	return 0x0;
//}

//TODO : retest for desync
//DEFINE_HOOK(0x520BE5, InfantryClass_DoingAI_DeadBodies, 0x6)
//{
//	GET(InfantryClass* const, pThis, ESI);
//
//	auto const Iter = GetDeathBodies(pThis->Type, {});
//	constexpr auto Die = [](int x) { return x - 11; };
//
//	if (!Iter.empty()) {
//		int nIdx = 0;
//
//		if(InfantryTypeExt::ExtMap.Find(pThis->Type)->DeathBodies_UseDieSequenceAsIndex.Get()){
//			auto pInfExt = InfantryExt::ExtMap.Find(pThis);
//			nIdx = Die(pInfExt->CurrentDoType);
//			if (nIdx > Iter.size())
//				nIdx = Iter.size();
//		} else {
//			nIdx = ScenarioGlobal->Random.RandomFromMax(Iter.size() - 1);
//		}
//
//		if (AnimTypeClass* pSelected = Iter.at(nIdx)) {
//			if (const auto pAnim = GameCreate<AnimClass>(pSelected, pThis->GetCoords(), 0, 1, AnimFlag::AnimFlag_400 | AnimFlag::AnimFlag_200, 0, 0)) {
//				AnimExt::SetAnimOwnerHouseKind(pAnim, pThis->GetOwningHouse(), nullptr, false);
//			}
//		}
//	}
//
//    pThis->UnInit();
//	return 0x520E52;
//}

#endif

//#include "TestTurrent.h"
//
//std::vector<std::pair<ImageDatas, ImageDatas>> TTypeEXt::AdditionalTurrents { };

//#include <JumpjetLocomotionClass.h>
//#include <Ext/Building/Body.h>
//
//static int GetHeight(CellClass* pCell, const Nullable<int>& nOffset)
//{
//	int nHeight = 0;
//	if (auto pBuilding = pCell->GetBuilding())
//	{
//	   auto pBldExt = BuildingExt::ExtMap.Find(pBuilding);
//
//		CoordStruct vCoords = { 0, 0, 0 };
//		if(!pBldExt->LimboID != -1)
//			pBuilding->Type->Dimension2(&vCoords);
//
//		nHeight = vCoords.Z;
//	}
//	else
//	{
//		if (pCell->FindTechnoNearestTo({ 0,0 }, 0, 0))
//			nHeight = 85;
//	}
//
//	if (nOffset.isset()) {
//		nHeight -= nOffset.Get();
//		if (nHeight < 0)
//			nHeight = 0;
//	}
//
//	int nCellHeight = pCell->GetFloorHeight({ 128,128 });
//	if (pCell->ContainsBridge())
//		nCellHeight += 416;
//
//	return nHeight + nCellHeight;
//}
//
//DEFINE_HOOK(0x54D820, JumpJetLocomotionClass_UpdateHeightAboveObject, 0x6)
//{
//	GET(JumpjetLocomotionClass*, pLoco, ECX);
//
//	auto pOwner = pLoco->LinkedTo;
//	auto pCell = Map.GetCellAt(pOwner->Location);
//	int nHeight = GetHeight(pCell, {});
//
//	if (pLoco->__currentSpeed <= 0.0)
//	{
//		R->EAX(nHeight);
//	}
//	else
//	{
//		auto nFace = pLoco->Facing.Current().GetFacing<8>();
//		auto nAdj = AdjacentCoord[nFace];
//		auto nRes = CoordStruct { pOwner->Location.X + nAdj.X , pOwner->Location.Y + nAdj.Y };
//		auto pCell2 = Map.GetCellAt(nRes);
//		int nHeight2 = GetHeight(pCell2, {});
//		if (nHeight2 <= nHeight)
//			nHeight2 = (nHeight + nHeight2) / 2;
//
//		R->EAX(nHeight2);
//	}
//
//	return 0x54D917;
//}

//int __cdecl UnitClass_GetFireError_AIDeployFire(REGISTERS* R)
//{
//
//	bool DeployToFire_RequiresAIOnly = false;
//	GET(InfantryClass*, v1, ESI);
//	auto v2 = v1->Type;
//	if (!DeployToFire_RequiresAIOnly || v1->Owner->IsControlledByCurrentPlayer())
//	{
//		if (!*(v2 + 3602) || !v1->CanDeployNow())
//			return 0x7410B7;
//
//		return !Map.GetCellAt(v1->Location)->IsBuildable() ? 0x7410A8 :0x7410B7;
//	}
//	else
//	{
//		auto v22 = v1->GetCoords();
//		auto v7 =  Map.GetCellAt(v22);
//		auto v8 = *(v7 + 236);
//		auto v9 = !v7->IsBuildable() && v8 != 2 && v8 != 6;
//		if (*(v3 + 989))
//		{
//			if (!v1->f.t.cargo.CargoHold)
//				return 0x7410B7;
//			if (!v9)
//			{
//				MapClass::NearByLocation(
//				  0x87F7E8,
//				  &cell1,
//				  &cell2,
//				  SPEED_FOOT,
//				  -1,
//				  MZONE_NORMAL,
//				  0,
//				  1,
//				  1,
//				  0,
//				  0,
//				  0,
//				  0,
//				  &cellstrunct::empty,
//				  0,
//				  0);
//				cell2 = cell1;
//				if (cell1.X || cell1.Y)
//				{
//					v12 = cell1.X + (cell1.Y << 9);
//					if (v12 > 0x3FFFF || (v13 = *(MEMORY[0x87F924] + 4 * v12)) == 0)
//					{
//						v13 = 11263056;
//						MEMORY[0xABDC74] = cell1;
//					}
//					(v1->f.t.r.m.o.a.vftable->Stop_Driver)(v1);
//					v1->f.t.r.m.o.a.vftable->t.Assign_Target(v1, 0);
//					v1->f.t.r.m.o.a.vftable->t.Assign_Destination(v1, v13, 1);
//					v1->f.t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(v1, MISSION_MOVE, 0);
//					v1->f.t.r.m.o.a.vftable->t.r.m.Commence(v1);
//					v1->f.t.r.m.o.a.vftable->t.r.m.Assign_Mission__framenumber(v1, MISSION_UNLOAD, 0);
//				}
//			}
//			v14 = v1->f.t.r.m.o.a.vftable->CanDeployNow(v1);
//			v15 = 0x7410B7;
//			if (v14)
//				v15 = 0x7410A8;
//			result = v15;
//		}
//		else
//		{
//			if (!v9)
//				return 0x7410B7;
//			v10 = !v1->f.t.r.m.o.a.vftable->CanDeployNow(v1);
//			result = 7606440;
//			if (v10)
//				return 0x7410B7;
//		}
//	}
//	return result;
//}
//#include <Ext/WeaponType/Body.h>
//
//DEFINE_HOOK(0x4A730D , DiskLaserClass_Update_CalculateFacing, 0x6)
//{
//	GET(DiskLaserClass*, pThis, ESI);
//	GET(int, nFace, EDX);
//
//	auto pExt = WeaponTypeExt::ExtMap.Find(pThis->Weapon);
//
//	if (!pExt->DiskLaser_FiringOffset.isset())
//		return 0x0;
//
//	auto nOffset = (pExt->DiskLaser_FiringOffset.Get().Y - pExt->DiskLaser_FiringOffset.Get().X) >> 3;
//	pThis->DrawOffset.X = ((((pExt->DiskLaser_FiringOffset.Get().Y - pExt->DiskLaser_FiringOffset.Get().X) >> 4)
//					+ (nOffset -1) & (((nFace >> 11) + 1) >> 1)))
//					% nOffset;
//
//	return 0x4A7329;
//}
//
//DEFINE_HOOK(0x4A748E , DiskLaserClass_Update_CalculatePoint, 0x5)
//{
//	GET(DiskLaserClass*, pThis, ESI);
//
//	auto pExt = WeaponTypeExt::ExtMap.Find(pThis->Weapon);
//
//	if (!pExt->DiskLaser_FiringOffset.isset())
//		return 0x0;
//
//	auto nOffset = (pExt->DiskLaser_FiringOffset.Get().Y - pExt->DiskLaser_FiringOffset.Get().X) >> 3;
//
//	LEA_STACK(CoordStruct*, pCoord, 0x44);
//
//	auto nFLH = pThis->Owner->GetFLH(pCoord, 0, CoordStruct::Empty);
//
//	R->Stack(0x38, nFLH->X);
//	R->Stack(0x3C, nFLH->Y);
//	R->EBP(nFLH->Z);
//	R->ECX(pThis->DrawOffset.Y);
//
//	R->EBX((pThis->DrawOffset.X + nOffset - pThis->DrawOffset.Y) % nOffset);
//	R->EDX((nOffset - pThis->DrawOffset.Y + pThis->DrawOffset.X - 1) % nOffset);
//	R->EAX((pThis->DrawOffset.X + pThis->DrawOffset.Y) % nOffset);
//	R->EDI((pThis->DrawOffset.Y + pThis->DrawOffset.X + 1) % nOffset);
//
//	return 0x4A7507;
//}
