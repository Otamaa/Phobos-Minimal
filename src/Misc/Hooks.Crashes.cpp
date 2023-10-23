#include <Ext/TechnoType/Body.h>

#ifdef debug_these
DEFINE_HOOK(0x6FC32B, TechnoClass_CanFire_NoWeapon, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponStruct*, pWeaponstruct, EAX);
	GET_STACK(int, nWeaponIdx, 0x28);

	if (!pWeaponstruct || !pWeaponstruct->WeaponType) {
		R->EDI<WeaponTypeClass*>(nullptr);
		//const auto pType = pThis->GetTechnoType();
		//Debug::Log("TechnoClass[%s] CanFire NoWeapon at idx [%d] \n", pType->get_ID() , nWeaponIdx);
		return 0x6FCD29;
	}

	return 0x0;
	//R->EDI(pWeaponstruct->WeaponType);
	//R->Stack(0x10, pWeaponstruct->WeaponType);
	//return 0x6FC339;
}

DEFINE_HOOK(0x709992, TechnoClass_TargetSomethingNearby_NoWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EDI);
	GET(WeaponStruct*, pWeaponstruct, EAX);

	if (!pTarget || !pWeaponstruct || !pWeaponstruct->WeaponType){

		//if(!pTarget)
		//	Debug::Log("TechnoClass[%s] TargetSomethingNearby Notarget\n", pThis->get_ID());
		//else
			//Debug::Log("TechnoClass[%s] TargetSomethingNearby NoWeapon Target[%s]\n", pThis->get_ID() , pTarget->get_ID());

		return 0x7099B8;
	}

	R->EAX(pWeaponstruct->WeaponType);
	return 0x70999C;
}

DEFINE_HOOK(0x6F7CD5, TechnoClass_EvalueateObj_NoWeapon, 0x6)
{
	GET(ObjectClass*, pTarget, ESI);
	GET(TechnoClass*, pThis, EDI);
	GET(WeaponStruct*, pWeaponstruct, EAX);

	if (!pWeaponstruct || !pWeaponstruct->WeaponType) {
		//Debug::Log("TechnoClass[%s] EvalueateObj NoWeapon Target[%s]\n", pThis->get_ID(), pTarget->get_ID());
		return 0x6F894F;
	}

	R->EBP(pWeaponstruct->WeaponType);
	R->EAX(R->Stack<DWORD>(0x40));
	return 0x6F7CDB;
}

DEFINE_HOOK(0x70CE90, TechnoClass_Coef_checkForTechno, 0x6)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(WeaponStruct*, pWeaponstruct, EAX);

	if (!pWeaponstruct || !pWeaponstruct->WeaponType) {
		//Debug::Log("TechnoClass[%s] Coef No NoWeapon Target[%s]\n", pThis->get_ID() , pTarget->get_ID());
		return 0x70CED2;
	}

	return 0x70CE96;
}
#endif

//DEFINE_HOOK(0x7431EE, UnitClass_GreatestThreat_GattlingWeaponError, 0x9)
//{
//	GET(UnitClass*, pThis, ESI);
//
//	const auto pWpi = pThis->GetCurrentGattlingStage();
//	const auto pWps = pThis->GetWeapon(pWpi);
//
//	if (!pWps || !pWps->WeaponType)
//		Debug::Log("WTF Techno[%s] Trying to find weaponIdx [%d] but it nullptr ?\n" , pThis->Type->ID, pWpi);
//
//	R->EAX(pWps);
//	return 0x7431F7;
//}

//DEFINE_SKIP_HOOK(0x5F5896, TechnoClass_Mark_RemoveUnused, 0x5, 5F58E1);
DEFINE_JUMP(LJMP, 0x5F5896, 0x5F58E1);

// //uhh , unit without ownership should not allowed to deploy to prevent this kind of stupid crashes ,..
// DEFINE_HOOK(0x739698, UnitClass_TryToDeploy_Verify, 0x5)
// {
// 	GET(UnitClass*, pThis, EBP);
//
// 	if (pThis->Owner) {
// 		R->EAX<void*>(YRMemory::Allocate(sizeof(BuildingClass)));
// 	} else {
// 		Debug::Log("Unit[%s] Trying to undeploy but missing Ownership!\n", pThis->Type->ID);
// 		R->EAX<void*>(nullptr);
// 	}
//
// 	return 0x7396A5;
// }

