#pragma once

#include <Utilities/Macro.h>

#include <AbstractClass.h>
#include <ObjectClass.h>
#include <AnimClass.h>
#include <BulletClass.h>
#include <VoxelAnimClass.h>
#include <BounceClass.h>
#include <MissionClass.h>
#include <TechnoClass.h>
#include <FootClass.h>
#include <AircraftClass.h>
#include <UnitClass.h>
#include <InfantryClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ParticleClass.h>
#include <WaveClass.h>
#include <RadSiteClass.h>
#include <SmudgeTypeClass.h>
#include <SuperClass.h>
#include <SuperWeaponTypeClass.h>
#include <TeamTypeClass.h>
#include <TeamClass.h>
#include <TiberiumClass.h>
#include <TerrainClass.h>
#include <TerrainTypeClass.h>
#include <TEventClass.h>
#include <ParticleSystemClass.h>
#include <ParticleSystemTypeClass.h>

#include <CellClass.h>
#include <VocClass.h>

#include <MapClass.h>


FORCEDINLINE COMPILETIMEEVAL int cell_Distance_Squared(CoordStruct& our_coord, CoordStruct& their_coord)
{
	const int64_t dx = int64_t(our_coord.X) - int64_t(their_coord.X);
    const int64_t dy = int64_t(our_coord.Y) - int64_t(their_coord.Y);
    const int64_t d2 = dx * dx + dy * dy;

    return d2 > INT_MAX ? INT_MAX : int(d2);
}

//limited usability
//some vtable calling may resulting on broken result
//be carefull when using function here
class NOVTABLE FakeObjectClass //: public ObjectClass
{
public:

	static void __fastcall _DrawRadialIndicator(ObjectClass* pThis, discard_t, int val);

	static int __fastcall _GetDistanceOfObj(ObjectClass* pThis, discard_t, AbstractClass* pThat)
	{
		int nResult = 0;
		if (pThat)
		{
			auto nThisCoord = pThis->GetCoords();
			auto nThatCoord = pThat->GetCoords();
			nResult = //(int)nThisCoord.DistanceFromXY(nThatCoord)
				cell_Distance_Squared(nThisCoord, nThatCoord);
			;
		}

		return nResult;
	}

	static int __fastcall _GetDistanceOfCoord(ObjectClass* pThis, discard_t, CoordStruct* pThat)
	{
		auto nThisCoord = pThis->GetCoords();
		return cell_Distance_Squared(nThisCoord, *pThat);
	}

	static CellClass* __fastcall _GetCell(ObjectClass* pThis, discard_t)
	{
		return MapClass::Instance->GetCellAt(pThis->Location);
	}

	static DamageState __fastcall __Take_Damage(ObjectClass* pThis, discard_t, int* damage, int distance, WarheadTypeClass* warhead, TechnoClass* source, bool ignoreDefenses, bool PreventsPassengerEscape, HouseClass* sourceHouse);

};
//static_assert(sizeof(FakeObjectClass) == sizeof(ObjectClass), "Invalid Size !");

//===================================================================================

//===================================================================================

class NOVTABLE FakeFootClass //: public FootClass
{
public:

	static DamageState __fastcall __Take_Damage(FootClass* pThis, discard_t, int* damage, int distance, WarheadTypeClass* warhead, TechnoClass* source, bool ignoreDefenses, bool PreventsPassengerEscape, HouseClass* sourceHouse);

};
//static_assert(sizeof(FakeFootClass) == sizeof(FootClass), "Invalid Size !");
//===================================================================================