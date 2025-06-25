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

class NOVTABLE FakeObjectClass : public ObjectClass {
public:
	void _DrawRadialIndicator(int val);
	int _GetDistanceOfObj(AbstractClass* pThat);
	int _GetDistanceOfCoord(CoordStruct* pThat);
};
static_assert(sizeof(FakeObjectClass) == sizeof(ObjectClass), "Invalid Size !");

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

class NOVTABLE FakeFootClass : public FootClass {
public:
};
static_assert(sizeof(FakeFootClass) == sizeof(FootClass), "Invalid Size !");
//===================================================================================