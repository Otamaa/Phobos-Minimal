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
#include <Ext/RadSite/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/Sidebar/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/SWType/Body.h>
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

#pragma region OtamaaStuffs
#include <Ext/Bomb/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/SHPReference/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/Convert/Body.h>
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
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailType.h>
#endif
#pragma endregion


//#include <New/Entity/BannerClass.h>

#include <Commands/Commands.h>

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
// T::PointerGotInvalid(void*, bool)
// T::ExtMap.PointerGotInvalid(void*, bool)
struct InvalidatePointerAction
{
	template <typename T>
	static void Process(void* ptr, bool removed)
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

	__forceinline static void InvalidatePointer(void* ptr, bool removed)
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
	//Debug::Log("Finished saving the game\n");
	return S_OK;
}

void Phobos::LoadGameDataAfter(IStream* pStm)
{
	//clear the loadgame flag 
	Phobos::Otamaa::DoingLoadGame = false;
	//Debug::Log("Finished loading the game\n");
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

DEFINE_HOOK(0x7258D0, AnnounceInvalidPointer_PhobosGlobal, 0x6)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);

	Process_InvalidatePtr<BulletExt>(pInvalid, removed);
	Process_InvalidatePtr<SWTypeExt>(pInvalid, removed);
	Process_InvalidatePtr<TActionExt>(pInvalid, removed);
	return 0;
}

// Clear static data from respective classes
DEFINE_HOOK(0x685659, Scenario_ClearClasses_PhobosGlobal, 0xA)
{
	ArmorTypeClass::Clear();
	BannerTypeClass::Clear();
	ColorTypeClass::Clear();
	ImmunityTypeClass::Clear();
	CursorTypeClass::Clear();
	ElectricBoltManager::Clear();
	FlyingStrings::Clear();
	PaletteManager::Clear();
	RadTypeClass::Clear();
	RulesExt::Clear();
	ScenarioExt::Clear();
	SidebarExt::Clear();
	ShieldTypeClass::Clear();
	TacticalExt::Clear();
	TrailType::Clear();
	HoverTypeClass::Clear();
	LaserTrailTypeClass::Clear();
	TActionExt::ExtMap.Clear();
	HouseExt::ExtMap.Clear();
	TunnelTypeClass::Clear();

	return 0;
}

DEFINE_HOOK(0x67D04E, Game_Save_SavegameInformation, 0x7)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x4A4, 0x3F4));
	Info.Version = Info.Version + SAVEGAME_ID;
	return 0;
}

DEFINE_HOOK(0x559F27, LoadOptionsClass_GetFileInfo, 0xA)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x400, 0x3F4));
	Info.Version = Info.Version - SAVEGAME_ID;
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

DEFINE_HOOK(0x67D32C, SaveGame_Phobos_Global, 0x5)
{
	//Debug::Log("Saving global Phobos data\n");
	GET(IStream*, pStm, ESI);

	bool ret =
		Process_Save<PaletteManager>(pStm) &&
		Process_Save<CursorTypeClass>(pStm) &&
		Process_Save<ArmorTypeClass>(pStm) &&
		Process_Save<ImmunityTypeClass>(pStm) &&
		Process_Save<ColorTypeClass>(pStm) &&
		Process_Save<HouseExt>(pStm) &&
		Process_Save<WeaponTypeExt>(pStm) &&
		Process_Save<SWTypeExt>(pStm) &&
		Process_Save<BuildingTypeExt>(pStm) &&
		Process_Save<BulletExt>(pStm) &&
		Process_Save<RadTypeClass>(pStm) &&
		Process_Save<ShieldTypeClass>(pStm) &&
		Process_Save<HoverTypeClass>(pStm) &&
		Process_Save<BannerTypeClass>(pStm) &&
		Process_Save<TrailType>(pStm) &&
		Process_Save<LaserTrailTypeClass>(pStm) && 
		Process_Save<TunnelTypeClass>(pStm)
		;

	if (!ret)
		Debug::Log("[Phobos] Global SaveGame Failed !\n");

	return 0;
}

DEFINE_HOOK(0x67E826, LoadGame_Phobos_Global, 0x6)
{
	//Debug::Log("Loading global Phobos data\n");
	GET(IStream*, pStm, ESI);
	Phobos::Otamaa::DoingLoadGame = true;

	bool ret = 
		Process_Load<PaletteManager>(pStm) &&
		Process_Load<CursorTypeClass>(pStm) &&
		Process_Load<ArmorTypeClass>(pStm) &&
		Process_Load<ImmunityTypeClass>(pStm) &&
		Process_Load<ColorTypeClass>(pStm) &&
		Process_Load<HouseExt>(pStm) &&
		Process_Load<WeaponTypeExt>(pStm) &&
		Process_Load<SWTypeExt>(pStm) &&
		Process_Load<BuildingTypeExt>(pStm) &&
		Process_Load<BulletExt>(pStm) &&
		Process_Load<RadTypeClass>(pStm) &&
		Process_Load<ShieldTypeClass>(pStm) &&
		Process_Load<HoverTypeClass>(pStm) &&
		Process_Load<BannerTypeClass>(pStm) &&
		Process_Load<TrailType>(pStm) &&
		Process_Load<LaserTrailTypeClass>(pStm) &&
		Process_Load<TunnelTypeClass>(pStm)
		;
	
	if (!ret)
		Debug::Log("[Phobos] Global LoadGame Failed !\n");

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

#pragma endregion