#include "EntityManager.h"

#include <Base/Always.h>
#include <CoordStruct.h>
#include <Timers.h>
#include <GeneralDefinitions.h>
#include <ArrayClasses.h>

struct AbstractBase {
	DWORD AbstractFlags;
};

struct AbsractType {
	char ID[0x18];
	char UINameLabel[0x20];
	const wchar_t* UIName; //Full_Name
	char Name[0x31];
};

struct PlacementStatusses {
	bool IsOnMap;
	bool InLimbo;
	bool IsOnBridge;

};

struct Transform {
	CoordStruct Location;
};

struct Movememnt {
	CoordStruct LastLocation;
	bool IsMoving;
};

struct ObjectBase {
	int Health;
	int EstimateHealth;
	bool IsAlive;
	ObjectBase* NextObject; 	//Next Object in the same cell or transport. This is a linked list of Objects.
};

struct MissionBase
{
	Mission  CurrentMission;
	Mission  SuspendedMission; //B0
	Mission  QueuedMission;
	bool     unknown_bool_B8;
	int      MissionStatus;
	int      CurrentMissionStartTime;	//in frames
	DWORD    unknown_C4;
	CDTimerClass UpdateTimer;
};

struct RadioBase
{
	RadioCommand LastCommands[3]; // new command updates these
	VectorClass<RadioBase*> RadioLinks;	//Docked units etc
};

struct Drawable {

};

struct Aircraft {

};

struct Infantry {

};

struct Unit {

};


struct Logic_A {

};

struct TurretObject {

};

/*
	[Entity] -> [AbstractBase] //common classes
			 -> [AbstractBase] [[ObjectBase] [PlacementStatusses] [Transform] //manipulatable object]

						 -> [AbstractBase] [[ObjectBase] [PlacementStatusses] [Transform] [TurretObject] [MissionBase] [RadioBase] // Manipulatable object with mission]
						 -> [AbstractBase] [[ObjectBase] [PlacementStatusses] [Transform] [Drawable] [MissionBase] [RadioBase] // Manipulatable object with mission]
						 -> [AbstractBase] [[ObjectBase] [PlacementStatusses] [Transform] [Drawable] [Movememnt] [Aircraft] [MissionBase] [RadioBase] // Manipulatable object with mission]
						 -> [AbstractBase] [[ObjectBase] [PlacementStatusses] [Transform] [Drawable] [Movememnt] [Infantry] [MissionBase] [RadioBase] // Manipulatable object with mission]
						 -> [AbstractBase] [[ObjectBase] [PlacementStatusses] [Transform] [TurretObject] [Drawable] [Movememnt] [Unit] [MissionBase] [RadioBase] // Manipulatable object with mission]

			-> [AbstractBase] [AbsractType] //Type object

			 std::vector<Entity> Entities {};

			 void UpdateEntitiy()
			 {
				Entities.for_each([](auto& ent)) {
					if(!ent.contins(AbsractType)) {
						ent.AI(); //anything that doesnt have [AbsractType] is logic attached
					}
				}
			}
*/