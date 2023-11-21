
//DEFINE_HOOK(0x5535D0, PCX_LoadScreen, 0x6)
//{
//	LEA_STACK(char*, name, 0x84);
//
//	char pFilename[0x20];
//	strcpy_s(pFilename, name);
//	_strlwr_s(pFilename);
//
//	if (CRT::strstr(pFilename, ".pcx")
//		//|| CRT::strstr(pFilename, ".png")
//		) {
//
//		BSurface* pCXSurf = nullptr;
//
//		if(PCX::Instance->LoadFile(pFilename))
//			pCXSurf = PCX::Instance->GetSurface(pFilename);
//
//		if (pCXSurf) {
//			GET_BASE(DSurface*, pSurf, 0x60);
//			RectangleStruct pSurfBounds = { 0, 0, pSurf->Width, pSurf->Height };
//			RectangleStruct pcxBounds = { 0, 0, pCXSurf->Width, pCXSurf->Height };
//
//			RectangleStruct destClip = { 0, 0, pCXSurf->Width, pCXSurf->Height };
//			destClip.X = (pSurf->Width - pCXSurf->Width) / 2;
//			destClip.Y = (pSurf->Height - pCXSurf->Height) / 2;
//
//			pSurf->Copy_From(pSurfBounds, destClip, pCXSurf, pcxBounds, pcxBounds, true, true);
//		}
//		return 0x553603;
//	}
//	return 0;
//}

/*
DEFINE_HOOK(0x552F81, PCX_LoadingScreen_Campaign, 0x5)
{
	GET(LoadProgressManager*, pThis, EBP);

	DSurface* pSurface = reinterpret_cast<DSurface*>(pThis->ProgressSurface);
	char pFilename[0x20];
	strcpy_s(pFilename, ScenarioClass::Instance->LS800BkgdName);
	_strlwr_s(pFilename);

	if (strstr(pFilename, ".pcx") == 0
		|| strstr(pFilename, ".png") == 0 )
	{
		BSurface* pCXSurf = nullptr;

		if (PCX::Instance->LoadFile(pFilename))
			pCXSurf = PCX::Instance->GetSurface(pFilename);

		if (pCXSurf)
		{
			RectangleStruct destClip = {
				(pSurface->Width - pCXSurf->Width) / 2,
				(pSurface->Height - pCXSurf->Height) / 2,
				pCXSurf->Width, pCXSurf->Height
			};

			PCX::Instance->BlitToSurface(&destClip, pSurface, pCXSurf);
		}

		R->EBX(R->EDI());
		return 0x552FC6;
	}

	return 0;
}*/


DEFINE_HOOK(0x5349E3, ScenarioClass_InitMixes, 0x6)
{
	GET(TheaterType, nType, EDI);
	GET(char*, pControlFileBuffer, EAX);
	LEA_STACK(char*, pDataMixBuffer, STACK_OFFS(0x6C, 0x10));
	LEA_STACK(char*, pDataMix2Buffer, STACK_OFFS(0x6C, 0x30));
	LEA_STACK(char*, pDataMDMixBuffer, STACK_OFFS(0x6C, 0x20));
	LEA_STACK(char*, pDataMDMix2Buffer, STACK_OFFS(0x6C, 0x40));

	Log::Exec(__FUNCTION__, CURRENT_THEATER);
	const auto pTheater = TheaterTypeClass::FindFromTheaterType(nType);

	const auto pExt = pTheater->Extension.data();
	R->Stack(STACK_OFFS(0x6C, 0x58), pExt);
	wsprintfA(pControlFileBuffer, "%s.MIX", pTheater->ControlFileName.data());
	wsprintfA(pDataMixBuffer, "%s.MIX", pTheater->PaletteFileName.data());
	wsprintfA(pDataMix2Buffer, "%s.MIX", pExt);
	wsprintfA(pDataMDMixBuffer, "%sMD.MIX", pTheater->ArtFileName.data());
	wsprintfA(pDataMDMix2Buffer, "%sMD.MIX", pTheater->ControlFileName.data());
	wsprintfA(pDataMix2Buffer, "%s.MIX", pExt);

	return 0x534A4D;
}


//DEFINE_HOOK(0x56BD8B, MapClass_PlaceCrate_InMapFix, 0x5)
//{
//	enum { CreateCrate = 0x56BE7B, DontCreate = 0x56BE91 };
//
//	const int MapX = MapClass::Instance->MapRect.Width * 60;
//	const int MapY = MapClass::Instance->MapRect.Height * 30;
//	const int X = ScenarioClass::Instance->Random.RandomRanged(0, MapX) - MapX / 2;
//	const int Y = ScenarioClass::Instance->Random.RandomRanged(0, MapY) + MapY / 2;
//
//	const auto result = Matrix3D::MatrixMultiply(TacticalClass::Instance->IsoTransformMatrix, { (float)X,(float)Y,0.0f });
//
//	if (const auto pCell = MapClass::Instance->TryGetCellAt({ (int)result.X,(int)result.Y,0 }))
//	{
//		REF_STACK(CellStruct, cell, STACK_OFFS(0x28, 0x18));
//
//		const auto SpeedType = pCell->LandType == LandType::Water ? SpeedType::Float : SpeedType::Track;
//		cell = MapClass::Instance->NearByLocation(pCell->MapCoords, SpeedType, -1, MovementZone::Normal, false, 1, 1, false,
//		false, false, true, CellStruct::Empty, false, false);
//
//		R->EAX(&cell);
//		return CreateCrate;
//	}
//
//	return DontCreate;
//}
//DEFINE_HOOK(0x65AAC0, RadioClass_Detach, 0x5)
//{
//	GET(RadioClass*, pThis, ECX);
//	GET_STACK(AbstractClass*, pTarget, 0x4);
//	GET_STACK(bool, bRemoved, 0x8);
//
//	pThis->ObjectClass::PointerExpired(pTarget, bRemoved);
//	auto datafirst = std::addressof(pThis->RadioLinks.Items[0]);
//	auto dataend = std::addressof(pThis->RadioLinks.Items[pThis->RadioLinks.Capacity]);
//
//	if (datafirst != dataend)
//	{
//		while (*datafirst != pTarget)
//		{
//			if (datafirst == dataend)
//				break;
//		}
//
//		std::memmove(datafirst, datafirst + 1, dataend - (datafirst + 1));
//		--pThis->RadioLinks.Capacity;
//	}
//
//	return 0x65AB08;
//}

