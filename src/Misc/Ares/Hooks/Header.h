#pragma once

#include <CellStruct.h>

class TechnoClass;
class BuildingClass;
class HouseClass;
class BSurface;
class TechnoTypeClass;
struct TechnoExt_ExtData
{
	static bool IsOperated(TechnoClass* pThis);
	static bool IsOperatedB(TechnoClass* pThis);
	static bool IsPowered(TechnoClass* pThis);
	static void EvalRaidStatus(BuildingClass* pThis);
};

struct TechnoTypeExt_ExtData
{
	static bool CameoIsElite(TechnoTypeClass* pType, HouseClass* pHouse);
	static BSurface* GetPCXSurface(TechnoTypeClass* pType, HouseClass* pHouse);
};

class ObjectClass;
struct FirewallFunctions
{
	static DWORD GetFirewallFlags(BuildingClass* pThis);
	static void ImmolateVictims(TechnoClass* pThis);
	static bool ImmolateVictim(TechnoClass* pThis, ObjectClass* const pVictim, bool const destroy = true);
	static void UpdateFirewall(BuildingClass* pThis, bool const changedState);
	static void UpdateFirewallLinks(BuildingClass* pThis);
	static bool IsActiveFirestormWall(BuildingClass* const pBuilding, HouseClass const* const pIgnore);
	static bool sameTrench(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static bool canLinkTo(BuildingClass* currentBuilding, BuildingClass* targetBuilding);
	static void BuildLines(BuildingClass* theBuilding, CellStruct selectedCell, HouseClass* buildingOwner);
	static int GetImageFrameIndex(BuildingClass* pThis);
};