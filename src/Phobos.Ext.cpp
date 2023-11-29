#include <Phobos.h>
#include <Phobos.Ext.h>

#include <Ext/Aircraft/Body.h>
#include <Ext/AITriggerType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/IsometricTileType/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/Sidebar/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/SWStateMachine.h>
#include <Ext/SWType/SuperWeaponSidebar.h>
#include <Ext/TAction/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/TriggerType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/ParticleSystem/Body.h>
#include <Ext/ParticleSystemType/Body.h>

//#include <Misc/TriggerMPOwner.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/PaletteManager.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/Type/ColorTypeClass.h>
#include <New/Type/HoverTypeClass.h>
#include <New/Type/TheaterTypeClass.h>
#include <New/Type/CursorTypeClass.h>
#include <New/Type/BannerTypeClass.h>
#include <New/Type/TunnelTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/GenericPrerequisite.h>
#include <New/Type/CrateTypeClass.h>

#pragma region OtamaaStuffs
#include <Ext/Bomb/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/SHPReference/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/DiskLaser/Body.h>
#include <Ext/Parasite/Body.h>
#include <Ext/Particle/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/ScriptType/Body.h>
#include <Ext/SpawnManager/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Wave/Body.h>
#include <Ext/Temporal/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/SmudgeType/Body.h>
#include <Ext/OverlayType/Body.h>

#include <New/Entity/FlyingStrings.h>
#include <New/Entity/VerticalLaserClass.h>
#include <New/Entity/HomingMissileTargetTracker.h>

#include <Ext/Tactical/Body.h>

//#include <New/Entity/FoggedObject.h>
#include <Misc/DynamicPatcher/Trails/TrailType.h>

#include <Misc/Ares/Hooks/Header.h>
#pragma endregion

//#include <New/Entity/BannerClass.h>

#include <Commands/Commands.h>
#include <Misc/PhobosGlobal.h>

#include <utility>
#include <LoadOptionsClass.h>


#pragma region Implementation details

#pragma region Concepts

// a hack to check if some type can be used as a specialization of a template
template <template <class...> class Template, class... Args>
void DerivedFromSpecialization(const Template<Args...>&);

template <class T, template <class...> class Template>
concept DerivedFromSpecializationOf =
	requires(const T & t) { DerivedFromSpecialization<Template>(t); };

template<typename TExt>
concept HasExtMap = requires { { TExt::ExtMap } -> DerivedFromSpecializationOf<Container>; };

template <typename T>
concept Clearable = requires { T::Clear(); };

template <typename T>
concept GlobalSaveLoadable = requires
{
	T::LoadGlobals(std::declval<PhobosStreamReader&>());
	T::SaveGlobals(std::declval<PhobosStreamWriter&>());
};

template <typename TAction, typename TProcessed, typename... ArgTypes>
concept DispatchesAction =
	requires (ArgTypes... args) { TAction::template Process<TProcessed>(args...); };

#pragma endregion

struct ClearAction
{
	template <typename T>
	static void Process()
	{
		if constexpr (Clearable<T>)
			T::Clear();
		else if constexpr (HasExtMap<T>)
			T::ExtMap.Clear();
	}
};

// calls:
// T::PointerGotInvalid(AbstractClass*, bool)
// T::ExtMap.PointerGotInvalid(AbstractClass*, bool)
struct InvalidatePointerAction
{
	template <typename T>
	static void Process(AbstractClass* ptr, bool removed)
	{
		if constexpr (HasExtMap<T>)
		{
			if constexpr (PointerInvalidationIgnorAble<T>)
				if (!T::ExtMap::InvalidateIgnorable(ptr))
					return;

			if constexpr (PointerInvalidationSubscribable<T>)
				T::ExtMap::PointerGotInvalid(ptr, removed);
		}
		else
		{
			if constexpr (PointerInvalidationIgnorAble<T>)
				if (!T::InvalidateIgnorable(ptr))
					return;

			if constexpr (PointerInvalidationSubscribable<T>)
				T::PointerGotInvalid(ptr, removed);
		}
	}
};

// calls:
// T::LoadGlobals(PhobosStreamReader&)
struct LoadGlobalsAction
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		if constexpr (GlobalSaveLoadable<T>)
		{
			PhobosByteStream stm(0);
			stm.ReadBlockFromStream(pStm);
			PhobosStreamReader reader(stm);

			return T::LoadGlobals(reader) && reader.ExpectEndOfBlock();
		}
		else
		{
			return true;
		}
	}
};