//DEFINE_HOOK(0x6F49D2, TechnoClass_Reveal_ThisOwnerMissing, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	if (auto pOwner = pThis->Owner) {
//		pOwner->RecheckPower = true;
//		pOwner->RecheckRadar = true;
//	}
//
//	pThis->DiscoveredByCurrentPlayer = true;
//
//	if (!pThis->IsOwnedByCurrentPlayer)
//	{
//		if (!Unsorted::ScenarioInit)
//		{
//			if (auto pTag = pThis->AttachedTag)
//				pTag->SpringEvent(TriggerEvent::DiscoveredByPlayer, pThis, CellStruct::Empty, false, nullptr);
//		}
//
//		if (pThis->Owner)
//			pThis->Owner->DiscoveredByPlayer = true;
//	}
//
//	return 0x6F497E;
//}

DEFINE_HOOK(0x70CD29, TechnoClass_Coef_CheckTarget, 0x6)
{
	GET(ObjectClass*, pTarget, ESI);
	return pTarget ? 0x0 : 0x70CD39;
}

DEFINE_HOOK(0x5D6BF1, MultiplayerGameMode_SetBaseSpawnCell_CheckAvail, 0x5)
{
	struct ScenStruct {
		DynamicVectorClass<CellStruct> CellVector;
	};

	GET(ScenStruct*, pScenStruct, EAX);

	//std::vector<std::tuple<HouseClass*, int , bool>> UnAssigned;
	//std::vector<bool> IsCurrentCellAssigned(pScenStruct->CellVector.Size());

	for (int i = 0; i < HouseClass::Array->Count; ++i)
	{
		auto pHouse = HouseClass::Array->GetItem(i);

		if (!pHouse->Type->MultiplayPassive)
		{
			if (pHouse->StartingPoint != -2)
			{
				const auto HouseID = pHouse->get_ID();

				// you dont want to read out of bound array here,..
				if (pHouse->StartingPoint < (int)pScenStruct->CellVector.size())
				{
					const auto& Cell = pScenStruct->CellVector.Items[pHouse->StartingPoint];
					//IsCurrentCellAssigned[pHouse->StartingPoint] = true;
					Debug::Log("SetBaseSpawnCellFor[%s at %d with [%d - %d]\n", HouseID, pHouse->StartingPoint, Cell.X, Cell.Y);
					pHouse->SetBaseSpawnCell(Cell);
					ScenarioClass::Instance->HouseIndices[pHouse->StartingPoint] = i;
				}
				else
				{
					Debug::Log("Failed SetBaseSpawnCellFor[%s at %d]\n", HouseID, pHouse->StartingPoint);
					//UnAssigned.emplace_back(pHouse , i , false);
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
	//			Debug::Log("SetBaseSpawnCellFor[%s at %d with [%d - %d]\n", who->get_ID(), i, Cell.X, Cell.Y);
	//			who->SetBaseSpawnCell(Cell);
	//			IsCurrentCellAssigned[i] = true;
	//		}
	//	}
	//}

	return 0x5D6C41;
}

DEFINE_HOOK(0x70F820, TechnoClass_GetOriginalOwner_ValidateCaptureManager, 0x6)
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

DEFINE_HOOK(0x65DC11, Do_Reinforcement_ValidateHouse, 0x6)
{
	GET(FootClass* const, pReinforcee, EBP);

	if (!pReinforcee->Owner)
	{
		R->EAX(Edge::North);
		return 0x65DC2B;
	}

	const Edge nEdge =
		pReinforcee->Owner->StartingEdge < Edge::North || pReinforcee->Owner->StartingEdge > Edge::West
		? pReinforcee->Owner->GetHouseEdge() : pReinforcee->Owner->StartingEdge;

	R->EAX(nEdge);
	return 0x65DC2B;
}
