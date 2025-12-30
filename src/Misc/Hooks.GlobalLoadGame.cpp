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
#include <Ext/Trigger/Body.h>
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

	if (count > 0)
	{
		//the game memory is quite sensitive at this state
		//do pre allocation of the vector instead of dynamicly expand
		//this will relive memory system quite a much
		//and also avoiding crash
		collection.reserve(count);
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
	int count;

	// Read count
	hr = stream->Read(&count, sizeof(int), 0);
	if (FAILED(hr)) return hr;

	// Clear and prepare collection
	collection.clear();

	if (count > 0)
	{
		collection.reserve(count);

		// Load array data
		for (int i = 0; i < count; ++i)
		{
			T itemPtr = nullptr;
			hr = stream->Read(&itemPtr, sizeof(T), 0);
			if (FAILED(hr)) return hr;

			// Add to collection if space available
			collection.push_back(itemPtr);
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
#include <Ext/Scenario/Body.h>

HRESULT Decode_All_Pointers(LPSTREAM stream)
{
	HRESULT hr = S_OK;

	Phobos::Otamaa::DoingLoadGame = true;
	ScenarioClass::ClearScenario();

	hr = ScenarioClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	ScenarioClass::IsUserInputLocked = ScenarioClass::Instance->UserInputLocked;

	hr = LoadObjectVector(stream, *SideClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	PrepareDisplaySurfaces();

	hr = EvadeClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	Theater::Init(ScenarioClass::Instance->Theater);

	hr = RulesClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	hr = MouseClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	hr = Game::Load_Misc_Values(stream);
	if (!SUCCEEDED(hr)) return hr;

	MapClass::Instance->Clear_SubzoneTracking();

	hr = MapClass::Logics->Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	if (auto pInstance = TacticalClass::Instance()) {
		pInstance->ClearPtr();
	}

	LPVOID tacticalMapObj = nullptr;
	hr = OleLoadFromStream(stream, IID_IUnknown, &tacticalMapObj);
	if (!SUCCEEDED(hr)) return hr;

	TacticalExtData::Allocate(TacticalClass::Instance());

	hr = LoadObjectVector(stream, *AnimTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TubeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TiberiumClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *HouseTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *HouseClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *UnitTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *UnitClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *InfantryTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *InfantryClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *BuildingTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *BuildingClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *AircraftTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *AircraftClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *AnimClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TaskForceClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TeamTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TeamClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *ScriptTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *ScriptClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TagTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TagClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TriggerTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TriggerClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *AITriggerTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TActionClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TEventClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *FactoryClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *VoxelAnimTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *VoxelAnimClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *WarheadTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *WeaponTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *ParticleTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *ParticleClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *ParticleSystemTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *ParticleSystemClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *BulletTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *BulletClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *WaypointPathClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *SmudgeTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *OverlayTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *LightSourceClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *BuildingLightClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *EMPulseClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *SuperWeaponTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *SuperClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadSimpleArray(stream, *SuperClass::ShowTimers);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadSimpleArray(stream, *BuildingClass::Secrets);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TerrainTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TerrainClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *FoggedObjectClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *AlphaShapeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *WaveClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	bool success = VeinholeMonsterClass::LoadVector(stream);
	if(!success) return E_FAIL;;

	success = RadarEventClass::LoadVector(stream);
	if(!success) return E_FAIL;;

	hr = LoadObjectVector(stream, *CaptureManagerClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *DiskLaserClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *ParasiteClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *TemporalClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *AirstrikeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *SpawnManagerClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *SlaveManagerClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	AircraftTrackerClass::Instance->Clear();
	hr = AircraftTrackerClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	Kamikaze::Instance->Clear();
	hr = Kamikaze::Instance->Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	BombListClass::Instance->Clear();
	hr = BombListClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *BombClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *RadSiteClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	// Game options section (known problematic area)
	if (SessionClass::Instance->GameMode == GameMode::Skirmish) {
		Debug::Log("Reading Skirmish Session.Options\n");
		const bool save_GameOptionsType = GameOptionsType::Instance->Load(stream);
		if (!save_GameOptionsType) {
			Debug::Log("\t***** GameOptionsType LOAD FAILED!\n");
			return E_FAIL;
		}
	}

	// Audio/visual systems
	hr = VocClass::Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	hr = VoxClass::Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	hr = ThemeClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) return hr;

	int value;
	ULONG out = 0;
	hr = stream->Read(&value, sizeof(value), &out);
	if (!SUCCEEDED(hr)) return hr;

	VoxClass::EVAIndex = value;

	// Final Phobos data
	hr = Phobos::LoadGameDataAfter(stream);
	if (!SUCCEEDED(hr)) return hr;

	// add more variable that need to be reset after loading an saved games
	if (SessionClass::Instance->GameMode == GameMode::Campaign)
	{
		if (std::exchange(Unsorted::MuteSWLaunches(), false))
		{// this will also make radar unusable
			auto pSide = SideClass::Array->operator[](HouseClass::CurrentPlayer()->Type->SideIndex);
			VoxClass::EVAIndex = SideExtContainer::Instance.Find(pSide)->EVAIndex;
		}
		// this variable need to be reset , especially after you play as an observer on skirmish
		// then load an save game of campaign mode , it will shutoff the radar and EVA's
	}

	// Post-load initialization
	MapClass::Instance->RedrawSidebar(2);
	if (SessionClass::Instance->GameMode == GameMode::Campaign)
	{
		ScenarioClass::ScenarioSaved = 1;
	}

	return S_OK;
}

#include <Phobos.SaveGame.h>

bool __fastcall Load_Saved_Game(const char* file_name)
{
	WCHAR wide_file_name[PATH_MAX];

	if (SpawnerMain::Configs::Enabled) {
		MultiByteToWideChar(CP_ACP, 0, SavedGames::FormatPath(file_name), -1, wide_file_name, std::size(wide_file_name));
	} else {
		MultiByteToWideChar(CP_ACP, 0, file_name, -1, wide_file_name, std::size(wide_file_name));
	}

	Debug::Log("\nLOADING GAME [%s]\n", file_name);

	if (!ExtensionSaveJson::Load(wide_file_name)) {
		Debug::FatalError("Cannot Load dll datas !");
		return false;
	}

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

	LARGE_INTEGER li = {};
	hr = docfile->Seek(li, STREAM_SEEK_SET, nullptr);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to seek CONTENTS stream to beginning.\n");
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

	Debug::Log("Calling Decode_All_Pointers().\n");

	if (FAILED(Decode_All_Pointers(stream))) {
		Debug::FatalErrorAndExit("Error loading save game \"%s\"!\n", file_name);
		return false;
	}

	Debug::Log("Unlinking content stream from decompressor.\n");
	hr = linkstream->Unlink_Stream(nullptr);
	if (FAILED(hr)) {
		Debug::FatalError("Unlinking content stream from decompressor.\n");
		return false;
	}

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