// calls:
// T::SaveGlobals(PhobosStreamWriter&)
struct SaveGlobalsAction
{
	template <typename T>
	static bool Process(IStream* pStm)
	{
		if constexpr (GlobalSaveLoadable<T>)
		{
			PhobosByteStream stm;
			PhobosStreamWriter writer(stm);

			return T::SaveGlobals(writer) && stm.WriteBlockToStream(pStm);
		}
		else
		{
			return true;
		}
	}
};

// this is a complicated thing that calls methods on classes. add types to the
// instantiation of this type, and the most appropriate method for each type
// will be called with no overhead of virtual functions.
template <typename... RegisteredTypes>
struct TypeRegistry
{
	constexpr __forceinline static void Clear()
	{
		va_list args;
		va_start(args, count);

		dispatch_void_mass_action<ClearAction>();
	}

	__forceinline static void InvalidatePointer(AbstractClass* ptr, bool removed)
	{
		dispatch_void_mass_action<InvalidatePointerAction>(ptr, removed);
	}

	__forceinline static bool LoadGlobals(IStream* pStm)
	{
		return dispatch_bool_mass_action<LoadGlobalsAction>(pStm);
	}

	__forceinline static bool SaveGlobals(IStream* pStm)
	{
		return dispatch_bool_mass_action<SaveGlobalsAction>(pStm);
	}

private:
	// TAction: the method dispatcher class to call with each type
	// ArgTypes: the argument types to call the method dispatcher's Process() method
	template <typename TAction, typename... ArgTypes>
		requires (DispatchesAction<TAction, RegisteredTypes, ArgTypes...> && ...)
	__forceinline static void dispatch_void_mass_action(ArgTypes... args)
	{
		// (pack expression op ...) is a fold expression which
		// unfolds the parameter pack into a full expression
		(TAction::template Process<RegisteredTypes>(args...) ...);
	}

	template <typename TAction, typename... ArgTypes>
		requires (DispatchesAction<TAction, RegisteredTypes, ArgTypes...> && ...)
	__forceinline static bool dispatch_bool_mass_action(ArgTypes... args)
	{
		// (pack expression op ...) is a fold expression which
		// unfolds the parameter pack into a full expression
		return (TAction::template Process<RegisteredTypes>(args...) && ...);
	}
};

#pragma endregion

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
// Global Pointer Invalidation Hooks

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

DEFINE_OVERRIDE_HOOK(0x7258D0, AnnounceInvalidPointer_PhobosGlobal, 0x6)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);

	if(Phobos::Otamaa::ExeTerminated)
		return 0;

	PhobosGlobal::PointerGotInvalid(pInvalid, removed);
	SWStateMachine::PointerGotInvalid(pInvalid, removed);
	Process_InvalidatePtr<SWTypeExtContainer>(pInvalid, removed);
	//Process_InvalidatePtr<TActionExt>(pInvalid, removed);
	return 0;
}

// Clear static data from respective classes
DEFINE_HOOK(0x685659, Scenario_ClearClasses_PhobosGlobal, 0xA)
{
	MouseClassExt::ClearCameos();
	TechnoTypeExtContainer::Instance.Clear();
	BulletTypeExtContainer::Instance.Clear();
	BuildingTypeExtContainer::Instance.Clear();
	HouseTypeExtContainer::Instance.Clear();
	IsometricTileTypeExtContainer::Instance.Clear();
	OverlayTypeExtContainer::Instance.Clear();
	PhobosGlobal::Clear();
	SWStateMachine::Clear();
	ArmorTypeClass::Clear();
	BannerTypeClass::Clear();
	ColorTypeClass::Clear();
	DigitalDisplayTypeClass::Clear();
	ImmunityTypeClass::Clear();
	CursorTypeClass::Clear();
	ElectricBoltManager::Clear();
	FlyingStrings::Clear();
	PaletteManager::Clear();
	RadTypeClass::Clear();
	RulesExtData::Clear();
	ScenarioExtData::Clear();
	SWTypeExtContainer::Instance.Clear();
	SidebarExtData::Clear();
	ShieldTypeClass::Clear();
	//TacticalExt::Clear();
	TrailType::Clear();
	HoverTypeClass::Clear();
	LaserTrailTypeClass::Clear();
	//TActionExt::ExtMap.Clear();
	HouseExtContainer::Instance.Clear();
	TunnelTypeClass::Clear();
	WeaponTypeExtContainer::Instance.Clear();
	WarheadTypeExtContainer::Instance.Clear();
	SuperWeaponSidebar::Clear();
	GenericPrerequisite::Clear();
	CrateTypeClass::Clear();

	return 0;
}