//DEFINE_HOOK(0x56BDE7, MapClass_PlaceRandomCrate_GenerationMechanism, 0x6)
//{
//	enum { SpawnCrate = 0x56BE7B, SkipSpawn = 0x56BE91 };
//
//	GET(CellStruct, candidate, ECX);
//	REF_STACK(CellStruct, cell, STACK_OFFSET(0x28, -0x18));
//
//	if (!MapClass::Instance->IsWithinUsableArea(candidate, true))
//		return SkipSpawn;
//
//	const auto pCell = MapClass::Instance->TryGetCellAt(candidate);
//
//	const bool isWater = pCell->LandType == LandType::Water;
//	if (isWater && RulesExtData::Instance()->Crate_LandOnly.Get())
//		return SkipSpawn;
//
//	cell = MapClass::Instance->NearByLocation(pCell->MapCoords,
//		isWater ? SpeedType::Float : SpeedType::Track,
//		-1, MovementZone::Normal, false, 1, 1, false, false, false, true, CellStruct::Empty, false, false);
//
//	R->EAX(&cell);
//
//	return SpawnCrate;
//}


//DEFINE_HOOK(0x4CDA78, FlyLocomotionClass_MovementAI_SpeedModifiers, 0x6)
//{
//	GET(FlyLocomotionClass*, pThis, ESI);
//
//	if (auto const pLinked = pThis->LinkedTo)
//	{
//		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
//			TechnoExtData::GetCurrentSpeedMultiplier(pLinked);
//
//		R->EAX(int(currentSpeed));
//	}
//
//	return 0;
//}

/*
DEFINE_HOOK(0x5F53A1, ObjectClass_ReceiveDamage_DeathcCounter, 0x5)
{
	GET(ObjectClass*, pThis, ESI);

	if (auto pUnit = specific_cast<UnitClass*>(pThis))
		if (pUnit->DeathFrameCounter > 0)
			return 0x5F5830;

	return 0x0;
}*/

