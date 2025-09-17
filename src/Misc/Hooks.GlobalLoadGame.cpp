#include <ScenarioClass.h>
#include <EvadeClass.h>
#include <MouseClass.h>
#include <TacticalClass.h>

#include <AnimTypeClass.h>
#include <AnimClass.h>
#include <TubeClass.h>
#include <HouseTypeClass.h>
#include <HouseClass.h>
#include <UnitTypeClass.h>
#include <UnitClass.h>
#include <InfantryTypeClass.h>
#include <InfantryClass.h>
#include <BuildingTypeClass.h>
#include <BuildingClass.h>
#include <AircraftTypeClass.h>
#include <AircraftClass.h>

#include <TaskForceClass.h>
#include <TeamTypeClass.h>
#include <TeamClass.h>
#include <ScriptTypeClass.h>
#include <ScriptClass.h>
#include <TagTypeClass.h>
#include <TagClass.h>
#include <TriggerTypeClass.h>
#include <TriggerClass.h>
#include <AITriggerTypeClass.h>
#include <AITrigger.h>
#include <TActionClass.h>
#include <TEventClass.h>
#include <TerrainTypeClass.h>
#include <TerrainClass.h>
#include <SmudgeTypeClass.h>
#include <SmudgeClass.h>
#include <ThemeClass.h>
#include <FoggedObjectClass.h>
#include <AlphaShapeClass.h>
#include <WaveClass.h>
#include <VeinholeMonsterClass.h>
#include <RadarEventClass.h>
#include <SpawnManagerClass.h>
#include <SlaveManagerClass.h>

#include <AircraftTrackerClass.h>
#include <Kamikaze.h>
#include <BombListClass.h>

#include <GScreenClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Swizzle.h>

#include <Phobos.Ext.h>

#include <atlbase.h>
#include <atlcomcli.h>
#include <objidlbase.h>

#include <New/Type/CursorTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/Type/ImmunityTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/TunnelTypeClass.h>
#include <New/Type/InsigniaTypeClass.h>
#include <New/Type/SelectBoxTypeClass.h>
#include <New/Type/TheaterTypeClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Bomb/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/UnitType/Body.h>
#include <Ext/Unit/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/InfantryType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/OverlayType/Body.h>
#include <Ext/ParticleType/Body.h>
#include <Ext/Particle/Body.h>
#include <Ext/ParticleSystemType/Body.h>
#include <Ext/ParticleSystem/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/SmudgeType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/Super/Body.h>
#include <Ext/TAction/Body.h>
#include <Ext/Tactical/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Temporal/Body.h>
#include <Ext/TEvent/Body.h>
#include <Ext/Tiberium/Body.h>
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Wave/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Misc/Ares/Hooks/Header.h>

#include <Misc/Spawner/Main.h>
#include <Misc/Spawner/SavedGamesInSubdir.h>

#include <CStreamClass.h>
#include <LoadOptionsClass.h>

//
template<typename T>
FORCEDINLINE bool Process_Global_Load(LPSTREAM stream)
{
	PhobosByteStream stm(0);
	stm.ReadBlockFromStream(stream);
	PhobosStreamReader reader(stm);

	Debug::LogInfo("[Process_Load] For object {} Start", PhobosCRT::GetTypeIDName<T>());
	return T::LoadGlobals(reader) && reader.ExpectEndOfBlock();
}

template<typename T>
FORCEDINLINE bool Process_Global_Load_SingleInstanceObject(LPSTREAM pStm)
{
	PhobosByteStream stm(0);
	stm.ReadBlockFromStream(pStm);
	PhobosStreamReader reader(stm);

	Debug::LogInfo("[Process_Save] For object {} Start", PhobosCRT::GetTypeIDName<T>());
	T::Instance()->LoadFromStream(reader);
	return reader.ExpectEndOfBlock();
}
//

