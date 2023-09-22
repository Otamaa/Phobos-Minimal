#pragma once

#include <GeneralDefinitions.h>
#include <Memory.h>

#include <vector>

class AbstractClass;
class AbstractTypeClass;
class TechnoClass;
class BuildingClass;
class AnimClass;
class AnimTypeClass;
class BuildingLightClass;
class SuperClass;
class HouseClass;
class TemporalClass;
class EBolt;
struct SHPStruct;
class HouseTypeClass;
class AresTechnoExt
{
public:
	AresTechnoExt(TechnoClass* Obj) : OwnerObj { Obj }
	{ }

public:
	TechnoClass* OwnerObj { 0 }; //0
	int initState { -1 }; //4
	BYTE idxSlot_Wave { 0 }; //5
	BYTE idxSlot_Beam { 0 }; //6
	BYTE idxSlot_Warp { 0 }; //7
	BYTE idxSlot_Parasite { 0 }; //8
	BuildingClass* GarrisonedIn { 0 }; //C
	AnimClass* EMPSparkleAnim { 0 }; //10
	Mission EMPLastMission { 0 }; //

	class JammerClass
	{
	public:
		int LastScan;							//!< Frame number when the last scan was performed.
		TechnoClass* AttachedToObject;			//!< Pointer to game object this jammer is on
		bool Registered;						//!< Did I jam anything at all? Used to skip endless unjam calls.
	};
	static_assert(sizeof(JammerClass) == 0xC, "InvalidSize!");
	JammerClass* RadarJammerUptr { 0 };

	class PoweredUnitClass
	{
	public:
		TechnoClass* Techno;
		int LastScan;
		bool Powered;
	};
	static_assert(sizeof(PoweredUnitClass) == 0xC, "InvalidSize!");
	PoweredUnitClass* PoweredUnitUptr { 0 };

	struct AEData
	{
		struct AEType
		{
			AbstractTypeClass* Owner;
			int Duration;
			BYTE Cumulative;
			BYTE ForceDecloak;
			BYTE DiscardOnEntry;
			BYTE gap;
			AnimTypeClass* AnimType;
			BYTE AnimResetOnReapply;
			BYTE TemporalHidesAnim;
			BYTE PenetratesIC;
			BYTE gap13[5];
			double FirepowerMultiplier;
			double ArmorMultiplier;
			double ROFMultiplier;
			double SpeedMultiplier;
			BYTE Cloakable;
			BYTE gap39[3];
			int Delay;
			int InitialDelay;
		};

		struct AEInternal
		{
			AEType* Type;
			AnimClass* Anim;
			int Duration;
			HouseClass* Invoker;
		};
		static_assert(sizeof(AEInternal) == 0x10, "InvalidSize!");

		// using std array will cause this to break ,..
		AEInternal* first;
		AEInternal* last;
		AEInternal* end;
		int Delay;
		BYTE NeedToRecreateAnim;
		BYTE Isset;
		BYTE Gap_AE_1;
		BYTE Gap_AE_2;
	};
	static_assert(sizeof(AEData) == 0x14, "InvalidSize!");

	AEData AEDatas; //0x20
	TemporalClass* MyOriginalTemporal { 0 }; //0x34
	BuildingLightClass* BuildingLight { 0 };
	EBolt* MyBolt { 0 };
	HouseTypeClass* OriginalHouseType { 0 };
	CDTimerClass CloakSkipTimer {}; //
	CDTimerClass DisableWeaponTimer {};
	CDTimerClass SelfHealingCombatTimer {};
	int HijackerHealth { 0 };
	HouseClass* HijackerOwner { 0 };
	float HijackerVeterancy;
	BYTE gap74[4] { 0 };
	double AE_ROF { 0 };
	double AE_FirePowerMult { 0 };
	double AE_ArmorMult { 0 };
	double AE_SpeedMult { 0 };
	BYTE AE_Cloak { 0 };
	BYTE AltOccupy_HasValue { 0 };
	BYTE AltOccupy_Value { 0 };
	BYTE Is_SurvivorsDone { 0 };
	BYTE Is_DriverKilled { 0 };
	BYTE Is_Operated { 0 };
	BYTE Is_InitialPayloadDone { 0 };
	BYTE Is_UnitLostMuted { 0 };
	SuperClass* AttachedSuperWeapon { 0 };
	AbstractClass* SuperTarget { 0 };
	int TechnoValueAmount { 0 };
	CellStruct EMPulseTarget { 0 };
	BYTE TakeVehicleMode { 0 };
	DWORD pad[2] { 0 };
};

static_assert(offsetof(AresTechnoExt, AEDatas) == 0x20, "ClassMember Shifted !");
static_assert(offsetof(AresTechnoExt, MyOriginalTemporal) == 0x34, "ClassMember Shifted !");
static_assert(offsetof(AresTechnoExt, AE_ROF) == 0x78, "ClassMember Shifted !");
static_assert(offsetof(AresTechnoExt, GarrisonedIn) == 0xC, "ClassMember Shifted !");
static_assert(offsetof(AresTechnoExt, EMPSparkleAnim) == 0x10, "ClassMember Shifted !");
static_assert(offsetof(AresTechnoExt, Is_DriverKilled) == 0x9C, "ClassMember Shifted !");
static_assert(offsetof(AresTechnoExt, TechnoValueAmount) == 0xA8, "ClassMember Shifted !");
static_assert(sizeof(AresTechnoExt) == 0xC0, "InvalidSize!");