/*
DEFINE_HOOK(0x737DBF, UnitClass_ReceiveDamage_DeathAnim, 0xA)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->Type->MaxDeathCounter > 0)
		pThis->DeathFrameCounter = 1;
	else
		pThis->DeathFrameCounter = 0;

	return 0x737DC9;
}*/
//DEFINE_HOOK(0x4CE4BF, FlyLocomotionClass_4CE4B0_SpeedModifiers, 0x6)
//{
//	GET(FlyLocomotionClass*, pThis, ECX);
//
//	if (auto const pLinked = pThis->LinkedTo)
//	{
//		const double currentSpeed = pLinked->GetTechnoType()->Speed * pThis->CurrentSpeed *
//			TechnoExtData::GetCurrentSpeedMultiplier(pLinked);
//
//		R->EAX(int(currentSpeed));
//	}
//
//	return 0;
//}
//template<typename T>
//static bool InvalidateVector(DynamicVectorClass<T>& nVec, T pItem)
//{
//	auto datafirst = std::addressof(nVec.Items[0]);
//	auto dataend = std::addressof(nVec.Items[nVec.Count]);
//
//	if (datafirst != dataend)
//	{
//		while (*datafirst != pItem)
//		{
//			if (datafirst == dataend)
//				return false;
//		}
//
//		std::memmove(datafirst, datafirst + 1, dataend - (datafirst + 1));
//		--nVec.Count;
//		return true;
//	}
//
//	return false;
//}
DEFINE_HOOK(0x469BD6, BulletClass_Logics_MindControlAlternative2, 0x6)
{
	GET(BulletClass*, pBullet, ESI);
	GET(AnimTypeClass*, pAnimType, EBX);

	if (!pBullet->Target)
		return 0;

	const auto pBulletWH = pBullet->WH;
	const auto pTarget = generic_cast<TechnoClass*>(pBullet->Target);

	if (pTarget
		&& pBullet->Owner
		&& pBulletWH
		&& pBulletWH->MindControl)
	{
		if (const auto pTargetType = pTarget->GetTechnoType())
		{
			auto const pWarheadExt = WarheadTypeExtContainer::Instance.Find(pBulletWH);
			{
				const double currentHealthPerc = pTarget->GetHealthPercentage();
				const bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;

				const bool skipMindControl = flipComparations ? (pWarheadExt->MindControl_Threshold > 0.0) : (pWarheadExt->MindControl_Threshold < 1.0);
				const bool healthComparation = flipComparations ? (currentHealthPerc <= pWarheadExt->MindControl_Threshold) : (currentHealthPerc >= pWarheadExt->MindControl_Threshold);

				if (skipMindControl
					&& healthComparation
					&& pWarheadExt->MindControl_AlternateDamage.isset()
					&& pWarheadExt->MindControl_AlternateWarhead.isset())
				{

					int damage = pWarheadExt->MindControl_AlternateDamage;
					WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead;
					const auto pAttacker = pBullet->Owner;
					const auto pAttackingHouse = pBullet->Owner->Owner;
					int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, TechnoExtData::GetArmor(pTarget), 0);

					if (!pWarheadExt->MindControl_CanKill && pTarget->Health <= realDamage)
					{
						pTarget->Health += abs(realDamage);
						realDamage = 1;
						pTarget->ReceiveDamage(&realDamage, 0, pAltWarhead, pAttacker, true, false, pAttackingHouse);
						pTarget->Health = 1;
					}
					else
					{
						pTarget->ReceiveDamage(&damage, 0, pAltWarhead, pAttacker, true, false, pAttackingHouse);
					}

					pAnimType = nullptr;

					// If the alternative Warhead have AnimList tag declared then use it
					if (pWarheadExt->MindControl_AlternateWarhead->AnimList.Count > 0)
					{
						pAnimType = MapClass::SelectDamageAnimation(damage, pAltWarhead, Map[pTarget->Location]->LandType, pTarget->Location);
					}

					R->EBX(pAnimType);
				}
			}
		}
	}

	return 0;
}
/*
DEFINE_HOOK(0x7BAE60 , Surface_GetPixel_CheckParameters, 0x5)
{
	GET(Surface*, pSurface, ECX);
	GET_STACK(Point2D*, pPoint, 0x4);

	R->EAX(NULL);

	if ((pPoint->X < 0 || pPoint->X >= pSurface->Width) ||
		(pPoint->Y < 0 || pPoint->Y >= pSurface->Height)) {
		return 0x7BAE9A;
	}

	if (auto pResult = pSurface->Lock(pPoint->X, pPoint->Y))
	{
		if (pSurface->Get_Bytes_Per_Pixel() == 1) {
			R->EAX<int>(*reinterpret_cast<int*>(pResult));
			pSurface->Unlock();
			return 0x7BAE9A;
		}

		R->EAX<int>(*reinterpret_cast<int*>(pResult));
		pSurface->Unlock();
	}

	return 0x7BAE9A;
}

DEFINE_HOOK(0x48439A, CellClass_GetColourComponents, 0x5)
{
	GET(int, Distance, EAX);
	GET(LightSourceClass*, LS, ESI);

	GET_STACK(int*, Intensity, 0x44);
	GET_STACK(int*, Tint_Red, 0x54);
	GET_STACK(int*, Tint_Green, 0x58);
	GET_STACK(int*, Tint_Blue, 0x5C);

	const int RangeVisibilityFactor = 1000;
	const int RangeDistanceFactor = 1000;
	const int LightMultiplier = 1000;

	int LSEffect = (RangeVisibilityFactor * LS->LightVisibility - RangeDistanceFactor * Distance) / LS->LightVisibility;
	*Intensity += int(LSEffect * LS->LightIntensity / LightMultiplier);
	*Tint_Red += int(LSEffect * LS->LightTint.Red / LightMultiplier);
	*Tint_Green += int(LSEffect * LS->LightTint.Green / LightMultiplier);
	*Tint_Blue += int(LSEffect * LS->LightTint.Blue / LightMultiplier);

	return 0x484440;
}
*/
#ifdef _Dis
/*
 *  Custom area damage logic for DTA.
 *  Coded in a way that makes it easy to adapt for many uses,
 *  hence takes an ObjectClass pointer instead of an AnimClass pointer.
 *
 *  @author: Rampastring
*/
void DTA_DoAtomDamage(const ObjectClass* object_ptr, const int damageradius, const int rawdamage, const int damagepercentageatmaxrange, const bool createsmudges, const WarheadTypeClass* warhead)
{
	Cell cell = Coord_Cell(object_ptr->Center_Coord());

	int				distance;	          // Distance to unit.
	ObjectClass* object;			      // Working object pointer.
	ObjectClass* objects[128];	      // Maximum number of objects that can be damaged.
	int             distances[128];       // Distances of the objects that can be damaged.
	int             count = 0;            // Number of objects to damage.

	for (int x = -damageradius; x <= damageradius; x++)
	{
		for (int y = -damageradius; y <= damageradius; y++)
		{
			int xpos = cell.X + x;
			int ypos = cell.Y + y;

			/*
			**	If the potential damage cell is outside of the map bounds,
			**	then don't process it. This unusual check method ensures that
			**	damage won't wrap from one side of the map to the other.
			*/
			if ((unsigned)xpos > MAP_CELL_W)
			{
				continue;
			}
			if ((unsigned)ypos > MAP_CELL_H)
			{
				continue;
			}
			Cell tcell = XY_Cell(xpos, ypos);
			if (!Map.In_Radar(tcell)) continue;

			Coordinate tcellcoord = Cell_Coord(tcell);

			object = Map[tcell].Cell_Occupier();
			while (object)
			{
				if (!object->IsToDamage)
				{
					object->IsToDamage = true;
					objects[count] = object;

					if (object->What_Am_I() == RTTI_BUILDING)
					{
						// Find the cell of the building that is closest
						// to the explosion point and use that as the reference point for the distance

						BuildingClass* building = reinterpret_cast<BuildingClass*>(object);

						Cell* occupy = building->Class->Occupy_List();
						distances[count] = INT_MAX;

						while (occupy->X != REFRESH_EOL && occupy->Y != REFRESH_EOL)
						{
							Coordinate buildingcellcoord = building->Coord + Cell_Coord(*occupy, true) - Coordinate(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2, 0);
							distance = Distance(Cell_Coord(cell, true), buildingcellcoord);
							distances[count] = MinImpl(distance, distances[count]);
							occupy++;
						}
					}
					else
					{
						// For non-building objects, just check the distance directly
						distances[count] = Distance(Cell_Coord(cell, true), object->Center_Coord());
					}

					count++;
					if (count >= ARRAY_SIZE(objects)) break;
				}

				object = object->Next;
			}
			if (count >= ARRAY_SIZE(objects)) break;

		}
	}

	int maxdistance = damageradius * CELL_LEPTON_W;

	/*
	**	Sweep through the objects to be damaged and damage them.
	*/
	for (int index = 0; index < count; index++)
	{
		object = objects[index];

		object->IsToDamage = false;
		if (object->IsActive)
		{
			distance = distances[index];

			float distancemult = (float)distance / (float)maxdistance;
			if (distancemult > 1.0f)
				distancemult = 1.0f;

			if (object->IsDown && !object->IsInLimbo)
			{
				int percentDecrease = (100 - damagepercentageatmaxrange) * distancemult;
				int damage = rawdamage - ((percentDecrease * rawdamage) / 100);

				// We've taken the distance into account already
				object->Take_Damage(damage, 0, warhead, nullptr, false);
			}
		}
	}

}
#endif