template<typename T>
HRESULT LoadObjectVector(LPSTREAM stream, DynamicVectorClass<T>& collection)
{
	HRESULT hr;
	std::string typeName = PhobosCRT::GetTypeIDName<T>();

	int count;
	// Read the count
	hr = stream->Read(&count, sizeof(int), 0);
	if (FAILED(hr)) return hr;
	Debug::Log("LoadObjectVector<%s>: Loaded Count %d\n", typeName.c_str(), count);

	if (count > 0) {
		//the game memory is quite sensitive at this state
		//do pre allocation of the vector instead of dynamicly expand
		//this will relive memory system quite a much
		//and also avoiding crash
		collection.Reserve(count);
	}

	// Load each object
	for (int i = 0; i < count; ++i)
	{
		LPVOID objPtr = nullptr;
		hr = OleLoadFromStream(stream, IID_IUnknown, &objPtr);
		if (FAILED(hr)) return hr;
	}
	return S_OK;
}

template<typename T>
HRESULT LoadSimpleArray(LPSTREAM stream, DynamicVectorClass<T>& collection)
{
	HRESULT hr;
	DWORD count;

	// Read count
	hr = stream->Read(&count, sizeof(int), 0);
	if (FAILED(hr)) return hr;

	// Clear and prepare collection
	collection.Clear();

	if (count > 0)
	{
		collection.Reserve(count);

		// Load array data
		for (int i = 0; i < count; ++i)
		{
			T itemPtr = nullptr;
			hr = stream->Read(&itemPtr, sizeof(T), 0);
			if (FAILED(hr)) return hr;

			// Add to collection if space available
			collection.AddItem(itemPtr);
		}

		auto what = PhobosCRT::GetTypeIDName<T>();
		// Swizzle pointers
		for (int i = 0; i < count; ++i)
		{
			PHOBOS_SWIZZLE_REQUEST_POINTER_REMAP(collection.Items[i], what.c_str());
		}
	}

	return S_OK;
}

HRESULT PrepareDisplaySurfaces()
{
	if (!Game::Prep_For_Side(ScenarioClass::Instance->PlayerSideIndex))
	{
		return E_FAIL;
	}

	RectangleStruct composite_surf_rect, sidebar_surf_rect, tile_surf_rect, clip_bounds;

	// Get sidebar clip bounds
	RectangleStruct* sidebarBounds = Game::Get_Sidebar_Clip_Bounds(&composite_surf_rect);

	// Setup rectangles
	sidebar_surf_rect.X = 0;
	sidebar_surf_rect.Y = 0;
	sidebar_surf_rect.Width = 168;
	sidebar_surf_rect.Height = DSurface::WindowBounds->Height;

	tile_surf_rect.X = 0;
	tile_surf_rect.Y = 0;
	tile_surf_rect.Width = sidebarBounds->Width;
	tile_surf_rect.Height = DSurface::WindowBounds->Height;

	clip_bounds.X = sidebarBounds->X;
	clip_bounds.Y = sidebarBounds->Y;
	clip_bounds.Width = sidebarBounds->Width;
	clip_bounds.Height = sidebarBounds->Height;

	composite_surf_rect.Width = sidebarBounds->Width;
	composite_surf_rect.Height = DSurface::WindowBounds->Height;
	composite_surf_rect.X = 0;
	composite_surf_rect.Y = 0;

	// Allocate surfaces
	Allocate_Surfaces(DSurface::WindowBounds.operator->(), &composite_surf_rect, &tile_surf_rect, &sidebar_surf_rect, 0);
	DisplayClass::Instance->Set_View_Dimensions(clip_bounds);

	return S_OK;
}

#include <Utilities/StreamUtils.h>

