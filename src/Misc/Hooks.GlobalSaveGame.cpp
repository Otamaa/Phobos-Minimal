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
#include <Ext/Cell/Body.h>
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
HRESULT SaveObjectVector(LPSTREAM pStm, DynamicVectorClass<T>& collection)
{
	HRESULT hr;
	// Get type name once
	std::string typeName = PhobosCRT::GetTypeIDName<T>();

	// Write the count
	int count = collection.Count;
	Debug::Log("SaveObjectVector<%s>: About to write count %d\n",
			   typeName.c_str(), count);

	hr = pStm->Write(&count, sizeof(int), nullptr);
	if (FAILED(hr))
	{
		Debug::Log("SaveObjectVector<%s>: FAILED to write count! HRESULT: 0x%08X\n",
				   typeName.c_str(), hr);
		return hr;
	}
	Debug::Log("SaveObjectVector<%s>: Successfully wrote count\n",
			   typeName.c_str());

	// Save each object
	for (int i = 0; i < count; ++i)
	{
		Debug::Log("SaveObjectVector<%s>: Saving object %d/%d\n",
				   typeName.c_str(), i + 1, count);

		LPPERSISTSTREAM pUnk = nullptr;
		hr = collection.Items[i]->QueryInterface(IID_IPersistStream,
												  reinterpret_cast<void**>(&pUnk));
		if (FAILED(hr)) return hr;
		hr = OleSaveToStream(pUnk, pStm);
		pUnk->Release();
		if (FAILED(hr)) return hr;
	}
	return S_OK;
}

template<typename T>
HRESULT SaveSimpleArray(LPSTREAM pStm, DynamicVectorClass<T>& collection)
{
	HRESULT hr;

	// Write count
	hr = pStm->Write(&collection.Count, sizeof(int), nullptr);
	if (FAILED(hr)) return hr;

	// Write array data
	for (int i = 0; i < collection.Count; ++i)
	{
		hr = pStm->Write(&collection.Items[i], sizeof(T), nullptr);
		if (FAILED(hr)) return hr;
	}

	return S_OK;
}

#include <Utilities/StreamUtils.h>
#include <Ext/SWType/NewSuperWeaponType/SWTypeHandler.h>

//dummy objects to trigger `CONTENTS` writing
class DECLSPEC_UUID("EE8D505F-12BB-4313-AEDC-4AEA30A5BA03")
DummyPersist : public IPersistStream
{
public:
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override
	{
		if (ppv == nullptr)
		{
			return E_POINTER;
		}
		*ppv = nullptr;

		if (riid == __uuidof(IUnknown))
		{
			*ppv = reinterpret_cast<IUnknown*>(this);
		}

		if (riid == __uuidof(IStream))
		{
			*ppv = reinterpret_cast<IStream*>(this);
		}

		if (riid == __uuidof(IPersistStream))
		{
			*ppv = static_cast<IPersistStream*>(this);
		}

		if (*ppv == nullptr)
		{
			return E_NOINTERFACE;
		}

		reinterpret_cast<IUnknown*>(*ppv)->AddRef();
		return S_OK;
	}

	STDMETHODIMP_(ULONG) AddRef() override { return 1; }
	STDMETHODIMP_(ULONG) Release() override { return 1; }

	STDMETHODIMP GetClassID(CLSID* pClassID) override
	{
		if (pClassID == nullptr) {
			return E_POINTER;
		}

		*pClassID = __uuidof(this);

		return S_OK;
	}

	STDMETHOD(IsDirty)() override { return S_OK; }

	STDMETHOD(Load)(IStream* pStm) override {

		if (!pStm) {
			return E_POINTER;
		}

		HRESULT hr = pStm->Read(this, sizeof(*this), nullptr);

		if (FAILED(hr)) {
			return hr;
		}

		return hr;
	}

	STDMETHOD(Save)(IStream* pStm, BOOL fClearDirty) override {

		if (!pStm) {
			return E_POINTER;
		}

		HRESULT hr = pStm->Write(this, sizeof(*this), nullptr);

		if (FAILED(hr)) {
			return hr;
		}

		return hr;
	}

	STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pSize) override {
	   if (!pSize) {
		 return E_POINTER;
	   }

	   pSize->LowPart = sizeof(*this); // Add size of swizzle "id".
	   pSize->HighPart = 0;

		return S_OK;
	}

private:
	int data;
};

template<typename T>
bool Process_Global_SaveB(LPSTREAM pStm) {
	PhobosByteStream stm;
	PhobosStreamWriter writer(stm);

	Debug::LogInfo("[Process_Save] For object {} Start", PhobosCRT::GetTypeIDName<T>());
	return T::SaveGlobals(writer) && stm.WriteToStream(pStm);
}

