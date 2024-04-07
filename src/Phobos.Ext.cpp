#include <Phobos.h>
#include <Phobos.Ext.h>

#pragma region Ext
#include <Ext/Abstract/Body.h>
#include <Ext/AbstractType/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/AITriggerType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Bomb/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/DiskLaser/Body.h>
#include <Ext/Foot/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/IsometricTileType/Body.h>
#include <Ext/Mission/Body.h>
#include <Ext/Object/Body.h>
#include <Ext/ObjectType/Body.h>
#include <Ext/OverlayType/Body.h>
#include <Ext/Particle/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/ParticleSystem/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/Radio/Body.h>
#include <Ext/Radsite/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/ScriptType/Body.h>
#include <Ext/SHPReference/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Sidebar/Body.h>
#include <Ext/SmudgeType/Body.h>
#include <Ext/SpawnManager/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Surface/Body.h>
#include <Ext/Tactical/Body.h>
#include <Ext/TAction/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Temporal/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/TriggerType/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/UnitType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Wave/Body.h>
#include <Ext/WeaponType/Body.h>
#pragma endregion

#include <utility>
#include <LoadOptionsClass.h>

HRESULT Phobos::SaveGameDataAfter(IStream* pStm)
{
	Debug::Log("[Phobos] Finished saving the game\n");
	return S_OK;
}

void Phobos::LoadGameDataAfter(IStream* pStm)
{
	//clear the loadgame flag
	Phobos::Otamaa::DoingLoadGame = false;
	Debug::Log("[Phobos] Finished loading the game\n");
}

#pragma region Hooks

template<typename T>
FORCEINLINE void Process_InvalidatePtr(AbstractClass* pInvalid, bool const removed)
{
	if constexpr (HasExtMap<T>)
	{
		if constexpr (PointerInvalidationIgnorAble<decltype(T::ExtMap)> &&
				PointerInvalidationSubscribable<decltype(T::ExtMap)>) {
			if (!T::ExtMap.InvalidateIgnorable(pInvalid)) {
				T::ExtMap.InvalidatePointer(pInvalid, removed);
			}
		}
		else if(PointerInvalidationSubscribable<decltype(T::ExtMap)>)
		{
			T::ExtMap.InvalidatePointer(pInvalid, removed);
		}
	}
	else
	{
		if constexpr (PointerInvalidationIgnorAble<T> &&
				PointerInvalidationSubscribable<T>) {
			if (!T::InvalidateIgnorable(pInvalid)) {
				T::InvalidatePointer(pInvalid, removed);
			}
		}
		else if(PointerInvalidationSubscribable<T>)
		{
			T::InvalidatePointer(pInvalid, removed);
		}
	}
}

DEFINE_HOOK(0x7258D0, AnnounceInvalidPointer_PhobosGlobal, 0x6)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);

	if(Phobos::Otamaa::ExeTerminated)
		return 0;

	//Process_InvalidatePtr<SWTypeExtContainer>(pInvalid, removed);

	return 0;
}

// Clear static data from respective classes
DEFINE_HOOK(0x685659, Scenario_ClearClasses_PhobosGlobal, 0xA)
{
	//TechnoTypeExtContainer::Instance.Clear();
	//BulletTypeExtContainer::Instance.Clear();
	//BuildingTypeExtContainer::Instance.Clear();
	//HouseTypeExtContainer::Instance.Clear();
	//IsometricTileTypeExtContainer::Instance.Clear();
	//OverlayTypeExtContainer::Instance.Clear();
	//SWStateMachine::Clear();
	//RulesExtData::Clear();
	//ScenarioExtData::Clear();
	//SWTypeExtContainer::Instance.Clear();
	//SidebarExtData::Clear();
	//HouseExtContainer::Instance.Clear();
	//WeaponTypeExtContainer::Instance.Clear();
	//WarheadTypeExtContainer::Instance.Clear();

	return 0;
}

