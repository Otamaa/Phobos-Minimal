#pragma once

#include <GeneralDefinitions.h>

class TechnoClass;
class BuildingClass;
class AnimClass;
class BuildingLightClass;
class SuperClass;
class HouseClass;
class AresTechnoExt
{
public:
	TechnoClass* OwnerObj; //0
	BYTE idxSlot_Wave; //1
	BYTE idxSlot_Beam; //2
	BYTE idxSlot_Warp; //3
	BYTE idxSlot_Parasite; //4
	DWORD dword8; //8
	BuildingClass* GarrisonedIn; //12
	AnimClass* EMPSparkleAnim; //16
	Mission EMPLastMission; //20
	DWORD dword18;
	DWORD dword1C;
	DWORD dword20; //
	DWORD dword24; //
	DWORD dword28; //
	DWORD dword2C; //
	BYTE gap30[4];
	DWORD dword34;
	BuildingLightClass* BuildingLight;
	DWORD dword3C;
	DWORD dword40;
	CDTimerClass CloakSkipTimer; //
	CDTimerClass DisableWeaponTimer;
	CDTimerClass SelfHealingCombatTimer;
	int HijackerHealth;
	HouseClass* HijackerOwner;
	float HijackerVeterancy;
	BYTE gap74[4];
	double AE_ROF;
	double AE_FirePowerMult;
	double AE_ArmorMult;
	double AE_SpeedMult;
	BYTE AE_Cloak;
	BYTE AltOccupy_HasValue;
	BYTE AltOccupy_Value;
	BYTE Is_SurvivorsDone;
	BYTE Is_DriverKilled;
	BYTE Is_Operated;
	BYTE Is_InitialPayloadDone;
	BYTE Is_UnitLostMuted;
	SuperClass* AttachedSuperWeapon;
	DWORD dwordA4;
	DWORD TechnoValueAmount;
	CellStruct EMPulseTarget;
	BYTE TakeVehicleMode;
};
