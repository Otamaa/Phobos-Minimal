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
#include <Ext/Ebolt/Body.h>
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
#include <New/Type/ImmunityTypeClass.h>
#include <New/Type/RocketTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/TechTreeTypeClass.h>
#include <New/Type/ThemeTypeClass.h>
#include <New/Type/HealthBarTypeClass.h>
#include <New/Type/BarTypeClass.h>
#include <New/Type/InsigniaTypeClass.h>
#include <New/Type/SelectBoxTypeClass.h>

#include <New/Entity/BannerClass.h>

#include <New/HugeBar.h>

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
#include <New/Entity/SWFirerClass.h>

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
		if COMPILETIMEEVAL (Clearable<T>)
			T::Clear();
		else if COMPILETIMEEVAL (HasExtMap<T>)
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
		if COMPILETIMEEVAL (HasExtMap<T> && PointerInvalidationSubscribable<T>) {
			T::ExtMap::PointerGotInvalid(ptr, removed);
		}
		else if COMPILETIMEEVAL(PointerInvalidationSubscribable<T>)
		{
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
		if COMPILETIMEEVAL (GlobalSaveLoadable<T>)
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
		if COMPILETIMEEVAL (GlobalSaveLoadable<T>)
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
	COMPILETIMEEVAL __forceinline static void Clear()
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
	Debug::LogInfo("[Phobos] Finished saving the game");
	return S_OK;
}

#include <BeaconManagerClass.h>

void Phobos::LoadGameDataAfter(IStream* pStm)
{
	//clear the loadgame flag
	Phobos::Otamaa::DoingLoadGame = false;

	if (auto pPlayerSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex)) {
		if (auto pSideMouse = SideExtContainer::Instance.Find(pPlayerSide)->MouseShape) {
			GameDelete<true, true>(std::exchange(MouseClass::ShapeData(), pSideMouse));
		}
	}

	BeaconManagerClass::Instance->LoadArt();
	Debug::LogInfo("[Phobos] Finished loading the game");
}

#pragma region Hooks
// Global Pointer Invalidation Hooks

template<typename T>
FORCEDINLINE void Process_InvalidatePtr(AbstractClass* pInvalid, bool const removed)
{
	if COMPILETIMEEVAL (HasExtMap<T>)
	{
		if COMPILETIMEEVAL (PointerInvalidationIgnorAble<decltype(T::ExtMap)> &&
			PointerInvalidationSubscribable<decltype(T::ExtMap)>) {
			T::ExtMap.InvalidatePointer(pInvalid, removed);
		}
		else if(PointerInvalidationSubscribable<decltype(T::ExtMap)>)
		{
			T::ExtMap.InvalidatePointer(pInvalid, removed);
		}
	}
	else
	{
		T::InvalidatePointer(pInvalid, removed);
	}
}

ASMJIT_PATCH(0x7258D0, AnnounceInvalidPointer_PhobosGlobal, 0x6)
{
	GET(AbstractClass* const, pInvalid, ECX);
	GET(bool const, removed, EDX);

	if(Phobos::Otamaa::ExeTerminated)
		return 0;

	TActionExtData::InvalidatePointer(pInvalid, removed);
	PhobosGlobal::PointerGotInvalid(pInvalid, removed);
	SWStateMachine::PointerGotInvalid(pInvalid, removed);
	Process_InvalidatePtr<SWTypeExtContainer>(pInvalid, removed);
	if (removed) {
		HouseExtData::AutoDeathObjects.erase_all_if([pInvalid](std::pair<TechnoClass*, KillMethod>& item) {
			return item.first == pInvalid;
		});
	}

	HugeBar::InvalidatePointer(pInvalid, removed);
	ShieldClass::Array.for_each([pInvalid , removed](ShieldClass* pShield) {
		if (pShield->IdleAnim.get() == pInvalid) {
			pShield->IdleAnim.release();
		}

	 });

	//SpawnManagerClass::Array->for_each([&](SpawnManagerClass* pThis) {
	//	if (pThis->Owner && removed) {
	//		for (int i = 0; i < pThis->SpawnedNodes.Count; ++i) {
	//			if (pThis->SpawnedNodes[i] && pThis->SpawnedNodes[i]->Unit == pInvalid) {
	//				pThis->SpawnedNodes[i]->Unit = nullptr;
	//				pThis->SpawnedNodes[i]->Status = SpawnNodeStatus::Dead;
	//			}
	//		}
	//	}
	//});

	//for (int i = 0; i < MapClass::Instance->Cells.Capacity; i++) {
	//	if (auto pCell = MapClass::Instance->Cells.Items[i]) {
	//		fast_remove_if(CellExtContainer::Instance.Find(pCell)->RadSites,
	//		[pInvalid](auto _el) { return pInvalid == _el; });
	//	}
	//}

	// EBolt::Array->for_each([&](EBolt* pThis) {
	// 	if (removed && pThis->Owner == pInvalid) {
	// 		pThis->Owner = nullptr;
	// 	}
	// });

	//PrismForwarding::Array.for_each([&](auto& pThis) {
	//	if (pThis) {
	//		pThis->InvalidatePointer(pInvalid, removed);
	//	}
	//});
	//Process_InvalidatePtr<TActionExt>(pInvalid, removed);
	return 0;
}

