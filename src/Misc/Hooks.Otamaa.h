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


FORCEDINLINE int cell_Distance_Squared(CoordStruct& our_coord, CoordStruct& their_coord)
{
	int our_cell_x = our_coord.X / Unsorted::LeptonsPerCell;
	int their_cell_x = their_coord.X / Unsorted::LeptonsPerCell;
	int our_cell_y = our_coord.Y / Unsorted::LeptonsPerCell;
	int their_cell_y = their_coord.Y / Unsorted::LeptonsPerCell;

	int x_distance = Math::abs(our_cell_x - their_cell_x);
	int y_distance = Math::abs(our_cell_y - their_cell_y);
	return (x_distance * x_distance) + (y_distance * y_distance);

	//return int(Point2D { our_coord.X - their_coord.X, our_coord.Y - their_coord.Y }.Length());
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
};
//static_assert(sizeof(FakeObjectClass) == sizeof(ObjectClass), "Invalid Size !");

//===================================================================================

class NOVTABLE FakeUnitClass : public UnitClass {
public:

	bool _Paradrop(CoordStruct* pCoords);
	CoordStruct* _GetFLH(CoordStruct* buffer, int wepon, CoordStruct base);
	int _Mission_Attack();

	void _SetOccupyBit(CoordStruct* pCrd);
	void _ClearOccupyBit(CoordStruct* pCrd);
};
static_assert(sizeof(FakeUnitClass) == sizeof(UnitClass), "Invalid Size !");

//===================================================================================

class NOVTABLE FakeFootClass //: public FootClass
{
public:
};
//static_assert(sizeof(FakeFootClass) == sizeof(FootClass), "Invalid Size !");
//===================================================================================