
#include <CStreamClass.h>

//void __fastcall Decode_All_Pointers(CStreamClass* stream)
//{
//	// Section 1: Initialize and Load Scenario
//	Clear_Scenario();
//	ScenarioClass::Load(Scen, stream);
//	UserInputLocked = Scen->UserInputLocked;
//
//	if (!Prep_For_Side(Scen->SideBarTypeseems))
//	{
//		return; // Early exit if preparation fails
//	}
//
//	// Section 2: Setup Display and Surfaces
//	SetupDisplaySurfaces();
//
//	// Section 3: Load Core Game Systems
//	if (!LoadCoreGameSystems(stream))
//	{
//		return; // Exit if core systems fail to load
//	}
//
//	// Section 4: Load Game Object Collections
//	if (!LoadGameObjectCollections(stream))
//	{
//		return; // Exit if object collections fail to load
//	}
//
//	// Section 5: Load Specialized Game Data
//	if (!LoadSpecializedGameData(stream))
//	{
//		return; // Exit if specialized data fails to load
//	}
//
//	// Section 6: Load Session-Specific Data
//	LoadSessionSpecificData(stream);
//
//	// Section 7: Finalize Loading
//	FinalizeGameLoading();
//}
//
//// Section 1 Helper: Setup Display Surfaces
//void SetupDisplaySurfaces()
//{
//	Rect* sidebar_bounds = Get_Sidebar_Clip_Bounds(&composite_surf_rect);
//
//	Rect sidebar_rect = { 0, 0, 168, VisibleRect.Height };
//	Rect view_rect = { sidebar_bounds->X, sidebar_bounds->Y,
//					  sidebar_bounds->Width, sidebar_bounds->Height };
//	Rect composite_rect = { 0, 0, sidebar_bounds->Width, VisibleRect.Height };
//
//	Allocate_Surfaces(&VisibleRect, &composite_rect, &view_rect, &sidebar_rect, 0);
//	DisplayClass::Set_View_Dimensions(&Map.sc.t.sb.p.r.d, &view_rect);
//}
//
//// Section 2 Helper: Load Core Game Systems
//bool LoadCoreGameSystems(CStreamClass* stream)
//{
//	// Load basic game systems
//	EvadeClass::Decode_Pointers(&Evade, stream);
//	Init_Theaters(Scen->Theater);
//	RulesClass::Decode_Pointers(Rule, stream);
//
//	// Load tactical map data
//	int objectCount = 0;
//	if (stream->vtable->CStreamClass_Read(stream, &objectCount, 4, 0) < 0)
//	{
//		return false;
//	}
//
//	// Load tactical objects
//	for (int i = 0; i < objectCount; i++)
//	{
//		LPVOID tacticalObject = nullptr;
//		if (OleLoadFromStream(stream, &IID_IUnknown_0, &tacticalObject) < 0)
//		{
//			return false;
//		}
//	}
//
//	return true;
//}
//
//// Section 3 Helper: Load Game Object Collections
//bool LoadGameObjectCollections(CStreamClass* stream)
//{
//	// Array of collection loaders for different game object types
//	struct CollectionLoader
//	{
//		const char* name;
//		bool (*loadFunc)(CStreamClass*);
//	};
//
//	CollectionLoader loaders[] = {
//		{"HouseTypes", LoadHouseTypesCollection},
//		{"Houses", LoadHousesCollection},
//		{"UnitTypes", LoadUnitTypesCollection},
//		{"Units", LoadUnitsCollection},
//		{"InfantryTypes", LoadInfantryTypesCollection},
//		{"Infantry", LoadInfantryCollection},
//		{"BuildingTypes", LoadBuildingTypesCollection},
//		{"Buildings", LoadBuildingsCollection},
//		{"AircraftTypes", LoadAircraftTypesCollection},
//		{"Aircraft", LoadAircraftCollection}
//	};
//
//	for (const auto& loader : loaders)
//	{
//		if (!loader.loadFunc(stream))
//		{
//			Debug::Log(("Failed to load " + std::string(loader.name)).c_str());
//			return false;
//		}
//	}
//
//	return true;
//}
//
//// Section 4: Generic Collection Loader Template
//template<typename T>
//bool LoadGenericCollection(CStreamClass* stream, const char* collectionName)
//{
//	int count = 0;
//	if (stream->vtable->CStreamClass_Read(stream, &count, 4, 0) < 0)
//	{
//		return false;
//	}
//
//	for (int i = 0; i < count; i++)
//	{
//		LPVOID object = nullptr;
//		if (OleLoadFromStream(stream, &IID_IUnknown_0, &object) < 0)
//		{
//			return false;
//		}
//		// Object will be automatically added to appropriate collection
//	}
//
//	return true;
//}
//
//// Section 5: Specific Collection Loaders
//bool LoadHouseTypesCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<HouseTypeClass*>(stream, "HouseTypes");
//}
//
//bool LoadHousesCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<HouseClass*>(stream, "Houses");
//}
//
//bool LoadUnitTypesCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<UnitTypeClass*>(stream, "UnitTypes");
//}
//
//bool LoadUnitsCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<UnitClass*>(stream, "Units");
//}
//
//bool LoadInfantryTypesCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<InfantryTypeClass*>(stream, "InfantryTypes");
//}
//
//bool LoadInfantryCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<InfantryClass*>(stream, "Infantry");
//}
//
//bool LoadBuildingTypesCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<BuildingTypeClass*>(stream, "BuildingTypes");
//}
//
//bool LoadBuildingsCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<BuildingClass*>(stream, "Buildings");
//}
//
//bool LoadAircraftTypesCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<AircraftTypeClass*>(stream, "AircraftTypes");
//}
//
//bool LoadAircraftCollection(CStreamClass* stream)
//{
//	return LoadGenericCollection<AircraftClass*>(stream, "Aircraft");
//}
//
//// Section 6: Load Specialized Game Data
//bool LoadSpecializedGameData(CStreamClass* stream)
//{
//	// Load mouse and input systems
//	MouseClass::Load(&Map, stream);
//
//	if (Load_Misc_Values(stream) < 0)
//	{
//		return false;
//	}
//
//	// Clear and reload layer tracking
//	MapClass::Clear_SubzoneTracking(&Map.sc.t.sb.p.r.d.m);
//	LayerClass::Decode_Pointers(&Logic, stream);
//
//	// Reinitialize tactical map
//	if (TacticalMap)
//	{
//		TacticalMap->a.vftable->abstr.SDTOR(TacticalMap);
//		TacticalMap = nullptr;
//	}
//
//	return LoadGameCollections(stream);
//}
//
//// Section 7: Load Various Game Collections
//bool LoadGameCollections(CStreamClass* stream)
//{
//	// Load super weapon data
//	ShownSupers.ActiveCount = 0;
//	VectorClass<SuperClass*>::Clear(&ShownSupers);
//
//	int superCount = 0;
//	if (stream->vtable->CStreamClass_Read(stream, &superCount, 4, 0) < 0)
//	{
//		return false;
//	}
//
//	for (int i = 0; i < superCount; i++)
//	{
//		LPVOID superWeapon = nullptr;
//		if (stream->vtable->CStreamClass_Read(stream, &superWeapon, 4, 0) < 0)
//		{
//			return false;
//		}
//
//		if (ShownSupers.ActiveCount < ShownSupers.VectorMax ||
//			ShownSupers.GrowthStep > 0)
//		{
//			ShownSupers.Vector_Item[ShownSupers.ActiveCount++] =
//				reinterpret_cast<SuperClass*>(superWeapon);
//		}
//	}
//
//	// Swizzle super weapon pointers
//	for (int i = 0; i < ShownSupers.ActiveCount; i++)
//	{
//		SwizzleManagerClass::Swizzle(&SwizzleManager, &ShownSupers.Vector_Item[i]);
//	}
//
//	return LoadSecretLabs(stream);
//}
//
//// Section 8: Load Secret Labs Data
//bool LoadSecretLabs(CStreamClass* stream)
//{
//	*(&SecretLabs + 4) = 0;
//	VectorClass<BuildingClass*>::Clear(&SecretLabs);
//
//	int labCount = 0;
//	if (stream->vtable->CStreamClass_Read(stream, &labCount, 4, 0) < 0)
//	{
//		return false;
//	}
//
//	for (int i = 0; i < labCount; i++)
//	{
//		LPVOID lab = nullptr;
//		if (stream->vtable->CStreamClass_Read(stream, &lab, 4, 0) < 0)
//		{
//			return false;
//		}
//
//		if (*(&SecretLabs + 4) < *(&SecretLabs + 2))
//		{
//			(*(&SecretLabs + 1))[*(&SecretLabs + 4)++] =
//				reinterpret_cast<BuildingClass*>(lab);
//		}
//	}
//
//	// Swizzle secret lab pointers
//	for (int i = 0; i < *(&SecretLabs + 4); i++)
//	{
//		SwizzleManagerClass::Swizzle(&SwizzleManager, &(*(&SecretLabs + 1))[i]);
//	}
//
//	return LoadAdvancedGameSystems(stream);
//}
//
//// Section 9: Load Advanced Game Systems
//bool LoadAdvancedGameSystems(CStreamClass* stream)
//{
//	// Load various advanced game systems with simplified error handling
//	if (!LoadVeinholeMonsters(stream) ||
//		!LoadRadarEvents(stream) ||
//		!LoadTrackers(stream) ||
//		!LoadBombSystems(stream))
//	{
//		return false;
//	}
//
//	return true;
//}
//
//bool LoadVeinholeMonsters(CStreamClass* stream)
//{
//	return VeinholeMonsterClass::Decode_Pointers(stream) != 0;
//}
//
//bool LoadRadarEvents(CStreamClass* stream)
//{
//	return Load_RadarEvents(stream, 0) != 0;
//}
//
//bool LoadTrackers(CStreamClass* stream)
//{
//	AircraftTrackerClass::Clear(&AircraftTracker);
//	if (AircraftTrackerClass::Load(&AircraftTracker, stream) < 0)
//	{
//		return false;
//	}
//
//	KamikazeTrackerClass::Clear(&KamikazeTracker);
//	if (KamikazeTrackerClass::Load(&KamikazeTracker, stream) < 0)
//	{
//		return false;
//	}
//
//	return true;
//}
//
//bool LoadBombSystems(CStreamClass* stream)
//{
//	BombTrackerClass::Clear(&BombTracker);
//	return BombTrackerClass::Load(&BombTracker, stream) >= 0;
//}
//
//// Section 10: Load Session and Finalize
//void LoadSessionSpecificData(CStreamClass* stream)
//{
//	// Load session-specific options for skirmish games
//	if (Session.Type == GAME_SKIRMISH)
//	{
//		WWDebugString("Reading Skirmish Session.Options\n");
//		if (!GameOptionsType::Read(&GameOptions, stream))
//		{
//			WWDebugString("\t***** FAILED!\n");
//			return;
//		}
//	}
//
//	// Load audio systems
//	if (VocClass::Read(stream) >= 0)
//	{
//		if (VoxClass::Load(stream) >= 0)
//		{
//			ThemeClass::Load(&Theme, stream);
//		}
//	}
//}
//
//void FinalizeGameLoading()
//{
//	// Set up final display state
//	GScreenClass::Flag_To_Redraw(&Map.sc.t.sb.p.r.d.m.gsc, 2);
//
//	// Set menu state for campaign games
//	if (Session.Type == GAME_CAMPAIGN)
//	{
//		IsMenusMaybe = 1;
//	}
//
//	Debug::Log("Game loading completed successfully\n");
//}
//
//// Broken into logical sections for better readability
//
//void __fastcall Put_All(LPSTREAM pStm)
//{
//	// Section 1: Save Core Game State
//	if (!SaveCoreGameState(pStm))
//	{
//		WWDebugString("Failed to save core game state\n");
//		return;
//	}
//
//	// Section 2: Save Game Object Collections
//	if (!SaveGameObjectCollections(pStm))
//	{
//		WWDebugString("Failed to save game object collections\n");
//		return;
//	}
//
//	// Section 3: Save AI and Scripting Systems
//	if (!SaveAIAndScriptingSystems(pStm))
//	{
//		WWDebugString("Failed to save AI and scripting systems\n");
//		return;
//	}
//
//	// Section 4: Save Specialized Game Data
//	if (!SaveSpecializedGameData(pStm))
//	{
//		WWDebugString("Failed to save specialized game data\n");
//		return;
//	}
//
//	// Section 5: Save Session-Specific Data
//	SaveSessionSpecificData(pStm);
//
//	WWDebugString("Game save completed successfully\n");
//}
//