//ASMJIT_PATCH(0x48CFB7, Game_Exit_RecordPoolSize, 0x6)
//{
//	TheMemoryPoolFactory->reportAllocation();
//	Debug::Log("PaletteManager %d\n", PaletteManager::Array.size());
//	return 0x0;
//}

#include <New/Interfaces/AdvancedDriveLocomotionClass.h>
#include <New/Interfaces/LevitateLocomotionClass.h>
#include <New/Interfaces/CustomRocketLocomotionClass.h>

unsigned Phobos::GetVersionNumber() {
	unsigned version = AresGlobalData::InternalVersion;

	version += sizeof(AnimExtData);
	version += sizeof(AnimTypeExtData);

	version += sizeof(BuildingExtData);
	version += sizeof(BuildingTypeExtData);

	version += sizeof(BulletExtData);
	version += sizeof(BulletTypeExtData);

	version += sizeof(CellExtData);

	version += sizeof(EboltExtData);

	version += sizeof(HouseExtData);
	version += sizeof(HouseTypeExtData);

	version += sizeof(InfantryExtData);
	version += sizeof(InfantryTypeExtData);

	version += sizeof(ParticleExtData);
	version += sizeof(ParticleTypeExtData);

	version += sizeof(ParticleSystemExtData);
	version += sizeof(ParticleSystemTypeExtData);

	version += sizeof(RadSiteExtData);
	version += sizeof(RulesExtData);
	version += sizeof(ScenarioExtData);
	version += sizeof(SidebarExtData);

	version += sizeof(TeamExtData);
	version += sizeof(SmudgeTypeExtData);

	version += sizeof(SuperExtData);
	version += sizeof(SWTypeExtData);

	version += sizeof(TechnoExtData);
	version += sizeof(TechnoTypeExtData);

	version += sizeof(VoxelAnimExtData);
	version += sizeof(VoxelAnimTypeExtData);

	version += sizeof(WeaponTypeExtData);
	version += sizeof(WarheadTypeExtData);
	version += sizeof(WaveExtData);

	version += sizeof(LaserTrailClass);
	version += sizeof(LauchSWData);

	version += sizeof(TActionExtData);

	version += sizeof(ShieldClass);
	version += sizeof(SWFirerClass);

	version += sizeof(AdvancedDriveLocomotionClass);
	version += sizeof(CustomRocketLocomotionClass);
	version += sizeof(LevitateLocomotionClass);
	version += sizeof(GenericPrerequisite);
	version += sizeof(Prereqs);
	version += sizeof(PaletteManager);
	version += sizeof(HugeBar);
	version += sizeof(StaticVars);

	version += sizeof(BannerClass);

#define AddTypeOf(cccc) version += sizeof(cccc##TypeClass);
		AddTypeOf(Armor)
		AddTypeOf(Banner)
		AddTypeOf(Bar)
		AddTypeOf(Color)
		AddTypeOf(Crate)
		AddTypeOf(Cursor)
		AddTypeOf(DigitalDisplay)
		AddTypeOf(HealthBar)
		AddTypeOf(Hover)
		AddTypeOf(Immunity)
		AddTypeOf(Insignia)
		AddTypeOf(LaserTrail)
		AddTypeOf(Rad)
		AddTypeOf(Rocket)
		AddTypeOf(SelectBox)
		AddTypeOf(Shield)
		AddTypeOf(TechTree)
		AddTypeOf(Theater)
		AddTypeOf(Theme)
		AddTypeOf(Tunnel)
#undef AddTypeOf
	return version;
}

