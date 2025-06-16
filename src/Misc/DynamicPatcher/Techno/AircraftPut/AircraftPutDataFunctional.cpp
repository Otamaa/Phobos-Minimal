#include "AircraftPutDataFunctional.h"
#include <Ext/Rules/Body.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include <Utilities/Helpers.h>

void AircraftPutDataFunctional::OnPut(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt, CoordStruct* pCoord)
{
	if (!pExt->AttachedToObject->Owner || pExt->AttachedToObject->WhatAmI() != AircraftClass::AbsID)
		return;

	auto const pTechno = (AircraftClass*)pExt->AttachedToObject;

	if (!pTechno->Spawned)
	{
		auto const pType = pTechno->Type;
		auto const Iter = make_iterator(RulesClass::Instance->PadAircraft);

		if (!Iter.empty() && Iter.contains(pType))
		{
			bool const bRemoveIfNoDock = RemoveIfNoDock(pTypeExt->MyPutData);
			// remove extra pad Aircraft
			if (pType->AirportBound && bRemoveIfNoDock)
			{
				int const count = Helpers_DP::CountAircraft(pTechno->Owner,Iter);
				if (pTechno->Owner->AirportDocks <= 0 ||
					pTechno->Owner->AirportDocks < count)
				{
					pTechno->Owner->TransactMoney(pType->Cost);
					pTechno->Limbo();
					//Debug::LogInfo(__FUNCTION__" Called ");
					TechnoExtData::HandleRemove(pTechno , nullptr , false , false);

					return;
				}
			}

			auto const nOffset = GetOffset(pTypeExt->MyPutData);

			// move location
			if (!pExt->aircraftPutOffsetFlag && nOffset.IsValid())
			{
				pExt->aircraftPutOffsetFlag = true;
				pExt->aircraftPutOffset = true;
				if (!IsForceOffset(pTypeExt->MyPutData))
				{
					// check Building has Helipad
					if (auto pCell = MapClass::Instance->TryGetCellAt(*pCoord))
					{
						auto const pBuilding = pCell->GetBuilding();
						if (pBuilding && pBuilding->Type->Helipad)
							pExt->aircraftPutOffset = false;
					}
				}

				if (pExt->aircraftPutOffset)
					*pCoord += nOffset;

			}
		}
	}
}

void AircraftPutDataFunctional::AI(TechnoExtData* pExt, TechnoTypeExtData* pTypeExt)
{
	if (!pExt->aircraftPutOffset)
		return;

	auto const nOffset = GetOffset(pTypeExt->MyPutData);

	if (nOffset.IsValid())
	{
		pExt->aircraftPutOffset = false;
		auto const pTechno = pExt->AttachedToObject;

		CoordStruct location = pTechno->Location;
		CoordStruct pos = location + nOffset;
		pTechno->SetLocation(pos);

		if (auto const pCell = MapClass::Instance->TryGetCellAt(location))
			pTechno->SetDestination(pCell, true);

		pTechno->QueueMission(Mission::Enter, false);
	}
}

CoordStruct AircraftPutDataFunctional::GetOffset(AircraftPutData& nData)
{
	auto const nOffset = nData.PosOffset.Get(RulesExtData::Instance()->MyPutData.PosOffset.Get());

	return { nOffset.X * 256 ,nOffset.Y * 256 , nOffset.Z * 256 };
}

bool AircraftPutDataFunctional::IsForceOffset(AircraftPutData& nData)
{
	return nData.ForceOffset.Get(RulesExtData::Instance()->MyPutData.ForceOffset.Get());
}

bool AircraftPutDataFunctional::RemoveIfNoDock(AircraftPutData& nData)
{
	return nData.RemoveIfNoDocks.Get(RulesExtData::Instance()->MyPutData.RemoveIfNoDocks.Get());
}
