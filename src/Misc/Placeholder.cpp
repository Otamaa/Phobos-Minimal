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
//	if (isWater && RulesExt::Global()->Crate_LandOnly.Get())
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
//			TechnoExt::GetCurrentSpeedMultiplier(pLinked);
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
//			TechnoExt::GetCurrentSpeedMultiplier(pLinked);
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
			auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH);
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
					int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, pTargetType->Armor, 0);

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
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		// Other hooks needed
		//DeployToFire.RequiresAIOnly

		if (pHouseOwner->CurrentPlayer && (SessionGlobal.GameMode == GameMode::Campaign || pHouseOwner->PlayerControl))
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


#ifdef Ares_ExperimentalHooks
#include "ViniferaStyle_Hooks.h"

DECLARE_PATCH(Ares_RecalculateStats_intercept_Armor)
{
	GET_REGISTER_STATIC_TYPE(TechnoClass*, pThis, edi);
	GET_REGISTER_STATIC_TYPE(DWORD, pTechnoExt, ecx);
	static double cur = *reinterpret_cast<double*>(pTechnoExt + 0x88);
	Debug::Log("Ares_CellClass_CrateBeingCollected_Armor2 GetCurrentMult for [%s] [%fl] \n", pThis->get_ID(), cur);
	static uintptr_t Ares_RecalculateStats_intercept_ret = ((Phobos::AresBaseAddress + (uintptr_t)0x46C10));
	_asm {mov ecx, pTechnoExt}//thiscall !
	JMP_REG(eax, Ares_RecalculateStats_intercept_ret);
}

DECLARE_PATCH(Ares_RecalculateStats_intercept_FP)
{
	GET_REGISTER_STATIC_TYPE(TechnoClass*, pThis, edi);
	GET_REGISTER_STATIC_TYPE(DWORD, pTechnoExt, ecx);
	static double cur = *reinterpret_cast<double*>(pTechnoExt + 0x80);
	Debug::Log("Ares_CellClass_CrateBeingCollected_FirePower2 GetCurrentMult for [%s] [%fl] \n", pThis->get_ID(), cur);
	static uintptr_t Ares_RecalculateStats_intercept_ret = ((Phobos::AresBaseAddress + (uintptr_t)0x46C10));
	_asm {mov ecx, pTechnoExt}//thiscall !
	JMP_REG(eax, Ares_RecalculateStats_intercept_ret);
}

DECLARE_PATCH(Ares_RecalculateStats_intercept_Speed)
{
	GET_REGISTER_STATIC_TYPE(TechnoClass*, pThis, esi);
	GET_REGISTER_STATIC_TYPE(DWORD, pTechnoExt, ecx);
	static double cur = *reinterpret_cast<double*>(pTechnoExt + 0x90);
	Debug::Log("CellClass_CrateBeingCollected_Speed2 GetCurrentMult for [%s] [%fl] \n", pThis->get_ID(), cur);
	static uintptr_t Ares_RecalculateStats_intercept_ret = ((Phobos::AresBaseAddress + (uintptr_t)0x46C10));
	_asm {mov ecx, pTechnoExt}//thiscall !
	JMP_REG(eax, Ares_RecalculateStats_intercept_ret);
}
#endif


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