// Clear static data from respective classes
// this function is executed after all game classes already cleared
ASMJIT_PATCH(0x685659, Scenario_ClearClasses_PhobosGlobal, 0xA)
{
	TActionExtData::Clear();
	CellExtContainer::Instance.Clear();
	PrismForwarding::Array.clear();
	MouseClassExt::ClearCameos();
	FakeAnimClass::Clear();
	TechnoTypeExtContainer::Instance.Clear();
	BulletTypeExtContainer::Instance.Clear();
	BuildingTypeExtContainer::Instance.Clear();
	HouseTypeExtContainer::Instance.Clear();
	IsometricTileTypeExtContainer::Instance.Clear();
	OverlayTypeExtContainer::Instance.Clear();
	TiberiumExtContainer::Instance.Clear();
	PhobosGlobal::Clear();
	SWStateMachine::Clear();
	ArmorTypeClass::Clear();
	BannerTypeClass::Clear();
	ColorTypeClass::Clear();
	DigitalDisplayTypeClass::Clear();
	ImmunityTypeClass::Clear();
	CursorTypeClass::Clear();
	//ElectricBoltManager::Clear();
	FlyingStrings::Clear();
	//PaletteManager::Clear();
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
	EboltExtData::Clear();
	WarheadTypeExtContainer::Instance.Clear();
	GenericPrerequisite::Clear();
	CrateTypeClass::Clear();
	StaticVars::Clear();
	PhobosAttachEffectTypeClass::Clear();
	PhobosAttachEffectTypeClass::GroupsMap.clear();
	TechTreeTypeClass::Clear();
	HugeBar::Clear();
	RocketTypeClass::Clear();
	BarTypeClass::Clear();
	SWFirerClass::Clear();
	ShieldClass::Array.clear();
	InsigniaTypeClass::Clear();
	SelectBoxTypeClass::Clear();
	BannerClass::Clear();

	if (!Phobos::Otamaa::ExeTerminated)
	{
		if (TheMemoryPoolFactory)
			TheMemoryPoolFactory->reset();

		VoxelAnimExtContainer::pools.init(4026);
		TeamExtContainer::pools.init(4026);
		RadSiteExtContainer::pools.init(4026);
		ParticleSystemExtContainer::pools.init(4026);
		ParticleExtContainer::pools.init(4026);
		BulletExtContainer::pools.init(4026);
		FakeAnimClass::pools.init(10000);
		TechnoExtContainer::pools.init(4026);
		SWFirerClass::Array.reserve(1000);
		CellExtContainer::Array.reserve(2000);
	}

	return 0;
}

#undef LogPool

// Ares saves its things at the end of the save
// Phobos will save the things at the beginning of the save
// Considering how DTA gets the scenario name, I decided to save it after Rules - secsome

template<typename T>
FORCEDINLINE bool Process_Load(IStream* pStm)
{
	PhobosByteStream stm(0);
	stm.ReadBlockFromStream(pStm);
	PhobosStreamReader reader(stm);

	if COMPILETIMEEVAL (HasExtMap<T>)
		return T::ExtMap.LoadGlobals(reader) && reader.ExpectEndOfBlock();
	else
		return T::LoadGlobals(reader) && reader.ExpectEndOfBlock();
}

template<typename T>
FORCEDINLINE bool Process_Save(IStream* pStm)
{
	PhobosByteStream stm;
	PhobosStreamWriter writer(stm);

	if COMPILETIMEEVAL (HasExtMap<T>)
		return T::ExtMap.SaveGlobals(writer) && stm.WriteBlockToStream(pStm);
	else
		return T::SaveGlobals(writer) && stm.WriteBlockToStream(pStm);
}

//ASMJIT_PATCH(0x67D32C, SaveGame_Phobos_Global, 0x5)
//{
//	Debug::LogInfo("Saving global Phobos data");
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
//		Debug::LogInfo("[Phobos] Global SaveGame Failed !");
//
//	return 0;
//}
//
ASMJIT_PATCH(0x67E826, LoadGame_Phobos_Global_Early, 0x6)
{
//	Debug::LogInfo("Loading global Phobos data");
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
//		Debug::LogInfo("[Phobos] Global LoadGame Failed !");
//
	return 0;
}

//there some classes that need to be re-init after load game done
//maybe worth taking a look at it at some point -Otamaa
ASMJIT_PATCH(0x67E65E, LoadGame_Phobos_AfterEverything, 0x6)
{
	GET_STACK(IStream*, pStm, 0x10);
	Phobos::LoadGameDataAfter(pStm);
	return 0;
}

ASMJIT_PATCH(0x67D1B4, SaveGame_Phobos_AfterEverything, 0x6)
{
	GET_STACK(IStream*, pStm, 0x1C);
	Phobos::SaveGameDataAfter(pStm);
	return 0;
}

ASMJIT_PATCH(0x67D300, SaveGame_Start, 5)
{
	Debug::LogInfo("About to save the game");
	return 0;
}

ASMJIT_PATCH(0x67E730, LoadGame_Start, 5)
{
	Debug::LogInfo("About to load the game");
	return 0;
}

