#pragma once

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