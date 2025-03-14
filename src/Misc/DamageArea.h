#pragma once

#include <GeneralDefinitions.h>

struct CoordStruct;
class TechnoClass;
class WarheadTypeClass;
class HouseClass;

struct DamageArea
{
	static DamageAreaResult __fastcall Apply(CoordStruct* pCoord,
		int damage,
		TechnoClass* pSource,
		WarheadTypeClass* pWarhead,
		bool affectTiberium,
		HouseClass* pHouse);
};