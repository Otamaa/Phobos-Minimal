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

class FakeTeamClass : public TeamClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeTeamClass) == sizeof(TeamClass), "Invalid Size !");

//===================================================================================

class FakeSuperWeaponTypeClass : public SuperWeaponTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeSuperWeaponTypeClass) == sizeof(SuperWeaponTypeClass), "Invalid Size !");

//===================================================================================

class FakeSuperClass : public SuperClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeSuperClass) == sizeof(SuperClass), "Invalid Size !");

//===================================================================================
class FakeSmudgeTypeClass : public SmudgeTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
};
static_assert(sizeof(FakeSmudgeTypeClass) == sizeof(SmudgeTypeClass), "Invalid Size !");

//===================================================================================
class FakeHouseTypeClass : public HouseTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
};
static_assert(sizeof(FakeHouseTypeClass) == sizeof(HouseTypeClass), "Invalid Size !");

//===================================================================================

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

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
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

	HouseClass* _GetOwningHouse() {
		return this->Owner;
	}

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm , bool clearDirty);
};
static_assert(sizeof(FakeAnimClass) == sizeof(AnimClass), "Invalid Size !");

//===================================================================================

class FakeAnimTypeClass : public AnimTypeClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
};
static_assert(sizeof(FakeAnimTypeClass) == sizeof(AnimTypeClass), "Invalid Size !");

//===================================================================================

class FakeTerrainClass : public TerrainClass
{
public:

	void _Detach(AbstractClass* target, bool all);

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeTerrainClass) == sizeof(TerrainClass), "Invalid Size !");

//===================================================================================

class FakeTerrainTypeClass : public TerrainTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeTerrainTypeClass) == sizeof(TerrainTypeClass), "Invalid Size !");

//===================================================================================

class FakeTEventClass : public TEventClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeTEventClass) == sizeof(TEventClass), "Invalid Size !");

//===================================================================================

class FakeVoxelAnimClass : public VoxelAnimClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

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

class FakeVoxelAnimTypeClass : public VoxelAnimTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeVoxelAnimTypeClass) == sizeof(VoxelAnimTypeClass), "Invalid Size !");

//===================================================================================

class FakeWaveClass : public WaveClass
{
public:

	void _Detach(AbstractClass* target, bool all);
};
static_assert(sizeof(FakeWaveClass) == sizeof(WaveClass), "Invalid Size !");

//===================================================================================

class FakeParticleSystemClass : public ParticleSystemClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
};
static_assert(sizeof(FakeParticleSystemClass) == sizeof(ParticleSystemClass), "Invalid Size !");

//===================================================================================

class FakeParticleSystemTypeClass : public ParticleSystemTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
};
static_assert(sizeof(FakeParticleSystemTypeClass) == sizeof(ParticleSystemTypeClass), "Invalid Size !");

//===================================================================================

class FakeParticleClass : public ParticleClass
{
public:
	void _Detach(AbstractClass* target, bool all);

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
};
static_assert(sizeof(FakeParticleClass) == sizeof(ParticleClass), "Invalid Size !");

//===================================================================================

class FakeParticleTypeClass : public ParticleTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
};
static_assert(sizeof(FakeParticleTypeClass) == sizeof(ParticleTypeClass), "Invalid Size !");

//===================================================================================
class FakeBulletClass : public BulletClass
{
public:

	void _AnimPointerExpired(AnimClass* pTarget) {
		this->ObjectClass::AnimPointerExpired(pTarget);
	}

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
	void _Detach(AbstractClass* target, bool all);
};
static_assert(sizeof(FakeBulletClass) == sizeof(BulletClass), "Invalid Size !");

//===================================================================================

class FakeBulletTypeClass : public BulletTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeBulletTypeClass) == sizeof(BulletTypeClass), "Invalid Size !");

//===================================================================================
class FakeTiberiumClass : public TiberiumClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeTiberiumClass) == sizeof(TiberiumClass), "Invalid Size !");

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

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);
};
static_assert(sizeof(FakeBuildingClass) == sizeof(BuildingClass), "Invalid Size !");

//===================================================================================

class FakeWarheadTypeClass : public WarheadTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeWarheadTypeClass) == sizeof(WarheadTypeClass), "Invalid Size !");

//===================================================================================

class FakeWeaponTypeClass : public WeaponTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

};
static_assert(sizeof(FakeWeaponTypeClass) == sizeof(WeaponTypeClass), "Invalid Size !");

//===================================================================================