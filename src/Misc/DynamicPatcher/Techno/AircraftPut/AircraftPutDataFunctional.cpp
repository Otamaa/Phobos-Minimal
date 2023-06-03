#include "AircraftPutDataFunctional.h"
#include <Ext/Rules/Body.h>
#include <Misc/DynamicPatcher/Helpers/Helpers.h>
#include <Utilities/Helpers.h>

void AircraftPutDataFunctional::OnPut(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt, CoordStruct* pCoord)
{
	if (!pExt->Get()->Owner)
		return;

	if (!Is_Aircraft(pExt->Get()))
		return;

	auto const pTechno = (AircraftClass*)pExt->Get();

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
					TechnoExt::HandleRemove(pTechno , nullptr , false , false);

					return;
				}
			}

			auto const nOffset = GetOffset(pTypeExt->MyPutData);

			// move location
			if (!pExt->aircraftPutOffsetFlag && nOffset)
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

void AircraftPutDataFunctional::AI(TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pExt || !pExt->aircraftPutOffset)
		return;

	if (auto const nOffset = GetOffset(pTypeExt->MyPutData))
	{
		pExt->aircraftPutOffset = false;
		auto const pTechno = pExt->Get();

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
	auto const nOffset = nData.PosOffset.Get(RulesExt::Global()->MyPutData.PosOffset.Get());

	return { nOffset.X * 256 ,nOffset.Y * 256 , nOffset.Z * 256 };
}

bool AircraftPutDataFunctional::IsForceOffset(AircraftPutData& nData)
{
	return nData.ForceOffset.Get(RulesExt::Global()->MyPutData.ForceOffset.Get());
}

bool AircraftPutDataFunctional::RemoveIfNoDock(AircraftPutData& nData)
{
	return nData.RemoveIfNoDocks.Get(RulesExt::Global()->MyPutData.RemoveIfNoDocks.Get());
}
