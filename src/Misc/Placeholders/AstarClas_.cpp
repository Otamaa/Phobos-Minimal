//static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<SubzoneTrackingStruct>, 0x87F874> const SubzoneTrackingStructVector {};
//#include <ExtraHeaders/AStarClass.h>

#ifdef _PATHFIND
//#pragma optimize("", off )
ASMJIT_PATCH(0x42C4FE, AstarClass_FindPath_nullptr, 0x9)
{
	GET(int, SubZoneTracking_Idx, EDX);
	GET(int, SubZoneConnection_Idx, ECX);
	GET(int, PassabilityData_To, EBX);
	GET_BASE(CellStruct*, pFrom, 0x8);
	GET_BASE(CellStruct*, pTo, 0xC);
	GET_BASE(MovementZone, movementZone, 0x10);
	GET_BASE(FootClass*, pFoot, 0x14);

	Debug::LogInfo("FindingPath for [%s(0x%x) - Owner[%s(0x%x)] from[%d , %d] to [%d , %d] MovementZone [%s(%d)] DriverKilled[%s] ",
		pFoot->get_ID(), pFoot,
		pFoot->Owner->get_ID(), pFoot->Owner,
		pFrom->X, pFrom->Y,
		pTo->X, pTo->Y,
		TechnoTypeClass::MovementZonesToString[int(movementZone)], int(movementZone),
		TechnoExtContainer::Instance.Find(pFoot)->Is_DriverKilled ? "Yes" : "No"
	);

	return 0x0;
	/*
	const auto SubZoneTrackingArray = &SubzoneTrackingStruct::Array[0];
	const auto SubZobneConnectionPtr = SubZoneTrackingArray->Items + SubZoneTracking_Idx;
	const auto SubZobneConnectionPtr_offsetted = SubZobneConnectionPtr + SubZoneConnection_Idx;
	if (SubZoneTrackingArray->Count <= SubZoneConnection_Idx)
		Debug::FatalErrorAndExit("AstarClass_FindPath trying to offset SubzoneConnection array pointer to [%d] but the array only has[%d]!" , SubZoneConnection_Idx, SubZoneTrackingArray->Count);

	const auto ptr = SubZobneConnectionPtr_offsetted->SubzoneConnections.Items;
	const auto array_count = SubZobneConnectionPtr_offsetted->SubzoneConnections.Count;

	R->EDX(ptr);
	R->ECX(array_count);

	//this keep the thing clean
	//`SubZobneConnectionPtr` will contain broken pointer at some point tho ,....
	if (array_count > 0 && ptr)
	{
		//if(!SubZobneConnectionPtr)
			//Debug::FatalErrorAndExit("AStarClass will crash because SubZone is nullptr , last access is from [%s(0x%x) - Owner : (%s) ", LastAccessThisFunc->get_ID(), LastAccessThisFunc, LastAccessThisFunc->Owner->get_ID());

		return 0x42C519;
	}

	return 0x42C740;
	*/

}
//#pragma optimize("", on )
#endif