DEFINE_HOOK(0x5206F9, InfantryClass_UpdateFiringState_AIDeploy, 0x8)
{
	GET(const FireError, nFireError, EAX);
	GET(InfantryClass*, pThis, EBP);
	GET_STACK(const int, nWeaponIdx, 0x10);

	DWORD Ret = 0x0;

	if (nFireError <= FireError::OK)
	{
		const auto pHouseOwner = pThis->GetOwningHouse();
		const auto pType = pThis->Type;
		const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pType);

		// Other hooks needed
		//DeployToFire.RequiresAIOnly

		if (pHouseOwner->IsHumanPlayer && (SessionGlobal.GameMode == GameMode::Campaign || pHouseOwner->PlayerControl))
		{
			const auto pTarget = pThis->Target;

			if (!pTarget
				|| ((pTarget->AbstractFlags & AbstractFlags::Object) == AbstractFlags::None)
				|| !pHouseOwner->IsAlliedWith(pTarget))
			{

				if (pType->Deployer && (((int)pThis->SequenceAnim) - 27 > 3))
				{
					const auto nDeployFireWeapon = pType->DeployFireWeapon == -1 ? pThis->SelectWeapon(pTarget) : pType->DeployFireWeapon;

					if (auto const pWeapon = pThis->GetWeapon(nDeployFireWeapon))
					{
						if (pWeapon->WeaponType && (pThis->GetFireError(pTarget, nDeployFireWeapon, pWeapon->WeaponType->AreaFire) == FireError::OK))
							pThis->PlayAnim(DoType::Deploy, true, false);
					}
				}
			}
		}

		Ret = 0x52078F;
	}
	else if (nFireError == FireError::ILLEGAL)
	{
		Ret = 0x520721;
	}
	else
	{
		if (nFireError == FireError::CLOAKED)
		{
			if (pThis->IsCloseEnough(pThis->Target, nWeaponIdx))
				pThis->Uncloak(false);

			Ret = 0x52094C;
		}

	}

	return Ret;
}

#ifdef ENABLE_CLR
DEFINE_HOOK(0x6BB9D2, PatcherLoader_Action, 0x6)
{
	if (AllocConsole())
	{
		CLR::Init();
		CLR::Load();
	}
	else
	{
		MessageBoxW(NULL, TEXT("Alloc console error"), TEXT("Phobos"), MB_OK);
	}

	return 0x0;
}
#endif


//static std::filesystem::path  g_target_executable_path;
//static std::wstring Ares_dll_Fullpath;

#ifdef ENABLE_CRT_HOOKS
DEFINE_HOOK(0x6BBFCE, _WinMain_InitPhobos_, 0x5)
{
	_set_controlfp(_RC_CHOP, _MCW_RC);
	fesetround(FE_TOWARDZERO);
	return 0x0;
}
#endif

#ifdef ENABLE_DLL_SYNC_FETCHRESOURCE
DEFINE_HOOK(0x4A3B4B, _YR_Fetch_Resource, 0x9)
{

	GET(LPCSTR, lpName, ECX);
	GET(LPCSTR, lpType, EDX);

	HMODULE hModule = Phobos::hInstance;
	if (HRSRC hResInfo = FindResource(hModule, lpName, lpType))
	{
		if (HGLOBAL hResData = LoadResource(hModule, hResInfo))
		{
			LockResource(hResData);
			R->EAX(hResData);
			return 0x4A3B73; //Resource locked and loaded (omg what a pun), return!
		}
	}

	return 0; //Nothing was found, try the game's own resources.
}
#endif

//DEFINE_HOOK(0x6BE1C2, _YR_ProgramEnd, 0x6)
//{
//	return 0x0;
//}

/*
#ifndef ENABLE_NEWHOOKS
DEFINE_HOOK(0x7C8E17, operator_new_AddExtraSize, 0x6)
{
	REF_STACK(size_t, nDataSize, 0x4);
	nDataSize += 0x4;
	return 0x0;
}
#endif

DEFINE_HOOK(0x7D5408, Game_strdup_replace, 0x5)
{
	GET_STACK(const char*, In, 0x4);

	char* str;
	char* p;
	int len = 0;

	while (In[len])
	{
		len++;
	}

	str = (char*)CRT::malloc(len + 1);
	p = str;

	while (*In)
	{
		*p++ = *In++;
	}

	*p = '\0';

	R->EAX(str);
	return 0x7D542E;
}

DEFINE_HOOK_AGAIN(0x777D71, WindowName_ApplyCustom, 0x5)
DEFINE_HOOK(0x777CCA, WindowName_ApplyCustom, 0x5)
{
	if (Phobos::AppName && strlen(Phobos::AppName) > 0)
		R->ESP(Phobos::AppName);

	return 0x0;
}

DEFINE_HOOK(0x7D107D, Game_msize_replace, 0x8)
{
	GET_STACK(void*, pVoid, 0x4);
	size_t nSize = 0;
	CRT::_lock(9);
	if (CRT::_sbh_find_block(pVoid))
	{
		DWORD nPtr = *reinterpret_cast<DWORD*>(pVoid);
		nSize = (nPtr - 4) - 9;
		CRT::_unlock(9);
	}
	else
	{
		CRT::_unlock(9);
		nSize = HeapSize(CRT_Heap, 0, pVoid);
	}

	R->EAX(nSize);
	return 0x7D10C1;
}
*/

//std::filesystem::path get_module_path(HMODULE module)
//{
//	WCHAR buf[4096];
//	return GetModuleFileNameW(module, buf, ARRAYSIZE(buf)) ? buf : std::filesystem::path();
//}


//DEFINE_HOOK(0x55D774, _YR_MainLoop_GameSpeed, 0xA)
//{ // We overwrite the instructions that force GameSpeed to 2 (GS4)
//	GameModeOptionsClass::Instance->GameSpeed = Phobos::Config::CampaignDefaultGameSpeed;
//	return 0x55D77E;
//}
//
//DEFINE_HOOK(0x55D78C, _YR_MainLoop_GameSpeed_B, 0x5)
//{// when speed control is off.
//	R->ECX(Phobos::Config::CampaignDefaultGameSpeed);
//	return 0x55D791;
//}

//DEFINE_HOOK(0x4F8A9B, HouseClass_AI_SuggestTeams, 0x6)
//{
//	GET(HouseClass*, pThis, ESI);
//	GET(int, suggestedCount, EAX);
//
//	if (suggestedCount <= 0){
//		Debug::Log("House [%s][Ratio : %d , AITriggersActive : %d ] Cannot get Any Team , please check your configuration!\n", pThis->get_ID() ,pThis->RatioAITriggerTeam, pThis->AITriggersActive);
//		return 0x4F8ABB;
//	}
//
//	return 0x4F8A9F;
//}