//
//// Section 2: Save Game Object Collections
//bool SaveGameObjectCollections(LPSTREAM pStm)
//{
//	// Array of collection savers
//	struct CollectionSaver
//	{
//		const char* name;
//		bool (*saveFunc)(LPSTREAM);
//	};
//
//	CollectionSaver savers[] = {
//		{"AnimTypes", SaveAnimTypesCollection},
//		{"Tubes", SaveTubesCollection},
//		{"HouseTypes", SaveHouseTypesCollection},
//		{"Houses", SaveHousesCollection},
//		{"Units", SaveUnitsCollection},
//		{"UnitTypes", SaveUnitTypesCollection},
//		{"InfantryTypes", SaveInfantryTypesCollection},
//		{"Infantry", SaveInfantryCollection},
//		{"BuildingTypes", SaveBuildingTypesCollection},
//		{"Buildings", SaveBuildingsCollection},
//		{"AircraftTypes", SaveAircraftTypesCollection},
//		{"Aircraft", SaveAircraftCollection},
//		{"Animations", SaveAnimationsCollection}
//	};
//
//	for (const auto& saver : savers)
//	{
//		if (!saver.saveFunc(pStm))
//		{
//			Debug::Log(("Failed to save " + std::string(saver.name)).c_str());
//			return false;
//		}
//	}
//
//	return true;
//}
//
//// Section 3: Generic Collection Saver Template
//template<typename T>
//bool SaveGenericCollection(LPSTREAM pStm, const VectorClass<T*>& collection, const char* collectionName)
//{
//	// Write collection count
//	int count = collection.ActiveCount;
//	if (pStm->lpVtbl->Write(pStm, &count, 4, 0) < 0)
//	{
//		return false;
//	}
//
//	// Write each object in collection
//	for (int i = 0; i < count; i++)
//	{
//		LPVOID persistStream = nullptr;
//
//		// Query for IPersistStream interface
//		if (collection.Vector_Item[i]->QueryInterface(CLSID_IPersistStream, &persistStream) < 0)
//		{
//			return false;
//		}
//
//		// Save object to stream
//		if (OleSaveToStream(persistStream, pStm) < 0)
//		{
//			return false;
//		}
//
//		// Release interface
//		static_cast<IPersistStream*>(persistStream)->Release();
//	}
//
//	return true;
//}
//
//// Section 4: Specific Collection Savers
//bool SaveAnimTypesCollection(LPSTREAM pStm)
//{
//	int count = *(&AnimTypes + 4);  // Get AnimTypes count
//	if (pStm->lpVtbl->Write(pStm, &count, 4, 0) < 0)
//	{
//		return false;
//	}
//
//	for (int i = 0; i < count; i++)
//	{
//		LPVOID persistStream = nullptr;
//		if ((*(&AnimTypes + 1))[i]->o.at.a.vftable->ot.at.a.QueryInterface(
//			(*(&AnimTypes + 1))[i], &CLSID_IPersistStream, &persistStream) < 0)
//		{
//			return false;
//		}
//
//		if (OleSaveToStream(persistStream, pStm) < 0 ||
//			static_cast<IPersistStream*>(persistStream)->Release() < 0)
//		{
//			return false;
//		}
//	}
//
//	return true;
//}
//
//bool SaveTubesCollection(LPSTREAM pStm)
//{
//	int count = *(&Tubes + 4);  // Get Tubes count
//	if (pStm->lpVtbl->Write(pStm, &count, 4, 0) < 0)
//	{
//		return false;
//	}
//
//	for (int i = 0; i < count; i++)
//	{
//		LPVOID persistStream = nullptr;
//		if ((*(&Tubes + 1))[i]->a.vftable->t.r.m.o.a.QueryInterface(
//			(*(&Tubes + 1))[i], &CLSID_IPersistStream, &persistStream) < 0)
//		{
//			return false;
//		}
//
//		if (OleSaveToStream(persistStream, pStm) < 0 ||
//			static_cast<IPersistStream*>(persistStream)->Release() < 0)
//		{
//			return false;
//		}
//	}
//
//	return true;
//}
//
//bool SaveHouseTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<HouseTypeClass*>*>(&HouseTypes), "HouseTypes");
//}
//
//bool SaveHousesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, Houses, "Houses");
//}
//
//bool SaveUnitsCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<UnitClass*>*>(&Units), "Units");
//}
//
//bool SaveUnitTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, UnitTypes, "UnitTypes");
//}
//
//bool SaveInfantryTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, InfantryTypes, "InfantryTypes");
//}
//
//bool SaveInfantryCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, Infantry, "Infantry");
//}
//
//bool SaveBuildingTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<BuildingTypeClass*>*>(&BuildingTypes), "BuildingTypes");
//}
//
//bool SaveBuildingsCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<BuildingClass*>*>(&BuildingClassDVC), "Buildings");
//}
//
//bool SaveAircraftTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<AircraftTypeClass*>*>(&AircraftTypes), "AircraftTypes");
//}
//
//bool SaveAircraftCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, Aircrafts, "Aircraft");
//}
//
//bool SaveAnimationsCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, Anims, "Animations");
//}
//
//// Section 5: Save AI and Scripting Systems
//bool SaveAIAndScriptingSystems(LPSTREAM pStm)
//{
//	// Array of AI system savers
//	struct AISystemSaver
//	{
//		const char* name;
//		bool (*saveFunc)(LPSTREAM);
//	};
//
//	AISystemSaver savers[] = {
//		{"TaskForces", SaveTaskForcesCollection},
//		{"TeamTypes", SaveTeamTypesCollection},
//		{"Teams", SaveTeamsCollection},
//		{"ScriptTypes", SaveScriptTypesCollection},
//		{"Scripts", SaveScriptsCollection},
//		{"TagTypes", SaveTagTypesCollection},
//		{"Tags", SaveTagsCollection},
//		{"TriggerTypes", SaveTriggerTypesCollection},
//		{"Triggers", SaveTriggersCollection},
//		{"AITriggerTypes", SaveAITriggerTypesCollection},
//		{"TActions", SaveTActionsCollection},
//		{"TEvents", SaveTEventsCollection}
//	};
//
//	for (const auto& saver : savers)
//	{
//		if (!saver.saveFunc(pStm))
//		{
//			Debug::Log(("Failed to save " + std::string(saver.name)).c_str());
//			return false;
//		}
//	}
//
//	return true;
//}
//
//// Section 6: AI System Savers (simplified versions)
//bool SaveTaskForcesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<TaskForceClass*>*>(&TaskForces), "TaskForces");
//}
//
//bool SaveTeamTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<TeamTypeClass*>*>(&TeamTypes), "TeamTypes");
//}
//
//bool SaveTeamsCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, Teams, "Teams");
//}
//
//bool SaveScriptTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, ScriptTypes, "ScriptTypes");
//}
//
//bool SaveScriptsCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, Scripts, "Scripts");
//}
//
//bool SaveTagTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<TagTypeClass*>*>(&TagTypes), "TagTypes");
//}
//
//bool SaveTagsCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<TagClass*>*>(&Tags), "Tags");
//}
//
//bool SaveTriggerTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<TriggerTypeClass*>*>(&TriggerTypes), "TriggerTypes");
//}
//
//bool SaveTriggersCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, Triggers, "Triggers");
//}
//
//bool SaveAITriggerTypesCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, AITriggerTypes, "AITriggerTypes");
//}
//
//bool SaveTActionsCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, *reinterpret_cast<VectorClass<TActionClass*>*>(&TActions), "TActions");
//}
//
//bool SaveTEventsCollection(LPSTREAM pStm)
//{
//	return SaveGenericCollection(pStm, TEvents, "TEvents");
//}
//
//// Section 7: Save Specialized Game Data
//bool SaveSpecializedGameData(LPSTREAM pStm)
//{
//	// Save various specialized vectors using helper functions
//	struct SpecializedSaver
//	{
//		const char* name;
//		std::function<bool(LPSTREAM)> saveFunc;
//	};
//
//	SpecializedSaver savers[] = {
//		{"Factories", [](LPSTREAM s) { return Save_Factories_Vector(s, &Factories) >= 0; }},
//		{"VoxelAnimTypes", [](LPSTREAM s) { return Save_VoxelAnimType_Vector(s, &VoxelAnimTypes) >= 0; }},
//		{"VoxelAnims", [](LPSTREAM s) { return Save_VoxelAnim_Vector(s, &VoxelAnims) >= 0; }},
//		{"Warheads", [](LPSTREAM s) { return Save_WarheadType_Vector(s, &Warheads) >= 0; }},
//		{"Weapons", [](LPSTREAM s) { return Save_WeaponType_Vector(s, &Weapons) >= 0; }},
//		{"ParticleTypes", [](LPSTREAM s) { return Save_ParticleType_Vector(s, &ParticleTypes) >= 0; }},
//		{"Particles", [](LPSTREAM s) { return Save_Particle_Vector(s, &Particles) >= 0; }},
//		{"ParticleSystemTypes", [](LPSTREAM s) { return Save_ParticleSystemType_Vector(s, &ParticleSystemTypes) >= 0; }},
//		{"ParticleSystems", [](LPSTREAM s) { return Save_ParticleSystem_Vector(s, &ParticleSystems) >= 0; }},
//		{"BulletTypes", [](LPSTREAM s) { return Save_BulletType_Vector(s, &BulletTypes) >= 0; }},
//		{"Bullets", [](LPSTREAM s) { return Save_Bullet_Vector(s, &Bullets) >= 0; }}
//	};
//
//	for (const auto& saver : savers)
//	{
//		if (!saver.saveFunc(pStm))
//		{
//			Debug::Log(("Failed to save " + std::string(saver.name)).c_str());
//			return false;
//		}
//	}
//
//	return SaveSuperWeaponData(pStm);
//}
//
//// Section 8: Save Super Weapon Data
//bool SaveSuperWeaponData(LPSTREAM pStm)
//{
//	// Save shown supers vector
//	int shownSupersCount = ShownSupers.ActiveCount;
//	if (pStm->lpVtbl->Write(pStm, &shownSupersCount, 4, 0) < 0)
//	{
//		return false;
//	}
//
//	for (int i = 0; i < shownSupersCount; i++)
//	{
//		SuperClass* super = ShownSupers.Vector_Item[i];
//		if (pStm->lpVtbl->Write(pStm, &super, 4, 0) < 0)
//		{
//			return false;
//		}
//	}
//
//	// Save secret labs vector
//	int secretLabsCount = *(&SecretLabs + 4);
//	if (pStm->lpVtbl->Write(pStm, &secretLabsCount, 4, 0) < 0)
//	{
//		return false;
//	}
//
//	for (int i = 0; i < secretLabsCount; i++)
//	{
//		BuildingClass* lab = (*(&SecretLabs + 1))[i];
//		if (pStm->lpVtbl->Write(pStm, &lab, 4, 0) < 0)
//		{
//			return false;
//		}
//	}
//
//	return true;
//}
//
//// Section 9: Save Additional Systems
//bool SaveAdditionalGameSystems(LPSTREAM pStm)
//{
//	// Save additional game systems with simplified error handling
//	std::vector<std::function<bool(LPSTREAM)>> systemSavers = {
//		[](LPSTREAM s) { return Save_TerrainType_Vector(s, &TerrainTypes) >= 0; },
//		[](LPSTREAM s) { return Save_Terrain_Vector(s, &Terrains) >= 0; },
//		[](LPSTREAM s) { return Save_FoggedObject_Vector(s, &FoggedObjects) >= 0; },
//		[](LPSTREAM s) { return Save_AlphaShape_Vector(s, &AlphaShapes) >= 0; },
//		[](LPSTREAM s) { return Save_Wave_Vector(s, &Waves) >= 0; },
//		[](LPSTREAM s) { return Save_VeinholeMonster_Vector(s) != 0; },
//		[](LPSTREAM s) { return Save_RadarEvents(s) != 0; },
//		[](LPSTREAM s) { return AircraftTrackerClass::Save(&AircraftTracker, s) >= 0; },
//		[](LPSTREAM s) { return KamikazeTrackerClass::Save(&KamikazeTracker, 0, s) >= 0; },
//		[](LPSTREAM s) { return BombTrackerClass::Save(&BombTracker, s) >= 0; }
//	};
//
//	for (const auto& saver : systemSavers)
//	{
//		if (!saver(pStm))
//		{
//			return false;
//		}
//	}
//
//	return true;
//}
//
//// Section 10: Save Session and Audio Data
//void SaveSessionSpecificData(LPSTREAM pStm)
//{
//	// Save additional specialized systems
//	SaveAdditionalGameSystems(pStm);
//
//	// Save session-specific data for skirmish games
//	if (Session.Type == GAME_SKIRMISH)
//	{
//		WWDebugString("Writing Skirmish Session.Options\n");
//		if (!GameOptionsType::Write(&GameOptions, pStm))
//		{
//			WWDebugString("\t***** FAILED!\n");
//			return;
//		}
//	}
//
//	// Save audio systems
//	if (VocClass::Write(pStm) >= 0)
//	{
//		if (VoxClass::Save(pStm) >= 0)
//		{
//			ThemeClass::Save(&Theme, pStm);
//		}
//	}
//
//	Debug::Log("Session and audio data saved successfully\n");
//}

