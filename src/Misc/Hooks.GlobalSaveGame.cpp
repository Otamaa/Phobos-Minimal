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
#include <Ext/VoxelAnimType/Body.h>
#include <Ext/VoxelAnim/Body.h>
#include <Ext/Wave/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Misc/Ares/Hooks/Header.h>

//
template<typename T>
FORCEDINLINE bool Process_Global_Save(LPSTREAM pStm)
{
	PhobosByteStream stm;
	PhobosStreamWriter writer(stm);

	Debug::LogInfo("[Process_Save] For object {} Start", PhobosCRT::GetTypeIDName<T>());
	return T::SaveGlobals(writer) && stm.WriteBlockToStream(pStm);
}

template<typename T>
FORCEDINLINE bool Process_Global_Save_SingleInstanceObject(LPSTREAM pStm)
{
	PhobosByteStream stm;
	PhobosStreamWriter writer(stm);

	Debug::LogInfo("[Process_Save] For object {} Start", PhobosCRT::GetTypeIDName<T>());
	T::Instance()->SaveToStream(writer);
	return stm.WriteBlockToStream(pStm);
}
//

template<typename T>
HRESULT SaveObjectVector(LPSTREAM pStm, DynamicVectorClass<T>& collection)
{
	HRESULT hr;

	// Write the count
	int count = collection.Count;
	hr = pStm->Write(&count, sizeof(int), nullptr);
	if (FAILED(hr)) return hr;

	// Save each object
	for (int i = 0; i < count; ++i)
	{
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

HRESULT Put_All(LPSTREAM pStm)
{
	Debug::LogInfo("About to save the game");
	HRESULT hr = S_OK;

	if (!Process_Global_Save<Phobos>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<CursorTypeClass>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<DigitalDisplayTypeClass>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<ArmorTypeClass>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<ImmunityTypeClass>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<LaserTrailTypeClass>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<TunnelTypeClass>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<InsigniaTypeClass>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<SelectBoxTypeClass>(pStm))
		return E_FAIL;

	hr = ScenarioClass::Instance->Save(pStm);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<SideExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *SideClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<TheaterTypeClass>(pStm))
		return E_FAIL;

	hr = EvadeClass::Instance->Save(pStm);
	if (FAILED(hr)) return hr;

	hr = RulesClass::Instance->Save(pStm);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<AnimTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *AnimTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<CellExtContainer>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<MouseClassExt>(pStm))
		return E_FAIL;

	hr = MouseClass::Instance->Save(pStm);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TubeClass::Array);
	if (FAILED(hr)) return hr;

	hr = Game::Save_Misc_Values(pStm);
	if (FAILED(hr)) return hr;

	hr = LogicClass::Instance->Save(pStm);
	if (FAILED(hr)) return hr;

	hr = OleSaveToStream(TacticalClass::Instance(), pStm);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save_SingleInstanceObject<TacticalExtData>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<HouseTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *HouseTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<HouseExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *HouseClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<UnitTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *UnitTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<UnitExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *UnitClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<InfantryTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *InfantryTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<InfantryExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *InfantryClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<BuildingTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *BuildingTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<BuildingExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *BuildingClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<AircraftTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *AircraftTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<AircraftExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *AircraftClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<AnimExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *AnimClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TaskForceClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TeamTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<TeamExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *TeamClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ScriptTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ScriptClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TagTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TagClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TriggerTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TriggerClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AITriggerTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *TActionClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<TActionExtData>(pStm))
		return E_FAIL;

	if (!Process_Global_Save<TEventExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *TEventClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *FactoryClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<VoxelAnimTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *VoxelAnimTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<VoxelAnimExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *VoxelAnimClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<WarheadTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *WarheadTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<WeaponTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *WeaponTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<ParticleTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *ParticleTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<ParticleExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *ParticleClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<ParticleSystemTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *ParticleSystemTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<ParticleSystemExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *ParticleSystemClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<BulletTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *BulletTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<BulletExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *BulletClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *WaypointPathClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<SmudgeTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *SmudgeTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<OverlayTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *OverlayTypeClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *LightSourceClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *BuildingLightClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<TiberiumExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *TiberiumClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *EMPulseClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<SWTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *SuperWeaponTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<SuperExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *SuperClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveSimpleArray(pStm, *SuperClass::ShowTimers);
	if (FAILED(hr)) return hr;

	hr = SaveSimpleArray(pStm, *BuildingClass::Secrets);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<TerrainTypeExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *TerrainTypeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<TerrainExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *TerrainClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *FoggedObjectClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AlphaShapeClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<WaveExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *WaveClass::Array);
	if (FAILED(hr)) return hr;

	if (!VeinholeMonsterClass::SaveVector(pStm))
		return E_FAIL;

	if (!RadarEventClass::SaveVector(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *CaptureManagerClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *DiskLaserClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *ParasiteClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<TemporalExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *TemporalClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *AirstrikeClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *SpawnManagerClass::Array);
	if (FAILED(hr)) return hr;

	hr = SaveObjectVector(pStm, *SlaveManagerClass::Array);
	if (FAILED(hr)) return hr;

	hr = AircraftTrackerClass::Instance->Save(pStm);
	if (FAILED(hr)) return hr;

	hr = Kamikaze::Instance->Save(pStm);
	if (FAILED(hr)) return hr;

	hr = BombListClass::Instance->Save(pStm);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<BombExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *BombClass::Array);
	if (FAILED(hr)) return hr;

	if (!Process_Global_Save<RadSiteExtContainer>(pStm))
		return E_FAIL;

	hr = SaveObjectVector(pStm, *RadSiteClass::Array);
	if (FAILED(hr)) return hr;

	if (SessionClass::Instance->GameMode == GameMode::Skirmish) {
		Debug::Log("Writing Skirmish Session.Options\n");
		const bool save_GameOptionsType = GameOptionsType::Instance->Save(pStm);

		if (!save_GameOptionsType) {
			Debug::Log("\t***** FAILED!\n");
			return E_FAIL;
		}
	}

	hr = VocClass::Save(pStm);
	if (FAILED(hr)) return hr;

	hr = VoxClass::Save(pStm);
	if (FAILED(hr)) return hr;

	hr = ThemeClass::Instance->Save(pStm);
	if (FAILED(hr)) return hr;

	hr = Phobos::SaveGameDataAfter(pStm);
	if (FAILED(hr)) return hr;

	return hr;
}

bool __fastcall wrap_SaveCoreScenarioData(LPSTREAM pStm) {
	return SUCCEEDED(Put_All(pStm));
}

DEFINE_FUNCTION_JUMP(CALL, 0x67D1AF, wrap_SaveCoreScenarioData)
