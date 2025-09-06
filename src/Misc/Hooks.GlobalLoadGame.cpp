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
	int count;

	// Read the count
	hr = stream->Read(&count, sizeof(int), 0);
	if (FAILED(hr)) return hr;

	// Load each object
	for (int i = 0; i < count; ++i)
	{
		T objPtr = nullptr;
		hr = OleLoadFromStream(stream, IID_IUnknown, &((LPVOID)objPtr));
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

HRESULT Decode_All_Pointers(LPSTREAM stream)
{
	HRESULT hr = S_OK;
	Debug::LogInfo("About to load the game");
	Phobos::Otamaa::DoingLoadGame = true;

	ScenarioClass::ClearScenario();

	if (!Process_Global_Load<Phobos>(stream))
		return E_FAIL;

	if (!Process_Global_Load<CursorTypeClass>(stream))
		return E_FAIL;

	if (!Process_Global_Load<DigitalDisplayTypeClass>(stream))
		return E_FAIL;

	if (!Process_Global_Load<ArmorTypeClass>(stream))
		return E_FAIL;

	if (!Process_Global_Load<ImmunityTypeClass>(stream))
		return E_FAIL;

	if (!Process_Global_Load<LaserTrailTypeClass>(stream))
		return E_FAIL;

	if (!Process_Global_Load<TunnelTypeClass>(stream))
		return E_FAIL;

	if (!Process_Global_Load<InsigniaTypeClass>(stream))
		return E_FAIL;

	if (!Process_Global_Load<SelectBoxTypeClass>(stream))
		return E_FAIL;

	hr = ScenarioClass::Instance->Load(stream);
	if (FAILED(hr)) return hr;

	ScenarioClass::IsUserInputLocked = ScenarioClass::Instance->UserInputLocked;

	if (!Process_Global_Load<SideExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *SideClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<TheaterTypeClass>(stream))
		return E_FAIL;

	PrepareDisplaySurfaces();

	hr = EvadeClass::Instance->Load(stream);
	if (FAILED(hr)) return hr;

	Theater::Init(ScenarioClass::Instance->Theater);

	hr = RulesClass::Instance->Load(stream);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<AnimTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *AnimTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<MouseClassExt>(stream))
		return E_FAIL;

	hr = MouseClass::Instance->Load(stream);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *TubeClass::Array);
	if (FAILED(hr)) return hr;

	hr = Game::Load_Misc_Values(stream);
	if (FAILED(hr)) return hr;

	MapClass::Instance->Clear_SubzoneTracking();
	hr = LogicClass::Instance->Load(stream);
	if (FAILED(hr)) return hr;

	if (auto pInstance = TacticalClass::Instance()) {
		pInstance->ClearPtr();
	}

	LPVOID tacticalMapObj = nullptr;
	hr = OleLoadFromStream(stream, IID_IUnknown, &tacticalMapObj);
	if (FAILED(hr)) return hr;

	TacticalExtData::Allocate(TacticalClass::Instance());

	if (!Process_Global_Load_SingleInstanceObject<TacticalExtData>(stream))
		return E_FAIL;

	if (!Process_Global_Load<HouseTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *HouseTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<HouseExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *HouseClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<UnitTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *UnitTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<UnitExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *UnitClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<InfantryTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *InfantryTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<InfantryExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *InfantryClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<BuildingTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *BuildingTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<BuildingExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *BuildingClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<AircraftTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *AircraftTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<AircraftExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *AircraftClass::Array);
	if (FAILED(hr)) return hr;


	if (!Process_Global_Load<AnimExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *AnimClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *TaskForceClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *TeamTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<TeamExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *TeamClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *ScriptTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *ScriptClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *TagTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *TagClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *TriggerTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *TriggerClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *AITriggerTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *TActionClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<TEventExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *TEventClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *FactoryClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<VoxelAnimTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *VoxelAnimTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<VoxelAnimExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *VoxelAnimClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<WarheadTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *WarheadTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<WeaponTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *WeaponTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<ParticleTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *ParticleTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<ParticleExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *ParticleClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<ParticleSystemTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *ParticleSystemTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<ParticleSystemExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *ParticleSystemClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<BulletTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *BulletTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<BulletExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *BulletClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *WaypointPathClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<SmudgeTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *SmudgeTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<OverlayTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *OverlayTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *LightSourceClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *BuildingLightClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<TiberiumExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *TiberiumClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *EMPulseClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<SWTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *SuperWeaponTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<SuperExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *SuperClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadSimpleArray(stream, *SuperClass::ShowTimers);
	if (FAILED(hr)) return hr;

	hr = LoadSimpleArray(stream, *BuildingClass::Secrets);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<TerrainTypeExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *TerrainTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<TerrainExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *TerrainClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *FoggedObjectClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *AlphaShapeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<WaveExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *WaveClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *CaptureManagerClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *DiskLaserClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *ParasiteClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<TemporalExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *TemporalClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *AirstrikeClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *SpawnManagerClass::Array);
	if (FAILED(hr)) return hr;

	hr = LoadObjectVector(stream, *SlaveManagerClass::Array);
	if (FAILED(hr)) return hr;

	AircraftTrackerClass::Instance->Clear();
	hr = AircraftTrackerClass::Instance->Load(stream);
	if (FAILED(hr)) return hr;

	Kamikaze::Instance->Clear();
	hr = Kamikaze::Instance->Load(stream);
	if (FAILED(hr)) return hr;

	BombListClass::Instance->Clear();
	hr = BombListClass::Instance->Load(stream);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<BombExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *BombClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Load<RadSiteExtContainer>(stream))
		return E_FAIL;

	hr = LoadObjectVector(stream, *RadSiteClass::Array);
	if (FAILED(hr)) return hr;

	if (SessionClass::Instance->GameMode == GameMode::Skirmish) {
		Debug::Log("Reading Skirmish Session.Options\n");
		if (!GameOptionsType::Instance->Load(stream)) {
			Debug::Log("\t***** FAILED!\n");
			return E_FAIL;
		}
	}

	hr = VocClass::Load(stream);
	if (FAILED(hr)) return hr;

	hr = VoxClass::Load(stream);
	if (FAILED(hr)) return hr;

	hr = ThemeClass::Instance->Load(stream);
	if (FAILED(hr)) return hr;

	hr = Phobos::LoadGameDataAfter(stream);
	if (FAILED(hr)) return hr;

	MapClass::Instance->RedrawSidebar(2);
	if (SessionClass::Instance->GameMode == GameMode::Campaign) {
		ScenarioClass::ScenarioSaved = 1;
	}

	return S_OK;
}

bool __fastcall wrap_Decode_All_Pointers(LPSTREAM stream) {
	return SUCCEEDED(Decode_All_Pointers(stream));
}

DEFINE_FUNCTION_JUMP(CALL, 0x67E659, wrap_Decode_All_Pointers)