#include <Lib/nlohmann/json.hpp>
#include <Lib/base64/base64.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

#include <ostream>

#include <ScenarioClass.h>

//https://github.com/ReneNyffenegger/cpp-base64
//https://raw.githubusercontent.com/ReneNyffenegger/cpp-base64/master/base64.h

class JSONSaveGameStream : public IStream
{
private:
	nlohmann::json m_gameState;
	std::vector<BYTE> m_buffer;
	LARGE_INTEGER m_position;
	ULONG m_refCount;
	bool m_isDirty;
	std::string m_currentSaveFile;

public:
	JSONSaveGameStream() : m_refCount(1), m_isDirty(false)
	{
		m_position.QuadPart = 0;
		InitializeGameState();
	}

	virtual ~JSONSaveGameStream() = default;

	// IUnknown methods
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override
	{
		if (!ppvObject) return E_POINTER;

		if (riid == IID_IUnknown || riid == IID_IStream || riid == IID_ISequentialStream)
		{
			*ppvObject = static_cast<IStream*>(this);
			AddRef();
			return S_OK;
		}

		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)() override
	{
		return InterlockedIncrement(&m_refCount);
	}

	STDMETHOD_(ULONG, Release)() override
	{
		ULONG count = InterlockedDecrement(&m_refCount);
		if (count == 0) delete this;
		return count;
	}