//DEFINE_HOOK(0x559F27, LoadOptionsClass_GetFileInfo, 0xA)
//{
//	REF_STACK(SavegameInformation, Info, STACK_OFFSET(0x400, -0x3F4));
//	Info.Version = Info.Version - SAVEGAME_ID;
//	Info.InternalVersion = Info.InternalVersion - SAVEGAME_ID;
//	return 0;
//}
//DEFINE_HOOK(0x559F29, LoadOptionsClass_GetFileInfo, 0x8)
//{
//	if (!R->BL()) return 0x55A03D; // vanilla overridden check
//
//	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x400, 0x3F4));
//	//Info.Version = Info.Version - SAVEGAME_ID;
//	Info.InternalVersion = Info.InternalVersion - SAVEGAME_ID;
//	return 0x559F29 + 0x8;
//}

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

//DEFINE_HOOK(0x67D32C, SaveGame_Phobos_Global, 0x5)
//{
//	Debug::Log("Saving global Phobos data\n");
//	GET(IStream*, pStm, ESI);
//
//	bool ret =
//		Process_Save<PaletteManager>(pStm) &&
//		Process_Save<CursorTypeClass>(pStm) &&
//		Process_Save<MouseClassExt>(pStm) &&
//		Process_Save<DigitalDisplayTypeClass>(pStm) &&
//		Process_Save<ArmorTypeClass>(pStm) &&
//		Process_Save<ImmunityTypeClass>(pStm) &&
//		Process_Save<ColorTypeClass>(pStm) &&
//		Process_Save<HouseExtContainer>(pStm) &&
//		Process_Save<WeaponTypeExtContainer>(pStm) &&
//		Process_Save<SWTypeExtContainer>(pStm) &&
//		Process_Save<BuildingTypeExtContainer>(pStm) &&
//		Process_Save<RadTypeClass>(pStm) &&
//		Process_Save<ShieldTypeClass>(pStm) &&
//		Process_Save<HoverTypeClass>(pStm) &&
//		Process_Save<BannerTypeClass>(pStm) &&
//		Process_Save<TrailType>(pStm) &&
//		Process_Save<LaserTrailTypeClass>(pStm) &&
//		Process_Save<TunnelTypeClass>(pStm) &&
//		Process_Save<SWStateMachine>(pStm) &&
//		Process_Save<PhobosGlobal>(pStm) &&
//		Process_Save<GenericPrerequisite>(pStm) &&
//		Process_Save<CrateTypeClass>(pStm)
//		;
//
//	if (!ret)
//		Debug::Log("[Phobos] Global SaveGame Failed !\n");
//
//	return 0;
//}
//
DEFINE_HOOK(0x67E826, LoadGame_Phobos_Global_Early, 0x6)
{
//	Debug::Log("Loading global Phobos data\n");
//	GET(IStream*, pStm, ESI);
	Phobos::Otamaa::DoingLoadGame = true;
//
//	bool ret =
//		Process_Load<PaletteManager>(pStm) &&
//		Process_Load<CursorTypeClass>(pStm) &&
//		Process_Load<MouseClassExt>(pStm) &&
//		Process_Load<DigitalDisplayTypeClass>(pStm) &&
//		Process_Load<ArmorTypeClass>(pStm) &&
//		Process_Load<ImmunityTypeClass>(pStm) &&
//		Process_Load<ColorTypeClass>(pStm) &&
//		Process_Load<HouseExtContainer>(pStm) &&
//		Process_Load<WeaponTypeExtContainer>(pStm) &&
//		Process_Load<SWTypeExtContainer>(pStm) &&
//		Process_Load<BuildingTypeExtContainer>(pStm) &&
//		Process_Load<RadTypeClass>(pStm) &&
//		Process_Load<ShieldTypeClass>(pStm) &&
//		Process_Load<HoverTypeClass>(pStm) &&
//		Process_Load<BannerTypeClass>(pStm) &&
//		Process_Load<TrailType>(pStm) &&
//		Process_Load<LaserTrailTypeClass>(pStm) &&
//		Process_Load<TunnelTypeClass>(pStm) &&
//		Process_Load<SWStateMachine>(pStm) &&
//		Process_Load<PhobosGlobal>(pStm) &&
//		Process_Load<GenericPrerequisite>(pStm) &&
//		Process_Load<CrateTypeClass>(pStm)
//		;
//
//	if (!ret)
//		Debug::Log("[Phobos] Global LoadGame Failed !\n");
//
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

DEFINE_OVERRIDE_HOOK(0x67D300, SaveGame_Start, 5)
{
	Debug::Log("About to save the game\n");
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x67E730, LoadGame_Start, 5)
{
	Debug::Log("About to load the game\n");
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x67F7C8, LoadGame_Phobos_Global_EndPart, 5)
{
	GET(IStream*, pStm, ESI);

	bool ret =
		Process_Load<PaletteManager>(pStm) &&
		Process_Load<CursorTypeClass>(pStm) &&
		Process_Load<MouseClassExt>(pStm) &&
		Process_Load<DigitalDisplayTypeClass>(pStm) &&
		Process_Load<ArmorTypeClass>(pStm) &&
		Process_Load<ImmunityTypeClass>(pStm) &&
		Process_Load<ColorTypeClass>(pStm) &&
		Process_Load<HouseExtContainer>(pStm) &&
		Process_Load<IsometricTileTypeExtContainer>(pStm) &&
		Process_Load<WeaponTypeExtContainer>(pStm) &&
		Process_Load<SWTypeExtContainer>(pStm) &&
		Process_Load<BuildingTypeExtContainer>(pStm) &&
		Process_Load<RadTypeClass>(pStm) &&
		Process_Load<ShieldTypeClass>(pStm) &&
		Process_Load<HoverTypeClass>(pStm) &&
		Process_Load<BannerTypeClass>(pStm) &&
		Process_Load<TrailType>(pStm) &&
		Process_Load<LaserTrailTypeClass>(pStm) &&
		Process_Load<TunnelTypeClass>(pStm) &&
		Process_Load<SWStateMachine>(pStm) &&
		Process_Load<PhobosGlobal>(pStm) &&
		Process_Load<GenericPrerequisite>(pStm) &&
		Process_Load<CrateTypeClass>(pStm) &&
		Process_Load<NewSWType>(pStm)
		;

	if (!ret)
		Debug::Log("[Phobos] Global LoadGame Failed !\n");

	// add more variable that need to be reset after loading an saved games
	if(SessionClass::Instance->GameMode == GameMode::Campaign)
	{
		Unsorted::MuteSWLaunches = false; // this will also make radar unusable
		// this variable need to be reset , especially after you play as an observer on skirmish 
		// then load an save game of campaign mode , it will shutoff the radar and EVA's
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x67E42E, SaveGame_Phobos_Global_EndPart, 5)
{
	GET(HRESULT, Status, EAX);

	if (SUCCEEDED(Status))
	{
		GET(IStream*, pStm, ESI);

		bool ret =
			Process_Save<PaletteManager>(pStm) &&
			Process_Save<CursorTypeClass>(pStm) &&
			Process_Save<MouseClassExt>(pStm) &&
			Process_Save<DigitalDisplayTypeClass>(pStm) &&
			Process_Save<ArmorTypeClass>(pStm) &&
			Process_Save<ImmunityTypeClass>(pStm) &&
			Process_Save<ColorTypeClass>(pStm) &&
			Process_Save<HouseExtContainer>(pStm) &&
			Process_Save<IsometricTileTypeExtContainer>(pStm) &&
			Process_Save<WeaponTypeExtContainer>(pStm) &&
			Process_Save<SWTypeExtContainer>(pStm) &&
			Process_Save<BuildingTypeExtContainer>(pStm) &&
			Process_Save<RadTypeClass>(pStm) &&
			Process_Save<ShieldTypeClass>(pStm) &&
			Process_Save<HoverTypeClass>(pStm) &&
			Process_Save<BannerTypeClass>(pStm) &&
			Process_Save<TrailType>(pStm) &&
			Process_Save<LaserTrailTypeClass>(pStm) &&
			Process_Save<TunnelTypeClass>(pStm) &&
			Process_Save<SWStateMachine>(pStm) &&
			Process_Save<PhobosGlobal>(pStm) &&
			Process_Save<GenericPrerequisite>(pStm) &&
			Process_Save<CrateTypeClass>(pStm) &&
			Process_Save<NewSWType>(pStm)
			;

		if (!ret)
			Debug::Log("[Phobos] Global SaveGame Failed !\n");

		R->EAX<HRESULT>(ret ? S_OK : E_FAIL);
	}

	return 0x0;
}

#pragma endregion