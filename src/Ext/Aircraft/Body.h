#pragma once

#include <Utilities/Enum.h>
#include <AircraftClass.h>

class AbstractClass;
class AircraftClass;
class WeaponTypeClass;
class AircraftExt
{
public:
	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber);
	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber , int WeaponIdx);
	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, AircraftFireMode shotNumber, int WeaponIdx, WeaponTypeClass* pWeapon);
	static void TriggerCrashWeapon(AircraftClass* pThis , int nMult);
	static bool IsValidLandingZone(AircraftClass* pThis);

	static bool PlaceReinforcementAircraft(AircraftClass* pThis, CellStruct edgeCell);
};

class AbstractClass;
class NOVTABLE FakeAircraftClass : public AircraftClass
{
public:

	WeaponStruct* _GetWeapon(int weaponIndex);
	void _SetTarget(AbstractClass* pTarget);
	void _Destroyed(int mult);
	AbstractClass* _GreatestThreat(ThreatType threatType, CoordStruct* pSelectCoords, bool onlyTargetHouseEnemy);
	int _Mission_Sleep();
	void _FootClass_Update_Wrapper();

	BulletClass* _FireAt(AbstractClass* target, int which);
};

static_assert(sizeof(FakeAircraftClass) == sizeof(AircraftClass), "Invalid Size !");