template<typename T>
bool Process_Global_Save(PhobosStreamWriter& writer)
{
	Debug::LogInfo("[Process_Save] For object {} Start", PhobosCRT::GetTypeIDName<T>());
	return T::SaveGlobals(writer);
}

#include <Ext/Scenario/Body.h>

HRESULT SavePhobosEarlyObjects(LPSTREAM pStm)
{
	PhobosAppendedStream stm;
	PhobosStreamWriter writer(stm);

	HRESULT hr = S_OK;

	bool success = Process_Global_Save<Phobos>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<CursorTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<ColorTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<SideExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<TheaterTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<MouseClassExt>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<CellExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<DigitalDisplayTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<ArmorTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<ImmunityTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<LaserTrailTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<TunnelTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<InsigniaTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<SelectBoxTypeClass>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<TiberiumExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<HouseTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<HouseExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<UnitTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<UnitExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<InfantryTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<InfantryExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<BuildingTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<BuildingExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<AircraftTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<AircraftExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<AnimTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<AnimExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<TeamExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<TriggerExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<TEventExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<VoxelAnimTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<VoxelAnimExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<WarheadTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<WeaponTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<ParticleTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<ParticleExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<ParticleSystemTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<ParticleSystemExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<BulletTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<BulletExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<TActionExtData>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<SmudgeTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<OverlayTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<SWTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<SuperExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<TerrainTypeExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<TerrainExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<WaveExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<BombExtContainer>(writer);
	if (!success) return E_FAIL;

	success = Process_Global_Save<RadSiteExtContainer>(writer);
	if (!success) return E_FAIL;

	if (!stm.WriteToStream(pStm))
		hr = E_FAIL;

	return hr;

}

