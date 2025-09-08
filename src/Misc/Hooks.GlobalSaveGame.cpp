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

#ifndef TRACK
#include <Utilities/StreamUtils.h>

HRESULT Put_All_WithValidation(LPSTREAM pStm, SavePositionTracker& tracker)
{
	Debug::Log("=== ENHANCED SAVE WITH POSITION VALIDATION ===\n");
	HRESULT hr = S_OK;

	// Track initial position
	ULARGE_INTEGER startPos = tracker.GetCurrentPosition();
	Debug::Log("About to save the game - starting at position: %llu\n", startPos.QuadPart);

	// Phobos extension data
	tracker.StartOperation("Process_Global_Save<Phobos>");
	bool success = Process_Global_Save<Phobos>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<CursorTypeClass>");
	success = Process_Global_Save<CursorTypeClass>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<DigitalDisplayTypeClass>");
	success = Process_Global_Save<DigitalDisplayTypeClass>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<ArmorTypeClass>");
	success = Process_Global_Save<ArmorTypeClass>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<ImmunityTypeClass>");
	success = Process_Global_Save<ImmunityTypeClass>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<LaserTrailTypeClass>");
	success = Process_Global_Save<LaserTrailTypeClass>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<TunnelTypeClass>");
	success = Process_Global_Save<TunnelTypeClass>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<InsigniaTypeClass>");
	success = Process_Global_Save<InsigniaTypeClass>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<SelectBoxTypeClass>");
	success = Process_Global_Save<SelectBoxTypeClass>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	// Core scenario
	tracker.StartOperation("ScenarioClass::Instance->Save");
	hr = ScenarioClass::Instance->Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Side system
	tracker.StartOperation("Process_Global_Save<SideExtContainer>");
	success = Process_Global_Save<SideExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(SideClass::Array)");
	hr = SaveObjectVector(pStm, *SideClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<TheaterTypeClass>");
	success = Process_Global_Save<TheaterTypeClass>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	// Core game rules
	tracker.StartOperation("EvadeClass::Instance->Save");
	hr = EvadeClass::Instance->Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("RulesClass::Instance->Save");
	hr = RulesClass::Instance->Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Animation system
	tracker.StartOperation("Process_Global_Save<AnimTypeExtContainer>");
	success = Process_Global_Save<AnimTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(AnimTypeClass::Array)");
	hr = SaveObjectVector(pStm, *AnimTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Cell and mouse data
	tracker.StartOperation("Process_Global_Save<CellExtContainer>");
	success = Process_Global_Save<CellExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<MouseClassExt>");
	success = Process_Global_Save<MouseClassExt>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("MouseClass::Instance->Save");
	hr = MouseClass::Instance->Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Tube system
	tracker.StartOperation("SaveObjectVector(TubeClass::Array)");
	hr = SaveObjectVector(pStm, *TubeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Game misc data
	tracker.StartOperation("Game::Save_Misc_Values");
	hr = Game::Save_Misc_Values(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Logic and tactical systems
	tracker.StartOperation("LogicClass::Instance->Save");
	hr = LogicClass::Instance->Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("OleSaveToStream(TacticalClass::Instance())");
	hr = OleSaveToStream(TacticalClass::Instance(), pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save_SingleInstanceObject<TacticalExtData>");
	success = Process_Global_Save_SingleInstanceObject<TacticalExtData>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	// House system
	tracker.StartOperation("Process_Global_Save<HouseTypeExtContainer>");
	success = Process_Global_Save<HouseTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(HouseTypeClass::Array)");
	hr = SaveObjectVector(pStm, *HouseTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<HouseExtContainer>");
	success = Process_Global_Save<HouseExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(HouseClass::Array)");
	hr = SaveObjectVector(pStm, *HouseClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Unit system
	tracker.StartOperation("Process_Global_Save<UnitTypeExtContainer>");
	success = Process_Global_Save<UnitTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(UnitTypeClass::Array)");
	hr = SaveObjectVector(pStm, *UnitTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<UnitExtContainer>");
	success = Process_Global_Save<UnitExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(UnitClass::Array)");
	hr = SaveObjectVector(pStm, *UnitClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Infantry system
	tracker.StartOperation("Process_Global_Save<InfantryTypeExtContainer>");
	success = Process_Global_Save<InfantryTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(InfantryTypeClass::Array)");
	hr = SaveObjectVector(pStm, *InfantryTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<InfantryExtContainer>");
	success = Process_Global_Save<InfantryExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(InfantryClass::Array)");
	hr = SaveObjectVector(pStm, *InfantryClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Building system
	tracker.StartOperation("Process_Global_Save<BuildingTypeExtContainer>");
	success = Process_Global_Save<BuildingTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(BuildingTypeClass::Array)");
	hr = SaveObjectVector(pStm, *BuildingTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<BuildingExtContainer>");
	success = Process_Global_Save<BuildingExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(BuildingClass::Array)");
	hr = SaveObjectVector(pStm, *BuildingClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Aircraft system
	tracker.StartOperation("Process_Global_Save<AircraftTypeExtContainer>");
	success = Process_Global_Save<AircraftTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(AircraftTypeClass::Array)");
	hr = SaveObjectVector(pStm, *AircraftTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<AircraftExtContainer>");
	success = Process_Global_Save<AircraftExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(AircraftClass::Array)");
	hr = SaveObjectVector(pStm, *AircraftClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Animation system
	tracker.StartOperation("Process_Global_Save<AnimExtContainer>");
	success = Process_Global_Save<AnimExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(AnimClass::Array)");
	hr = SaveObjectVector(pStm, *AnimClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// AI system
	tracker.StartOperation("SaveObjectVector(TaskForceClass::Array)");
	hr = SaveObjectVector(pStm, *TaskForceClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(TeamTypeClass::Array)");
	hr = SaveObjectVector(pStm, *TeamTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<TeamExtContainer>");
	success = Process_Global_Save<TeamExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(TeamClass::Array)");
	hr = SaveObjectVector(pStm, *TeamClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(ScriptTypeClass::Array)");
	hr = SaveObjectVector(pStm, *ScriptTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(ScriptClass::Array)");
	hr = SaveObjectVector(pStm, *ScriptClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(TagTypeClass::Array)");
	hr = SaveObjectVector(pStm, *TagTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(TagClass::Array)");
	hr = SaveObjectVector(pStm, *TagClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(TriggerTypeClass::Array)");
	hr = SaveObjectVector(pStm, *TriggerTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(TriggerClass::Array)");
	hr = SaveObjectVector(pStm, *TriggerClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(AITriggerTypeClass::Array)");
	hr = SaveObjectVector(pStm, *AITriggerTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Action and event system
	tracker.StartOperation("SaveObjectVector(TActionClass::Array)");
	hr = SaveObjectVector(pStm, *TActionClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<TActionExtData>");
	success = Process_Global_Save<TActionExtData>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("Process_Global_Save<TEventExtContainer>");
	success = Process_Global_Save<TEventExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(TEventClass::Array)");
	hr = SaveObjectVector(pStm, *TEventClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Factory system
	tracker.StartOperation("SaveObjectVector(FactoryClass::Array)");
	hr = SaveObjectVector(pStm, *FactoryClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Voxel animation system
	tracker.StartOperation("Process_Global_Save<VoxelAnimTypeExtContainer>");
	success = Process_Global_Save<VoxelAnimTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(VoxelAnimTypeClass::Array)");
	hr = SaveObjectVector(pStm, *VoxelAnimTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<VoxelAnimExtContainer>");
	success = Process_Global_Save<VoxelAnimExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(VoxelAnimClass::Array)");
	hr = SaveObjectVector(pStm, *VoxelAnimClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Weapon and combat system
	tracker.StartOperation("Process_Global_Save<WarheadTypeExtContainer>");
	success = Process_Global_Save<WarheadTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(WarheadTypeClass::Array)");
	hr = SaveObjectVector(pStm, *WarheadTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<WeaponTypeExtContainer>");
	success = Process_Global_Save<WeaponTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(WeaponTypeClass::Array)");
	hr = SaveObjectVector(pStm, *WeaponTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Particle system
	tracker.StartOperation("Process_Global_Save<ParticleTypeExtContainer>");
	success = Process_Global_Save<ParticleTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(ParticleTypeClass::Array)");
	hr = SaveObjectVector(pStm, *ParticleTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<ParticleExtContainer>");
	success = Process_Global_Save<ParticleExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(ParticleClass::Array)");
	hr = SaveObjectVector(pStm, *ParticleClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<ParticleSystemTypeExtContainer>");
	success = Process_Global_Save<ParticleSystemTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(ParticleSystemTypeClass::Array)");
	hr = SaveObjectVector(pStm, *ParticleSystemTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<ParticleSystemExtContainer>");
	success = Process_Global_Save<ParticleSystemExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(ParticleSystemClass::Array)");
	hr = SaveObjectVector(pStm, *ParticleSystemClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Bullet system
	tracker.StartOperation("Process_Global_Save<BulletTypeExtContainer>");
	success = Process_Global_Save<BulletTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(BulletTypeClass::Array)");
	hr = SaveObjectVector(pStm, *BulletTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<BulletExtContainer>");
	success = Process_Global_Save<BulletExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(BulletClass::Array)");
	hr = SaveObjectVector(pStm, *BulletClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Waypoint system
	tracker.StartOperation("SaveObjectVector(WaypointPathClass::Array)");
	hr = SaveObjectVector(pStm, *WaypointPathClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Smudge system
	tracker.StartOperation("Process_Global_Save<SmudgeTypeExtContainer>");
	success = Process_Global_Save<SmudgeTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(SmudgeTypeClass::Array)");
	hr = SaveObjectVector(pStm, *SmudgeTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Overlay system
	tracker.StartOperation("Process_Global_Save<OverlayTypeExtContainer>");
	success = Process_Global_Save<OverlayTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(OverlayTypeClass::Array)");
	hr = SaveObjectVector(pStm, *OverlayTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Light system
	tracker.StartOperation("SaveObjectVector(LightSourceClass::Array)");
	hr = SaveObjectVector(pStm, *LightSourceClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(BuildingLightClass::Array)");
	hr = SaveObjectVector(pStm, *BuildingLightClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Tiberium system
	tracker.StartOperation("Process_Global_Save<TiberiumExtContainer>");
	success = Process_Global_Save<TiberiumExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(TiberiumClass::Array)");
	hr = SaveObjectVector(pStm, *TiberiumClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// EMP system
	tracker.StartOperation("SaveObjectVector(EMPulseClass::Array)");
	hr = SaveObjectVector(pStm, *EMPulseClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Super weapon system
	tracker.StartOperation("Process_Global_Save<SWTypeExtContainer>");
	success = Process_Global_Save<SWTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(SuperWeaponTypeClass::Array)");
	hr = SaveObjectVector(pStm, *SuperWeaponTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<SuperExtContainer>");
	success = Process_Global_Save<SuperExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(SuperClass::Array)");
	hr = SaveObjectVector(pStm, *SuperClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Simple arrays
	tracker.StartOperation("SaveSimpleArray(*SuperClass::ShowTimers)");
	hr = SaveSimpleArray(pStm, *SuperClass::ShowTimers);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveSimpleArray(*BuildingClass::Secrets)");
	hr = SaveSimpleArray(pStm, *BuildingClass::Secrets);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Terrain system
	tracker.StartOperation("Process_Global_Save<TerrainTypeExtContainer>");
	success = Process_Global_Save<TerrainTypeExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(TerrainTypeClass::Array)");
	hr = SaveObjectVector(pStm, *TerrainTypeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Process_Global_Save<TerrainExtContainer>");
	success = Process_Global_Save<TerrainExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(TerrainClass::Array)");
	hr = SaveObjectVector(pStm, *TerrainClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Visual effects
	tracker.StartOperation("SaveObjectVector(FoggedObjectClass::Array)");
	hr = SaveObjectVector(pStm, *FoggedObjectClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(AlphaShapeClass::Array)");
	hr = SaveObjectVector(pStm, *AlphaShapeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Wave system
	tracker.StartOperation("Process_Global_Save<WaveExtContainer>");
	success = Process_Global_Save<WaveExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(WaveClass::Array)");
	hr = SaveObjectVector(pStm, *WaveClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Special save methods
	tracker.StartOperation("VeinholeMonsterClass::SaveVector");
	success = VeinholeMonsterClass::SaveVector(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("RadarEventClass::SaveVector");
	success = RadarEventClass::SaveVector(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	// Special systems
	tracker.StartOperation("SaveObjectVector(CaptureManagerClass::Array)");
	hr = SaveObjectVector(pStm, *CaptureManagerClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(DiskLaserClass::Array)");
	hr = SaveObjectVector(pStm, *DiskLaserClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(ParasiteClass::Array)");
	hr = SaveObjectVector(pStm, *ParasiteClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Critical section - Temporal system
	tracker.StartOperation("Process_Global_Save<TemporalExtContainer>");
	success = Process_Global_Save<TemporalExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(TemporalClass::Array)");
	hr = SaveObjectVector(pStm, *TemporalClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Advanced systems
	tracker.StartOperation("SaveObjectVector(AirstrikeClass::Array)");
	hr = SaveObjectVector(pStm, *AirstrikeClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(SpawnManagerClass::Array)");
	hr = SaveObjectVector(pStm, *SpawnManagerClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("SaveObjectVector(SlaveManagerClass::Array)");
	hr = SaveObjectVector(pStm, *SlaveManagerClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Tracker systems
	tracker.StartOperation("AircraftTrackerClass::Instance->Save");
	hr = AircraftTrackerClass::Instance->Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("Kamikaze::Instance->Save");
	hr = Kamikaze::Instance->Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("BombListClass::Instance->Save");
	hr = BombListClass::Instance->Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Bomb system
	tracker.StartOperation("Process_Global_Save<BombExtContainer>");
	success = Process_Global_Save<BombExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(BombClass::Array)");
	hr = SaveObjectVector(pStm, *BombClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Radiation system
	tracker.StartOperation("Process_Global_Save<RadSiteExtContainer>");
	success = Process_Global_Save<RadSiteExtContainer>(pStm);
	if (!tracker.EndOperation(success)) return E_FAIL;

	tracker.StartOperation("SaveObjectVector(RadSiteClass::Array)");
	hr = SaveObjectVector(pStm, *RadSiteClass::Array);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Game options section (known problematic area)
	if (SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		tracker.StartOperation("GameOptionsType::Instance->Save");
		Debug::Log("Writing Skirmish Session.Options\n");
		const bool save_GameOptionsType = GameOptionsType::Instance->Save(pStm);
		if (!tracker.EndOperation(save_GameOptionsType))
		{
			Debug::Log("\t***** GameOptionsType SAVE FAILED!\n");
			return E_FAIL;
		}
	}

	// Audio/visual systems
	tracker.StartOperation("VocClass::Save");
	hr = VocClass::Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("VoxClass::Save");
	hr = VoxClass::Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	tracker.StartOperation("ThemeClass::Instance->Save");
	hr = ThemeClass::Instance->Save(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Final Phobos data
	tracker.StartOperation("Phobos::SaveGameDataAfter");
	hr = Phobos::SaveGameDataAfter(pStm);
	if (!tracker.EndOperation(SUCCEEDED(hr))) return hr;

	// Log final summary
	ULARGE_INTEGER endPos = tracker.GetCurrentPosition();
	Debug::Log("=== SAVE COMPLETED SUCCESSFULLY ===\n");
	Debug::Log("Total bytes written: %llu\n", endPos.QuadPart - startPos.QuadPart);
	tracker.LogAllOperations();

	return hr;
}

#else
HRESULT Put_All(LPSTREAM pStm)
{
	Debug::Log("About to save the game");
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
#endif

//bool __fastcall wrap_SaveCoreScenarioData(LPSTREAM pStm) {
//	return SUCCEEDED(Put_All_WithValidation(pStm));
//}
//DEFINE_FUNCTION_JUMP(CALL, 0x67D1AF, wrap_SaveCoreScenarioData)

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

	SavegameInformation saveversion;
	saveversion.InternalVersion = Game::Savegame_Magic();
	saveversion.ScenarioDescription = descr;
	saveversion.Version = AresGlobalData::version;
	saveversion.PlayerHouse = HouseClass::CurrentPlayer->Type->UIName;
	saveversion.Campaign = ScenarioClass::Instance->CampaignIndex;
	saveversion.ScenarioNumber = ScenarioClass::Instance->TechLevel;
	sprintf_s(saveversion.ExecutableName.data(), "GAMEMD.EXE + Phobos Minimal + Mod %s ver %s",
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
		Debug::FatalError("Failed to read version information.\n");
		return false;
	}

	Debug::Log("Creating content stream.\n");
	ATL::CComPtr<IStream> docfile;
	hr = storage->CreateStream(L"CONTENTS", STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &docfile);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to create content stream.\n");
		return false;
	}

	Debug::Log("Creating stream wrapper for tracking.\n");
	StreamWrapperWithTracking* wrappedDocFile = new StreamWrapperWithTracking(docfile);
	SavePositionTracker tracker(wrappedDocFile, "SAVE");

	Debug::Log("Linking content stream to compressor.\n");
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

	hr = linkstream->Link_Stream(wrappedDocFile);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to link stream to compressor.\n");
		return false;
	}

	ATL::CComPtr<IStream> stream;
	linkstream->QueryInterface(__uuidof(IStream), (void**)&stream);

	bool result = SUCCEEDED(Put_All_WithValidation(stream , tracker));

	Debug::Log("Unlinking content stream from compressor.\n");
	hr = linkstream->Unlink_Stream(nullptr);
	if (FAILED(hr)) {
		Debug::FatalError("Failed to link unstream from compressor.\n");
		return false;
	}

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