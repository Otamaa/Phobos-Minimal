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
#include <WaypointPathClass.h>
#include <EMPulseClass.h>
#include <CaptureManagerClass.h>
#include <AircraftTrackerClass.h>
#include <Kamikaze.h>
#include <BombListClass.h>
#include <AirstrikeClass.h>
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
#include <New/Type/GenericPrerequisite.h>
#include <New/Type/BannerTypeClass.h>
#include <New/Type/BarTypeClass.h>
#include <New/Type/HealthBarTypeClass.h>
#include <New/Type/RocketTypeClass.h>
#include <New/Type/ThemeTypeClass.h>

#include <New/Entity/FlyingStrings.h>
#include <New/Entity/SWFirerClass.h>
#include <New/Entity/BannerClass.h>

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
#include <Ext/Mouse/Body.h>

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
HRESULT SaveSimpleData(LPSTREAM pStm, T& data)
{
	HRESULT hr;

	// Write count
	hr = pStm->Write(&data, sizeof(T), nullptr);

	if (FAILED(hr)) return hr;

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
#include <Ext/Scenario/Body.h>

template<typename T, bool hasArray = true>
HRESULT WriteBlocksToStream(IStream* pStm) {
	//reserve the stream block
	size_t _re = 0u;
	if constexpr(hasArray)
		_re = (sizeof(sizeof(T) * T::Array.size()));
	else
		_re = (sizeof(sizeof(T)));

	PhobosByteStream saver(_re);
	PhobosStreamWriter writer(saver);

	if (T::SaveGlobals(writer)) {
		if (saver.WriteToStream(pStm)) {
			return S_OK;
		}
	}

	return S_FALSE;

}

template<typename T>
HRESULT WriteBlocksToStreamB(T& who_ , IStream* pStm)
{
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;

	//reserve the stream block
	PhobosByteStream saver(sizeof(sizeof(Base)));
	PhobosStreamWriter writer(saver);

	//mark the block
	bool _SaveResult = false;
	if constexpr (std::is_pointer_v<T>) {
		_SaveResult = who_->SaveGlobal(writer);
	} else {
		_SaveResult = who_.SaveGlobal(writer);
	}

	if (_SaveResult) {
		if (saver.WriteToStream(pStm)) {
			return S_OK;
		}
	}

	return S_FALSE;

}

template<typename T>
HRESULT WriteBlocksToStreamC(T& who_, IStream* pStm)
{
	//reserve the stream block
	PhobosByteStream saver(sizeof(sizeof(T)));
	PhobosStreamWriter writer(saver);

	if (who_.SaveAll(writer)) {
		if (saver.WriteToStream(pStm)) {
			return S_OK;
		}
	}

	return S_FALSE;

}

HRESULT Phobos::SaveAllExtData(IStream* pStm)
{
	HRESULT hr = S_OK;

	//Global
	hr = WriteBlocksToStream<Phobos, false>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<CursorTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<MouseClassExt, false>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<TheaterTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<GenericPrerequisite>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<BannerTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<ArmorTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<BarTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<CrateTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<DigitalDisplayTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<HealthBarTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<HoverTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<ImmunityTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<InsigniaTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<LaserTrailTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<RadTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<RocketTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<SelectBoxTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<ShieldTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<TechTreeTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<TechTreeTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<ThemeTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<TunnelTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<PhobosAttachEffectTypeClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	//
	hr = WriteBlocksToStreamB(FlyingStrings::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStreamB(SWFirerManagerClass::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStreamB(BannerManagerClass::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<PhobosGlobal, false>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<HugeBar, false>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<SWStateMachine, false>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<TActionExtData, false>(pStm);
	if (!SUCCEEDED(hr)) return hr;

	hr = WriteBlocksToStream<ShieldClass>(pStm);
	if (!SUCCEEDED(hr)) return hr;
	
	//Ext
	hr = WriteBlocksToStreamC(SideExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(AnimTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(TiberiumExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(HouseTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(HouseExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(UnitTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(UnitExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(InfantryTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(InfantryExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(BuildingTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(BuildingExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(AircraftTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(AircraftExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(AnimExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	//hr = WriteBlocksToStreamC(TeamTypeExtContainer::Instance, pStm);
	//	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(TeamExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	//hr = WriteBlocksToStreamC(ScriptTypeExtContainer::Instance, pStm);
	//	if (!SUCCEEDED(hr)) return hr;
	//hr = WriteBlocksToStreamC(ScriptExtContainer::Instance, pStm);
	//if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(AITriggerTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	//hr = WriteBlocksToStreamC(TActionExtContainer::Instance, pStm);
	//if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(TEventExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(VoxelAnimTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(VoxelAnimExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(WarheadTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(WeaponTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(ParticleTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(ParticleExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(ParticleSystemTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(ParticleSystemExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(BulletTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(BulletExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(SmudgeTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(OverlayTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(SWTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(SuperExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(BuildingExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(TerrainTypeExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(TerrainExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(WaveExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	//hr = WriteBlocksToStreamC(CaptureManagerExtContainer::Instance, pStm);
	//	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(DiskLaserExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	//hr = WriteBlocksToStreamC(ParasiteExtContainer::Instance, pStm);
	//	if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(TemporalExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	//hr = WriteBlocksToStreamC(AirstrikeExtContainer::Instance, pStm);
	//	if (!SUCCEEDED(hr)) return hr;
	//hr = WriteBlocksToStreamC(SpawnManagerExtContainer::Instance, pStm);
	// if (!SUCCEEDED(hr)) return hr;
	//hr = WriteBlocksToStreamC(SlaveManagerExtContainer::Instance, pStm);
	//if (!SUCCEEDED(hr)) return hr;
	hr = WriteBlocksToStreamC(RadSiteExtContainer::Instance, pStm);
	if (!SUCCEEDED(hr)) return hr;
	//more

	return hr;
}


HRESULT Put_All_Pointers(LPSTREAM pStm)
{
	HRESULT hr = S_OK;

	//save the voice Index for the current player , this is used to restore the correct voice when loading a save game
	ULONG out = 0;
	hr = pStm->Write(&VoxClass::EVAIndex(), sizeof(int), &out);
	if (!SUCCEEDED(hr)) return hr;

	hr = ScenarioClass::Instance->Save(pStm);
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
		if (!GameOptionsType::Instance->Save(pStm)){
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

	return hr;
}

#include <Utilities/CompressedStream.h>

bool __fastcall Make_Save_Game(const char* file_name, const wchar_t* descr, bool)
{
	WCHAR wide_file_name[PATH_MAX];
	HRESULT hr;
	bool result = false;

	// -----------------------------------------------------------------
	// Resolve file path
	// -----------------------------------------------------------------
	{
		if (SpawnerMain::Configs::Enabled && SavedGames::CreateSubdir())
		{
			MultiByteToWideChar(CP_ACP, 0, SavedGames::FormatPath(file_name), -1,
				wide_file_name, std::size(wide_file_name));
		}
		else
		{
			MultiByteToWideChar(CP_ACP, 0, file_name, -1,
				wide_file_name, std::size(wide_file_name));
		}
	}

	Debug::Log("\nSAVING GAME [%s - %ls]\n", file_name, descr);

	// =================================================================
	// PHASE 1: Vanilla-identical layout (property set + "C")
	// Must match original allocation pattern exactly to prevent
	// the compound file FAT allocator from stomping property set sectors
	// =================================================================

	ATL::CComPtr<IStorage> storage;

	// -----------------------------------------------------------------
	// Create compound file
	// -----------------------------------------------------------------
	{
		hr = StgCreateDocfile(wide_file_name,
			STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			0, &storage);

		if (FAILED(hr))
		{
			Debug::FatalError("Failed to create storage. hr=0x%08X\n", hr);
			return false;
		}
	}

	// -----------------------------------------------------------------
	// Write save header (property set) — no commit
	// -----------------------------------------------------------------
	{
		SavegameInformation saveversion {};
		saveversion.InternalVersion = Game::Savegame_Magic();
		saveversion.ScenarioDescription = descr;
		saveversion.Version = Phobos::Config::version;
		saveversion.PlayerHouse = HouseClass::CurrentPlayer->Type->UIName;
		saveversion.Campaign = ScenarioClass::Instance->CampaignIndex;
		saveversion.ScenarioNumber = ScenarioClass::Instance->TechLevel;
		sprintf_s(saveversion.ExecutableName.raw(),
			"GAMEMD.EXE + Phobos Minimal + Mod %s ver %s",
			Phobos::Config::ModName, Phobos::Config::ModVersion);
		saveversion.GameType = SessionClass::Instance->GameMode;

		FILETIME filetime;
		CoFileTimeNow(&filetime);
		saveversion.LastSaveTime = filetime;
		saveversion.StartTime = filetime;
		saveversion.PlayTime = filetime;

		Debug::Log("Saving version information.\n");
		hr = saveversion.Write(storage);
		if (FAILED(hr))
		{
			Debug::FatalError("Failed to write version information. hr=0x%08X\n", hr);
			return false;
		}
	}

	// -----------------------------------------------------------------
	// "C" — compressed game data, immediately after property set
	// This matches the original: property set then "C", nothing else
	// -----------------------------------------------------------------
	{
		Debug::Log("Calling Put_All_Pointers().\n");

		CompressedStream game;
		hr = game.Create(storage, L"C");
		if (FAILED(hr))
		{
			Debug::FatalError("Failed to create C stream. hr=0x%08X\n", hr);
			return false;
		}

		result = SUCCEEDED(Put_All_Pointers(game.Stream));
		game.Close();

		if (!result)
		{
			Debug::FatalError("Put_All_Pointers failed.\n");
			return false;
		}
	}

	// -----------------------------------------------------------------
	// Single commit + full close — vanilla-identical, FAT finalized
	// -----------------------------------------------------------------
	{
		hr = storage->Commit(STGC_DEFAULT);
		if (FAILED(hr))
		{
			Debug::FatalError("Failed to commit storage. hr=0x%08X\n", hr);
			return false;
		}

		storage.Release();
	}

	// =================================================================
	// PHASE 2: Reopen for extension data
	// FAT is fully finalized on disk — new streams get fresh sectors,
	// cannot stomp property set or "C"
	// =================================================================

	// -----------------------------------------------------------------
	// Reopen storage
	// -----------------------------------------------------------------
	{
		Debug::Log("Reopening storage for extension data.\n");
		hr = StgOpenStorage(wide_file_name, nullptr,
			STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			nullptr, 0, &storage);

		if (FAILED(hr))
		{
			Debug::FatalError("Failed to reopen storage. hr=0x%08X\n", hr);
			return false;
		}
	}

	// -----------------------------------------------------------------
	// PHOBOS_EXT — compressed extension data
	// -----------------------------------------------------------------
	{
		Debug::Log("Saving Phobos extension data (compressed).\n");

		CompressedStream ext;
		hr = ext.Create(storage, L"PHOBOS_EXT");
		if (FAILED(hr))
		{
			Debug::FatalError("Failed to create PHOBOS_EXT stream. hr=0x%08X\n", hr);
			storage.Release();
			return false;
		}

		hr = Phobos::SaveAllExtData(ext.Stream);
		ext.Close();

		if (FAILED(hr))
		{
			Debug::FatalError("Failed to save extension data. hr=0x%08X\n", hr);
			storage.Release();
			return false;
		}
	}

	// -----------------------------------------------------------------
	// CustomMissionID — own named stream (uncompressed)
	// -----------------------------------------------------------------
	{
		if (SessionClass::IsCampaign())
		{
			if (SpawnerMain::GetGameConfigs()->CustomMissionID)
			{
				Debug::Log("Writing CustomMissionID to storage.\n");
				if (!SavedGames::WriteToStorage<CustomMissionID>(storage))
				{
					Debug::FatalError("Failed to write CustomMissionID.\n");
				}
			}
		}
	}

	// -----------------------------------------------------------------
	// Final commit + close
	// -----------------------------------------------------------------
	{
		hr = storage->Commit(STGC_DEFAULT);
		if (FAILED(hr))
		{
			Debug::FatalError("Failed to commit storage. hr=0x%08X\n", hr);
			storage.Release();
			return false;
		}

		storage.Release();
	}

	Phobos::SaveGameDataAfter();
	Debug::Log("SAVING GAME [%s] - Complete.\n", file_name);
	return result;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x67CEF0, Make_Save_Game)
