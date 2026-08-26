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
#include <VocClass.h>
#include <VoxClass.h>

#include <AircraftTrackerClass.h>
#include <Kamikaze.h>
#include <BombListClass.h>

#include <GScreenClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Swizzle.h>

#include <Phobos.Ext.h>
#include <Phobos.Lua.h>

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
#include <New/Type/GenericPrerequisite.h>
#include <New/Type/BannerTypeClass.h>
#include <New/Type/BarTypeClass.h>
#include <New/Type/HealthBarTypeClass.h>
#include <New/Type/RocketTypeClass.h>
#include <New/Type/ThemeTypeClass.h>
#include <New/Type/HoverTypeClass.h>

#include <New/Entity/FlyingStrings.h>
#include <New/Entity/SWFirerClass.h>
#include <New/Entity/BannerClass.h>
#include <New/Entity/FoggedObject.h>

#include <New/ChoiceBox/Entities/Base/MapChoiceBoxClass.h>
#include <New/ChoiceBox/Types/ChoiceBoxTypeClass.h>

#include <New/TextBox/Entities/Base/MapTextBoxClass.h>
#include <New/TextBox/Types/TextBoxTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Aircraft/Body.h>
#include <Ext/AircraftType/Body.h>
#include <Ext/AITriggerType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <Ext/Bomb/Body.h>
#include <Ext/CaptureManager/Body.h>
#include <Ext/Cell/Body.h>
#include <Ext/DiskLaser/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Script/Body.h>
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
#include <Ext/Parasite/Body.h>
#include <Ext/RadSite/Body.h>
#include <Ext/SmudgeType/Body.h>
#include <Ext/SWType/Body.h>
#include <Ext/SWType/NewSuperWeaponType/SWStateMachine.h>
#include <Ext/Super/Body.h>
#include <Ext/ScriptType/Body.h>
#include <Ext/TAction/Body.h>
#include <Ext/Tactical/Body.h>
#include <Ext/TerrainType/Body.h>
#include <Ext/Terrain/Body.h>
#include <Ext/TaskForce/Body.h>
#include <Ext/TeamType/Body.h>
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
#include <Ext/Mouse/Body.h>
#include <Ext/IonBlast/Body.h>

#include <Misc/Spawner/Main.h>
#include <Misc/Spawner/SavedGamesInSubdir.h>

#include <CStreamClass.h>
#include <LoadOptionsClass.h>
#include <AirstrikeClass.h>
#include <WaypointPathClass.h>
#include <EMPulseClass.h>
#include <CaptureManagerClass.h>

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
		if (FAILED(hr))
		{
			Debug::Log("LoadObjectVector<%s>: FAILED at index %d/%d (hr=0x%08X)\n",
				typeName.c_str(), i + 1, count, hr);
			return hr;
		}
	}
	Debug::Log("LoadObjectVector<%s>: Done (%d objects)\n", typeName.c_str(), count);
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

template<typename T>
HRESULT LoadSimpleData(LPSTREAM stream, T& data)
{
	HRESULT hr;

	// Read count
	hr = stream->Read(&data, sizeof(T), 0);
	if (FAILED(hr)) return hr;

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
	Allocate_Surfaces(DSurface::WindowBounds(), composite_surf_rect, tile_surf_rect, sidebar_surf_rect, 0);
	DisplayClass::Instance->Set_View_Dimensions(clip_bounds);

	return S_OK;
}

#include <Utilities/StreamUtils.h>
#include <Ext/Scenario/Body.h>

template<typename T>
HRESULT ReadBlocksFromStream(IStream* pStm)
{
	const auto typeName = PhobosCRT::GetTypeIDName<T>();
	Debug::Log("[ExtLoad] Loading %s ...\n", typeName.c_str());

	PhobosByteStream loader(0);
	if (!loader.ReadFromStream(pStm))
	{
		Debug::Log("[ExtLoad]   FAILED %s - ReadFromStream returned false\n", typeName.c_str());
		return S_FALSE;
	}

	PhobosStreamReader reader(loader);
	if (!T::LoadGlobals(reader))
	{
		Debug::Log("[ExtLoad]   FAILED %s - LoadGlobals returned false\n", typeName.c_str());
		return S_FALSE;
	}

	if (!reader.ExpectEndOfBlock())
	{
		Debug::Log("[ExtLoad]   FAILED %s - ExpectEndOfBlock (stream desync or leftover bytes)\n", typeName.c_str());
		return S_FALSE;
	}

	Debug::Log("[ExtLoad]   OK %s\n", typeName.c_str());
	return S_OK;
}