ASMJIT_PATCH(0x67F7C8, LoadGame_Phobos_Global_EndPart, 5)
{
	GET(IStream*, pStm, ESI);

	int value;
	ULONG out = 0;

	if (!SUCCEEDED(pStm->Read(&value, sizeof(value), &out))) {
		Debug::LogInfo("[Phobos] Global LoadGame Failed !");
		return 0x0;
	}

	VoxClass::EVAIndex = value;

	bool ret =
		Process_Load<FakeAnimClass>(pStm) &&
		Process_Load<CursorTypeClass>(pStm) &&
		Process_Load<MouseClassExt>(pStm) &&
		Process_Load<DigitalDisplayTypeClass>(pStm) &&
		Process_Load<ArmorTypeClass>(pStm) &&
		Process_Load<ImmunityTypeClass>(pStm) &&
		Process_Load<ColorTypeClass>(pStm) &&
		Process_Load<HouseExtContainer>(pStm) &&
		Process_Load<IsometricTileTypeExtContainer>(pStm) &&
		Process_Load<WeaponTypeExtContainer>(pStm) &&
		Process_Load<EboltExtData>(pStm) &&
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
		Process_Load<NewSWType>(pStm) &&
		Process_Load<TiberiumExtContainer>(pStm) &&
		Process_Load<PhobosAttachEffectTypeClass>(pStm) &&
		Process_Load<TechTreeTypeClass>(pStm) &&
		Process_Load<StaticVars>(pStm) &&
		Process_Load<HugeBar>(pStm) &&
		Process_Load<RocketTypeClass>(pStm) &&
		Process_Load<BarTypeClass>(pStm) &&
		Process_Load<SWFirerClass>(pStm) &&
		Process_Load<InsigniaTypeClass>(pStm) &&
		Process_Load<SelectBoxTypeClass>(pStm) &&
		Process_Load<ShieldClass>(pStm) &&
		Process_Load<PrismForwarding>(pStm) &&
		Process_Load<BannerClass>(pStm) &&
		Process_Load<TActionExtData>(pStm)
		;

	if (!ret)
		Debug::LogInfo("[Phobos] Global LoadGame Failed !");

	// add more variable that need to be reset after loading an saved games
	if(SessionClass::Instance->GameMode == GameMode::Campaign)
	{
		if(std::exchange(Unsorted::MuteSWLaunches(), false)) {// this will also make radar unusable
			auto pSide = SideClass::Array->operator[](HouseClass::CurrentPlayer()->Type->SideIndex);
			VoxClass::EVAIndex = SideExtContainer::Instance.Find(pSide)->EVAIndex;
		}
		// this variable need to be reset , especially after you play as an observer on skirmish
		// then load an save game of campaign mode , it will shutoff the radar and EVA's
	}

	return 0;
}

ASMJIT_PATCH(0x67E42E, SaveGame_Phobos_Global_EndPart, 5)
{
	GET(HRESULT, Status, EAX);

	if (SUCCEEDED(Status))
	{
		GET(IStream*, pStm, ESI);

		ULONG out = 0;
		const int value = VoxClass::EVAIndex();

		if (!SUCCEEDED(pStm->Write(&value, sizeof(value), &out)))
		{
			Debug::LogInfo("[Phobos] Global SaveGame Failed !");
			R->EAX<HRESULT>(E_FAIL);
			return 0x0;
		}

		bool ret =
			Process_Save<FakeAnimClass>(pStm) &&
			Process_Save<CursorTypeClass>(pStm) &&
			Process_Save<MouseClassExt>(pStm) &&
			Process_Save<DigitalDisplayTypeClass>(pStm) &&
			Process_Save<ArmorTypeClass>(pStm) &&
			Process_Save<ImmunityTypeClass>(pStm) &&
			Process_Save<ColorTypeClass>(pStm) &&
			Process_Save<HouseExtContainer>(pStm) &&
			Process_Save<IsometricTileTypeExtContainer>(pStm) &&
			Process_Save<WeaponTypeExtContainer>(pStm) &&
			Process_Save<EboltExtData>(pStm) &&
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
			Process_Save<NewSWType>(pStm) &&
			Process_Save<TiberiumExtContainer>(pStm) &&
			Process_Save<PhobosAttachEffectTypeClass>(pStm) &&
			Process_Save<TechTreeTypeClass>(pStm) &&
			Process_Save<StaticVars>(pStm) &&
			Process_Save<HugeBar>(pStm) &&
			Process_Save<RocketTypeClass>(pStm) &&
			Process_Save<BarTypeClass>(pStm) &&
			Process_Save<SWFirerClass>(pStm) &&
			Process_Save<InsigniaTypeClass>(pStm) &&
			Process_Save<SelectBoxTypeClass>(pStm) &&
			Process_Save<ShieldClass>(pStm) &&
			Process_Save<PrismForwarding>(pStm) &&
			Process_Save<BannerClass>(pStm) &&
			Process_Save<TActionExtData>(pStm)
			;

		if (!ret)
			Debug::LogInfo("[Phobos] Global SaveGame Failed !");

		R->EAX<HRESULT>(ret ? S_OK : E_FAIL);
	}

	return 0x0;
}

#pragma endregion