	// ISequentialStream methods
	STDMETHOD(Read)(void* pv, ULONG cb, ULONG* pcbRead) override
	{
		if (!pv) return STG_E_INVALIDPOINTER;

		if (m_isDirty)
		{
			SerializeToBuffer();
			m_isDirty = false;
		}

		ULONG bytesToRead = MinImpl(cb, static_cast<ULONG>(m_buffer.size() - m_position.QuadPart));

		if (bytesToRead > 0)
		{
			memcpy(pv, m_buffer.data() + m_position.QuadPart, bytesToRead);
			m_position.QuadPart += bytesToRead;
		}

		if (pcbRead) *pcbRead = bytesToRead;
		return (bytesToRead == cb) ? S_OK : S_FALSE;
	}

	STDMETHOD(Write)(const void* pv, ULONG cb, ULONG* pcbWritten) override
	{
		if (!pv) return STG_E_INVALIDPOINTER;

		ULONGLONG newSize = m_position.QuadPart + cb;
		if (newSize > m_buffer.size())
		{
			m_buffer.resize(static_cast<size_t>(newSize));
		}

		memcpy(m_buffer.data() + m_position.QuadPart, pv, cb);
		m_position.QuadPart += cb;
		m_isDirty = true;

		if (pcbWritten) *pcbWritten = cb;
		return S_OK;
	}

