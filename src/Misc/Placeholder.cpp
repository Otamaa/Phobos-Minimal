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
							distances[count] = std::min(distance, distances[count]);
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

// Aircraft Decloak to fire ?
DEFINE_HOOK(0x6FCA30, TechnoClass_GetFireError_DecloakToFire, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(const WeaponTypeClass*, pWeapon, EBX);

	auto const pTransporter = pThis->Transporter;

	if (pTransporter && (pTransporter->CloakState != CloakState::Uncloaked))
		return 0x6FCA4F;

	if (!static_cast<int>(pThis->CloakState))
		return 7326302;

	if (!pWeapon->DecloakToFire && pThis->WhatAmI() != AbstractType::Aircraft)
		return 0x6FCA4F;

	return pThis->CloakState == CloakState::Cloaked ? 0x6FCA4F : 0x6FCA5E;
}
