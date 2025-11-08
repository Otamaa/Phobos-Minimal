#include <Helpers/Macro.h>
#include <Utilities/Macro.h>

#include "Body.h"

#include <MapClass.h>

CellStruct FakeScenarioClass::_Get_Waypoint_Location(int idx) {
	return ScenarioExtData::Instance()->Waypoints[idx];
}

DEFINE_FUNCTION_JUMP(LJMP, 0x68BCC0, FakeScenarioClass::_Get_Waypoint_Location);

ASMJIT_PATCH(0x68BCE4, ScenarioClass_Get_Waypoint_Cell_0, 0x7)
{
	GET_STACK(int, nWaypoint, 0x4);
	R->ECX(&ScenarioExtData::Instance()->Waypoints[nWaypoint]);
	return 0x68BCEB;
}

ASMJIT_PATCH(0x68BD08, ScenarioClass_Get_Waypoint, 0x7)
{
	GET_STACK(int, nWaypoint, STACK_OFFS(0x10, -0x8));
	R->ECX(&ScenarioExtData::Instance()->Waypoints[nWaypoint]);
	return 0x68BD0F;
}

ASMJIT_PATCH(0x68BD60, ScenarioClass_Clear_All_Waypoints, 0x6)
{
	if (auto const pScen = ScenarioExtData::Instance())
			pScen->Waypoints.clear();

	return 0x68BD79;
}

ASMJIT_PATCH(0x68BD80, ScenarioClass_Is_Waypoint_Valid, 0x5)
{
	GET_STACK(int, nWaypoint, 0x4);
	const auto waypoints = &ScenarioExtData::Instance()->Waypoints;
	R->AL(nWaypoint >= 0 && waypoints->contains(nWaypoint) && (*waypoints)[nWaypoint].IsValid());

	return 0x68BDB3;
}

ASMJIT_PATCH(0x68BDC0, ScenarioClass_ReadWaypoints, 0x8)
{
	GET_STACK(INIClass* const, pINI, 0x4);

	for (int i = 0; i < pINI->GetKeyCount(GameStrings::Waypoints()); ++i)
	{
		const auto pName = pINI->GetKeyName(GameStrings::Waypoints(), i);
		int id;
		if (sscanf_s(pName, "%d", &id) != 1 || id < 0)
			Debug::LogInfo("[Phobos Developer Warning] Failed to parse waypoint {}.", pName);

		int nCoord = pINI->ReadInteger(GameStrings::Waypoints(), pName, 0);

		CellStruct buffer = CellStruct::Empty;

		if (nCoord)
		{
			buffer.X = static_cast<short>(nCoord % 1000);
			buffer.Y = static_cast<short>(nCoord / 1000);

			if (auto const pCell = MapClass::Instance->GetCellAt(buffer))
				pCell->Flags |= CellFlags::IsWaypoint;
			else if (ScenarioExtData::CellParsed)
				Debug::LogInfo("[Phobos Developer Warning] Can not get waypoint {} : [{}, {}]!", id, buffer.X, buffer.Y);
		}
		else
			Debug::LogInfo("[Phobos Developer Warning] Invalid waypoint {}!", id);


		//Debug::LogInfo("Parse waypoint Result [%d][%d, %d] ! ", id, buffer.X, buffer.Y);
		ScenarioExtData::Instance()->Waypoints[id] = buffer;
	}

	return 0x68BE8C;
}

ASMJIT_PATCH(0x6874E7, ScenarioClass_ReadINI_CellParsed, 0x6)
{
	ScenarioExtData::CellParsed = true;
	return 0;
}

ASMJIT_PATCH(0x68BE90, ScenarioClass_Write_Waypoints, 0x5) //was 5 and crash ?
{
	GET_STACK(INIClass*, pINI, 0x4);
	pINI->Clear(GameStrings::Waypoints(), nullptr);
	for (const auto& [nidx,nCell] : ScenarioExtData::Instance()->Waypoints)
	{
		if (!nCell.IsValid())
			continue;

		pINI->WriteInteger(GameStrings::Waypoints(), std::to_string(nidx).c_str(), nCell.X + 1000 * nCell.Y, false);
	}

	return 0x68BF1F;
}

//ASMJIT_PATCH(0x68BF30, ScenarioClass_Set_Default_Waypoint, 0xA)
//{
//	Debug::LogInfo(__FUNCTION__" called! Caller = %p", R->Stack32(0x0));
//	return 0;
//}

ASMJIT_PATCH(0x68BF50, ScenarioClass_Set_Waypoint, 0x8)
{
	GET_STACK(int, nWaypoint, 0x4);
	GET_STACK(CellStruct, cell, 0x8);
	ScenarioExtData::Instance()->Waypoints[nWaypoint] = cell;
	return 0x68BF5F;
}

