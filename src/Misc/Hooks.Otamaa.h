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
#include <TerrainClass.h>
#include <WaveClass.h>
#include <RadSiteClass.h>

#include <CellClass.h>
#include <VocClass.h>

#include <MapClass.h>

class FakeHouseClass : public HouseClass
{
public:
	bool _IsAlliedWith(HouseClass* pOther);
	void _Detach(AbstractClass* target, bool all);
};
static_assert(sizeof(FakeHouseClass) == sizeof(HouseClass), "Invalid Size !");

//===================================================================================

class FakeRadSiteClass : public RadSiteClass
{
public:
	void _Detach(AbstractClass* target, bool all);
	HouseClass* _GetOwningHouse();
	CoordStruct __GetAltCoords() {
		auto const pCell = MapClass::Instance->GetCellAt(this->BaseCell);
		return pCell->GetCoordsWithBridge();
	}
};
static_assert(sizeof(FakeRadSiteClass) == sizeof(RadSiteClass), "Invalid Size !");

//===================================================================================

class FakeObjectClass : public ObjectClass
{
public:
	void _DrawRadialIndicator(int val);

};
static_assert(sizeof(FakeObjectClass) == sizeof(ObjectClass), "Invalid Size !");

//===================================================================================
class FakeAnimClass : public AnimClass
{
public:

	void _Detach(AbstractClass* target, bool all);
	HouseClass* _GetOwningHouse() {
		return this->Owner;
	}

};
static_assert(sizeof(FakeAnimClass) == sizeof(AnimClass), "Invalid Size !");

//===================================================================================

class FakeTerrainClass : public TerrainClass
{
public:

	void _Detach(AbstractClass* target, bool all);

};
static_assert(sizeof(FakeTerrainClass) == sizeof(TerrainClass), "Invalid Size !");

//===================================================================================

class FakeVoxelAnimClass : public VoxelAnimClass
{
public:

	void _Detach(AbstractClass* target, bool all);
	void _RemoveThis()
	{
		if (this->Type)
			VocClass::PlayIndexAtPos(this->Type->StopSound, this->Location);

		this->ObjectClass::UnInit();
	}

};
static_assert(sizeof(FakeVoxelAnimClass) == sizeof(VoxelAnimClass), "Invalid Size !");

//===================================================================================

class FakeWaveClass : public WaveClass
{
public:

	void _Detach(AbstractClass* target, bool all);
};
static_assert(sizeof(FakeWaveClass) == sizeof(WaveClass), "Invalid Size !");

//===================================================================================

class FakeParticleClass : public ParticleClass
{
public:
	void _Detach(AbstractClass* target, bool all);
};
static_assert(sizeof(FakeParticleClass) == sizeof(ParticleClass), "Invalid Size !");

//===================================================================================

class FakeBulletClass : public BulletClass
{
public:

	void _AnimPointerExpired(AnimClass* pTarget) {
		this->ObjectClass::AnimPointerExpired(pTarget);
	}

	void _Detach(AbstractClass* target, bool all);
};
static_assert(sizeof(FakeBulletClass) == sizeof(BulletClass), "Invalid Size !");

//===================================================================================

class FakeUnitClass : public UnitClass {
public:

	bool _Paradrop(CoordStruct* pCoords);
	CoordStruct* _GetFLH(CoordStruct* buffer, int wepon, CoordStruct base);
};
static_assert(sizeof(FakeUnitClass) == sizeof(UnitClass), "Invalid Size !");

//===================================================================================

class FakeAircraftClass : public AircraftClass
{
public:

	WeaponStruct* _GetWeapon(int weaponIndex);
	void _SetTarget(AbstractClass* pTarget);
	void _Destroyed(int mult);

};
static_assert(sizeof(FakeAircraftClass) == sizeof(AircraftClass), "Invalid Size !");

//===================================================================================

class FakeInfantryClass : public InfantryClass
{
public:
	void _Dummy(Mission, bool) RX;
	DamageState _IronCurtain(int nDur, HouseClass* pSource, bool bIsFC)
	{
		if (this->Type->Engineer && this->TemporalTargetingMe && this->Destination) {
			if (auto const pCell = this->GetCell()) {
				if (auto const pBld = pCell->GetBuilding()) {
					if (this->Destination == pBld && pBld->Type->BridgeRepairHut) {
						return DamageState::Unaffected;
					}
				}
			}
		}

		return this->TechnoClass::IronCurtain(nDur, pSource, bIsFC);
	}
};
static_assert(sizeof(FakeInfantryClass) == sizeof(InfantryClass), "Invalid Size !");

//===================================================================================

class FakeBuildingClass : public BuildingClass
{
public:
	void _Detach(AbstractClass* target, bool all);
	bool _IsFactory();
	CoordStruct* _GetFLH(CoordStruct* pCrd, int weaponIndex);
	int _Mission_Missile();
	void _Spawn_Refinery_Smoke_Particles();
	bool _SetOwningHouse(HouseClass* pHouse, bool announce) {
		const bool res = this->BuildingClass::SetOwningHouse(pHouse, announce);

		// Fix : update powered anims
		if (res && (this->Type->Powered || this->Type->PoweredSpecial))
			this->UpdatePowerDown();

		return res;
	}
};
static_assert(sizeof(FakeBuildingClass) == sizeof(BuildingClass), "Invalid Size !");

//===================================================================================