HRESULT Decode_All_Pointers_WithValidation(LPSTREAM stream, SavePositionTracker& tracker)
{
	Debug::Log("=== ENHANCED LOAD WITH POSITION VALIDATION ===\n");
	HRESULT hr = S_OK;

	// Track initial position
	ULARGE_INTEGER startPos = tracker.GetCurrentPosition();
	Debug::Log("About to load the game - starting at position: %llu\n", startPos.QuadPart);

	Phobos::Otamaa::DoingLoadGame = true;
	ScenarioClass::ClearScenario();

	// Phobos extension data
	tracker.StartOperation("Process_Global_Load<Phobos>");
	bool success = Process_Global_Load<Phobos>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<CursorTypeClass>");
	success = Process_Global_Load<CursorTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<ColorTypeClass>");
	success = Process_Global_Load<ColorTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("ScenarioClass::Instance->Load");
	hr = ScenarioClass::Instance->Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	ScenarioClass::IsUserInputLocked = ScenarioClass::Instance->UserInputLocked;

	tracker.StartOperation("Process_Global_Load<SideExtContainer>");
	success = Process_Global_Load<SideExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(SideClass::Array)");
	hr = LoadObjectVector(stream, *SideClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<TheaterTypeClass>");
	success = Process_Global_Load<TheaterTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	PrepareDisplaySurfaces();

	tracker.StartOperation("EvadeClass::Instance->Load");
	hr = EvadeClass::Instance->Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	Theater::Init(ScenarioClass::Instance->Theater);

	tracker.StartOperation("RulesClass::Instance->Load");
	hr = RulesClass::Instance->Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<CellExtContainer>");
	success = Process_Global_Load<CellExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<MouseClassExt>");
	success = Process_Global_Load<MouseClassExt>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("MouseClass::Instance->Load");
	hr = MouseClass::Instance->Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Game::Load_Misc_Values");
	hr = Game::Load_Misc_Values(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	MapClass::Instance->Clear_SubzoneTracking();

	tracker.StartOperation("LogicClass::Instance->Load");
	hr = LogicClass::Instance->Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	if (auto pInstance = TacticalClass::Instance()) {
		pInstance->ClearPtr();
	}

	tracker.StartOperation("OleLoadFromStream(TacticalClass)");
	LPVOID tacticalMapObj = nullptr;
	hr = OleLoadFromStream(stream, IID_IUnknown, &tacticalMapObj);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	TacticalExtData::Allocate(TacticalClass::Instance());

	tracker.StartOperation("Process_Global_Load_SingleInstanceObject<TacticalExtData>");
	success = Process_Global_Load_SingleInstanceObject<TacticalExtData>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<DigitalDisplayTypeClass>");
	success = Process_Global_Load<DigitalDisplayTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<ArmorTypeClass>");
	success = Process_Global_Load<ArmorTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<ImmunityTypeClass>");
	success = Process_Global_Load<ImmunityTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<LaserTrailTypeClass>");
	success = Process_Global_Load<LaserTrailTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<TunnelTypeClass>");
	success = Process_Global_Load<TunnelTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<InsigniaTypeClass>");
	success = Process_Global_Load<InsigniaTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<SelectBoxTypeClass>");
	success = Process_Global_Load<SelectBoxTypeClass>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<AnimTypeExtContainer>");
	success = Process_Global_Load<AnimTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(AnimTypeClass::Array)");
	hr = LoadObjectVector(stream, *AnimTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(TubeClass::Array)");
	hr = LoadObjectVector(stream, *TubeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<TiberiumExtContainer>");
	success = Process_Global_Load<TiberiumExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(TiberiumClass::Array)");
	hr = LoadObjectVector(stream, *TiberiumClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<HouseTypeExtContainer>");
	success = Process_Global_Load<HouseTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(HouseTypeClass::Array)");
	hr = LoadObjectVector(stream, *HouseTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<HouseExtContainer>");
	success = Process_Global_Load<HouseExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(HouseClass::Array)");
	hr = LoadObjectVector(stream, *HouseClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<UnitTypeExtContainer>");
	success = Process_Global_Load<UnitTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(UnitTypeClass::Array)");
	hr = LoadObjectVector(stream, *UnitTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<UnitExtContainer>");
	success = Process_Global_Load<UnitExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(UnitClass::Array)");
	hr = LoadObjectVector(stream, *UnitClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<InfantryTypeExtContainer>");
	success = Process_Global_Load<InfantryTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(InfantryTypeClass::Array)");
	hr = LoadObjectVector(stream, *InfantryTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<InfantryExtContainer>");
	success = Process_Global_Load<InfantryExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(InfantryClass::Array)");
	hr = LoadObjectVector(stream, *InfantryClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<BuildingTypeExtContainer>");
	success = Process_Global_Load<BuildingTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(BuildingTypeClass::Array)");
	hr = LoadObjectVector(stream, *BuildingTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<BuildingExtContainer>");
	success = Process_Global_Load<BuildingExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(BuildingClass::Array)");
	hr = LoadObjectVector(stream, *BuildingClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<AircraftTypeExtContainer>");
	success = Process_Global_Load<AircraftTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(AircraftTypeClass::Array)");
	hr = LoadObjectVector(stream, *AircraftTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<AircraftExtContainer>");
	success = Process_Global_Load<AircraftExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(AircraftClass::Array)");
	hr = LoadObjectVector(stream, *AircraftClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<AnimExtContainer>");
	success = Process_Global_Load<AnimExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(AnimClass::Array)");
	hr = LoadObjectVector(stream, *AnimClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(TaskForceClass::Array)");
	hr = LoadObjectVector(stream, *TaskForceClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(TeamTypeClass::Array)");
	hr = LoadObjectVector(stream, *TeamTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<TeamExtContainer>");
	success = Process_Global_Load<TeamExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(TeamClass::Array)");
	hr = LoadObjectVector(stream, *TeamClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(ScriptTypeClass::Array)");
	hr = LoadObjectVector(stream, *ScriptTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(ScriptClass::Array)");
	hr = LoadObjectVector(stream, *ScriptClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(TagTypeClass::Array)");
	hr = LoadObjectVector(stream, *TagTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(TagClass::Array)");
	hr = LoadObjectVector(stream, *TagClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(TriggerTypeClass::Array)");
	hr = LoadObjectVector(stream, *TriggerTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(TriggerClass::Array)");
	hr = LoadObjectVector(stream, *TriggerClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(AITriggerTypeClass::Array)");
	hr = LoadObjectVector(stream, *AITriggerTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(TActionClass::Array)");
	hr = LoadObjectVector(stream, *TActionClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<TActionExtData>");
	success = Process_Global_Load<TActionExtData>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Load<TEventExtContainer>");
	success = Process_Global_Load<TEventExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(TEventClass::Array)");
	hr = LoadObjectVector(stream, *TEventClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(FactoryClass::Array)");
	hr = LoadObjectVector(stream, *FactoryClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<VoxelAnimTypeExtContainer>");
	success = Process_Global_Load<VoxelAnimTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(VoxelAnimTypeClass::Array)");
	hr = LoadObjectVector(stream, *VoxelAnimTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<VoxelAnimExtContainer>");
	success = Process_Global_Load<VoxelAnimExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(VoxelAnimClass::Array)");
	hr = LoadObjectVector(stream, *VoxelAnimClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<WarheadTypeExtContainer>");
	success = Process_Global_Load<WarheadTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(WarheadTypeClass::Array)");
	hr = LoadObjectVector(stream, *WarheadTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<WeaponTypeExtContainer>");
	success = Process_Global_Load<WeaponTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(WeaponTypeClass::Array)");
	hr = LoadObjectVector(stream, *WeaponTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<ParticleTypeExtContainer>");
	success = Process_Global_Load<ParticleTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(ParticleTypeClass::Array)");
	hr = LoadObjectVector(stream, *ParticleTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<ParticleExtContainer>");
	success = Process_Global_Load<ParticleExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(ParticleClass::Array)");
	hr = LoadObjectVector(stream, *ParticleClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<ParticleSystemTypeExtContainer>");
	success = Process_Global_Load<ParticleSystemTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(ParticleSystemTypeClass::Array)");
	hr = LoadObjectVector(stream, *ParticleSystemTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<ParticleSystemExtContainer>");
	success = Process_Global_Load<ParticleSystemExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(ParticleSystemClass::Array)");
	hr = LoadObjectVector(stream, *ParticleSystemClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<BulletTypeExtContainer>");
	success = Process_Global_Load<BulletTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(BulletTypeClass::Array)");
	hr = LoadObjectVector(stream, *BulletTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<BulletExtContainer>");
	success = Process_Global_Load<BulletExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(BulletClass::Array)");
	hr = LoadObjectVector(stream, *BulletClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(WaypointPathClass::Array)");
	hr = LoadObjectVector(stream, *WaypointPathClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<SmudgeTypeExtContainer>");
	success = Process_Global_Load<SmudgeTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(SmudgeTypeClass::Array)");
	hr = LoadObjectVector(stream, *SmudgeTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<OverlayTypeExtContainer>");
	success = Process_Global_Load<OverlayTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(OverlayTypeClass::Array)");
	hr = LoadObjectVector(stream, *OverlayTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(LightSourceClass::Array)");
	hr = LoadObjectVector(stream, *LightSourceClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(BuildingLightClass::Array)");
	hr = LoadObjectVector(stream, *BuildingLightClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(EMPulseClass::Array)");
	hr = LoadObjectVector(stream, *EMPulseClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<SWTypeExtContainer>");
	success = Process_Global_Load<SWTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(SuperWeaponTypeClass::Array)");
	hr = LoadObjectVector(stream, *SuperWeaponTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<SuperExtContainer>");
	success = Process_Global_Load<SuperExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(SuperClass::Array)");
	hr = LoadObjectVector(stream, *SuperClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadSimpleArray(*SuperClass::ShowTimers)");
	hr = LoadSimpleArray(stream, *SuperClass::ShowTimers);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadSimpleArray(*BuildingClass::Secrets)");
	hr = LoadSimpleArray(stream, *BuildingClass::Secrets);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<TerrainTypeExtContainer>");
	success = Process_Global_Load<TerrainTypeExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(TerrainTypeClass::Array)");
	hr = LoadObjectVector(stream, *TerrainTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<TerrainExtContainer>");
	success = Process_Global_Load<TerrainExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(TerrainClass::Array)");
	hr = LoadObjectVector(stream, *TerrainClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(FoggedObjectClass::Array)");
	hr = LoadObjectVector(stream, *FoggedObjectClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(AlphaShapeClass::Array)");
	hr = LoadObjectVector(stream, *AlphaShapeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<WaveExtContainer>");
	success = Process_Global_Load<WaveExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(WaveClass::Array)");
	hr = LoadObjectVector(stream, *WaveClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("VeinholeMonsterClass::LoadVector");
	success = VeinholeMonsterClass::LoadVector(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("RadarEventClass::LoadVector");
	success = RadarEventClass::LoadVector(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(CaptureManagerClass::Array)");
	hr = LoadObjectVector(stream, *CaptureManagerClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(DiskLaserClass::Array)");
	hr = LoadObjectVector(stream, *DiskLaserClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(ParasiteClass::Array)");
	hr = LoadObjectVector(stream, *ParasiteClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<TemporalExtContainer>");
	success = Process_Global_Load<TemporalExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(TemporalClass::Array)");
	hr = LoadObjectVector(stream, *TemporalClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(AirstrikeClass::Array)");
	hr = LoadObjectVector(stream, *AirstrikeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(SpawnManagerClass::Array)");
	hr = LoadObjectVector(stream, *SpawnManagerClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("LoadObjectVector(SlaveManagerClass::Array)");
	hr = LoadObjectVector(stream, *SlaveManagerClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	AircraftTrackerClass::Instance->Clear();
	tracker.StartOperation("AircraftTrackerClass::Instance->Load");
	hr = AircraftTrackerClass::Instance->Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	Kamikaze::Instance->Clear();
	tracker.StartOperation("Kamikaze::Instance->Load");
	hr = Kamikaze::Instance->Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	BombListClass::Instance->Clear();
	tracker.StartOperation("BombListClass::Instance->Load");
	hr = BombListClass::Instance->Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<BombExtContainer>");
	success = Process_Global_Load<BombExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(BombClass::Array)");
	hr = LoadObjectVector(stream, *BombClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Load<RadSiteExtContainer>");
	success = Process_Global_Load<RadSiteExtContainer>(stream);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("LoadObjectVector(RadSiteClass::Array)");
	hr = LoadObjectVector(stream, *RadSiteClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Game options section (known problematic area)
	if (SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		tracker.StartOperation("GameOptionsType::Instance->Load");
		Debug::Log("Reading Skirmish Session.Options\n");
		const bool save_GameOptionsType = GameOptionsType::Instance->Load(stream);
		if (!tracker.EndOperation(save_GameOptionsType))
		{
			Debug::Log("\t***** GameOptionsType LOAD FAILED!\n");
			return E_FAIL;
		}
	}

	// Audio/visual systems
	tracker.StartOperation("VocClass::Load");
	hr = VocClass::Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("VoxClass::Load");
	hr = VoxClass::Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("ThemeClass::Instance->Load");
	hr = ThemeClass::Instance->Load(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Final Phobos data
	tracker.StartOperation("Phobos::LoadGameDataAfter");
	hr = Phobos::LoadGameDataAfter(stream);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Post-load initialization
	MapClass::Instance->RedrawSidebar(2);
	if (SessionClass::Instance->GameMode == GameMode::Campaign)
	{
		ScenarioClass::ScenarioSaved = 1;
	}

	// Log final summary
	ULARGE_INTEGER endPos = tracker.GetCurrentPosition();
	Debug::Log("=== LOAD COMPLETED SUCCESSFULLY ===\n");
	Debug::Log("Total bytes read: %llu\n", endPos.QuadPart - startPos.QuadPart);
	tracker.LogAllOperations();

	return S_OK;
}

bool __fastcall Load_Saved_Game(const char* file_name)
{
	WCHAR wide_file_name[PATH_MAX];

	if (SpawnerMain::Configs::Enabled) {
		MultiByteToWideChar(CP_ACP, 0, SavedGames::FormatPath(file_name), -1, wide_file_name, std::size(wide_file_name));
	} else {
		MultiByteToWideChar(CP_ACP, 0, file_name, -1, wide_file_name, std::size(wide_file_name));
	}

	Debug::Log("\nLOADING GAME [%s]\n", file_name);

	/**
	 *  Convert the file name to a wide string.
	 */

	Debug::Log("Opening DocFile\n");
	ATL::CComPtr<IStorage> storage;
	HRESULT hr = StgOpenStorage(wide_file_name, nullptr, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, nullptr, 0, &storage);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to open storage.\n");
		return false;
	}

	/**
	 *  Read the save file header.
	 */
	SavegameInformation saveversion;
	hr = saveversion.Read(storage);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to read version information.\n");
		return false;
	}

	storage.Release();
	SessionClass::Instance->GameMode = saveversion.GameType;
	SwizzleManagerClass::Instance->Reset();

	Debug::Log("Opening DocFile\n");
	hr = StgOpenStorage(wide_file_name, nullptr, STGM_SHARE_DENY_WRITE, nullptr, 0, &storage);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to open storage.\n");
		return false;
	}

	Debug::Log("Opening content stream.\n");
	ATL::CComPtr<IStream> docfile;
	hr = storage->OpenStream(L"CONTENTS", nullptr, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &docfile);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to open content stream.\n");
		return false;
	}

	Debug::Log("Linking content stream to decompressor.\n");
	IUnknown* pUnknown = nullptr;
	ATL::CComPtr<ILinkStream> linkstream;
	hr = CoCreateInstance(__uuidof(CStreamClass), nullptr, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&pUnknown);
	if (SUCCEEDED(hr)) {
		hr = OleRun(pUnknown);
		if (SUCCEEDED(hr)) {
			pUnknown->QueryInterface(__uuidof(ILinkStream), (void**)&linkstream);
		}
		pUnknown->Release();
	}

	hr = linkstream->Link_Stream(docfile);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to link stream to decompressor.\n");
		return false;
	}

	ATL::CComPtr<IStream> stream;
	linkstream->QueryInterface(__uuidof(IStream), (void**)&stream);
	Debug::Log("Creating stream wrapper for tracking.\n");

	auto wrappedStream = std::make_unique<StreamWrapperWithTracking>(stream);
	LoadPositionTracker tracker(wrappedStream.get(), "LOAD");

	Debug::Log("Calling Decode_All_Pointers().\n");

	if (FAILED(Decode_All_Pointers_WithValidation(wrappedStream.get(), tracker))) {
		Debug::FatalErrorAndExit("Error loading save game \"%s\"!\n", file_name);
		return false;
	}

	Debug::Log("Unlinking content stream from decompressor.\n");
	linkstream->Unlink_Stream(nullptr);

	SwizzleManagerClass::Instance->Reset();
	ScenarioClass::InitScenariostuff();
	TabClass::Instance->Init_IO();
	TabClass::Instance->Activate(1);
	SidebarClass::Instance->CloseWindow();
	TiberiumClass::sub_722D00();
	TiberiumClass::sub_0x722240();
	RadarClass::Instance->Map_AI();
	Game::InScenario2 = 1;
	Game::InScenario1 = 1;
	ScenarioClass::ToggleDisplayMode(1);
	Game::Reset_SomeShapes_Post_Movie();

	Debug::Log("LOADING GAME [%s] - Complete\n", file_name);

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x67E440, Load_Saved_Game)
