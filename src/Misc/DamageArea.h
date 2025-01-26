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
		const WarheadTypeClass* const pWarhead,
		bool affectTiberium,
		HouseClass* pHouse);
};