//DEFINE_HOOK(0x7272AE ,TriggerTypeClass_LoadFromINI_CountryName, 7)
//{
//	GET(TriggerTypeClass*, pThis, EBP);
//	GET(const char*, pData, ESI);
//	const int nParam = atoi(pData);
//
//	Debug::Log("TriggerTypeClass_LoadFromINI_CountryName[%s] %s - %d \n", pThis->ID, pData, nParam);
//
//	if ((nParam - 4475) > 7) {
//		R->EDX(HouseTypeClass::Array->GetItem(HouseTypeClass::FindIndexByIdAndName(pData)));
//		return 0x7272C1;
//	} else {
//		TriggerTypeExt::ExtMap.Find(pThis)->HouseParam = nParam;
//		return 0x7272A4;
//	}
//}

//#include <Ext/Cell/Body.h>

//TODO : another place to reset ?
// this one address not really convincing ,..
/*
DEFINE_HOOK(0x56C1D3, MapClass_RemoveCrate_Override, 7)
{
	GET(CellClass*, pThis, EBX);

	if(auto pExt = CellExtContainer::Instance.Find(pThis))
		pExt->NewPowerups = -1;

	return 0;
}

DEFINE_HOOK(0x56BFBE, MapClass_PlaceCrate_Override, 7)
{
	LEA_STACK(CellStruct*, pos, 0x18);
	GET_STACK(int, OverlayData, 0x1C);

	auto CellExt = CellExtContainer::Instance.Find(MapClass::Instance->GetCellAt(pos));

	if (!CellExt)
		return 0x0;

	//ugly XD
	//20 is random , dont occupy that
	//custom powerup
	const auto NewPowerUp = OverlayData > 20 ? (OverlayData - 20) : (-1);
	//original powerup
	OverlayData = OverlayData > 20 ? 20 : OverlayData;
	OverlayData = OverlayData == 19 ? 0 : OverlayData; //19 is empty replace it with money instead

	//18,6,15,13 absolute crate replace to speed
  //  OverlayData = OverlayData == 13 || OverlayData == 18 || OverlayData == 6 || OverlayData == 15 ? 10 : OverlayData;

	CellExt->AttachedToObject->OverlayData = (unsigned char)(OverlayData);
	CellExt->NewPowerups = NewPowerUp;

	Debug::Log("MapClass_PlaceCrate_Override NewPowerUps [%d] Original PowerUps[%d] cell NewPowerups [%d] \n", NewPowerUp, OverlayData, CellExt->NewPowerups);
	return 0x56BFFF; //return true;
}

DEFINE_HOOK(0x481ACE, CellHasPowerUp_Override, 5)
{
	GET(CellClass*, pThis, ESI);

	enum {
		keeproll = 0x481AD3, SpawnSpesific = 0x481B22
	};

	const auto CellExt = CellExtContainer::Instance.TryFind(pThis);

	if (CellExt && CellExt->NewPowerups > -1)
	{
		Debug::Log("CellHasPowerUp_Override Original PowerUps [%d] cell NewPowerups [%d] \n", pThis->OverlayData, CellExt->NewPowerups);

		R->EBX(PowerupEffects::Darkness);//force spawn Darkness
		return SpawnSpesific;
	} else if(pThis->OverlayData < 19u) {
	   R->EBX(pThis->OverlayData);
		return SpawnSpesific;
	}

	return keeproll;
}

DEFINE_HOOK(0x73844A, UnitClass_ReceiveDamage_PlaceCrate_override, 6)
{
	GET(CellStruct, Where, EAX);
	GET(UnitClass*, pthis, ESI);

	const auto CrateType = abs(TechnoTypeExt::ExtMap.Find(pthis->GetTechnoType())->CrateType);
	const auto Success = CrateStufs::Place_Crate(Where, (PowerupEffects)CrateType);
	Debug::Log("Unit[%s] to crate [%d] X [%d] Y[%d] succes [%d]\n", pthis->Type->ID, CrateType, Where.X, Where.Y, Success);
	return 0x738457;
}

DEFINE_HOOK(0x442215, BuildingTypeClass_Destroy_PlaceCrate_override, 7)
{
	GET(BuildingTypeClass*, pBldType, EDX);
	GET(BuildingClass *, Building, EBX);

	const auto CrateType = abs(TechnoTypeExt::ExtMap.Find((pBldType))->CrateType);
	const auto Success = CrateStufs::Place_Crate(Building->GetMapCoords(), (PowerupEffects)CrateType);
	Debug::Log("Building[%s] to crate [%d] succes ? [%d] \n", pBldType->ID, CrateType, Success);
	R->AL(Success);

	return 0x442226;
}

//Shroud easy to handle without breaking everything else
//jump goes to the end of the function because need to replace the animation
//for more fit with the stuffs
DEFINE_HOOK(481F87, CellClass_CrateCollected_Shroud_Override, 7)
{
	if (CrateTypeClass::Array.empty())
	{
		Debug::Log("CrateType is empty return 0 \n");
		return 0;
	}

	GET(TechnoClass*, Collector, EAX);

	bool pass = false;//return default

	if (auto const pHouse = Collector->Owner)
	{
		auto& CrateType = CrateTypeClass::Array;
		//accesing thru Powerups::Anim causing access violation crash
		auto Powerups_Animarray = reinterpret_cast<int*>(0x81DAD8);

		CellStruct BufferCellStruct = { 0,0 };
		BufferCellStruct.X = static_cast<short>(R->EDX());
		BufferCellStruct.Y = static_cast<short>(R->ECX());

		auto Cell = MapClass::Instance->TryGetCellAt(BufferCellStruct);
		auto height = MapClass::Instance->GetCellFloorHeight(CellClass::Cell2Coord(BufferCellStruct));
		auto animCoord = CellClass::Cell2Coord(BufferCellStruct, 200 + height);

		auto& Randomizer = ScenarioClass::Instance->Random;
		auto& Rules = RulesClass::Instance;
		auto dice = Randomizer.RandomRanged(0, CrateType.size() - 1); //Pick  random from array
		auto pickedupDice = Randomizer.RandomRanged(0, Rules->CrateMaximum);

		//chance 0 cause crash fix
		bool allowspawn = abs(CrateType[dice]->Chance.Get()) < pickedupDice && abs(CrateType[dice]->Chance.Get()) > 0;

		if (CellExt::ExtMap.Find(Cell)->NewPowerups > -1)
		{
			dice = abs(CellExt::ExtMap.Find(Cell)->NewPowerups);
			dice = dice - 1;
			Debug::Log("Crate type Check cell which to spawn [%d]\n", dice);
			allowspawn = true; //forced
		}
		bool LandTypeEligible = false;
		auto type = CrateType[dice]->Type.Get();
		auto pType = CrateType[dice]->Anim.Get();
		auto pSound = CrateType[dice]->Sound.Get();
		auto pEva = CrateType[dice]->Eva.Get();
		bool NotObserver = !pHouse->IsObserver() && !pHouse->IsPlayerObserver();
		auto isWater = Cell->LandType == LandType::Water && Cell->Tile_Is_Water() && Cell->Tile_Is_Wet();
		LandTypeEligible = (CrateType[dice]->AllowWater && isWater) || !isWater;

		if (allowspawn && LandTypeEligible)
		{
			switch (type)
			{
			case 1: //Super Weapon
			{
				auto SWIDX = CrateType[dice]->Super.Get();
				auto pSuper = pHouse->Supers.GetItem(SWIDX);

				if (CrateType[dice]->SuperGrant.Get())
				{
					if (pSuper->Grant(true, NotObserver, false))
					{
						if (NotObserver && pHouse == HouseClass::Player)
						{
							if (MouseClass::Instance->AddCameo(AbstractType::Special, SWIDX))
								MouseClass::Instance->RepaintSidebar(1);
						}
					}
				}
				else
				{
					//Abused By AI ?
					pSuper->IsCharged = true;
					pSuper->Launch(Cell->MapCoords, true);

				}
				pass = true;
			}
			break;
			case 2: //Weapon
			{
				auto Weapon = CrateType[dice]->WeaponType.Get();
				if (Weapon && (!Weapon->Warhead->MindControl || !Weapon->LimboLaunch))
				{
					if (auto pBulletC = Weapon->Projectile->CreateBullet(Cell, Collector, Weapon->Damage, Weapon->Warhead, Weapon->Speed, Weapon->Bright))
					{
						pBulletC->SetWeaponType(Weapon);
						pBulletC->SetLocation(CellClass::Cell2Coord(BufferCellStruct));
						if (Weapon->Projectile->ShrapnelCount > 0)
							pBulletC->Shrapnel();
						pBulletC->Detonate(CellClass::Cell2Coord(BufferCellStruct));
						pBulletC->Remove();
						pBulletC->UnInit();

						pass = true;
					}
				}
			}
			break;
			case 3: //case 3 is overrided reshroud
			{
				if (!pType)
					pType = AnimTypeClass::Array->GetItem(Powerups_Animarray[7]);
				MapClass::Instance->Reshroud(pHouse);
				pass = true;
			}
			break;
			case 4: //random Unit
			{
				if (auto Unit = CrateType[dice]->Unit.GetElements())
				{
					auto const pUnit = static_cast<TechnoClass*>(Unit.at(Randomizer.Random() % Unit.size())->CreateObject(pHouse));

					auto TrygeteligibleArea = TechnoExt::GetPutLocation(CellClass::Cell2Coord(BufferCellStruct), 6); //try to not get stuck
					auto facing = static_cast<short>(Randomizer.RandomRanged(0, 255));
					++Unsorted::IKnowWhatImDoing;
					auto succes = pUnit->Put(TrygeteligibleArea, facing);
					--Unsorted::IKnowWhatImDoing;

					if (!succes)
					{
						GameDelete(pUnit);
					}
					else
					{
						if (!pUnit->InLimbo)
						{
							pUnit->NeedsRedraw = true;
							pUnit->Update();
							pUnit->QueueMission(Mission::Guard, 1);
							pUnit->NextMission();
						}

						if (NotObserver)
							pHouse->RecheckTechTree = true;

						pass = true;
					}
				}
			}
			break;
			case 5: //Money
			{
				if (!pType)
					pType = AnimTypeClass::Array->GetItem(Powerups_Animarray[0]);
				if (!pSound)
					pSound = Rules->CrateMoneySound;

				auto MoneyMin = abs(CrateType[dice]->MoneyMin.Get());
				auto MoneyMax = abs(CrateType[dice]->MoneyMax.Get());
				if (MoneyMax > MoneyMin)
				{
					pHouse->GiveMoney(Randomizer.RandomRanged(MoneyMin, MoneyMax));
					pass = true;
				}
			}
			break;
			case 6: //Heall All
			{
				if (!pType)
					pType = AnimTypeClass::Array->GetItem(Powerups_Animarray[2]);
				if (!pSound)
					pSound = Rules->HealCrateSound;

				for (auto const& pTechno : *TechnoClass::Array)
				{
					bool Allowed = !pTechno->InLimbo || !pTechno->TemporalTargetingMe || !pTechno->IsSinking || !pTechno->IsCrashing;

					if (pTechno->Owner == pHouse && pTechno->IsAlive && Allowed)
					{
						auto damage = (pTechno->GetTechnoType()->Strength * pTechno->Health) * -1;

						pTechno->ReceiveDamage(&damage, 0, Rules->C4Warhead, Collector, true, true, pHouse);
						pTechno->Flash(100);
					}
				}

				pass = true;
			}
			break;
			default:
				break;
			}

			if (pass)
			{
				if (pType)
					if (auto anim = GameCreate<AnimClass>(pType, animCoord))
						anim->Owner = pHouse;

				if (pSound)
					VocClass::PlayAt(pSound, animCoord, nullptr);

				if (pEva && pHouse->ControlledByCurrentPlayer() && NotObserver)
					VoxClass::PlayAtPos(pEva, &animCoord);
			}
		}
		else
		{
			return 0x481AD3; //reroll it instead
			//reroll may cause game to freeze if only Shroud crate is activated (chance > 0)
			//need atlast 2 (Shroud + other)
			//Possible doubling Score for multiplayer play which is undesirable side affects

			// Send money instead
			//if (!pType)
			//	pType = AnimTypeClass::Array->GetItem(Powerups_Animarray[0]);
			//	  if (!pSound)
			//		  pSound = Rules->CrateMoneySound;
			//
			//	  pHouse->GiveMoney(Rules->SoloCrateMoney);
			//
			//	  pass = true;
		}

	}

	return pass ? 0x483389 : 0x0;
}
*/

