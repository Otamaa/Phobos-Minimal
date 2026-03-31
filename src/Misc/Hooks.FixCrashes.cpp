#include <Ext/TechnoType/Body.h>


ASMJIT_PATCH(0x5D6BF1, MultiplayerGameMode_SetBaseSpawnCell_CheckAvail, 0x5)
{
	struct ScenStruct {
		DynamicVectorClass<CellStruct> CellVector;
	};

	GET(ScenStruct*, pScenStruct, EAX);

	for (int i = 0; i < HouseClass::Array->Count; ++i)
	{
		auto pHouse = HouseClass::Array->Items[i];

		if (!pHouse->Type->MultiplayPassive)
		{
			if (pHouse->StartingPoint != -2)
			{
				const auto HouseID = pHouse->get_ID();

				// you dont want to read out of bound array here,..
				if ((size_t)pHouse->StartingPoint < pScenStruct->CellVector.size())
				{
					const auto& Cell = pScenStruct->CellVector.Items[pHouse->StartingPoint];
					Debug::LogInfo("SetBaseSpawnCellFor[{} at {} with [{} - {}]", HouseID, pHouse->StartingPoint, Cell.X, Cell.Y);
					pHouse->SetBaseSpawnCell(Cell);
					ScenarioClass::Instance->HouseIndices[pHouse->StartingPoint] = i;
				}
				else
				{
					Debug::LogInfo("Failed SetBaseSpawnCellFor[{} at {}]", HouseID, pHouse->StartingPoint);
				}
			}
		}
	}

	//for (size_t i = 0; i < UnAssigned.size(); ++i) {
	//	auto& [who, idx, IsAssigned] = UnAssigned[i];
	//	if (!IsAssigned) {
	//		IsAssigned = true;
	//		if (!IsCurrentCellAssigned[i]) {
	//			const auto& Cell = pScenStruct->CellVector.Items[i];
	//			Debug::LogInfo("SetBaseSpawnCellFor[%s at %d with [%d - %d]", who->get_ID(), i, Cell.X, Cell.Y);
	//			who->SetBaseSpawnCell(Cell);
	//			IsCurrentCellAssigned[i] = true;
	//		}
	//	}
	//}

	return 0x5D6C41;
}

ASMJIT_PATCH(0x70F820, TechnoClass_GetOriginalOwner_ValidateCaptureManager, 0x6)
{
	GET(TechnoClass* const, pThis, ECX);

	if (pThis->MindControlledBy && pThis->MindControlledBy->CaptureManager)
	{
		R->EAX(pThis->MindControlledBy);
		return 0x70F82A;
	}

	HouseClass* pOwner = pThis->Owner;

	if ((pThis->MindControlledByHouse || pThis->MindControlledByAUnit)
		&& pThis->OriginallyOwnedByHouse) { // chek this first before assign it
											// game crash will occur if this return nullptr
		pOwner = pThis->OriginallyOwnedByHouse;
	}

	R->EAX(pOwner);
	return 0x70F84E;
}

ASMJIT_PATCH(0x65DC11, Do_Reinforcement_ValidateHouse, 0x6)
{
	GET(FootClass* const, pReinforcee, EBP);

	if (!pReinforcee->Owner)
	{
		R->EAX(Edge::North);
		return 0x65DC2B;
	}

	const Edge nEdge =
		pReinforcee->Owner->StaticData.StartingEdge < Edge::North || pReinforcee->Owner->StaticData.StartingEdge > Edge::West
		? pReinforcee->Owner->GetHouseEdge() : pReinforcee->Owner->StaticData.StartingEdge;

	R->EAX(nEdge);
	return 0x65DC2B;
}

ASMJIT_PATCH(0x43A002, Bounclass_Update_FixCrash, 0x9)
{
	GET(int, ramp, EAX);
	//GET_STACK(BounceClass* , pThis , 0x24);

	//the fuck this ramp result is wrong ,..
	if(ramp > 11){
		//Debug::LogInfo("Updating Bounce with rampIdx %d", ramp);
		return 0x43A05D;
	}

	return 0x0;
}