// Ares saves its things at the end of the save
// Phobos will save the things at the beginning of the save
// Considering how DTA gets the scenario name, I decided to save it after Rules - secsome

template<typename T>
FORCEINLINE bool Process_Load(IStream* pStm)
{
	PhobosByteStream stm(0);
	stm.ReadBlockFromStream(pStm);
	PhobosStreamReader reader(stm);

	if constexpr (HasExtMap<T>)
		return T::ExtMap.LoadGlobals(reader) && reader.ExpectEndOfBlock();
	else
		return T::LoadGlobals(reader) && reader.ExpectEndOfBlock();
}

template<typename T>
FORCEINLINE bool Process_Save(IStream* pStm)
{
	PhobosByteStream stm;
	PhobosStreamWriter writer(stm);

	if constexpr (HasExtMap<T>)
		return T::ExtMap.SaveGlobals(writer) && stm.WriteBlockToStream(pStm);
	else
		return T::SaveGlobals(writer) && stm.WriteBlockToStream(pStm);
}

DEFINE_HOOK(0x67E826, LoadGame_Phobos_Global_Early, 0x6)
{
	Phobos::Otamaa::DoingLoadGame = true;
	return 0;
}

//there some classes that need to be re-init after load game done
//maybe worth taking a look at it at some point -Otamaa
DEFINE_HOOK(0x67E65E, LoadGame_Phobos_AfterEverything, 0x6)
{
	GET_STACK(IStream*, pStm, 0x10);
	Phobos::LoadGameDataAfter(pStm);
	return 0;
}

DEFINE_HOOK(0x67D1B4, SaveGame_Phobos_AfterEverything, 0x6)
{
	GET_STACK(IStream*, pStm, 0x1C);
	Phobos::SaveGameDataAfter(pStm);
	return 0;
}

DEFINE_HOOK(0x67D300, SaveGame_Start, 5)
{
	Debug::Log("About to save the game\n");
	return 0;
}

DEFINE_HOOK(0x67E730, LoadGame_Start, 5)
{
	Debug::Log("About to load the game\n");
	return 0;
}

DEFINE_HOOK(0x67F7C8, LoadGame_Phobos_Global_EndPart, 5)
{
	GET(IStream*, pStm, ESI);

	//bool ret =
	//	Process_Load<HouseExtContainer>(pStm) &&
	//	Process_Load<IsometricTileTypeExtContainer>(pStm) &&
	//	Process_Load<WeaponTypeExtContainer>(pStm) &&
	//	Process_Load<SWTypeExtContainer>(pStm) &&
	//	Process_Load<BuildingTypeExtContainer>(pStm);

	//if (!ret)
	//	Debug::Log("[Phobos] Global LoadGame Failed !\n");

	//// add more variable that need to be reset after loading an saved games
	//if(SessionClass::Instance->GameMode == GameMode::Campaign)
	//{
	//	Unsorted::MuteSWLaunches = false; // this will also make radar unusable
	//	// this variable need to be reset , especially after you play as an observer on skirmish
	//	// then load an save game of campaign mode , it will shutoff the radar and EVA's
	//}

	return 0;
}

DEFINE_HOOK(0x67E42E, SaveGame_Phobos_Global_EndPart, 5)
{
	GET(HRESULT, Status, EAX);

	//if (SUCCEEDED(Status))
	//{
	//	GET(IStream*, pStm, ESI);

	//	bool ret =
	//		Process_Save<HouseExtContainer>(pStm) &&
	//		Process_Save<IsometricTileTypeExtContainer>(pStm) &&
	//		Process_Save<WeaponTypeExtContainer>(pStm) &&
	//		Process_Save<SWTypeExtContainer>(pStm) &&
	//		Process_Save<BuildingTypeExtContainer>(pStm)
	//		;

	//	if (!ret)
	//		Debug::Log("[Phobos] Global SaveGame Failed !\n");

	//	R->EAX<HRESULT>(ret ? S_OK : E_FAIL);
	//}

	return 0x0;
}

#pragma endregion