//DEFINE_HOOK(0x4C762A, EventClass_Execute_Idle, 0x6)
//{
//	GET(TechnoClass*, pTech, ESI);
//
//	if(pTech->GetHeight() > 0 && !pTech->IsInAir()){
//		if (auto pBld = pTech->GetCell()->GetBuilding()) {
//			if (BuildingTypeExtContainer::Instance.Find(pBld->Type)->Firestorm_Wall)
//				// this can be possible fix
//				// but the satter not really desirable
//				// which make techno still stand on top of the firestrom wall
//				// idk , need more deep fix i guess
//				pTech->Scatter(pTech->Location, true, true);
//		}
//	}
//
//	return 0x0;
//}

//int AdjustDamageWithArmor(int damage, WarheadTypeClass* pWH, ObjectClass* pTarget, int distance) {
//	return MapClass::ModifyDamage(damage, pWH, TechnoExtData::GetArmor(pTarget), distance);
//}

//handled on ObjectClass_ReceiveDamage_Handled
//DEFINE_HOOK(0x5F53F7, ObjectClass_ReceiveDamage_Adjust, 0x5)
//{
//	GET(ObjectClass*, pThis, ESI);
//	GET(int, distance, EAX);
//	GET(int*, pDamage, EDI);
//	GET_STACK(WarheadTypeClass*, pWH, 0x24 + 0xC);
//
//	R->EAX(AdjustDamageWithArmor(*pDamage, pWH, pThis, distance));
//	return 0x5F5414;
//}

