#include "Anim.h"

#include <WarheadTypeClass.h>
#include <AnimClass.h>
#include <CellClass.h>
#include <SmudgeTypeClass.h>
#include <MapClass.h>
#include <ScenarioClass.h>
#include <BuildingClass.h>
#include <SmudgeClass.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/Anim/Body.h>

void Vinivera::ApplyAreaDamage(AnimClass* pThis)
{
	const auto object_ptr = pThis;
	const int damageradius = 0;
	const int rawdamage = 0;
	const int damagepercentageatmaxrange = 0;
	const int smudgechance = 0;
	const int flamechance = 0;
	WarheadTypeClass* warhead = nullptr;
	int unitDamageMultiplier = 0;
	auto cell = CellClass::Coord2Cell(object_ptr->GetCoords());

	int				distance;	          // Distance to unit.
	ObjectClass*	object;			      // Working object pointer.
	ObjectClass*	objects[128];	      // Maximum number of objects that can be damaged.
	int             distances[128];       // Distances of the objects that can be damaged.
	int             count = 0;            // Number of objects to damage.

	// If we should create smudges,
	// gather all valid smudgetypes for it
	SmudgeTypeClass* smudgetypes[6];
	int smudgetypecount = 0;

	if (smudgechance > 0) {

		for (int i = 0; i < SmudgeTypeClass::Array->Count && smudgetypecount < ARRAY_SIZE(smudgetypes); i++)
		{
			SmudgeTypeClass* smudgetype = SmudgeTypeClass::Array->Items[i];

			if (smudgetype->Burn) {
				smudgetypes[smudgetypecount] = smudgetype;
				smudgetypecount++;
			}
		}
	}

	for (int x = -damageradius; x <= damageradius; x++) {
		for (int y = -damageradius; y <= damageradius; y++) {
			int xpos = cell.X + x;
			int ypos = cell.Y + y;

			auto tcell = CellStruct(xpos, ypos);

			if (!MapClass::Instance->IsValidCell(tcell)) continue;

			Coordinate tcellcoord = CellClass::Cell2Coord(tcell);

			object = MapClass::Instance->GetCellAt(tcell)->FirstObject;
			while (object)
			{
				if (!object->IsToDamage)
				{
					object->IsToDamage = true;
					objects[count] = object;

					if (object->WhatAmI() == BuildingClass::AbsID)
					{
						// Find the cell of the building that is closest
						// to the explosion point and use that as the reference point for the distance

						BuildingClass* building = reinterpret_cast<BuildingClass*>(object);

						auto occupy = building->Type->GetFoundationData(false);
						distances[count] = INT_MAX;

						while (occupy->X != 32767 && occupy->Y != 32767)
						{
							Coordinate buildingcellcoord = building->Location + CellClass::Cell2Coord(*occupy) - Coordinate(256 / 2, 256 / 2, 0);
							distance = (int)CellClass::Cell2Coord(cell).DistanceFrom(buildingcellcoord);
							distances[count] = std::min(distance, distances[count]);
							occupy++;
						}
					}
					else
					{
						// For non-building objects, just check the distance directly
						distances[count] = (int)CellClass::Cell2Coord(cell).DistanceFrom(object->GetCoords());
					}

					count++;
					if (count >= ARRAY_SIZE(objects)) break;
				}

				object = object->NextObject;
			}
			if (count >= ARRAY_SIZE(objects)) break;

			if (smudgechance > 0 && smudgetypecount > 0)
			{

				if (smudgechance >= 100 || ScenarioClass::Instance->Random.RandomRanged(0, 100) < smudgechance)
				{
					// Create a smudge on the cell
					int smudgeindex = ScenarioClass::Instance->Random.RandomFromMax(smudgetypecount - 1);

					SmudgeTypeClass* smudgetype = smudgetypes[smudgeindex];
					GameCreate<SmudgeClass>(smudgetype, tcellcoord);
				}
			}

			if (flamechance > 0 && RulesClass::Instance->OnFire.Count > 0)
			{

				if (flamechance >= 100 || ScenarioClass::Instance->Random.RandomFromMax(100) < flamechance)
				{
					// Create a flame anim on the cell
					AnimTypeClass* animtype = RulesClass::Instance->OnFire.Items[ScenarioClass::Instance->Random.RandomFromMax(RulesClass::Instance->OnFire.Count -1)];
					GameCreate<AnimClass>(animtype, tcellcoord);
				}
			}
		}
	}

	int maxdistance = damageradius * 256;

	auto pTechno = AnimExtData::GetTechnoInvoker(pThis);
	TechnoClass* const pTechOwner = AnimExtData::GetTechnoInvoker(pThis);
	auto const pOwner = !pThis->Owner && pTechOwner ? pTechOwner->Owner : pThis->Owner;

	/*
	**	Sweep through the objects to be damaged and damage them.
	*/
	for (int index = 0; index < count; index++)
	{
		object = objects[index];

		object->IsToDamage = false;
		if (object->IsAlive)
		{
			distance = distances[index];

			float distancemult = (float)distance / (float)maxdistance;
			if (distancemult > 1.0f)
				distancemult = 1.0f;

			if (object->IsOnMap && !object->InLimbo)
			{
				int percentDecrease = (100 - damagepercentageatmaxrange) * distancemult;
				int damage = rawdamage - ((percentDecrease * rawdamage) / 100);

				// Adjust damage against units if necessary
				if (unitDamageMultiplier != 100 && object->AbstractFlags & AbstractFlags::Foot)
				{
					damage = (damage * unitDamageMultiplier) / 100;
				}

				// We've taken the distance into account already
				object->ReceiveDamage(&damage, 0, warhead, pTechno, false, false , pOwner);
			}
		}
	}
	auto const pWHExt = WarheadTypeExtContainer::Instance.Find(warhead);

	if (!pWHExt->ShakeIsLocal || TacticalClass::Instance->IsCoordsToClientVisible(object_ptr->GetCoords()))
	{

		if (warhead->ShakeXhi || warhead->ShakeXlo)
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeX, Random2Class::NonCriticalRandomNumber->RandomRanged(warhead->ShakeXhi, warhead->ShakeXlo), pWHExt->Shake_UseAlternativeCalculation);

		if (warhead->ShakeYhi || warhead->ShakeYlo)
			GeneralUtils::CalculateShakeVal(GScreenClass::Instance->ScreenShakeY, Random2Class::NonCriticalRandomNumber->RandomRanged(warhead->ShakeYhi, warhead->ShakeYlo), pWHExt->Shake_UseAlternativeCalculation);
	}
}