HRESULT Put_All_Pointers(LPSTREAM pStm)
{
	HRESULT hr = S_OK;

	hr = ScenarioClass::Instance->Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = SavePhobosEarlyObjects(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *SideClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = EvadeClass::Instance->Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = RulesClass::Instance->Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = MouseClass::Instance->Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	// Game misc data
	hr = Game::Save_Misc_Values(pStm);
	if (!SUCCEEDED(hr)) return hr;

	// Logic and tactical systems
	hr = MapClass::Logics->Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = OleSaveToStream(TacticalClass::Instance(), pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AnimTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TubeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TiberiumClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *HouseTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *HouseClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *UnitTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *UnitClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *InfantryTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *InfantryClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *BuildingTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *BuildingClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AircraftTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AircraftClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AnimClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TaskForceClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TeamTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TeamClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ScriptTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ScriptClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TagTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TagClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TriggerTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TriggerClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AITriggerTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TActionClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TEventClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *FactoryClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *VoxelAnimTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *VoxelAnimClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *WarheadTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *WeaponTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ParticleTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ParticleClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ParticleSystemTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ParticleSystemClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *BulletTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *BulletClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *WaypointPathClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *SmudgeTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *OverlayTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *LightSourceClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *BuildingLightClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *EMPulseClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *SuperWeaponTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *SuperClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveSimpleArray(pStm, *SuperClass::ShowTimers);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveSimpleArray(pStm, *BuildingClass::Secrets);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TerrainTypeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TerrainClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *FoggedObjectClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AlphaShapeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *WaveClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	if (!VeinholeMonsterClass::SaveVector(pStm)) return E_FAIL;

	if (!RadarEventClass::SaveVector(pStm)) return E_FAIL;

	hr = SaveObjectVector(pStm, *CaptureManagerClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *DiskLaserClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ParasiteClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TemporalClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AirstrikeClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *SpawnManagerClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *SlaveManagerClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = AircraftTrackerClass::Instance->Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = Kamikaze::Instance->Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = BombListClass::Instance->Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *BombClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	hr = SaveObjectVector(pStm, *RadSiteClass::Array);
	if (!SUCCEEDED(hr)) return hr;

	// Game options section (known problematic area)
	if (SessionClass::Instance->GameMode == GameMode::Skirmish) {
		Debug::Log("Writing Skirmish Session.Options\n");
		if (!GameOptionsType::Instance->Save(pStm))
		{
			Debug::Log("\t***** GameOptionsType SAVE FAILED!\n");
			return E_FAIL;
		}
	}

	hr = VocClass::Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = VoxClass::Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = ThemeClass::Instance->Save(pStm);
	if (!SUCCEEDED(hr)) return hr;

	ULONG out = 0;
	hr = pStm->Write(&VoxClass::EVAIndex(), sizeof(int), &out);
	if (!SUCCEEDED(hr)) return hr;

	hr = Phobos::SaveGameDataAfter(pStm);
	if (!SUCCEEDED(hr)) return hr;

	return hr;
}

bool __fastcall Make_Save_Game(const char* file_name, const wchar_t* descr, bool)
{
	WCHAR wide_file_name[PATH_MAX];

	if (SpawnerMain::Configs::Enabled && SavedGames::CreateSubdir()) {
		MultiByteToWideChar(CP_ACP, 0, SavedGames::FormatPath(file_name), -1, wide_file_name, std::size(wide_file_name));
	} else {
		MultiByteToWideChar(CP_ACP, 0, file_name, -1, wide_file_name, std::size(wide_file_name));
	}

	Debug::Log("\nSAVING GAME [%s - %ls]\n", file_name, descr);

	Debug::Log("Creating DocFile\n");

	ATL::CComPtr<IStorage> storage;
	HRESULT hr = StgCreateDocfile(wide_file_name, STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &storage);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to create storage.\n");
		return false;
	}

	/**
	 *  Write the save file header.
	 */

	SavegameInformation saveversion {};
	saveversion.InternalVersion = Game::Savegame_Magic();
	saveversion.ScenarioDescription = descr;
	saveversion.Version = AresGlobalData::version;
	saveversion.PlayerHouse = HouseClass::CurrentPlayer->Type->UIName;
	saveversion.Campaign = ScenarioClass::Instance->CampaignIndex;
	saveversion.ScenarioNumber = ScenarioClass::Instance->TechLevel;
	sprintf_s(saveversion.ExecutableName.raw(), "GAMEMD.EXE + Phobos Minimal + Mod %s ver %s",
	AresGlobalData::ModName,
	AresGlobalData::ModVersion);
	saveversion.GameType = SessionClass::Instance->GameMode;

	FILETIME filetime;
	CoFileTimeNow(&filetime);
	saveversion.LastSaveTime = filetime;
	saveversion.StartTime = filetime;
	saveversion.PlayTime = filetime;

	Debug::Log("Saving version information\n");
	hr = saveversion.Write(storage);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to write version information.\n");
		return false;
	}

	// === ensure the property-set (SavegameInformation) is committed and finalized
	// so the compound-file allocator won't reuse its space for the CONTENTS stream.
	Debug::Log("Committing storage after SavegameInformation write to finalize property-set...\n");
	hr = storage->Commit(STGC_DEFAULT);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to commit storage after SavegameInformation write. hr=0x%08X\n", hr);
		return false;
	}

	Debug::Log("Creating content stream.\n");
	ATL::CComPtr<IStream> docfile;
	hr = storage->CreateStream(L"CONTENTS", STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &docfile);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to create content stream.\n");
		return false;
	}

	hr = docfile->Commit(STGC_OVERWRITE); // flush stream buffers
	if (FAILED(hr)) {
		Debug::FatalError("Failed to commit CONTENTS stream.\n");
		return false;
	}

	LARGE_INTEGER li = {};
	hr = docfile->Seek(li, STREAM_SEEK_SET, nullptr);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to seek CONTENTS stream to beginning.\n");
		return false;
	}


	//disable compression so it can be read on hex editor
	//Debug::Log("Linking content stream to compressor.\n");
	//IUnknown* pUnknown = nullptr;
	//ATL::CComPtr<ILinkStream> linkstream;
	//hr = CoCreateInstance(__uuidof(CStreamClass), nullptr, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&pUnknown);
	//if (SUCCEEDED(hr)) {
	//	hr = OleRun(pUnknown);
	//	if (SUCCEEDED(hr)) {
	//		pUnknown->QueryInterface(__uuidof(ILinkStream), (void**)&linkstream);
	//	}
	//	pUnknown->Release();
	//}

	//hr = linkstream->Link_Stream(docfile);
	//if (FAILED(hr)) {
	//	Debug::FatalError("Failed to link stream to compressor.\n");
	//	return false;
	//}

	//ATL::CComPtr<IStream> stream;
	//linkstream->QueryInterface(__uuidof(IStream), (void**)&stream

	Debug::Log("Calling Put_All_Pointers().\n");

	bool result = SUCCEEDED(Put_All_Pointers(docfile));

	//Debug::Log("Unlinking content stream from compressor.\n");
	//hr = linkstream->Unlink_Stream(nullptr);
	//if (FAILED(hr)) {
	//	Debug::FatalError("Failed to link unstream from compressor.\n");
	//	return false;
	//}

	Debug::Log("Releasing content stream.\n");
	docfile.Release();

	Debug::Log("Closing DocFile.\n");
	hr = storage->Commit(STGC_DEFAULT);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to commit storage.\n");
		return false;
	}

	Debug::Log("SAVING GAME [%s] - Complete.\n", file_name);
	return result;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x67CEF0, Make_Save_Game)


//const DLGTEMPLATE* __fastcall Show_Message_With_Cancel_Only(LPARAM lParam, LPARAM a2, LONG dwNewLong)
//{
//	return nullptr;
//}
//
//DEFINE_FUNCTION_JUMP(CALL ,0x559E5E, Show_Message_With_Cancel_Only)