// for berzerk , unused 
//DEFINE_HOOK(0x701D4D, TechnoClass_ReceiveDamage_Adjust, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//	GET(WarheadTypeClass*, pWH, EBP);
//	GET(int*, pDamage, EBX);
//	R->EAX(AdjustDamageWithArmor(*pDamage , pWH , pThis , 0));
//	return 0x701D69;
//}


//DEFINE_HOOK(0x72593E, Game_InvalidatePointers_AllAbstractptrVectorCrash, 0x5)
//{
//	GET(int, AllAbsCount, EAX);
//	GET(AbstractClass*, pTarget, ESI);
//	GET(bool, bRemoved, EDI);
//
//	for (int i = 0; i < AllAbsCount; ++i) {
//		AbstractClass::Array->Items[i]->PointerExpired(pTarget, bRemoved);
//	}
//
//	return 0x725961;
//}


//DEFINE_HOOK(0x4191AF, AircraftClass_ReceiveMessage_RunAway_StackOverflow, 0x6)
//{
//	GET(AircraftClass*, pThis, ESI);
//	GET(TechnoClass*, pSender, EDI);
//	GET_STACK(AbstractClass*, last, 0x1C);
//	GET(RadioCommand, command, EBP);
//
//	pThis->QueueMission(Mission::Move, false);
//	const auto lZ = pThis->GoodLandingZone_();
//	pThis->SetDestination(lZ, true);
//	pThis->SendToFirstLink(RadioCommand::NotifyUnlink);
//	R->EAX(pThis->FootClass::ReceiveCommand(pSender, command, last));
//	return 0x4191EB;
//}

//DEFINE_HOOK(0x4190B0, AircraftClass_ReceiveMessage_RunAway_StackOverflow_entryPointLog, 0x5)
//{
//	GET(AircraftClass*, pThis, ECX);
//	GET_STACK(RadioCommand, command, 0x8);
//	GET_STACK(DWORD, address, 0x0);
//
//	if (command == RadioCommand::NotifyLeave)
//		Debug::Log("Aircraft[%s - 0x%x ,Receive NotifyLeave RadioCommand From[0x%x]\n", pThis->Type->ID, pThis, address);
//
//	return 0x0;
//}

//DEFINE_HOOK(0x4F9A56, HouseClass_IsAlliedWithOffendingPtrIsNotHouse, 0x6)
//{
//	GET(HouseClass*, pThis, EAX);
//	GET(HouseClass*, pOffender, ECX);
//
//	if (!pOffender || VTable::Get(pOffender) != HouseClass::vtable){
//		GET_STACK(unsigned int, callerAddress, 0x0);
//		Debug::Log("[%08x]Trying to get Ally from Invalid House Pointer[%08x]!\n", callerAddress, pOffender);
//		return 0x4F9A8A;
//	}
//
//	return pThis == pOffender ? 0x4F9A5E : 0x4F9A63;
//}

//DEFINE_HOOK(0x4CED12 , FlyLocomotionClass_UpdateLanding_Finish, 8)
//{
//	if (R->EDI<int>() <= 104)
//		return 0;
//
//	R->ESI<FlyLocomotionClass*>()->IsTakingOff = false;
//	R->Stack(0x13, 1);
//
//	return 0x4CED2D;
//}

//int __fastcall UnitClass_MI_Open_(UnitClass* pThis)
//{
//	return 450;
//}
//
//DEFINE_JUMP(VTABLE , 0x7F5EC4 , GET_OFFSET(UnitClass_MI_Open_))
//
//int __fastcall InfantryClass_MI_Open_(InfantryClass* pThis)
//{
//	return 450;
//}
//
//DEFINE_JUMP(VTABLE , 0x7EB2AC , GET_OFFSET(InfantryClass_MI_Open_))

//DEFINE_HOOK(0x4DEFF1 , FootClass_FindNearestBuildingOfType_OverFlowFix, 8)
//{
//	GET(FootClass*, pFoot, EDI);
//	GET(BuildingClass*, pDock, ESI);
//	GET(int*, pDest, EBX);
//
//	const int distance = pFoot ? int(pFoot->GetCoords().DistanceFrom_(pDock->GetCoords())) : 0;
//
//	if (*pDest == -1 || distance > *pDest || pFoot->IsTeamLeader)
//	{
//		R->Stack(0x14, pFoot);
//		*pDest = distance;
//	}
//
//	return 0x4DF014;
//}