template<typename T>
HRESULT ReadBlocksFromStreamStreamB(T& who_, IStream* pStm)
{
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;
	const auto typeName = PhobosCRT::GetTypeIDName<Base>();
	Debug::Log("[ExtLoad] Loading (B) %s ...\n", typeName.c_str());

	PhobosByteStream loader(0);
	if (!loader.ReadFromStream(pStm))
	{
		Debug::Log("[ExtLoad]   FAILED (B) %s - ReadFromStream returned false\n", typeName.c_str());
		return S_FALSE;
	}

	PhobosStreamReader reader(loader);
	bool _LoadResult = false;

	if constexpr (std::is_pointer_v<T>)
	{
		_LoadResult = who_->LoadGlobal(reader);
	}
	else
	{
		_LoadResult = who_.LoadGlobal(reader);
	}

	if (!_LoadResult)
	{
		Debug::Log("[ExtLoad]   FAILED (B) %s - LoadGlobal returned false\n", typeName.c_str());
		return S_FALSE;
	}

	if (!reader.ExpectEndOfBlock())
	{
		Debug::Log("[ExtLoad]   FAILED (B) %s - ExpectEndOfBlock (stream desync or leftover bytes)\n", typeName.c_str());
		return S_FALSE;
	}

	Debug::Log("[ExtLoad]   OK (B) %s\n", typeName.c_str());
	return S_OK;
}

template<typename T>
HRESULT ReadBlocksFromStreamStreamC(T& who_, IStream* pStm)
{
	const auto typeName = PhobosCRT::GetTypeIDName<T>();
	Debug::Log("[ExtLoad] Loading (C) %s ...\n", typeName.c_str());

	PhobosByteStream loader(0);
	if (!loader.ReadFromStream(pStm))
	{
		Debug::Log("[ExtLoad]   FAILED (C) %s - ReadFromStream returned false\n", typeName.c_str());
		return S_FALSE;
	}

	PhobosStreamReader reader(loader);
	if (!who_.LoadAll(reader))
	{
		Debug::Log("[ExtLoad]   FAILED (C) %s - LoadAll returned false\n", typeName.c_str());
		return S_FALSE;
	}

	if (!reader.ExpectEndOfBlock())
	{
		Debug::Log("[ExtLoad]   FAILED (C) %s - ExpectEndOfBlock (stream desync or leftover bytes)\n", typeName.c_str());
		return S_FALSE;
	}

	Debug::Log("[ExtLoad]   OK (C) %s\n", typeName.c_str());
	return S_OK;
}

template<typename T>
HRESULT ReadBlocksFromStreamStreamD(T& who_, IStream* pStm, DWORD canary)
{
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;
	const auto typeName = PhobosCRT::GetTypeIDName<Base>();
	Debug::Log("[ExtLoad] Loading (B) %s ...\n", typeName.c_str());

	PhobosByteStream loader(0);
	if (!loader.ReadFromStream(pStm))
	{
		Debug::Log("[ExtLoad]   FAILED (B) %s - ReadFromStream returned false\n", typeName.c_str());
		return S_FALSE;
	}

	PhobosStreamReader reader(loader);

	if (!reader.Expect(canary))
		return false;

	int Count = 0;
	if (!reader.Load(Count))
		return false;

	for (int i = 0; i < Count; ++i)
	{
		uintptr_t savedPtr {};
		if (!reader.Load(savedPtr))
			return false;

		auto buffer = new Base::ExtT();

		//specifically for immedietely restore the extension pointers 
		ExtensionSwizzleManager::RegisterExtensionPointer((void*)savedPtr, buffer);
		//for others that need to be remapped using vanilla swizzle manager
		PhobosSwizzleManager.Here_I_Am(savedPtr, buffer);

		Debug::Log("Loading %s [At (%d) - %x] to stream\n",
				Base::ClassName, i, savedPtr);

		buffer->LoadFromStream(reader);
		if (!reader.Success())
			return false;

		who_.Array.emplace_back(buffer);
	}

	if (!reader.ExpectEndOfBlock())
	{
		Debug::Log("[ExtLoad]   FAILED (B) %s - ExpectEndOfBlock (stream desync or leftover bytes)\n", typeName.c_str());
		return S_FALSE;
	}

	Debug::Log("[ExtLoad]   OK (B) %s\n", typeName.c_str());
	return S_OK;
}

