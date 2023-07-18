#pragma once

class TechnoClass;
class BuildingClass;
struct TechnoExt_ExtData
{
	static bool IsOperated(TechnoClass* pThis);
	static bool IsPowered(TechnoClass* pThis);
	static void EvalRaidStatus(BuildingClass* pThis);
};