ASMJIT_PATCH(0x68BF74, ScenarioClass_Get_Waypoint_Cell, 0x7)
{
	GET_STACK(int, nWaypoint, 0x4);
	R->ECX(&ScenarioExtData::Instance()->Waypoints[nWaypoint]);
	return 0x68BF7B;
}

ASMJIT_PATCH(0x763610, Waypoint_To_String, 0x5)
{
	char buffer[8] { '\0' };
	GET(int, nWaypoint, ECX);

	if (nWaypoint < 0)
		R->EAX("0");
	else if (nWaypoint == INT_MAX)
		R->EAX("FXSHRXX");
	else
	{
		++nWaypoint;
		int pos = 7;
		while (nWaypoint > 0)
		{
			--pos;
			char m = nWaypoint % 26;
			if (m == 0) m = 26;
			buffer[pos] = m + '@'; // '@' = 'A' - 1
			nWaypoint = (nWaypoint - m) / 26;
		}
		R->EAX(buffer + pos);
	}
	return 0x763621;
}

ASMJIT_PATCH(0x763690, String_To_Waypoint, 0x7)
{
	GET(char*, pString, ECX);

	int n = 0;
	int len = strlen(pString);
	for (int i = len - 1, j = 1; i >= 0; i--, j *= 26)
	{
		int c = std::toupper(pString[i]);
		if (c < 'A' || c > 'Z') return 0;
		n += ((int)c - 64) * j;
	}
	R->EAX(n - 1);

	return 0x7636DF;
}

// This function is really strange, it returns empty string if a wp is valid, but why? - secsome
ASMJIT_PATCH(0x68BF90, ScenarioClass_Get_Waypoint_As_String, 0x6)
{
	R->EAX(0x889F64);
	return 0x68BFD7;
}

ASMJIT_PATCH(0x6883B7, ScenStruct_ScenStruct_1, 0x6)
{
	GET(int, nCount, ESI);

	// Waypoint 0-7 are used as multiplayer starting location
	for (int i = 0; i < 8; ++i)
	{
		if (!ScenarioClass::Instance->IsDefinedWaypoint(i))
			break;
		else
			++nCount;
	}

	R->ESI(nCount);

	return 0x6883EB;
}

ASMJIT_PATCH(0x68843B, ScenStruct_ScenStruct_2, 0x6)
{
	REF_STACK(DynamicVectorClass<CellStruct>, waypoints, STACK_OFFS(0x40, 0x18));
	REF_STACK(CellStruct, buffer, STACK_OFFS(0x40, 0x20));
	GET(int, i, ESI);

	if (ScenarioClass::Instance->IsDefinedWaypoint(i)) {
		buffer = ScenarioExtData::Instance()->Waypoints[i];
		if(waypoints.push_back(buffer))
			Debug::LogInfo("Multiplayer start waypoint found at cell [{}][{},{}] , With waypoints Size {} ",i, buffer.X, buffer.Y , waypoints.Count);
	}


	return 0x6884EF;
}

//ASMJIT_PATCH(0x6D6070, Tactical_SetTacticalPosition, 0x5)
//{
//	GET_STACK(DWORD, dwCaller, 0x0);
//	GET_STACK(CoordStruct*, pCoord, 0x4);
//
//	CellStruct cell = CellClass::Coord2Cell(*pCoord);
//	Debug::LogInfo(__FUNCTION__ "Caller = %p Cell = (%d,%d)", dwCaller, cell.X, cell.Y);
//
//	return 0;
//}

ASMJIT_PATCH(0x684CB7, Scen_Waypoint_Call_1, 0x7)
{
	GET(int, nWaypoint, EAX);
	R->EAX(ScenarioExtData::Instance()->Waypoints[nWaypoint].Pack());
	return 0x684CBE;
}

ASMJIT_PATCH(0x6855E4, Scen_Waypoint_Call_2, 0x5)
{
	ScenarioExtData::Instance()->Waypoints.clear();
	return 0x6855FC;
}

ASMJIT_PATCH(0x68AFE7, Scen_Waypoint_Call_3, 0x7) //5
{
	GET(int, nWaypoint, EDI);
	R->EAX(ScenarioExtData::Instance()->Waypoints[nWaypoint].Pack());
	return 0x68AFEE;
}

ASMJIT_PATCH(0x68AF45, Scen_Waypoint_Call_4, 0x6)
{
	int nStartingPoints = 0;
	for (int i = 0; i < 8; ++i)
	{
		if (ScenarioClass::Instance->IsDefinedWaypoint(i))
			++nStartingPoints;
		else
			break;
	}

	R->EDX(nStartingPoints);
	return 0x68AF86;
}

//ASMJIT_PATCH(0x5D6C1D, Scen_Multiplay_BaseSpawnCell_replace, 0x5)
//{
//	GET(CellStruct, nCell, ESI);
//
//	R->EDX(ScenarioExtData::Instance()->Waypoints());
//}