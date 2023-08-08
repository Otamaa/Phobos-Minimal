#pragma once

#include <GeneralDefinitions.h>

class AbstractClass;
class TechnoClass;
class BuildingClass;
class AnimClass;
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
	TechnoClass* OwnerObj; //0
	DWORD initState; //4
	BYTE idxSlot_Wave; //5
	BYTE idxSlot_Beam; //6
	BYTE idxSlot_Warp; //7
	BYTE idxSlot_Parasite; //8
	BuildingClass* GarrisonedIn; //C
	AnimClass* EMPSparkleAnim; //10
	Mission EMPLastMission; //

	class JammerClass
	{
		int LastScan;							//!< Frame number when the last scan was performed.
		TechnoClass* AttachedToObject;			//!< Pointer to game object this jammer is on
		bool Registered;						//!< Did I jam anything at all? Used to skip endless unjam calls.
	};
	static_assert(sizeof(JammerClass) == 0xC, "InvalidSize!");
	JammerClass* RadarJammerUptr;

	class PoweredUnitClass
	{
	public:
		TechnoClass* Techno;
		int LastScan;
		bool Powered;
	};
	static_assert(sizeof(PoweredUnitClass) == 0xC, "InvalidSize!");
	PoweredUnitClass* PoweredUnitUptr;

	struct AEData
	{
		struct AE
		{
			struct DataHere
			{
				BYTE Daaa[0x11];
				BYTE last;
			};
			DataHere* unk0; //pointer to type ?
			AnimClass* Anim;
			int Duration;
			HouseClass* Invoker;
		};

		//void InvalidatePointer(void* ptr) {
		//	for (auto first = &this->ArrFirst; first != &this->ArrLast; ++first) {
		//		if ((*first)->Anim == ptr)
		//			(*first)->Anim = nullptr;
		//	}
		//}

		//static void Update(AEData& data)
		//{
		//	if(!data.NeedToRecreateAnim) {
		//		data.NeedToRecreateAnim = true;
		//		for (auto first = &data.ArrFirst; first != &data.ArrLast; ++first) {
		//			if ((*first)->unk0->last) {
		//				if (auto pAnim = std::exchange((*first)->Anim, nullptr)) {
		//					pAnim->UnInit();
		//				}
		//			}
		//		}
		//	}
		//}

		AE* ArrFirst;
		AE* ArrLast;
		AE* ArrEnd;
		int Delay;
		bool NeedToRecreateAnim;
	};
	static_assert(sizeof(AEData) == 0x14, "InvalidSize!");

	AEData AEDatas;
	TemporalClass* MyOriginalTemporal;
	BuildingLightClass* BuildingLight;
	EBolt* MyBolt;
	HouseTypeClass* OriginalHouseType;
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
	AbstractClass* SuperTarget;
	int TechnoValueAmount;
	CellStruct EMPulseTarget;
	BYTE TakeVehicleMode;
	DWORD pad[2];
};
static_assert(offsetof(AresTechnoExt, GarrisonedIn) == 0xC, "ClassMember Shifted !");
static_assert(offsetof(AresTechnoExt, EMPSparkleAnim) == 0x10, "ClassMember Shifted !");
static_assert(offsetof(AresTechnoExt, Is_DriverKilled) == 0x9C, "ClassMember Shifted !");
static_assert(offsetof(AresTechnoExt, TechnoValueAmount) == 0xA8, "ClassMember Shifted !");
static_assert(sizeof(AresTechnoExt) == 0xC0, "InvalidSize!");