//DEFINE_HOOK(0x40A463, AudioDriverStart_NonamePrefill_PeekBuffer, 0x5)
//{
//	GET(AudioDriverChannelTag*, pTag, ECX);
//
//	if (!AudioDriverChannelTag::noname_prefil(pTag, pTag->dwBufferBytes)) {
//		return 0x40A5CB //0x40A470
//			;
//	}
//
//	return 0x40A5CB ;
//}

//called always 0040276a
//DEFINE_HOOK(0x40A340, AudioDriverStart_Log, 0x9)
//{
//	GET(AudioChannelTag*, pTag, ECX);
//	GET_STACK(uintptr_t, callerAddress, 0x0);
//
//	Debug::Log(__FUNCTION__" [%x] , Caller: %08x \n", pTag ,callerAddress);
//	return 0x0;
//}
//
//DEFINE_HOOK(0x40A470, AudioDriverStart_Log_Failed, 0x5)
//{
//	GET(AudioDriverChannelTag* , pTag , EBX);
//
//	Debug::Log(__FUNCTION__" [%x]\n", pTag);
//	return 0x0;
//}

//int InfantryClass_Mission_Harvest(InfantryClass* pThis)
//{
//	if (pThis->Type->Slaved)
//	{
//		if (pThis->Type->Storage)
//		{
//			auto pCell = pThis->GetCell();
//			if (!pCell->HasTiberium() || pThis->GetStoragePercentage() == 1.0)
//			{
//				pThis->PlayAnim(DoType::Ready);
//				pThis->QueueMission(Mission::Guard, false);
//				return 1;
//			}
//			else
//			{
//				if (pThis->SequenceAnim != DoType::Shovel)
//				{
//					pThis->PlayAnim(DoType::Shovel);
//				}
//
//				auto tiberium = pCell->GetContainedTiberiumIndex();
//				auto curamount = pThis->Type->Storage - pThis->Tiberium.GetTotalAmount();
//				const auto reduceamount = curamount <= 1.0 ? curamount : 1.0;
//
//				pCell->ReduceTiberium(reduceamount);
//				pThis->Tiberium.AddAmount(reduceamount, tiberium);
//				return pThis->Type->HarvestRate;
//			}
//		}
//	}
//
//	if (!pThis->Destination)
//		return 1;
//
//	auto pCell = pThis->GetCell();
//	if (pThis->Type->ResourceGatherer && pThis->GetStoragePercentage() < 1.0 && pCell->HasTiberium())
//	{
//		auto tiberium = pCell->GetContainedTiberiumIndex();
//	}
//}

//DEFINE_HOOK(0x54C531, JumpjetLocomotionClass_State3_DeployToLand_Convert, 0x6)
//{
//	GET(CellClass*, pCell, EAX);
//	GET(FootClass*, pLinked, EDI);
//
//	if (auto pUnit = specific_cast<UnitClass*>(pLinked)) {
//
//		if (!pUnit->Type->DeployToLand)
//			return 0x0;
//
//		if(MapClass::Instance->IsWithinUsableArea(pCell->MapCoords , true)){
//			if (auto pConvert = TechnoTypeExtContainer::Instance.Find(pUnit->Type)->Convert_Deploy) {
//				if (!MapClass::Instance->CanMoveHere(pCell->MapCoords, 1, 1, pConvert->SpeedType, -1, pConvert->MovementZone, -1, false, false))
//					return 0x54C53B;
//				else
//					return 0x54C544;
//			}
//		}
//	}
//
//	return 0x0;
//}

//DEFINE_HOOK(0x6F917D, TechnoClass_GreatestThreat_Occupy, 0x8)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (pThis->WhatAmI() == InfantryClass::AbsID && AresGarrisonedIn(pThis)) {
//		return 0x6F9193;
//	}
//
//	return 0x0;
//}

// Sink sound //4DAC7B
//todo :

// https://bugs.launchpad.net/ares/+bug/1840387
// https://bugs.launchpad.net/ares/+bug/1777260
// https://bugs.launchpad.net/ares/+bug/1324156
// https://bugs.launchpad.net/ares/+bug/1911093
// https://blueprints.launchpad.net/ares/+spec/set-veterancy-of-paradropped-units
// https://bugs.launchpad.net/ares/+bug/1525515
// https://bugs.launchpad.net/ares/+bug/896353

//700E47
//740031
//700DA8
// https://bugs.launchpad.net/ares/+bug/1384794

// TODO :
//  - Ice stuffs using WW pointer heap logic
//#include <ExtraHeaders/Placeholders/ExtraAudio.h>

//DEFINE_HOOK(0x407B60, AudioStreamer_Open_LogFileName, 0x5)
//{
//	GET(const char*, pName, EDX);
//	GET_STACK(uintptr_t, callerAddress, 0x0);
//
//	Debug::Log(__FUNCTION__"[%s] Caller: %08x \n", pName , callerAddress);
//
//	return 0x0;
//}

//7F5DA0 unit
//7EB188 inf
//7E3FEC bld
//DEFINE_HOOK(0x41BE80, ObjectClass_DrawRadialIndicator, 0x3)
//{
//	GET(ObjectClass*, pThis, ECX);
//	GET_STACK(int, var, 0x4);
//	ObjectClassExt::_DrawFootRadialIndicator(pThis ,0,var);
//	return 0x0;
//}

//DEFINE_JUMP(LJMP, 0x41BE80 ,GET_OFFSET(ObjectClassExt::_DrawFootRadialIndicator))

//https://bugs.launchpad.net/ares/+bug/1577493
// stack 0x8 seems occupied by something else ?
//DEFINE_HOOK(0x4684FF, BulletClass_InvalidatePointer_CloakOwner, 0xA)
//{
//	GET(BulletClass*, pThis, ESI);
//	GET_STACK(bool, bRemove, 0x8);
//	GET(AbstractClass*, pTarget, EDI);
//	GET(TechnoClass*, pOwner, EAX);
//   //nope , the third ags , seems not used consistenly , it can cause dangling pointer
//	if (bRemove && pOwner == pTarget)
//		pThis->Owner = nullptr;
//
//	return 0x468509;
//}