HRESULT Phobos::LoadAllEarlyData(IStream* pStm)
{
	HRESULT hr = S_OK;

	hr = ReadBlocksFromStream<Phobos>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<CursorTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<TheaterTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<BannerTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<ArmorTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<ThemeTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<DigitalDisplayTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStreamStreamB(FlyingStrings::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<ChoiceBoxTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<MapChoiceBoxClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStreamStreamD(CellExtContainer::Instance, pStm, CellExtData::Canary);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStreamStreamD(TiberiumExtContainer::Instance, pStm, TiberiumExtData::Canary);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStreamStreamD(UnitTypeExtContainer::Instance, pStm, UnitTypeExtData::Canary);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStreamStreamD(InfantryTypeExtContainer::Instance, pStm, InfantryTypeExtData::Canary);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStreamStreamD(BuildingTypeExtContainer::Instance, pStm, BuildingTypeExtData::Canary);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStreamStreamD(AircraftTypeExtContainer::Instance, pStm, AircraftTypeExtData::Canary);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStreamStreamD(AnimExtContainer::Instance, pStm, AnimExtData::Canary);
	if (!SUCCEEDED(hr)) return hr;

	//more
	return hr;
}

HRESULT Phobos::LoadAllLateData(IStream* pStm)
{
	HRESULT hr = S_OK;

	hr = ReadBlocksFromStream<MouseClassExt>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<GenericPrerequisite>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	//hr = ReadBlocksFromStream<BarTypeClass>(pStm);
	//if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<CrateTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<HealthBarTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<HoverTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<ImmunityTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<InsigniaTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<LaserTrailTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<RadTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<RocketTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<SelectBoxTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<ShieldTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<TechTreeTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<TunnelTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<PhobosAttachEffectTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	//

	hr = ReadBlocksFromStreamStreamB(SWFirerManagerClass::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStreamStreamB(BannerManagerClass::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<PhobosGlobal>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<HugeBar>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<SWStateMachine>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<ShieldClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	//Ext
	hr = ReadBlocksFromStreamStreamB(SideExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(AnimTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(HouseTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(HouseExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(UnitExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(InfantryExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(BuildingExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(AircraftExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(AnimExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(TaskForceExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(TeamTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(TeamExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(ScriptTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	//hr = ReadBlocksFromStreamStreamB(ScriptExtContainer::Instance, pStm);
	//if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(TriggerExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(AITriggerTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(TActionExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(TEventExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(VoxelAnimTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(VoxelAnimExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(WarheadTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(WeaponTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(ParticleTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(ParticleExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(ParticleSystemTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(ParticleSystemExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(BulletTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(BulletExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(SmudgeTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(OverlayTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(SWTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(SuperExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(TerrainTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(TerrainExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(WaveExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(CaptureExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(DiskLaserExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(ParasiteExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(TemporalExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	//hr = ReadBlocksFromStreamStreamB(AirstrikeExtContainer::Instance, pStm);
	//	if (!SUCCEEDED(hr)) return hr;
	//hr = ReadBlocksFromStreamStreamB(SpawnManagerExtContainer::Instance, pStm);
	// if (!SUCCEEDED(hr)) return hr;
	//hr = ReadBlocksFromStreamStreamB(SlaveManagerExtContainer::Instance, pStm);
	//if (!SUCCEEDED(hr)) return hr;
	hr = ReadBlocksFromStreamStreamB(RadSiteExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;


	hr = ReadBlocksFromStream<FakeIonBlastClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<TextBoxTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ReadBlocksFromStream<MapTextBoxClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	return hr;
}

HRESULT Decode_All_Pointers(LPSTREAM stream)
{
	HRESULT hr = S_OK;

	//load the voice index for the current player , this is used to restore the correct voice when loading a save game
	int value = 0;
	ULONG out = 0;
	hr = stream->Read(&value, sizeof(int), &out);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED reading EVA index (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] EVA index = %d\n", value);

	Debug::Log("[Decode] Loading ScenarioClass ...\n");
	hr = ScenarioClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED ScenarioClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK ScenarioClass\n");

	ScenarioClass::IsUserInputLocked = ScenarioClass::Instance->UserInputLocked;

	hr = LoadObjectVector(stream, *SideClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	PrepareDisplaySurfaces();

	Debug::Log("[Decode] Loading EvadeClass ...\n");
	hr = EvadeClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED EvadeClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK EvadeClass\n");

	Theater::Init(ScenarioClass::Instance->Theater);

	Debug::Log("[Decode] Loading RulesClass ...\n");
	hr = RulesClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED RulesClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK RulesClass\n");

	Debug::Log("[Decode] Loading FoggedObject ...\n");
	hr = FoggedObject::LoadGlobal(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED FoggedObject (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK FoggedObject\n");

	Debug::Log("[Decode] Loading MouseClass ...\n");
	hr = MouseClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED MouseClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK MouseClass\n");

	Debug::Log("[Decode] Loading Misc values ...\n");
	hr = Game::Load_Misc_Values(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED Misc values (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK Misc values\n");

	MapClass::Instance->Clear_SubzoneTracking();

	Debug::Log("[Decode] Loading MapClass::Logics ...\n");
	hr = MapClass::Logics->Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED MapClass::Logics (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK MapClass::Logics\n");

	if (auto pInstance = TacticalClass::Instance())
	{
		pInstance->ClearPtr();
	}

	Debug::Log("[Decode] Loading TacticalClass ...\n");
	LPVOID tacticalMapObj = nullptr;
	hr = OleLoadFromStream(stream, IID_IUnknown, &tacticalMapObj);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED TacticalClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK TacticalClass\n");

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
	if (!success) return E_FAIL;;

	success = RadarEventClass::LoadVector(stream);
	if (!success) return E_FAIL;;

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

	Debug::Log("[Decode] Loading AircraftTrackerClass ...\n");
	AircraftTrackerClass::Instance->Clear();
	hr = AircraftTrackerClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED AircraftTrackerClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK AircraftTrackerClass\n");

	Debug::Log("[Decode] Loading Kamikaze ...\n");
	Kamikaze::Instance->Clear();
	hr = Kamikaze::Instance->Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED Kamikaze (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK Kamikaze\n");

	Debug::Log("[Decode] Loading BombListClass ...\n");
	BombListClass::Instance->Clear();
	hr = BombListClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED BombListClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK BombListClass\n");

	hr = LoadObjectVector(stream, *BombClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = LoadObjectVector(stream, *RadSiteClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	// this was change from only for skirmish but for all modes
	// DEFINE_JUMP(LJMP, 0x67F72E, 0x67F744);
	if (!GameOptionsType::Instance->Load(stream)) {
		Debug::Log("\t***** GameOptionsType LOAD FAILED!\n");
		return E_FAIL;
	}

	// Audio/visual systems
	Debug::Log("[Decode] Loading VocClass ...\n");
	hr = VocClass::Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED VocClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK VocClass\n");

	Debug::Log("[Decode] Loading VoxClass ...\n");
	hr = VoxClass::Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED VoxClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK VoxClass\n");

	Debug::Log("[Decode] Loading ThemeClass ...\n");
	hr = ThemeClass::Instance->Load(stream);
	if (!SUCCEEDED(hr)) { Debug::Log("[Decode] FAILED ThemeClass (hr=0x%08X)\n", hr); return hr; }
	Debug::Log("[Decode] OK ThemeClass\n");

	VoxClass::EVAIndex = value;

	// add more variable that need to be reset after loading an saved games
	if (SessionClass::Instance->GameMode == GameMode::Campaign)
	{
		if (std::exchange(Unsorted::MuteSWLaunches(), false))
		{// this will also make radar unusable
			auto pSide = SideClass::Array->operator[](HouseClass::CurrentPlayer()->Type->SideIndex);
			auto pHouseType = HouseTypeExtContainer::Instance.Find(HouseClass::CurrentPlayer()->Type);

			VoxClass::EVAIndex = pHouseType->EVAIndex.Get(SideExtContainer::Instance.Find(pSide)->EVAIndex);
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

#include <Utilities/CompressedStream.h>

bool RetFlag(bool flag) // set the DoingLoadGame flag to false
{
	Phobos::Otamaa::DoingLoadGame = false;
	return flag;
}
#include <ProgressScreenClass.h>

bool __fastcall Make_Load_Game(const char* file_name, bool)
{
	std::wstring wide_file_name;
	HRESULT hr;

	// -----------------------------------------------------------------
	// Resolve file path
	// -----------------------------------------------------------------
	{
		if (SpawnerMain::Configs::Enabled && SavedGames::CreateSubdir()) {
			wide_file_name = PhobosCRT::StringToWideString(SavedGames::FormatPath(file_name));
		} else {
			wide_file_name = PhobosCRT::StringToWideString(file_name);
		}
	}

	Debug::Log("\nLOADING GAME [%s]\n", file_name);
	Phobos::Otamaa::DoingLoadGame = true;

	// -----------------------------------------------------------------
	// Open compound file
	// -----------------------------------------------------------------
	ATL::CComPtr<IStorage> storage;
	{
		hr = StgOpenStorage(wide_file_name.c_str(), nullptr,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			nullptr, 0, &storage);

		if (FAILED(hr))
		{
			Debug::FatalError("Failed to open storage. hr=0x%08X\n", hr);
			return RetFlag(false);
		}
	}

	// -----------------------------------------------------------------
	// Read and validate save header (property set)
	// -----------------------------------------------------------------
	{
		SavegameInformation saveversion {};
		hr = saveversion.Read(storage);
		if (FAILED(hr))
		{
			Debug::FatalError("Failed to read version information. hr=0x%08X\n", hr);
			return RetFlag(false);
		}

		if ((DWORD)saveversion.InternalVersion != Game::Savegame_Magic())
		{
			Debug::FatalError("Save version mismatch. Expected 0x%08X, got 0x%08X\n",
				Game::Savegame_Magic(), saveversion.InternalVersion);
			return RetFlag(false);
		}

		Debug::Log("Save version validated OK.\n");

		SessionClass::Instance->GameMode = saveversion.GameType;
		SwizzleManagerClass::Instance->Reset();
	}

	// -----------------------------------------------------------------
	// CustomMissionID — independent named stream, load early
	// -----------------------------------------------------------------
	{
		auto mission = SavedGames::ReadFromStorage<CustomMissionID>(storage);

		if (mission.has_value())
		{
			Debug::Log("Loaded CustomMissionID: %d\n", mission->Number);
			SpawnerMain::GetGameConfigs()->CustomMissionID = mission->Number;
		}
		else
		{
			Debug::Log("No CustomMissionID in save — using default.\n");
			SpawnerMain::GetGameConfigs()->CustomMissionID = 0;
		}
	}

	// clear the scenario here very ahead 
	ProgressScreenClass::Instance->Init(100.0, 0, NULL);
	Debug::Log("Begin clearing scenario.\n");
	ScenarioClass::ClearScenario();

	constexpr bool  Save_CompressionEnabled = false;
	const std::string _IsCompressed = Save_CompressionEnabled ? "compressed" : "uncompressed";

	// -----------------------------------------------------------------
	// PHOBOS_DATA_EARLY — load dll data (that must be ready
	// before game objects reference them)
	// -----------------------------------------------------------------
	{

		CompressedStream<!Save_CompressionEnabled> ext {};
		hr = ext.Open(storage, L"PHOBOS_DATA_EARLY");

		if (SUCCEEDED(hr))
		{
			Debug::Log("Loading Phobos early data (%s).\n", _IsCompressed.c_str());
			hr = Phobos::LoadAllEarlyData(ext.Stream);
			ext.Close();

			if (FAILED(hr))
			{
				Debug::FatalError("Failed to load Phobos early. hr=0x%08X\n", hr);
				return RetFlag(false);
			}
		}
		else
		{
			Debug::Log("No PHOBOS_DATA_EARLY stream — vanilla or legacy save.\n");
		}
	}

	// -----------------------------------------------------------------
	// "C" — decompress and load all game data
	// -----------------------------------------------------------------
	{
		Debug::Log("Calling Get_All_Pointers().\n");

		CompressedStream<!Save_CompressionEnabled> game;
		hr = game.Open(storage, L"C");
		if (FAILED(hr))
		{
			Debug::FatalError("Failed to open C stream. hr=0x%08X\n", hr);
			return RetFlag(false);
		}

		hr = Decode_All_Pointers(game.Stream);
		game.Close();

		if (FAILED(hr))
		{
			Debug::FatalError("Get_All_Pointers failed. hr=0x%08X\n", hr);
			return RetFlag(false);
		}
	}

	// -----------------------------------------------------------------
	// PHOBOS_DATA_LATE — load dll data (that must be ready
	// after game objects references )
	// -----------------------------------------------------------------
	{

		CompressedStream<!Save_CompressionEnabled> ext {};
		hr = ext.Open(storage, L"PHOBOS_DATA_LATE");

		if (SUCCEEDED(hr))
		{
			Debug::Log("Loading Phobos late data (%s).\n", _IsCompressed.c_str());
			hr = Phobos::LoadAllLateData(ext.Stream);
			ext.Close();

			if (FAILED(hr))
			{
				Debug::FatalError("Failed to load Phobos late. hr=0x%08X\n", hr);
				return RetFlag(false);
			}
		}
		else
		{
			Debug::Log("No PHOBOS_DATA_LATE stream — vanilla or legacy save.\n");
		}
	}

	// -----------------------------------------------------------------
	// PHOBOS_LUA_DATA — Lua scripting engine state, own named stream
	//
	// Missing stream (old save from before this feature existed) is treated
	// the same as PHOBOS_DATA_EARLY/LATE above: non-fatal, just nothing to
	// hand the script. A genuine read/IStream failure on a stream that DOES
	// exist is fatal, matching every other extension stream's contract.
	// -----------------------------------------------------------------
	{
		CompressedStream<!Save_CompressionEnabled> ext {};
		hr = ext.Open(storage, L"PHOBOS_LUA_DATA");

		if (SUCCEEDED(hr))
		{
			Debug::Log("Loading Phobos Lua data (%s).\n", _IsCompressed.c_str());
			hr = LuaAPI::OnGlobalGameLoad(ext.Stream);
			ext.Close();

			if (FAILED(hr))
			{
				// NOTE: LuaAPI::OnGlobalGameLoad swallows Lua script errors
				// internally (logs and returns S_OK) - a FAILED hr reaching
				// here means a genuine IStream/COM I/O failure, not a broken
				// script or a script-fingerprint mismatch (that's informational,
				// handed to the script as a boolean, never surfaced as HRESULT).
				Debug::FatalError("Failed to load Lua extension data. hr=0x%08X\n", hr);
				return RetFlag(false);
			}
		}
		else
		{
			Debug::Log("No PHOBOS_LUA_DATA stream — vanilla, legacy, or pre-Lua save.\n");
		}
	}

	Phobos::LoadGameDataAfter();

	// -----------------------------------------------------------------
	// Cleanup
	// -----------------------------------------------------------------
	{
		storage.Release();
	}

	Debug::Log("LOADING GAME [%s] - Complete.\n", file_name);
	return RetFlag(true);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x67E440, Make_Load_Game)