	// IStream methods (minimal implementation)
	STDMETHOD(Seek)(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) override
	{
		LARGE_INTEGER newPos = { 0 };

		switch (dwOrigin)
		{
		case STREAM_SEEK_SET: newPos = dlibMove; break;
		case STREAM_SEEK_CUR: newPos.QuadPart = m_position.QuadPart + dlibMove.QuadPart; break;
		case STREAM_SEEK_END: newPos.QuadPart = m_buffer.size() + dlibMove.QuadPart; break;
		default: return STG_E_INVALIDFUNCTION;
		}

		if (newPos.QuadPart < 0) return STG_E_INVALIDFUNCTION;

		m_position = newPos;
		if (plibNewPosition) plibNewPosition->QuadPart = newPos.QuadPart;

		return S_OK;
	}

	STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize) override
	{
		m_buffer.resize(static_cast<size_t>(libNewSize.QuadPart));
		return S_OK;
	}

	STDMETHOD(CopyTo)(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) override { return E_NOTIMPL; }
	STDMETHOD(Commit)(DWORD) override { return S_OK; }
	STDMETHOD(Revert)() override { return S_OK; }
	STDMETHOD(LockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return STG_E_INVALIDFUNCTION; }
	STDMETHOD(UnlockRegion)(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return STG_E_INVALIDFUNCTION; }

	STDMETHOD(Stat)(STATSTG* pstatstg, DWORD) override
	{
		if (!pstatstg) return STG_E_INVALIDPOINTER;
		ZeroMemory(pstatstg, sizeof(STATSTG));
		pstatstg->type = STGTY_STREAM;
		pstatstg->cbSize.QuadPart = m_buffer.size();
		pstatstg->grfMode = STGM_READWRITE;
		return S_OK;
	}

	STDMETHOD(Clone)(IStream** ppstm) override
	{
		if (!ppstm) return STG_E_INVALIDPOINTER;
		auto clone = new JSONSaveGameStream();
		clone->m_gameState = m_gameState;
		clone->SerializeToBuffer();
		*ppstm = clone;
		return S_OK;
	}

	// JSON-specific methods
	void SetCurrentSaveFile(const std::string& filename)
	{
		m_currentSaveFile = filename;
	}

	void SaveToJSONFile()
	{
		if (m_currentSaveFile.empty()) return;

		// Add metadata
		m_gameState["metadata"]["saveTime"] = GetCurrentTimeString();
		m_gameState["metadata"]["originalData"] = GetBufferAsBase64();

		std::string jsonFile = m_currentSaveFile + ".json";
		std::ofstream file(jsonFile);
		if (file.is_open())
		{
			file << m_gameState.dump(2);
			file.close();
		}
	}

	bool LoadFromJSONFile()
	{
		if (m_currentSaveFile.empty()) return false;

		std::string jsonFile = m_currentSaveFile + ".json";
		std::ifstream file(jsonFile);
		if (!file.is_open()) return false;

		try
		{
			file >> m_gameState;

			// Restore original data if present
			if (m_gameState.contains("metadata") && m_gameState["metadata"].contains("originalData"))
			{
				RestoreBufferFromBase64(m_gameState["metadata"]["originalData"]);
				m_position.QuadPart = 0;
			}

			return true;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

private:
	void InitializeGameState()
	{
		m_gameState = nlohmann::json::object();
		m_gameState["metadata"] = nlohmann::json::object();
	}

	void SerializeToBuffer()
	{
		std::string jsonStr = m_gameState.dump();
		m_buffer.assign(jsonStr.begin(), jsonStr.end());
	}

	std::string GetCurrentTimeString() const
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		char timeStr[64];
		sprintf_s(timeStr, "%04d-%02d-%02d %02d:%02d:%02d",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		return std::string(timeStr);
	}

	std::string GetBufferAsBase64() const
	{
		if (m_buffer.empty()) return "";

		// Use cpp-base64 library - pass raw pointer and size
		return base64_encode(m_buffer.data(), static_cast<unsigned int>(m_buffer.size()));
	}

	void RestoreBufferFromBase64(const std::string& base64)
	{
		if (base64.empty())
		{
			m_buffer.clear();
			return;
		}

		try
		{
			// Use cpp-base64 library for decoding
			std::string decoded = base64_decode(base64);

			// Convert decoded string to byte vector
			m_buffer.clear();
			m_buffer.reserve(decoded.size());
			m_buffer.assign(decoded.begin(), decoded.end());
		}
		catch (const std::exception& e)
		{
			// Log error and clear buffer on decode failure
			std::string errorMsg = "Base64 decode failed: " + std::string(e.what()) + "\n";
			Debug::Log(errorMsg.c_str());
			m_buffer.clear();
		}
	}
};

// Global JSON stream instance
std::unique_ptr<JSONSaveGameStream> g_jsonStream;

// Section 1: Save Core Game State
//bool SaveCoreGameState(LPSTREAM pStm)
//{
//	// Save scenario data
//	ScenarioClass::Save(Scen, pStm);
//
//	// Save evade and rules systems
//	EvadeClass::Code_Pointers(&Evade, pStm);
//	RulesClass::Code_Pointers(Rule, pStm);
//
//	// Save mouse and input state
//	MouseClass::Save(&Map, pStm);
//
//	// Save miscellaneous values
//	if (Save_Misc_Values(pStm) < 0)
//	{
//		return false;
//	}
//
//	// Save layer and logic systems
//	LayerClass::Code_Pointers(&Logic.l, pStm);
//
//	// Save tactical map
//	if (OleSaveToStream(TacticalMap, pStm) < 0)
//	{
//		return false;
//	}
//
//	return true;
//}

#include <EvadeClass.h>

template<class T>
static HRESULT Save_D_Vector(LPSTREAM& pStm, DynamicVectorClass<T>& list, const char* heap_name)
{
	Debug::LogInfo("Saving {} DVC ...", heap_name);

	/**
	 *  Save the number of instances of this class.
	 */
	int count = list.Count;

	HRESULT hr = pStm->Write(&count, sizeof(count), nullptr);
	if (FAILED(hr))
	{
		return hr;
	}

	if (count <= 0) {
		return hr;
	}

	/**
	 *  Save each instance of this class.
	 */
	for (int index = 0; index < count; ++index)
	{

		/**
		 *  Tell the extension class to persist itself into the data stream.
		 */
		IPersistStream* lpPS = nullptr;
		hr = list[index]->QueryInterface(__uuidof(IPersistStream), (LPVOID*)&lpPS);

		if (FAILED(hr)) {
			return hr;
		}

		/**
		 *  Save the object itself.
		 */
		hr = OleSaveToStream(lpPS, pStm);
		if (FAILED(hr)) {
			return false;
		}

		/**
		 *  Release the interface.
		 */
		hr = lpPS->Release();
		if (FAILED(hr)) {
			return false;
		}

	}

	return hr;
}

void __fastcall Hooked_Put_All(LPSTREAM pStm)
{
	// Create our JSON stream if it doesn't exist
	if (!g_jsonStream)
	{
		g_jsonStream = std::make_unique<JSONSaveGameStream>();
	}

	// Try to extract save filename from the stream
	// This is a simplified approach - you might need to inspect the stream more carefully
	STATSTG stat;
	if (SUCCEEDED(pStm->Stat(&stat, STATFLAG_NONAME)))
	{
		// Set a default filename - in practice, you'd extract this from RA2's save dialog
		g_jsonStream->SetCurrentSaveFile("ra2_save");
	}

	ScenarioClass::SaveAll(g_jsonStream.get());

	// After the original save completes, save our JSON version
	g_jsonStream->SaveToJSONFile();

	// Log the save operation
	Debug::Log("RA2 Save intercepted and JSON backup created\n");
}

// Hook for Decode_All_Pointers (Load Game)
void __fastcall Hooked_Decode_All_Pointers(LPSTREAM stream)
{
	// Create our JSON stream if it doesn't exist
	if (!g_jsonStream)
	{
		g_jsonStream = std::make_unique<JSONSaveGameStream>();
	}

	// Try to load from JSON first
	// In practice, you'd need to determine the save filename from RA2's load dialog
	g_jsonStream->SetCurrentSaveFile("ra2_save");


	// then load our json
	bool jsonLoaded = g_jsonStream->LoadFromJSONFile();

	if (jsonLoaded)
	{
		Debug::Log("JSON save data loaded successfully\n");
		// You could potentially replace the stream parameter here
		// to load from your JSON data instead
	}

	// Call original load function first
	ScenarioClass::LoadAll(g_jsonStream.get());

	Debug::Log("RA2 Load intercepted\n");
}

//DEFINE_FUNCTION_JUMP(CALL , 0x67E659 , Hooked_Decode_All_Pointers)
//DEFINE_FUNCTION_JUMP(CALL, 0x67D1AF, Hooked_Put_All)