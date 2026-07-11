#include "Body.h"

#include <FPSCounter.h>
#include <GameOptionsClass.h>

#include <New/Type/CursorTypeClass.h>
#include <New/Type/RadTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/Type/HoverTypeClass.h>
#include <New/Type/ImmunityTypeClass.h>
#include <New/Type/TunnelTypeClass.h>
#include <New/Type/GenericPrerequisite.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/CrateTypeClass.h>
#include <New/Type/TechTreeTypeClass.h>
#include <New/Type/RocketTypeClass.h>
#include <New/Type/InsigniaTypeClass.h>
#include <New/Type/SelectBoxTypeClass.h>
#include <New/Type/BannerTypeClass.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>
#include <Utilities/TemplateDef.h>

#include <Misc/PhobosGlobal.h>

#include <TriggerTypeClass.h>
#include <GameStrings.h>
#include <TerrainTypeClass.h>
#include <IsometricTileTypeClass.h>
#include <SmudgeTypeClass.h>
#include <VeinholeMonsterClass.h>
#include <TEventClass.h>
#include <TActionClass.h>

std::unique_ptr<RulesExtData> RulesExtData::Data {};
IStream* RulesExtData::g_pStm;

void RulesExtData::Allocate(RulesClass* pThis)
{
	Data = std::make_unique<RulesExtData>();
	Data->AttachedToObject = pThis;
}

void RulesExtData::Remove(RulesClass* pThis)
{
	Data = nullptr;
}

//most of here is either do  :
// 1. Do add default value if any
// 2. Load the list from the ini before parsing it later
// the purpose is preparing the list before any parsing happen after this , because when the list is not ready and it is get parsed 
// everything just fall all over the places
void RulesExtData::Initialize(CCINIClass* pINI)
{
	CursorTypeClass::AddDefaults();
	CursorTypeClass::LoadFromINIList_New(pINI);
	ColorTypeClass::LoadFromINIList_New(pINI);
	SelectBoxTypeClass::AddDefaults();

	ArmorTypeClass::LoadFromINIList_New(pINI);
	CrateTypeClass::ReadFromINIList(pINI);
	TunnelTypeClass::LoadFromINIList(pINI);

	RocketTypeClass::AddDefaults();
	RocketTypeClass::LoadFromINIOnlyTheList(pINI);

	GenericPrerequisite::AddDefaults();
	GenericPrerequisite::LoadFromINIOnlyTheList(pINI);

	LaserTrailTypeClass::LoadFromINIOnlyTheList(CCINIClass::INI_Art.operator->());

	ShieldTypeClass::AddDefaults();
	ShieldTypeClass::LoadFromINIOnlyTheList(pINI);

	DigitalDisplayTypeClass::LoadFromINIOnlyTheList(pINI);
}

void RulesExtData::ReplaceVoxelLightSources()
{
	bool needCacheFlush = false;

	if (this->VoxelLightSource.isset())
	{
		needCacheFlush = true;
		auto source = this->VoxelLightSource->Normalized();
		Game::VoxelLightSource = Game::VoxelDefaultMatrix.get() * source;
	}

	if (this->VoxelShadowLightSource.isset())
	{
		needCacheFlush = true;
		auto source = this->VoxelShadowLightSource->Normalized();
		Game::VoxelShadowLightSource = Game::VoxelDefaultMatrix.get() * source;
	}

	if (needCacheFlush)
		Game::DestroyVoxelCaches();
}

// do everything before `TypeData::ReadFromINI` executed
// to makesure everything is properly allocated from the list
void RulesExtData::s_LoadBeforeTypeData(FakeRulesClass* pThis, CCINIClass* pINI)
{
	RadTypeClass::AddDefaults();
	HoverTypeClass::AddDefaults();

	ImmunityTypeClass::LoadFromINIList(pINI);
	ArmorTypeClass::EvaluateDefault();

	//TrailType::LoadFromINIList(&CCINIClass::INI_Art.get());

	RadTypeClass::LoadFromINIOnlyTheList(pINI);

	HoverTypeClass::LoadFromINIOnlyTheList(pINI);
	LaserTrailTypeClass::LoadFromINIList(CCINIClass::INI_Art.operator->());
	DigitalDisplayTypeClass::LoadFromINIList(pINI);
	SelectBoxTypeClass::LoadFromINIList(pINI);

	PhobosAttachEffectTypeClass::LoadFromINIOnlyTheList(pINI);

	TechTreeTypeClass::LoadFromINIOnlyTheList(pINI);

	BannerTypeClass::LoadFromINIList(pINI);

	if (Data->HugeBar_Config.empty())
	{
		Data->HugeBar_Config.emplace_back(DisplayInfoType::Health);
		Data->HugeBar_Config.emplace_back(DisplayInfoType::Shield);
	}

	for (auto& huge_bar : Data->HugeBar_Config) {
		huge_bar.LoadFromINI(pINI);
	}

	Data->LoadBeforeTypeData(pThis, pINI);
}

#include <Ext/SWType/Body.h>

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExtData::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	INI_EX exINI(pINI);

	auto pData = RulesExtData::Instance();

	CrateTypeClass::ReadListFromINI(pINI);
	HoverTypeClass::ReadListFromINI(pINI);
	ShieldTypeClass::ReadListFromINI(pINI);
	RadTypeClass::ReadListFromINI(pINI);
	PhobosAttachEffectTypeClass::ReadListFromINI(pINI);
	TechTreeTypeClass::ReadListFromINI(pINI);

	pData->BattlePoints.Read(exINI, GameStrings::General, "BattlePoints");
	pData->BattlePoints_DefaultValue.Read(exINI, GameStrings::General, "BattlePoints.DefaultValue");
	pData->BattlePoints_DefaultFriendlyValue.Read(exINI, GameStrings::General, "BattlePoints.DefaultFriendlyValue");

	pData->DamagedSpeed.Read(exINI, GameStrings::General, "DamagedSpeed");

	pData->InfantrySpeedData.Crawls.Read(exINI, GameStrings::General, "ProneSpeed.Crawls");
	pData->InfantrySpeedData.NoCrawls.Read(exINI, GameStrings::General, "ProneSpeed.NoCrawls");

	pData->BuildingGuardRetryDelay.Read(exINI, GameStrings::General, "BuildingGuardRetryDelay");
	pData->DiscardOn_ConsiderHoverAsMoving.Read(exINI, GameStrings::General, "DiscardOn.MoveBasedOnDestination");

	//got invalidated early , so parse it again
	detail::ParseVector(exINI, pData->AITargetTypesLists, "AITargetTypes");
	detail::ParseVector<ScriptTypeClass*, true>(exINI, pData->AIScriptsLists, "AIScriptsList");
	detail::ParseVector<HouseTypeClass*>(exINI, pData->AIHateHousesLists, "AIHateHousesList");
	detail::ParseVector<HouseTypeClass*>(exINI, pData->AIHousesLists, "AIHousesList");
	detail::ParseVector(exINI, pData->AIConditionsLists, "AIConditionsList", true, false, "/");
	detail::ParseVector<AITriggerTypeClass*, true>(exINI, pData->AITriggersLists, "AITriggersList");

	pData->AIChronoSphereSW.Read(exINI, GameStrings::General, "AIChronoSphereSW");
	pData->AIChronoWarpSW.Read(exINI, GameStrings::General, "AIChronoWarpSW");
	pData->AutoRemoveEarliestBeacon.Read(exINI, GameStrings::General, "AutoRemoveEarliestBeacon");
	pData->AllowChatBoxInSinglePlayer.Read(exINI, GameStrings::General, "AllowChatBoxInSinglePlayer");

	pData->DefaultAircraftDamagedSmoke = AnimTypeClass::Find(GameStrings::SGRYSMK1());

	pData->DamageToFirestormDamageCoefficient.Read(exINI, GameStrings::General(), "DamageToFirestormDamageCoefficient");
	pData->Bounty_Enablers.Read(exINI, GameStrings::General(), "BountyEnablers");

	Data->WallTowers.Read(exINI, GameStrings::General(), "WallTowers");

	if (pThis->WallTower && !pData->WallTowers.Contains(pThis->WallTower))
		pData->WallTowers.push_back(pThis->WallTower);
}

static bool NOINLINE IsVanillaDummy(const char* ID)
{
	static COMPILETIMEEVAL const char* exception[] = { "DeathDummy" , "WEEDGUY" , "YDUM" };

	for (auto const& gameDummy : exception) {
		if (IS_SAME_STR_(ID, gameDummy))
			return true;
	}

	return false;
}

#include <Ext/SWType/NewSuperWeaponType/SWTypeHandler.h>

std::unordered_map<VoxelStruct*, std::string > RulesExtData::Owners;

ASMJIT_PATCH(0x5F61A0 , VoxelStruct_DTOR, 0x6){
	GET(VoxelStruct*, pThis, EAX);
	RulesExtData::Owners.erase(pThis);
	return 0x0;
}

////void __fastcall Voxel_Calc_Normals_2(VoxLib* pVox, int headerentry, int tailerentry, Matrix3D* matrix1, Matrix3D* matrix2, Vector3D<float>* light, float exponent) {
////	JMP_FAST(0x753D00);
////}
////
////void __fastcall Voxel_Calc_Normals_2_intercept(VoxLib* pVox, int headerentry, int tailerentry, Matrix3D* matrix1, Matrix3D* matrix2, Vector3D<float>* light, float exponent)
////{
////	if (!pVox->HeaderData || !pVox->TailerData)
////		return;
////
////	Voxel_Calc_Normals_2(pVox, headerentry, tailerentry, matrix1, matrix2, light, exponent);
////}
////
////DEFINE_FUNCTION_JUMP(CALL, 0x706F4D, Voxel_Calc_Normals_2_intercept);
//
//#pragma optimize("", off)
//static void WatchPointer(void* fieldAddress, const char* name)
//{
//	DWORD value = *reinterpret_cast<DWORD*>(fieldAddress);
//	Debug::LogInfo("WatchPointer: {} @ 0x{:X} = 0x{:X}", name, (DWORD)fieldAddress, value);
//
//	// This is the key — set hardware write-breakpoint
//	// When the debugger is attached, it will break on write
//	CONTEXT ctx {};
//	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
//	HANDLE hThread = GetCurrentThread();
//	GetThreadContext(hThread, &ctx);
//
//	ctx.Dr0 = reinterpret_cast<DWORD>(fieldAddress);
//	ctx.Dr7 = (ctx.Dr7 & ~0xF0000) // clear DR0 control bits
//		| 0x1                    // enable DR0 locally
//		| (0x1 << 16)           // write-only condition
//		| (0x3 << 18);          // 4-byte size
//
//	SetThreadContext(hThread, &ctx);
//}
//#pragma optimize("", on)
//
//ASMJIT_PATCH(0x7564B0, VoxLib_GetData, 7)
//{
//	GET(VoxLib*, pVox, ECX);
//	GET_STACK(DWORD, caller, 0x0);
//	GET_STACK(int, header, 0x4);
//	GET_STACK(int, layer, 0x8);
//
//	// Check if the memory is still valid
//	//MEMORY_BASIC_INFORMATION mbi;
//	//VirtualQuery(pVox, &mbi, sizeof(mbi));
//
//	//if (mbi.State != MEM_COMMIT ||
//	//	!(mbi.Protect & (PAGE_READWRITE | PAGE_READONLY | PAGE_EXECUTE_READ)))
//	//{
//	//	Debug::FatalError("VoxLib @ 0x%X is FREED MEMORY! State=0x%X Protect=0x%X caller=0x%X",
//	//		(DWORD)pVox, mbi.State, mbi.Protect, caller);
//	//}
//
//	if (!pVox->HeaderData || !pVox->TailerData)
//	{		
//		// Log the raw pointer value to see if it's a stale pointer to freed mem
//		//Debug::FatalError("VoxLib @ 0x%X: HeaderData=0x%X TailerData=0x%X caller=0x%X",
//		//	(DWORD)pVox, (DWORD)pVox->HeaderData, (DWORD)pVox->TailerData, caller);
//
//		std::string owner = GameStrings::NoneStr();
//		for (auto& ii : RulesExtData::Owners)
//		{
//			if (ii.first->VXL == pVox)
//			{
//				owner = ii.second;
//				break;
//			}
//		}
//		Debug::FatalError("VoxelLibraryClass::Get_Voxel_Layer_Info %s input is broken ! caller 0x%x", owner.c_str(), caller);
//	}
//
//	auto pData = &pVox->TailerData[layer + pVox->HeaderData[header].limb_number];
//
//	R->EAX(pData);
//	return 0x7564CF;
//}

template<typename T>
static COMPILETIMEEVAL FORCEDINLINE void FillSecrets(DynamicVectorClass<T>& secrets) {

	for(auto opt : secrets){
		RulesExtData::Instance()->Secrets.emplace_back(opt);
		//Debug::LogInfo("Adding [{} - {}] onto Global Secrets pool" , Option->ID, Option->GetThisClassName());
	}
}

void RulesExtData::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	if (pINI == CCINIClass::INI_Rules())
	{
		//Load All the default value here
		this->ElectricDeath = AnimTypeClass::FindOrAllocate("ELECTRO");
		this->DefaultParaPlane = AircraftTypeClass::FindOrAllocate(GameStrings::PDPLANE());
		this->DefaultVeinParticle = ParticleTypeClass::FindOrAllocate(GameStrings::GASCLUDM1());
		this->DefaultGlobalParticleInstance = ParticleSystemTypeClass::FindOrAllocate(GameStrings::GasCloudSys());
		this->DefaultSquidAnim = AnimTypeClass::FindOrAllocate(GameStrings::SQDG());
		this->CarryAll_LandAnim = AnimTypeClass::FindOrAllocate(GameStrings::CARYLAND());
		this->DropShip_LandAnim = AnimTypeClass::FindOrAllocate(GameStrings::DROPLAND());
		this->DropPodTrailer = AnimTypeClass::FindOrAllocate(GameStrings::SMOKEY());
		this->Droppod_ImageInfantry = FileSystem::LoadSHPFile(GameStrings::POD_SHP);
		this->FirestormActiveAnim = AnimTypeClass::FindOrAllocate("GAFSDF_A");
		this->FirestormIdleAnim = AnimTypeClass::FindOrAllocate("FSIDLE");
		this->FirestormGroundAnim = AnimTypeClass::FindOrAllocate("FSGRND");
		this->FirestormAirAnim = AnimTypeClass::FindOrAllocate("FSAIR");
		this->XGRYMED1_ = AnimTypeClass::FindOrAllocate("XGRYMED1");
		this->XGRYMED2_ = AnimTypeClass::FindOrAllocate("XGRYMED2");
		this->XGRYSML1_ = AnimTypeClass::FindOrAllocate("XGRYSML1");
	}

	INI_EX exINI(pINI);

	#pragma region General
	this->TeamDelays_DynamicType.Read(exINI, GameStrings::General, "TeamDelays.DynamicType");
	std::string _teamDelay_tag = "TeamDelays.Count";
	
	for (size_t i = 0; i < 8; i++) {
		this->TeamDelays[i].Read(exINI, GameStrings::General, (_teamDelay_tag + std::to_string(i + 1)).c_str());
	}

	this->ParadropDelay.Read(exINI, GameStrings::General, "ParadropDelay");
	this->ParadropEndDelay.Read(exINI, GameStrings::General, "ParadropEndDelay");
	this->IsDischargedMemberAutocreateRecruitable.Read(exINI, GameStrings::General, "IsDischargedMemberAutocreateRecruitable");
	this->DiscardOn_ConsiderHarvestingAsStationary.Read(exINI, GameStrings::General, "DiscardOn.ConsiderHarvestingAsStationary");
	this->PrismRelay_SupportTimeout.Read(exINI, GameStrings::General, "PrismRelay.SupportTimeout");
	exINI.Read3Bool(GameStrings::General, "CampaignAllowHarvesterScanUnderShroud", this->CampaignAllowHarvesterScanUnderShroud);
	this->AttackMove_IgnoreWeaponCheck.Read(exINI, GameStrings::General, "AttackMove.IgnoreWeaponCheck");
	this->AttackMove_StopWhenTargetAcquired.Read(exINI, GameStrings::General, "AttackMove.StopWhenTargetAcquired");

	this->OpenTopped_DecloakToFire.Read(exINI, GameStrings::General, "OpenTopped.DecloakToFire");
	this->AISellCapturedBuilding.Read(exINI, GameStrings::General, "AISellCapturedBuilding");
	this->InfantryAutoDeploy.Read(exINI, GameStrings::General, "InfantryAutoDeploy");
	this->EnablePassiveAcquireMode.Read(exINI, GameStrings::General, "EnablePassiveAcquireMode");
	this->PlayerGuardModePursuit.Read(exINI, GameStrings::General, "PlayerGuardModePursuit");
	this->PlayerGuardModeGuardRangeMultiplier.Read(exINI, GameStrings::General, "PlayerGuardModeGuardRangeMultiplier");
	this->PlayerGuardModeGuardRangeAddend.Read(exINI, GameStrings::General, "PlayerGuardModeGuardRangeAddend");
	this->PlayerGuardModeGuardRangeMax.Read(exINI, GameStrings::General, "PlayerGuardModeGuardRangeMax");
	this->PlayerGuardStationaryStray.Read(exINI, GameStrings::General, "PlayerGuardStationaryStray");
	this->AIGuardModePursuit.Read(exINI, GameStrings::General, "AIGuardModePursuit");
	this->AIGuardModeGuardRangeMultiplier.Read(exINI, GameStrings::General, "AIGuardModeGuardRangeMultiplier");
	this->AIGuardModeGuardRangeAddend.Read(exINI, GameStrings::General, "AIGuardModeGuardRangeAddend");
	this->AIGuardModeGuardRangeMax.Read(exINI, GameStrings::General, "AIGuardModeGuardRangeMax");
	this->AIGuardStationaryStray.Read(exINI, GameStrings::General, "AIGuardStationaryStray");
	this->IgnoreCenterMinorRadarEvent.Read(exINI, GameStrings::General, "IgnoreCenterMinorRadarEvent");
	this->FallingDownTargetingFix.Read(exINI, GameStrings::General, "FallingDownTargetingFix");
	this->AIAirTargetingFix.Read(exINI, GameStrings::General, "AIAirTargetingFix");
	this->SortCameoByName.Read(exINI, GameStrings::General, "SortCameoByName");
	this->AllowDeployControlledMCV.Read(exINI, GameStrings::General, "AllowDeployControlledMCV");
	this->AllowBeaconHotKeyInSinglePlayer.Read(exINI, GameStrings::General, "AllowBeaconHotKeyInSinglePlayer");
	this->TypeSelectUseIFVMode.Read(exINI, GameStrings::General, "TypeSelectUseIFVMode");
	this->BuildingRadioLink_SyncOwner.Read(exINI, GameStrings::General, "BuildingRadioLink.SyncOwner");
	this->Parasite_AllowWaterExit.Read(exINI, GameStrings::General, "Parasite.AllowWaterExit");

	this->ChasingExtraRange.Read(exINI, GameStrings::General, "ChasingExtraRange");
	this->ChasingExtraRange_CloseRangeOnly.Read(exINI, GameStrings::General, "ChasingExtraRange.CloseRangeOnly");
	this->PrefiringExtraRange.Read(exINI, GameStrings::General, "PrefiringExtraRange");
	this->PrefiringExtraRange_IncludeBurst.Read(exINI, GameStrings::General, "PrefiringExtraRange.IncludeBurst");
	this->ExtraRange_FirerMoving.Read(exINI, GameStrings::General, "ExtraRange.FirerMoving");

	this->ChasingExtraRange.Read(exINI, GameStrings::General, "ExtraRange.TargetMoving");
	this->ChasingExtraRange_CloseRangeOnly.Read(exINI, GameStrings::General, "ExtraRange.TargetMoving.CloseRangeOnly");
	this->ExtraRange_FirerMoving.Read(exINI, GameStrings::General, "ExtraRange.FirerMoving");
	this->PrefiringExtraRange.Read(exINI, GameStrings::General, "ExtraRange.Prefiring");
	this->PrefiringExtraRange_IncludeBurst.Read(exINI, GameStrings::General, "ExtraRange.Prefiring.IncludeBurst");

	this->SecondaryFireSequenceLandOnly.Read(exINI, GameStrings::General, "SecondaryFireSequenceLandOnly");

	this->ExtendedPlayerRepair.Read(exINI, GameStrings::General, "ExtendedPlayerRepair");
	this->UpdateInvisoImmediately.Read(exINI, GameStrings::General, "UpdateInvisoImmediately");
	this->AutoTarget_NoThreatBuildings.Read(exINI, GameStrings::General, "AutoTarget.NoThreatBuildings");
	this->AutoTargetAI_NoThreatBuildings.Read(exINI, GameStrings::General, "AutoTargetAI.NoThreatBuildings");
	this->HarvesterDumpAmount.Read(exINI, GameStrings::General, "HarvesterDumpAmount");
	this->AttackMove_Aggressive.Read(exINI, GameStrings::General, "AttackMove.Aggressive");
	this->AttackMove_UpdateTarget.Read(exINI, GameStrings::General, "AttackMove.UpdateTarget");
	this->HarvesterScanAfterUnload.Read(exINI, GameStrings::General, "HarvesterScanAfterUnload");
	this->GiveMoneyIfStorageFull.Read(exINI, GameStrings::General, "Storage.GiveMoneyIfFull");
	this->AutoBuilding.Read(exINI, GameStrings::General, "AutoBuilding");
	this->AIAngerOnAlly.Read(exINI, GameStrings::General, "AIAngerOnAlly");
	this->BuildingTypeSelectable.Read(exINI, GameStrings::General, "BuildingTypeSelectable");
	this->BuildingWaypoint.Read(exINI, GameStrings::General, "BuildingWaypoint");
	this->NoTurret_TrackTarget.Read(exINI, GameStrings::General, "NoTurret.TrackTarget");
	this->RecountBurst.Read(exINI, GameStrings::General, "RecountBurst");
	this->AmphibiousEnter.Read(exINI, GameStrings::General, "AmphibiousEnter");
	this->AmphibiousUnload.Read(exINI, GameStrings::General, "AmphibiousUnload");
	this->ExpandAircraftMission.Read(exINI, GameStrings::General, "ExtendedAircraftMissions");
	this->ExtendedAircraftMissions_UnlandDamage.Read(exINI, GameStrings::General, "ExtendedAircraftMissions.UnlandDamage");
	this->AssignUnitMissionAfterParadropped.Read(exINI, GameStrings::General, "AssignUnitMissionAfterParadropped");
	this->NoQueueUpToEnter.Read(exINI, GameStrings::General, "NoQueueUpToEnter");
	this->NoQueueUpToUnload.Read(exINI, GameStrings::General, "NoQueueUpToUnload");
	this->NoQueueUpToEnter_Buildings.Read(exINI, GameStrings::General, "NoQueueUpToEnter.Buildings");
	this->NoQueueUpToUnload_Buildings.Read(exINI, GameStrings::General, "NoQueueUpToUnload.Buildings");
	this->NoRearm_UnderEMP.Read(exINI, GameStrings::General, "NoRearm.UnderEMP");
	this->NoRearm_Temporal.Read(exINI, GameStrings::General, "NoRearm.Temporal");
	this->NoReload_UnderEMP.Read(exINI, GameStrings::General, "NoReload.UnderEMP");
	this->NoReload_Temporal.Read(exINI, GameStrings::General, "NoReload.Temporal");
	this->AttackMindControlledDelay.Read(exINI, GameStrings::General, "AttackMindControlledDelay");
	this->ExpandBuildingQueue.Read(exINI, GameStrings::General, "BuildingProductionQueue");
	exINI.ReadSpeed(GameStrings::General, "SubterraneanSpeed", &this->SubterraneanSpeed);
	this->CheckUnitBaseNormal.Read(exINI, GameStrings::General, "CheckUnitBaseNormal");

	//TODO : fuck this break AI
	this->ExtendedBuildingPlacing.Read(exINI, GameStrings::General, "ExtendedBuildingPlacing");

	this->AISuperWeaponDelay.Read(exINI, GameStrings::General, "AISuperWeaponDelay");
	this->ChronoSpherePreDelay.Read(exINI, GameStrings::General, "ChronoSpherePreDelay");
	this->ChronoSphereDelay.Read(exINI, GameStrings::General, "ChronoSphereDelay");
	this->AINormalTargetingDelay.Read(exINI, GameStrings::General, "AINormalTargetingDelay");
	this->PlayerNormalTargetingDelay.Read(exINI, GameStrings::General, "PlayerNormalTargetingDelay");
	this->AIGuardAreaTargetingDelay.Read(exINI, GameStrings::General, "AIGuardAreaTargetingDelay");
	this->PlayerGuardAreaTargetingDelay.Read(exINI, GameStrings::General, "PlayerGuardAreaTargetingDelay");
	this->AIAttackMoveTargetingDelay.Read(exINI, GameStrings::General, "AIAttackMoveTargetingDelay");
	this->PlayerAttackMoveTargetingDelay.Read(exINI, GameStrings::General, "PlayerAttackMoveTargetingDelay");
	this->DistributeTargetingFrame.Read(exINI, GameStrings::General, "DistributeTargetingFrame");
	this->DistributeTargetingFrame_AIOnly.Read(exINI, GameStrings::General, "DistributeTargetingFrame.AIOnly");
	this->StartInMultiplayerUnitCost.Read(exINI, GameStrings::General(), "StartInMultiplayerUnitCost");
	this->TypeSelectUseDeploy.Read(exINI, GameStrings::General(), "TypeSelectUseDeploy");
	this->AIDetectDisguise_Percent.Read(exINI, GameStrings::General(), "AIDisguiseDetectionPercent");
	this->CanDrive.Read(exINI, GameStrings::General(), "EveryoneCanDrive");
	this->TogglePowerAllowed.Read(exINI, GameStrings::General(), "TogglePowerAllowed");
	this->TogglePowerDelay.Read(exINI, GameStrings::General(), "TogglePowerDelay");
	this->GainSelfHealAllowMultiplayPassive.Read(exINI, GameStrings::General(), "GainSelfHealAllowMultiplayPassive");
	this->GainSelfHealFromPlayerControl.Read(exINI, GameStrings::General, "GainSelfHealFromPlayerControl");
	this->GainSelfHealFromAllies.Read(exINI, GameStrings::General, "GainSelfHealFromAllies");
	this->VeinsDamagingWeightTreshold.Read(exINI, GameStrings::General(), "VeinsDamagingWeightTreshold");
	this->VeinholePal.Read(exINI, GameStrings::General(), "VeinholePalette");
	this->DegradeEnabled.Read(exINI, GameStrings::General(), "Degrade.Enabled");
	this->DegradePercentage.Read(exINI, GameStrings::General(), "Degrade.Percentage");
	this->DegradeAmountNormal.Read(exINI, GameStrings::General(), "Degrade.AmountNormal");
	this->DegradeAmountConsumer.Read(exINI, GameStrings::General(), "Degrade.AmountConsumer");
	this->EngineerDamage.Read(exINI, GameStrings::General(), "EngineerDamage");
	this->EngineerAlwaysCaptureTech.Read(exINI, GameStrings::General(), "EngineerAlwaysCaptureTech");
	this->EngineerDamageCursor.Read(exINI, GameStrings::General(), "EngineerDamageCursor");
	this->DefaultParaPlane.Read(exINI, GameStrings::General(), "ParadropPlane", true);
	this->DropPodTrailer.Read(exINI, GameStrings::General(), "DropPodTrailer", true);
	this->DroppodTrailerSpawnDelay.Read(exINI, GameStrings::General(), "DropPodTrailerSpawnDelay");
	this->DropPodTypes.Read(exINI, GameStrings::General(), "DropPodTypes");
	this->DropPodMinimum.Read(exINI, GameStrings::General(), "DropPodMinimum");
	this->DropPodMaximum.Read(exINI, GameStrings::General(), "DropPodMaximum");
	this->ReturnStructures.Read(exINI, GameStrings::General(), "ReturnStructures");
	this->MessageSilosNeeded.Read(exINI, GameStrings::General(), "Message.SilosNeeded");
	this->HunterSeekerDetonateProximity.Read(exINI, GameStrings::General(), "HunterSeekerDetonateProximity");
	this->HunterSeekerDescendProximity.Read(exINI, GameStrings::General(), "HunterSeekerDescendProximity");
	this->HunterSeekerAscentSpeed.Read(exINI, GameStrings::General(), "HunterSeekerAscentSpeed");
	this->HunterSeekerDescentSpeed.Read(exINI, GameStrings::General(), "HunterSeekerDescentSpeed");
	this->HunterSeekerEmergeSpeed.Read(exINI, GameStrings::General(), "HunterSeekerEmergeSpeed");
	this->Units_UnSellable.Read(exINI, GameStrings::General(), "UnitsUnsellable");
	this->Veins_PerCellAmount.Read(exINI, GameStrings::General(), "VeinsPerCellStorageAmount");
	this->MultipleFactoryCap.Read(exINI, GameStrings::General());
	this->ChronoSparkleDisplayDelay.Read(exINI, GameStrings::General(), "ChronoSparkleDisplayDelay");
	this->ChronoSparkleBuildingDisplayPositions.Read(exINI, GameStrings::General(), "ChronoSparkleBuildingDisplayPositions");
	this->RepairStopOnInsufficientFunds.Read(exINI, GameStrings::General(), "RepairStopOnInsufficientFunds");
	this->TeamRetaliate.Read(exINI, GameStrings::General(), "TeamRetaliate");
	this->AI_CostMult.Read(exINI, GameStrings::General(), "AICostMult");
	this->AI_SpyMoneyStealPercent.Read(exINI, GameStrings::General(), "AI.SpyMoneyStealPercent");
	this->AI_AutoSellHealthRatio.Read(exINI, GameStrings::General(), "AI.AutoSellHealthRatio");
	this->DisablePathfindFailureLog.Read(exINI, GameStrings::General(), "DisablePathfindFailureLog");
	this->AIFriendlyDistance.Read(exINI, GameStrings::General(), "AIFriendlyDistance");
	//this->MyPutData.Read(exINI, GameStrings::General());
	this->Storage_TiberiumIndex.Read(exINI, GameStrings::General(), "Storage.TiberiumIndex");
	this->ChronoInfantryCrush.Read(exINI, GameStrings::General(), "ChronoInfantryCrush");
	this->EnemyWrench.Read(exINI, GameStrings::General(), "EnemyWrench");
	this->Bounty_Value_Option.Read(exINI, GameStrings::General(), "BountyRewardOption");
	this->CloakHeight.Read(exINI, GameStrings::General(), "CloakHeight");
	this->EnemyInsignia.Read(exINI, GameStrings::General(), "EnemyInsignia");
	this->DisguiseBlinkingVisibility.Read(exINI, GameStrings::General(), "DisguiseBlinkingVisibility");
	this->Tiberium_DamageEnabled.Read(exINI, GameStrings::General(), "TiberiumDamageEnabled");
	this->Tiberium_HealEnabled.Read(exINI, GameStrings::General(), "TiberiumHealEnabled");
	this->OverlayExplodeThreshold.Read(exINI, GameStrings::General(), "OverlayExplodeThreshold");
	this->InfantryGainSelfHealCap.Read(exINI, GameStrings::General(), "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GameStrings::General(), "UnitsGainSelfHealCap");
	this->JumpjetClimbPredictHeight.Read(exINI, GameStrings::General, "JumpjetClimbPredictHeight");
	this->JumpjetClimbWithoutCutOut.Read(exINI, GameStrings::General, "JumpjetClimbWithoutCutOut");
	this->JumpjetClimbIgnoreBuilding.Read(exINI, GameStrings::General, "JumpjetClimbIgnoreBuilding");
	this->RegroupWhenMCVDeploy.Read(exINI, GameStrings::General, "GatherWhenMCVDeploy");
	this->AISellAllOnLastLegs.Read(exINI, GameStrings::General, "AIFireSale");
	this->AISellAllDelay.Read(exINI, GameStrings::General, "AIFireSaleDelay");
	this->AIAllInOnLastLegs.Read(exINI, GameStrings::General, "AIAllToHunt");
	this->RepairBaseNodes.Read(exINI, GameStrings::General, "RepairBaseNodes");
	this->MCVRedeploysInCampaign.Read(exINI, GameStrings::General, "MCVRedeploysInCampaign");
	this->UnitsUnsellable.Read(exINI, GameStrings::General, "UnitsUnsellable");
	this->ParadropMission.Read(exINI, GameStrings::General, "ParadropMission");
	this->AIParadropMission.Read(exINI, GameStrings::General, "AIParadropMission");
	this->CylinderRangefinding.Read(exINI, GameStrings::General, "CylinderRangefinding");
	this->DefaultToGuardArea.Read(exINI, GameStrings::General, "DefaultToGuardArea");
	this->ExtraThreat_IsThreat.Read(exINI, GameStrings::General, "ExtraThreat.IsThreat");
	this->ExtraThreat_InRange.Read(exINI, GameStrings::General, "ExtraThreat.InRange");
	this->ExtraThreatCoefficient_InRangeDistance.Read(exINI, GameStrings::General, "ExtraThreatCoefficient.InRangeDistance");
	this->ExtraThreatCoefficient_Facing.Read(exINI, GameStrings::General, "ExtraThreatCoefficient.Facing");
	this->ExtraThreatCoefficient_DistanceToLastTarget.Read(exINI, GameStrings::General, "ExtraThreatCoefficient.DistanceToLastTarget");
	this->DisableOveroptimizationInTargeting.Read(exINI, GameStrings::General, "DisableOveroptimizationInTargeting");
	this->AreaGuard_UseSelfAsCenter.Read(exINI, GameStrings::General, "AreaGuard.UseSelfAsCenter");
	this->AreaGuard_TargetingInRange.Read(exINI, GameStrings::General, "AreaGuard.TargetingInRange");
	this->AreaGuard_StrayIgnoreDestination.Read(exINI, GameStrings::General, "AreaGuard.StrayIgnoreDestination");
	this->BunkerStateUpdateDelay.Read(exINI, GameStrings::General, "BunkerStateUpdateDelay");
	#pragma endregion

	#pragma region GlobalControls
	if (pINI->ReadString(GLOBALCONTROLS_SECTION, "AllowBypassBuildLimit", "", Phobos::readBuffer) > 0) 	{
		bool temp[3] {};
		for (int i = 0; i < (int)Parser<bool, 3>::Parse(Phobos::readBuffer, temp); ++i) {
			int diffIdx = 2 - i; // remapping so that HouseClass::AIDifficulty can be used as an index
			this->AllowBypassBuildLimit[diffIdx] = temp[i];
		}
	}
	this->AllowParallelAIQueues.Read(exINI, GLOBALCONTROLS_SECTION, "AllowParallelAIQueues");
	this->ForbidParallelAIQueues_Infantry.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Infantry");
	this->ForbidParallelAIQueues_Vehicle.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Vehicle");
	this->ForbidParallelAIQueues_Navy.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Navy");
	this->ForbidParallelAIQueues_Aircraft.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Aircraft");
	this->ForbidParallelAIQueues_Building.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Building");
	#pragma endregion
}

bool RulesExtData::DetailsCurrentlyEnabled()
{
	// not only checks for the min frame rate from the rules, but also whether
	// the low frame rate is actually desired. in that case, don't reduce.
	auto const current = FPSCounter::CurrentFrameRate();
	auto const wanted = static_cast<unsigned int>(
		60 / std::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));

	return current >= wanted || current >= Detail::GetMinFrameRate();
}

bool RulesExtData::DetailsCurrentlyEnabled(int const minDetailLevel)
{
	return GameOptionsClass::Instance->DetailLevel >= minDetailLevel
		&& DetailsCurrentlyEnabled();
}

#pragma region save_load

template <typename T>
void RulesExtData::Serialize(T& Stm)
{
	//Debug::LogInfo("Processing RulesExt ! ");

	Stm
		.Process(this->Initialized)

		.Process(this->Pips_Shield)
		.Process(this->Pips_Shield_Buildings)

		.Process(this->RadApplicationDelay_Building)
		.Process(this->RadBuildingDamageMaxCount)
		.Process(this->MissingCameo)

		.Process(this->AITargetTypesLists)
		.Process(this->AIScriptsLists)
		.Process(this->AIHateHousesLists)
		.Process(this->AIConditionsLists)
		.Process(this->AITriggersLists)
		.Process(this->AIHousesLists)

		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->JumpjetTurnToTarget)
		.Process(this->JumpjetCrash_Rotate)
		.Process(this->JumpjetClimbPredictHeight)
		.Process(this->JumpjetClimbWithoutCutOut)
		.Process(this->JumpjetClimbIgnoreBuilding)
		.Process(this->Storage_TiberiumIndex)
		.Process(this->PlacementGrid_TranslucentLevel)
		.Process(this->BuildingPlacementPreview_TranslucentLevel)

		.Process(this->Pips_Shield_Background_SHP)
		.Process(this->Pips_Shield_Building)
		.Process(this->Pips_Shield_Building_Empty)

		.Process(this->Pips_SelfHeal_Infantry)
		.Process(this->Pips_SelfHeal_Units)
		.Process(this->Pips_SelfHeal_Buildings)
		.Process(this->Pips_SelfHeal_Infantry_Offset)
		.Process(this->Pips_SelfHeal_Units_Offset)
		.Process(this->Pips_SelfHeal_Buildings_Offset)

		.Process(this->Pips_Generic_Size)
		.Process(this->Pips_Generic_Buildings_Size)
		.Process(this->Pips_Ammo_Size)
		.Process(this->Pips_Ammo_Buildings_Size)

		.Process(this->Pips_Tiberiums_Frames)
		.Process(this->Pips_Tiberiums_DisplayOrder)

		.Process(this->InfantryGainSelfHealCap)
		.Process(this->UnitsGainSelfHealCap)
		.Process(this->EnemyInsignia)
		.Process(this->DisguiseBlinkingVisibility)
		.Process(this->DrawInsignia_UsePixelSelectionBracketDelta)

		.Process(this->SHP_SelectBrdSHP_INF)
		.Process(this->SHP_SelectBrdPAL_INF)
		.Process(this->SHP_SelectBrdSHP_UNIT)
		.Process(this->SHP_SelectBrdPAL_UNIT)

		.Process(this->UseSelectBrd)

		.Process(this->SelectBrd_Frame_Infantry)
		.Process(this->SelectBrd_DrawOffset_Infantry)
		.Process(this->SelectBrd_Frame_Unit)
		.Process(this->SelectBrd_DrawOffset_Unit)

		.Process(this->SelectBrd_DefaultTranslucentLevel)
		.Process(this->SelectBrd_DefaultShowEnemy)

		.Process(this->RadWarhead_Detonate)
		.Process(this->RadHasOwner)
		.Process(this->RadHasInvoker)
		.Process(this->ShieldUseArmorplier)
		.Process(this->UseGlobalRadApplicationDelay)
		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->ForceShield_KeptOnDeploy)
		.Process(this->ForceShield_EffectOnOrganics)
		.Process(this->ForceShield_KillOrganicsWarhead)
		.Process(this->AllowWeaponSelectAgainstWalls)
		.Process(this->IronCurtain_EffectOnOrganics)
		.Process(this->IronCurtain_KillOrganicsWarhead)
		.Process(this->ROF_RandomDelay)

		.Process(this->ToolTip_Background_Color)
		.Process(this->ToolTip_Background_Opacity)
		.Process(this->ToolTip_Background_BlurSize)
		.Process(this->ToolTip_ExcludeSidebar)
		.Process(this->Crate_LandOnly)

		.Process(this->NewTeamsSelector)
		.Process(this->NewTeamsSelector_SplitTriggersByCategory)
		.Process(this->NewTeamsSelector_EnableFallback)
		.Process(this->NewTeamsSelector_MergeUnclassifiedCategoryWith)
		.Process(this->NewTeamsSelector_UnclassifiedCategoryPercentage)
		.Process(this->NewTeamsSelector_GroundCategoryPercentage)
		.Process(this->NewTeamsSelector_NavalCategoryPercentage)
		.Process(this->NewTeamsSelector_AirCategoryPercentage)

		.Process(this->IC_Flash)
		.Process(this->VeteranFlashTimer)

		.Process(this->Tiberium_DamageEnabled)
		.Process(this->Tiberium_HealEnabled)
		.Process(this->Tiberium_ExplosiveWarhead)
		.Process(this->Tiberium_ExplosiveAnim)
		.Process(this->OverlayExplodeThreshold)
		.Process(this->AlliedSolidTransparency)
		.Process(this->DecloakSound)
		.Process(this->VeinholeParticle)
		.Process(this->DefaultVeinParticle)
		.Process(this->DefaultSquidAnim)
		.Process(this->NukeWarheadName)
		.Process(this->Building_PlacementPreview)
		.Process(this->PlacementGrid_TranslucencyWithPreview)
		.Process(this->AI_AutoSellHealthRatio)

		.Process(this->CarryAll_LandAnim)
		.Process(this->DropShip_LandAnim)
		.Process(this->Aircraft_LandAnim)
		.Process(this->LandingAnim)
		.Process(this->Aircraft_TakeOffAnim)

		.Process(this->DisablePathfindFailureLog)
		.Process(this->CreateSound_PlayerOnly)

		.Process(this->CivilianSideIndex)
		.Process(this->SpecialCountryIndex)
		.Process(this->NeutralCountryIndex)

		.Process(this->WallTowers)
		.Process(this->CanTargetAI_IronCurtained)
		.Process(this->CanTarget_IronCurtained)
		.Process(this->AutoTarget_IronCurtained)
		.Process(this->AI_SpyMoneyStealPercent)
		.Process(this->DoggiePanicMax)
		.Process(this->HunterSeeker_Damage)
		.Process(this->AutoRepelAI)
		.Process(this->AutoRepelPlayer)
		.Process(this->AIFriendlyDistance)

		.Process(this->StealthSpeakDelay)
		.Process(this->SubterraneanSpeakDelay)
		.Process(this->RandomCrateMoney)

		.Process(this->ChronoSparkleDisplayDelay)
		.Process(this->ChronoSparkleBuildingDisplayPositions)
		.Process(this->RepairStopOnInsufficientFunds)
		.Process(this->DropPodTrailer)
		.Process(this->DroppodTrailerSpawnDelay)
		.Process(this->Droppod_ImageInfantry)
		.Process(this->ElectricDeath)
		.Process(this->HunterSeekerBuildings)
		.Process(this->HunterSeekerDetonateProximity)
		.Process(this->HunterSeekerDescendProximity)
		.Process(this->HunterSeekerAscentSpeed)
		.Process(this->HunterSeekerDescentSpeed)
		.Process(this->HunterSeekerEmergeSpeed)

		.Process(this->Units_UnSellable)
		.Process(this->DrawTurretShadow)
		.Process(this->Bounty_Enablers)
		.Process(this->Bounty_Display)
		.Process(this->Bounty_Value_Option)
		.Process(this->BerserkROFMultiplier)
		.Process(this->TeamRetaliate)
		.Process(this->AI_CostMult)

		.Process(this->DeactivateDim_Powered)
		.Process(this->DeactivateDim_EMP)
		.Process(this->DeactivateDim_Operator)

		.Process(this->ChainReact_Multiplier)
		.Process(this->ChainReact_SpreadChance)
		.Process(this->ChainReact_MinDelay)
		.Process(this->ChainReact_MaxDelay)
		.Process(this->ChronoInfantryCrush)

		.Process(this->EnemyWrench)
		.Process(this->AllowParallelAIQueues)
		.Process(this->ForbidParallelAIQueues_Infantry)
		.Process(this->ForbidParallelAIQueues_Vehicle)
		.Process(this->ForbidParallelAIQueues_Navy)
		.Process(this->ForbidParallelAIQueues_Aircraft)
		.Process(this->ForbidParallelAIQueues_Building)

		.Process(this->EngineerDamage)
		.Process(this->EngineerAlwaysCaptureTech)
		.Process(this->EngineerDamageCursor)

		.Process(this->DefaultParaPlane)

		.Process(this->DropPodTypes)
		.Process(this->DropPodMinimum)
		.Process(this->DropPodMaximum)
		.Process(this->ReturnStructures)
		.Process(this->MessageSilosNeeded)

		.Process(this->CloakAnim)
		.Process(this->DecloakAnim)
		.Process(this->Cloak_KickOutParasite)

		.Process(this->DamageAirConsiderBridges)
		.Process(this->DiskLaserAnimEnabled)

		.Process(this->Buildings_DefaultDigitalDisplayTypes)
		.Process(this->Infantry_DefaultDigitalDisplayTypes)
		.Process(this->Vehicles_DefaultDigitalDisplayTypes)
		.Process(this->Aircraft_DefaultDigitalDisplayTypes)

		.Process(this->AnimRemapDefaultColorScheme)
		.Process(this->EMPAIRecoverMission)
		.Process(this->TimerBlinkColorScheme)
		.Process(this->AllowBypassBuildLimit)

		.Process(this->DegradeEnabled)
		.Process(this->DegradePercentage)
		.Process(this->DegradeAmountNormal)
		.Process(this->DegradeAmountConsumer)

		.Process(this->TogglePowerAllowed)
		.Process(this->TogglePowerDelay)
		.Process(this->TogglePowerIQ)

		.Process(this->GainSelfHealAllowMultiplayPassive)
		.Process(this->GainSelfHealFromPlayerControl)
		.Process(this->GainSelfHealFromAllies)

		.Process(this->VeinsDamagingWeightTreshold)
		.Process(this->VeinholePal)
		.Process(this->Veinhole_Warhead)
		.Process(this->Veins_PerCellAmount)

		.Process(this->FirestormActiveAnim)
		.Process(this->FirestormIdleAnim)
		.Process(this->FirestormGroundAnim)
		.Process(this->FirestormAirAnim)
		.Process(this->FirestormWarhead)
		.Process(this->DamageToFirestormDamageCoefficient)
		.Process(this->MultipleFactoryCap)
		.Process(this->CloakHeight)

		.Process(this->CanDrive)
		.Process(this->DefaultAircraftDamagedSmoke)
		.Process(this->AIDetectDisguise_Percent)

		.Process(this->DisplayIncome)
		.Process(this->DisplayIncome_AllowAI)
		.Process(this->DisplayIncome_Houses)

		.Process(this->DisplayCreditsDelay)
		.Process(this->TypeSelectUseDeploy)
		.Process(this->StartInMultiplayerUnitCost)
		.Process(this->FPSCounter)

		.Process(this->DrawInsigniaOnlyOnSelected)
		.Process(this->DrawInsignia_AdjustPos_Infantry)
		.Process(this->DrawInsignia_AdjustPos_Buildings)
		.Process(this->DrawInsignia_AdjustPos_BuildingsAnchor)
		.Process(this->DrawInsignia_AdjustPos_Units)

		.Process(this->SelectFlashTimer)
		.Process(this->WarheadParticleAlphaImageIsLightFlash)
		.Process(this->CombatLightDetailLevel)
		.Process(this->CombatLightDetailLevel_CheckColored)
		.Process(this->LightFlashAlphaImageDetailLevel)

		.Process(this->Promote_Vet_Anim)
		.Process(this->Promote_Elite_Anim)
		.Process(this->Promote_Vet_PlaySpotlight)
		.Process(this->Promote_Elite_PlaySpotlight)

		.Process(this->DefaultGlobalParticleInstance)

		.Process(this->Shield_ConditionGreen)
		.Process(this->Shield_ConditionYellow)
		.Process(this->Shield_ConditionRed)
		.Process(this->ConditionYellow_Terrain)

		.Process(this->UnitCrateVehicleCap)
		.Process(this->FreeMCV_CreditsThreshold)

		.Process(this->AirShadowBaseScale_log)
		.Process(this->HeightShadowScaling)
		.Process(this->HeightShadowScaling_MinScale)

		.Process(this->VeinsAttack_interval)
		.Process(this->BuildingFlameSpawnBlockFrames)

		.Process(this->PrimaryFactoryIndicator)
		.Process(this->PrimaryFactoryIndicator_Palette)
		.Process(this->DefaultBulletType)
		.Process(this->AIChronoSphereSW)
		.Process(this->AIChronoWarpSW)
		.Process(this->DamageOwnerMultiplier)
		.Process(this->DamageAlliesMultiplier)
		.Process(this->DamageEnemiesMultiplier)
		.Process(this->DamageOwnerMultiplier_Berzerk)
		.Process(this->DamageAlliesMultiplier_Berzerk)
		.Process(this->DamageEnemiesMultiplier_Berzerk)
		.Process(this->DamageOwnerMultiplier_NotAffectsEnemies)
		.Process(this->DamageAlliesMultiplier_NotAffectsEnemies)
		.Process(this->FactoryProgressDisplay)
		.Process(this->MainSWProgressDisplay)
		.Process(this->CombatAlert)
		.Process(this->CombatAlert_MakeAVoice)
		.Process(this->CombatAlert_IgnoreBuilding)
		.Process(this->CombatAlert_EVA)
		.Process(this->CombatAlert_UseFeedbackVoice)
		.Process(this->CombatAlert_UseAttackVoice)
		.Process(this->CombatAlert_SuppressIfInScreen)
		.Process(this->CombatAlert_Interval)
		.Process(this->CombatAlert_SuppressIfAllyDamage)
		.Process(this->SubterraneanSpeed)
		.Process(this->InfantrySpeedData)
		.Process(this->DamagedSpeed)
		.Process(this->DefaultInfantrySelectBox)
		.Process(this->DefaultUnitSelectBox)
		.Process(this->ColorAddUse8BitRGB)
		.Process(this->IronCurtain_ExtraTintIntensity)
		.Process(this->ForceShield_ExtraTintIntensity)
		.Process(this->SubterraneanHeight)
		.Process(this->StartDistributionModeSound)
		.Process(this->EndDistributionModeSound)
		.Process(this->AddDistributionModeCommandSound)
		.Process(this->VoxelLightSource)
		.Process(this->VoxelShadowLightSource)
		.Process(this->UseFixedVoxelLighting)
		.Process(this->HugeBar_Config)

		.Process(this->RegroupWhenMCVDeploy)
		.Process(this->AISellAllOnLastLegs)
		.Process(this->AISellAllDelay)
		.Process(this->AIAllInOnLastLegs)
		.Process(this->RepairBaseNodes)
		.Process(this->MCVRedeploysInCampaign)

		.Process(this->AircraftLevelLightMultiplier)
		.Process(this->AircraftCellLightLevelMultiplier)
		.Process(this->JumpjetLevelLightMultiplier)
		.Process(this->JumpjetCellLightLevelMultiplier)
		.Process(this->JumpjetCellLightApplyBridgeHeight)

		.Process(this->AINormalTargetingDelay)
		.Process(this->PlayerNormalTargetingDelay)
		.Process(this->AIGuardAreaTargetingDelay)
		.Process(this->PlayerGuardAreaTargetingDelay)
		.Process(this->AIAttackMoveTargetingDelay)
		.Process(this->PlayerAttackMoveTargetingDelay)
		.Process(this->DistributeTargetingFrame)
		.Process(this->DistributeTargetingFrame_AIOnly)
		.Process(this->CheckUnitBaseNormal)
		.Process(this->ExtendedBuildingPlacing)
		.Process(this->DefaultExplodeFireAnim)
		.Process(this->CheckExpandPlaceGrid)
		.Process(this->ExpandLandGridFrames)
		.Process(this->ExpandWaterGridFrames)
		.Process(this->AISuperWeaponDelay)
		.Process(this->ChronoSpherePreDelay)
		.Process(this->ChronoSphereDelay)
		.Process(this->LaserZAdjust)
		.Process(this->EBoltZAdjust)
		.Process(this->EBoltZAdjust_ClampInitialDepthForBuilding)
		.Process(this->DisplayIncome_Delay)
		.Process(this->EnablePowerSurplus)
		.Process(this->ShakeScreenUseTSCalculation)
		.Process(this->UnitIdleRotateTurret)
		.Process(this->UnitIdlePointToMouse)
		.Process(this->UnitIdleActionRestartMin)
		.Process(this->UnitIdleActionRestartMax)
		.Process(this->UnitIdleActionIntervalMin)
		.Process(this->UnitIdleActionIntervalMax)
		.Process(this->ExpandAircraftMission)
		.Process(this->ExtendedAircraftMissions_UnlandDamage)
		.Process(this->AssignUnitMissionAfterParadropped)
		.Process(this->LandTypeConfigExts)
		.Process(this->Secrets)

		.Process(this->NoQueueUpToEnter)
		.Process(this->NoQueueUpToUnload)
		.Process(this->NoQueueUpToEnter_Buildings)
		.Process(this->NoQueueUpToUnload_Buildings)

		.Process(this->NoRearm_UnderEMP)
		.Process(this->NoRearm_Temporal)
		.Process(this->NoReload_UnderEMP)
		.Process(this->NoReload_Temporal)

		.Process(this->AttackMindControlledDelay)
		.Process(this->MergeBuildingDamage)
		.Process(this->ExpandBuildingQueue)
		.Process(this->Cameo_AlwaysExist)
		.Process(this->Cameo_OverlayShapes)
		.Process(this->Cameo_OverlayFrames)
		.Process(this->Cameo_OverlayPalette)

		.Process(this->AutoBuilding)
		.Process(this->AIAngerOnAlly)
		.Process(this->BuildingTypeSelectable)
		.Process(this->BuildingWaypoint)

		.Process(this->AIAutoDeployMCV)
		.Process(this->AISetBaseCenter)
		.Process(this->AIBiasSpawnCell)
		.Process(this->AIForbidConYard)
		.Process(this->AINodeWallsOnly)
		.Process(this->AICleanWallNode)

		.Process(this->JumpjetTilt)
		.Process(this->NoTurret_TrackTarget)

		.Process(this->RecountBurst)
		.Process(this->AirstrikeLineColor)
		.Process(this->AmphibiousEnter)
		.Process(this->AmphibiousUnload)
		.Process(this->XGRYMED1_)
		.Process(this->XGRYMED2_)
		.Process(this->XGRYSML1_)

		.Process(this->GiveMoneyIfStorageFull)

		.Process(this->VisualScatter_Min)
		.Process(this->VisualScatter_Max)
		.Process(this->HarvesterDumpAmount)
		.Process(this->HarvesterScanAfterUnload)
		.Process(this->AttackMove_Aggressive)
		.Process(this->AttackMove_UpdateTarget)
		.Process(this->Infantry_IgnoreBuildingSizeLimit)

		.Process(this->BattlePoints)
		.Process(this->BattlePoints_DefaultValue)
		.Process(this->BattlePoints_DefaultFriendlyValue)

		.Process(this->SuperWeaponSidebar_AllowByDefault)
		.Process(this->CampaignAllowHarvesterScanUnderShroud)
		.Process(this->BerzerkTargeting)
		.Process(this->ParadropMission)
		.Process(this->AIParadropMission)
		.Process(this->AttackMove_IgnoreWeaponCheck)
		.Process(this->AttackMove_StopWhenTargetAcquired)
		.Process(this->ShowPowerPlantEnhancerRange)
		.Process(this->OpenTopped_DecloakToFire)
		.Process(this->Temporal_ConsiderVersus)
		.Process(this->Temporal_ApplyMultiplier)
		.Process(this->PenetratesTransport_Level)
		.Process(this->DamageWallRecursivly)
		.Process(this->AirstrikeLineZAdjust)
		.Process(this->AdjacentWallDamage)
		.Process(this->AISellCapturedBuilding)
		.Process(this->InfantryAutoDeploy)
		.Process(this->EnablePassiveAcquireMode)
		.Process(this->UseRetintFix)
		//.Process(this->MyPutData)

		.Process(this->AIAdjacentMax)
		.Process(this->AIAdjacentMax_Campaign)
		.Process(this->PowerSurplus_ScaleToDrainAmount)
		.Process(this->PlayerGuardModePursuit)
		.Process(this->PlayerGuardModeGuardRangeMultiplier)
		.Process(this->PlayerGuardModeGuardRangeAddend)
		.Process(this->PlayerGuardModeGuardRangeMax)
		.Process(this->PlayerGuardStationaryStray)
		.Process(this->AIGuardModePursuit)
		.Process(this->AIGuardModeGuardRangeMultiplier)
		.Process(this->AIGuardModeGuardRangeAddend)
		.Process(this->AIGuardModeGuardRangeMax)
		.Process(this->AIGuardStationaryStray)
		.Process(this->IgnoreCenterMinorRadarEvent)
		.Process(this->WarheadAnimZAdjust)
		.Process(this->IvanBombAttachToCenter)
		.Process(this->FallingDownTargetingFix)
		.Process(this->AIAirTargetingFix)
		.Process(this->SortCameoByName)
		.Process(this->AllowDeployControlledMCV)
		.Process(this->AllowBeaconHotKeyInSinglePlayer)
		.Process(this->TypeSelectUseIFVMode)
		.Process(this->BuildingRadioLink_SyncOwner)
		.Process(this->ApplyPerTargetEffectsOnDetonate)
		.Process(this->ChasingExtraRange)
		.Process(this->ChasingExtraRange_CloseRangeOnly)
		.Process(this->PrefiringExtraRange)
		.Process(this->PrefiringExtraRange_IncludeBurst)
		.Process(this->ExtraRange_FirerMoving)
		.Process(this->FiringAnim_Update)
		.Process(this->ExtendedPlayerRepair)
		.Process(this->UpdateInvisoImmediately)
		.Process(this->AutoTarget_NoThreatBuildings)
		.Process(this->AutoTargetAI_NoThreatBuildings)
		.Process(this->WalkLocomotorMakesWake)
		.Process(this->AllowBerzerkOnAllies)
		.Process(this->UnitsUnsellable)

		.Process(this->DrainMoneyDisplay)
		.Process(this->DrainMoneyDisplay_Houses)
		.Process(this->DrainMoneyDisplay_OnTarget)
		.Process(this->DrainMoneyDisplay_OnTarget_UseDisplayIncome)

		.Process(this->CylinderRangefinding)
		.Process(this->DefaultToGuardArea)

		.Process(this->ExtraThreat_IsThreat)
		.Process(this->ExtraThreat_InRange)
		.Process(this->ExtraThreatCoefficient_InRangeDistance)
		.Process(this->ExtraThreatCoefficient_Facing)
		.Process(this->ExtraThreatCoefficient_DistanceToLastTarget)
		.Process(this->DriverKilled_KillPassengers)
		.Process(this->Psychedelic_StackingMode)
		.Process(this->BuildingGuardRetryDelay)
		.Process(this->DiscardOn_ConsiderHoverAsMoving)
		.Process(this->DisableOveroptimizationInTargeting)

		.Process(this->AreaGuard_UseSelfAsCenter)
		.Process(this->AreaGuard_TargetingInRange)
		.Process(this->AreaGuard_StrayIgnoreDestination)
		.Process(this->Shrapnel_IgnoreHitBuildings)
		.Process(this->AffectsInvokerOnly_IgnoreInvokerState)
		.Process(this->Shrapnel_ObeyWarheadTriggerConditions)
		.Process(this->BunkerStateUpdateDelay)
		.Process(this->PrismRelay_SupportTimeout)
		.Process(this->RemoveMindControl_Silent)
		.Process(this->TeamDelays_DynamicType)
		.Process(this->TeamDelays)
		.Process(this->Parasite_AllowWaterExit)
		.Process(this->FlyNoWobbles)
		.Process(this->ColorAdds)
		.Process(this->BerzerkMission)
		.Process(this->AutoRemoveEarliestBeacon)
		.Process(this->AllowChatBoxInSinglePlayer)
		.Process(this->SecondaryFireSequenceLandOnly)
		.Process(this->ParadropDelay)
		.Process(this->ParadropEndDelay)
		.Process(this->DiscardOn_ConsiderHarvestingAsStationary)
		.Process(this->IsDischargedMemberAutocreateRecruitable)
		;
}

#pragma endregion

#pragma region ContainerHooks

ASMJIT_PATCH(0x667A1D, RulesClass_CTOR, 0x5)
{
	GET(RulesClass*, pItem, ESI);

	RulesExtData::Allocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x667A30, RulesClass_DTOR, 0x5)
{
	GET(RulesClass*, pItem, ECX);

	if(!Phobos::Otamaa::ExeTerminated)
		RulesExtData::Remove(pItem);

	return 0;
}

ASMJIT_PATCH(0x675210, RulesClass_SaveLoad_Prefix, 0x5)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(IStream*, pStm, 0x4);

	pItem->BarrelDebris.clear();
	pItem->DeadBodies.clear();
	pItem->DropPod.clear();
	pItem->MetallicDebris.clear();
	pItem->BridgeExplosions.clear();
	pItem->DamageFireTypes.clear();
	pItem->WeatherConClouds.clear();
	pItem->WeatherConBolts.clear();

	RulesExtData::g_pStm = pStm;

	return 0;
}ASMJIT_PATCH_AGAIN(0x674730, RulesClass_SaveLoad_Prefix, 0x6)

ASMJIT_PATCH(0x678841, RulesClass_Load_Suffix, 0x7)
{
	auto buffer = RulesExtData::Instance();

	PhobosByteStream Stm(0);
	if (Stm.ReadFromStream(RulesExtData::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(RulesExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

ASMJIT_PATCH(0x675205, RulesClass_Save_Suffix, 0x8)
{
	auto buffer = RulesExtData::Instance();
	// negative 4 for the AttachedToObjectPointer , it doesnot get S/L
	PhobosByteStream saver((sizeof(RulesExtData) - 4u));
	PhobosStreamWriter writer(saver);

	writer.Save(RulesExtData::Canary);
	writer.Save(buffer);

	buffer->SaveToStream(writer);
	saver.WriteToStream(RulesExtData::g_pStm);

	return 0;
}

#pragma endregion

//Game_Init , remove early init for read ini for Anim and building type that causing alot of logging error
DEFINE_JUMP(LJMP, 0x52C9C4, 0x52CA37);

void FakeRulesClass::_ReadPowerups(CCINIClass* pINI)
{
	CrateTypeClass::ReadFromPowerups(pINI);
}
DEFINE_FUNCTION_JUMP(LJMP, 0x673E80, FakeRulesClass::_ReadPowerups);

void FakeRulesClass::_ReadGeneral(CCINIClass* pINI)
{
	static constexpr const char* section = "General";

	if (!pINI->GetSection(section))
		return;

	GenericPrerequisite::LoadFromINIList_New(pINI);

	INI_EX exINI(pINI);

	// -------------------------------------------------------------------------
	// TypeList<AnimTypeClass*>
	// -------------------------------------------------------------------------
	detail::ParseVector(this->DamageFireTypes, exINI, section, "DamageFireTypes");
	detail::ParseVector(this->DropPod, exINI, section, "DropPod");
	detail::ParseVector(this->DeadBodies, exINI, section, "DeadBodies");
	detail::ParseVector(this->MetallicDebris, exINI, section, "MetallicDebris");
	detail::ParseVector(this->BridgeExplosions, exINI, section, "BridgeExplosions");
	detail::ParseVector(this->WeatherConClouds, exINI, section, "WeatherConClouds");
	detail::ParseVector(this->WeatherConBolts, exINI, section, "WeatherConBolts");

	// -------------------------------------------------------------------------
	// TypeList<VoxelAnimTypeClass*>
	// -------------------------------------------------------------------------
	detail::ParseVector(this->BarrelDebris, exINI, section, "BarrelDebris");
	detail::ParseVector(this->ExplosiveVoxelDebris, exINI, section, "ExplosiveVoxelDebris");

	// -------------------------------------------------------------------------
	// TypeList<BuildingTypeClass*>
	// -------------------------------------------------------------------------
	detail::ParseVector(this->RepairBay, exINI, section, "RepairBay");
	detail::ParseVector(this->Shipyard, exINI, section, "Shipyard");

	// -------------------------------------------------------------------------
	// TypeList<UnitTypeClass*>
	// -------------------------------------------------------------------------
	detail::ParseVector(this->BaseUnit, exINI, section, "BaseUnit");
	detail::ParseVector(this->HarvesterUnit, exINI, section, "HarvesterUnit");

	// -------------------------------------------------------------------------
	// TypeList<InfantryTypeClass*>
	// -------------------------------------------------------------------------
	detail::ParseVector(this->SecretInfantry, exINI, section, "SecretInfantry");
	detail::ParseVector(this->AmerParaDropInf, exINI, section, "AmerParaDropInf");
	detail::ParseVector(this->AllyParaDropInf, exINI, section, "AllyParaDropInf");
	detail::ParseVector(this->SovParaDropInf, exINI, section, "SovParaDropInf");
	detail::ParseVector(this->YuriParaDropInf, exINI, section, "YuriParaDropInf");
	detail::ParseVector(this->AnimToInfantry, exINI, section, "AnimToInfantry");

	// -------------------------------------------------------------------------
	// TypeList<UnitTypeClass*> (vehicles)
	// -------------------------------------------------------------------------
	detail::ParseVector(this->SecretUnits, exINI, section, "SecretUnits");

	// -------------------------------------------------------------------------
	// TypeList<BuildingTypeClass*> (secret)
	// -------------------------------------------------------------------------
	detail::ParseVector(this->SecretBuildings, exINI, section, "SecretBuildings");

	// Recompute SecretSum after loading all three secret lists
	this->SecretSum = this->SecretInfantry.Count
		+ this->SecretUnits.Count
		+ this->SecretBuildings.Count;

	// -------------------------------------------------------------------------
	// TypeList<AircraftTypeClass*>
	// -------------------------------------------------------------------------
	detail::ParseVector(this->PadAircraft, exINI, section, "PadAircraft");

	// -------------------------------------------------------------------------
	// TypeList<TerrainTypeClass*>
	// -------------------------------------------------------------------------
	detail::ParseVector(this->DefaultMirageDisguises, exINI, section, "DefaultMirageDisguises");

	// -------------------------------------------------------------------------
	// Prerequisite TypeList<int> — vanilla uses CCINIClass::Get_PrerequisiteTypes
	// VERIFY: confirm detail::ParseVector handles prerequisite int lists correctly
	// -------------------------------------------------------------------------
	GenericPrerequisite::Parse(pINI, section, "PrerequisitePower", this->PrerequisitePower);
	GenericPrerequisite::Parse(pINI, section, "PrerequisiteFactory", this->PrerequisiteFactory);
	GenericPrerequisite::Parse(pINI, section, "PrerequisiteBarracks", this->PrerequisiteBarracks);
	GenericPrerequisite::Parse(pINI, section, "PrerequisiteRadar", this->PrerequisiteRadar);
	GenericPrerequisite::Parse(pINI, section, "PrerequisiteTech", this->PrerequisiteTech);
	GenericPrerequisite::Parse(pINI, section, "PrerequisiteProc", this->PrerequisiteProc);

	// -------------------------------------------------------------------------
	// AI TypeList<int> — vanilla uses CCINIClass::Get_TypeList
	// -------------------------------------------------------------------------
	detail::ParseVector(this->TeamDelays, exINI, section, "TeamDelays");
	detail::ParseVector(this->AIHateDelays, exINI, section, "AIHateDelays");
	detail::ParseVector(this->FillEarliestTeamProbability, exINI, section, "FillEarliestTeamProbability");
	detail::ParseVector(this->MinimumAIDefensiveTeams, exINI, section, "MinimumAIDefensiveTeams");
	detail::ParseVector(this->MaximumAIDefensiveTeams, exINI, section, "MaximumAIDefensiveTeams");
	detail::ParseVector(this->TotalAITeamCap, exINI, section, "TotalAITeamCap");
	detail::ParseVector(this->AlliedBaseDefenseCounts, exINI, section, "AlliedBaseDefenseCounts");
	detail::ParseVector(this->SovietBaseDefenseCounts, exINI, section, "SovietBaseDefenseCounts");
	detail::ParseVector(this->ThirdBaseDefenseCounts, exINI, section, "ThirdBaseDefenseCounts");
	detail::ParseVector(this->AIPickWallDefensePercent, exINI, section, "AIPickWallDefensePercent");
	detail::ParseVector(this->DisabledDisguiseDetectionPercent, exINI, section, "DisabledDisguiseDetectionPercent");
	detail::ParseVector(this->AIAutoDeployFrameDelay, exINI, section, "AIAutoDeployFrameDelay");
	detail::ParseVector(this->AISuperDefenseProbability, exINI, section, "AISuperDefenseProbability");
	detail::ParseVector(this->AICaptureNormal, exINI, section, "AICaptureNormal");
	detail::ParseVector(this->AICaptureWounded, exINI, section, "AICaptureWounded");
	detail::ParseVector(this->AICaptureLowPower, exINI, section, "AICaptureLowPower");
	detail::ParseVector(this->AICaptureLowMoney, exINI, section, "AICaptureLowMoney");
	detail::ParseVector(this->MultiplayerAICM, exINI, section, "MultiplayerAICM");
	detail::ParseVector(this->AIVirtualPurifiers, exINI, section, "AIVirtualPurifiers");
	detail::ParseVector(this->AISlaveMinerNumber, exINI, section, "AISlaveMinerNumber");
	detail::ParseVector(this->HarvestersPerRefinery, exINI, section, "HarvestersPerRefinery");
	detail::ParseVector(this->AIExtraRefineries, exINI, section, "AIExtraRefineries");
	detail::ParseVector(this->AmerParaDropNum, exINI, section, "AmerParaDropNum");
	detail::ParseVector(this->AllyParaDropNum, exINI, section, "AllyParaDropNum");
	detail::ParseVector(this->SovParaDropNum, exINI, section, "SovParaDropNum");
	detail::ParseVector(this->YuriParaDropNum, exINI, section, "YuriParaDropNum");
	detail::ParseVector(this->AIIonCannonConYardValue, exINI, section, "AIIonCannonConYardValue");
	detail::ParseVector(this->AIIonCannonWarFactoryValue, exINI, section, "AIIonCannonWarFactoryValue");
	detail::ParseVector(this->AIIonCannonPowerValue, exINI, section, "AIIonCannonPowerValue");
	detail::ParseVector(this->AIIonCannonTechCenterValue, exINI, section, "AIIonCannonTechCenterValue");
	detail::ParseVector(this->AIIonCannonEngineerValue, exINI, section, "AIIonCannonEngineerValue");
	detail::ParseVector(this->AIIonCannonThiefValue, exINI, section, "AIIonCannonThiefValue");
	detail::ParseVector(this->AIIonCannonHarvesterValue, exINI, section, "AIIonCannonHarvesterValue");
	detail::ParseVector(this->AIIonCannonMCVValue, exINI, section, "AIIonCannonMCVValue");
	detail::ParseVector(this->AIIonCannonAPCValue, exINI, section, "AIIonCannonAPCValue");
	detail::ParseVector(this->AIIonCannonBaseDefenseValue, exINI, section, "AIIonCannonBaseDefenseValue");
	detail::ParseVector(this->AIIonCannonPlugValue, exINI, section, "AIIonCannonPlugValue");
	detail::ParseVector(this->AIIonCannonHelipadValue, exINI, section, "AIIonCannonHelipadValue");
	detail::ParseVector(this->AIIonCannonTempleValue, exINI, section, "AIIonCannonTempleValue");
	detail::ParseVector(this->RadarEventSuppressionDistances, exINI, section, "RadarEventSuppressionDistances");
	detail::ParseVector(this->RadarEventVisibilityDurations, exINI, section, "RadarEventVisibilityDurations");
	detail::ParseVector(this->RadarEventDurations, exINI, section, "RadarEventDurations");

	// -------------------------------------------------------------------------
	// Single AnimTypeClass* pointers — Find_Or_Make → pass true
	// -------------------------------------------------------------------------
	detail::read(this->OreTwinkle, exINI, section, "OreTwinkle", true);
	detail::read(this->NukeTakeOff, exINI, section, "NukeTakeOff", true);
	detail::read(this->Wake, exINI, section, "Wake", true);
	detail::read(this->IonBlast, exINI, section, "IonBlast", true);
	detail::read(this->IonBeam, exINI, section, "IonBeam", true);
	detail::read(this->WeatherConBoltExplosion, exINI, section, "WeatherConBoltExplosion", true);
	detail::read(this->DominatorFirstAnim, exINI, section, "DominatorFirstAnim", true);
	detail::read(this->DominatorSecondAnim, exINI, section, "DominatorSecondAnim", true);
	detail::read(this->ChronoPlacement, exINI, section, "ChronoPlacement", true);
	detail::read(this->ChronoBeam, exINI, section, "ChronoBeam", true);
	detail::read(this->ChronoBlast, exINI, section, "ChronoBlast", true);
	detail::read(this->ChronoBlastDest, exINI, section, "ChronoBlastDest", true);
	detail::read(this->WarpIn, exINI, section, "WarpIn", true);
	detail::read(this->WarpOut, exINI, section, "WarpOut", true);
	detail::read(this->WarpAway, exINI, section, "WarpAway", true);
	detail::read(this->IronCurtainInvokeAnim, exINI, section, "IronCurtainInvokeAnim", true);
	detail::read(this->ForceShieldInvokeAnim, exINI, section, "ForceShieldInvokeAnim", true);
	detail::read(this->WeaponNullifyAnim, exINI, section, "WeaponNullifyAnim", true);
	detail::read(this->ChronoSparkle1, exINI, section, "ChronoSparkle1", true); // key is "ChronoSparkle1", not "ChronoSparkle"
	detail::read(this->InfantryExplode, exINI, section, "InfantryExplode", true);
	detail::read(this->FlamingInfantry, exINI, section, "FlamingInfantry", true);
	detail::read(this->InfantryHeadPop, exINI, section, "InfantryHeadPop", true);
	detail::read(this->InfantryNuked, exINI, section, "InfantryNuked", true);
	detail::read(this->InfantryVirus, exINI, section, "InfantryVirus", true);
	detail::read(this->InfantryBrute, exINI, section, "InfantryBrute", true);
	detail::read(this->InfantryMutate, exINI, section, "InfantryMutate", true);
	detail::read(this->Behind, exINI, section, "Behind", true);
	detail::read(this->MoveFlash, exINI, section, "MoveFlash", true);
	detail::read(this->Parachute, exINI, section, "Parachute", true);
	detail::read(this->BombParachute, exINI, section, "BombParachute", true);
	detail::read(this->DropZoneAnim, exINI, section, "DropZoneAnim", true);
	detail::read(this->EMPulseSparkles, exINI, section, "EMPulseSparkles", true);

	// -------------------------------------------------------------------------
	// Single VoxelAnimTypeClass* pointers
	// -------------------------------------------------------------------------
	detail::read(this->TireVoxelDebris, exINI, section, "TireVoxelDebris", true);
	detail::read(this->ScrapVoxelDebris, exINI, section, "ScrapVoxelDebris", true);

	// -------------------------------------------------------------------------
	// Single BuildingTypeClass* pointers
	// -------------------------------------------------------------------------
	detail::read(this->GDIGateOne, exINI, section, "GDIGateOne", true);
	detail::read(this->GDIGateTwo, exINI, section, "GDIGateTwo", true);
	detail::read(this->NodGateOne, exINI, section, "NodGateOne", true);
	detail::read(this->NodGateTwo, exINI, section, "NodGateTwo", true);
	detail::read(this->GDIPowerPlant, exINI, section, "GDIPowerPlant", true);
	detail::read(this->NodRegularPower, exINI, section, "NodRegularPower", true);
	detail::read(this->NodAdvancedPower, exINI, section, "NodAdvancedPower", true);
	detail::read(this->ThirdPowerPlant, exINI, section, "ThirdPowerPlant", true);
	detail::read(this->PrismType, exINI, section, "PrismType", true); // ini_BuildingTypeClass::Find_Or_Make in vanilla

	// -------------------------------------------------------------------------
	// Single UnitTypeClass* pointers
	// -------------------------------------------------------------------------
	detail::read(this->LargeVisceroid, exINI, section, "LargeVisceroid", true);
	detail::read(this->SmallVisceroid, exINI, section, "SmallVisceroid", true);
	detail::read(this->PrerequisiteProcAlternate, exINI, section, "PrerequisiteProcAlternate", true);

	// -------------------------------------------------------------------------
	// Single InfantryTypeClass* pointers — ini_InfantryTypeClass::Find_Or_Make
	// -------------------------------------------------------------------------
	detail::read(this->Paratrooper, exINI, section, "Paratrooper", true);
	detail::read(this->AlliedDisguise, exINI, section, "AlliedDisguise", true);
	detail::read(this->SovietDisguise, exINI, section, "SovietDisguise", true);
	detail::read(this->ThirdDisguise, exINI, section, "ThirdDisguise", true);
	detail::read(this->Engineer, exINI, section, "Engineer", true);
	detail::read(this->Technician, exINI, section, "Technician", true);
	detail::read(this->Pilot, exINI, section, "Pilot", true);
	detail::read(this->AlliedCrew, exINI, section, "AlliedCrew", true);
	detail::read(this->SovietCrew, exINI, section, "SovietCrew", true);
	detail::read(this->ThirdCrew, exINI, section, "ThirdCrew", true);

	// -------------------------------------------------------------------------
	// Single WeaponTypeClass*
	// -------------------------------------------------------------------------
	detail::read(this->DropPodWeapon, exINI, section, "DropPodWeapon", true);

	// -------------------------------------------------------------------------
	// Single WarheadTypeClass*
	// -------------------------------------------------------------------------
	detail::read(this->DominatorWarhead, exINI, section, "DominatorWarhead", true);
	detail::read(this->LightningWarhead, exINI, section, "LightningWarhead", true); // ini_WarheadTypeClass::Find_Or_Make

	// -------------------------------------------------------------------------
	// Single ParticleSystemTypeClass*
	// -------------------------------------------------------------------------
	detail::read(this->BarrelParticle, exINI, section, "BarrelParticle", true);

	// -------------------------------------------------------------------------
	// Single BarrelExplode AnimTypeClass*
	// -------------------------------------------------------------------------
	detail::read(this->BarrelExplode, exINI, section, "BarrelExplode", true);

	// -------------------------------------------------------------------------
	// Single AircraftTypeClass* — ini_AircraftTypeClass::Find_Or_Make
	// -------------------------------------------------------------------------
	detail::read(this->V3Rocket.Type, exINI, section, "V3RocketType", true);
	detail::read(this->DMisl.Type, exINI, section, "DMislType", true);
	detail::read(this->CMisl.Type, exINI, section, "CMislType", true);

	// -------------------------------------------------------------------------
	// Single TerrainTypeClass*
	// -------------------------------------------------------------------------
	detail::read(this->VeinholeTypeClass, exINI, section, "VeinholeTypeClass", true);

	// -------------------------------------------------------------------------
	// Scalar doubles
	// -------------------------------------------------------------------------
	detail::read(this->TiberiumHeal, exINI, section, "TiberiumHeal");
	detail::read(this->ZoomInFactor, exINI, section, "ZoomInFactor");
	detail::read(this->MinLowPowerProductionSpeed, exINI, section, "MinLowPowerProductionSpeed");
	detail::read(this->MaxLowPowerProductionSpeed, exINI, section, "MaxLowPowerProductionSpeed");
	detail::read(this->LowPowerPenaltyModifier, exINI, section, "LowPowerPenaltyModifier");
	detail::read(this->MultipleFactory, exINI, section, "MultipleFactory");
	detail::read(this->TreeFlammability, exINI, section, "TreeFlammability");
	detail::read(this->MissileROTVar, exINI, section, "MissileROTVar");
	detail::read(this->MissileSpeedVar, exINI, section, "MissileSpeedVar");
	detail::read(this->CrewEscape, exINI, section, "CrewEscape");
	detail::read(this->TunnelSpeed, exINI, section, "TunnelSpeed");
	detail::read(this->HoverDampen, exINI, section, "HoverDampen");
	detail::read(this->HoverBob, exINI, section, "HoverBob");
	detail::read(this->HoverBoost, exINI, section, "HoverBoost");
	detail::read(this->HoverAcceleration, exINI, section, "HoverAcceleration");
	detail::read(this->HoverBrake, exINI, section, "HoverBrake");
	detail::read(this->VeteranRatio, exINI, section, "VeteranRatio");
	detail::read(this->VeteranCombat, exINI, section, "VeteranCombat");
	detail::read(this->VeteranSpeed, exINI, section, "VeteranSpeed");
	detail::read(this->VeteranSight, exINI, section, "VeteranSight");
	detail::read(this->VeteranArmor, exINI, section, "VeteranArmor");
	detail::read(this->VeteranROF, exINI, section, "VeteranROF");
	detail::read(this->VeteranCap, exINI, section, "VeteranCap");
	detail::read(this->ShipSinkingWeight, exINI, section, "ShipSinkingWeight");
	detail::read(this->IceCrackingWeight, exINI, section, "IceCrackingWeight");
	detail::read(this->IceBreakingWeight, exINI, section, "IceBreakingWeight");
	detail::read(this->PlacementDelay, exINI, section, "PlacementDelay");
	detail::read(this->TrackedUphill, exINI, section, "TrackedUphill");
	detail::read(this->TrackedDownhill, exINI, section, "TrackedDownhill");
	detail::read(this->WheeledUphill, exINI, section, "WheeledUphill");
	detail::read(this->WheeledDownhill, exINI, section, "WheeledDownhill");
	detail::read(this->SpyMoneyStealPercent, exINI, section, "SpyMoneyStealPercent");
	detail::read(this->PurifierBonus, exINI, section, "PurifierBonus");
	detail::read(this->AIUseTurbineUpgradeProbability, exINI, section, "AIUseTurbineUpgradeProbability");
	detail::read(this->AIMinorSuperReadyPercent, exINI, section, "AIMinorSuperReadyPercent");
	detail::read(this->AICaptureWoundedMark, exINI, section, "AICaptureWoundedMark");
	detail::read(this->CloakDelay, exINI, section, "CloakDelay");
	detail::read(this->GameSpeedBias, exINI, section, "GameSpeedBias");
	detail::read(this->BaseBias, exINI, section, "BaseBias"); // note: field is NervousBias, key is "BaseBias"
	detail::read(this->BaseDefenseDelay, exINI, section, "BaseDefenseDelay");
	detail::read(this->SuspendDelay, exINI, section, "SuspendDelay");
	detail::read(this->SurvivorRate, exINI, section, "SurvivorRate"); // field SurvivorFraction, key "SurvivorRate"
	detail::read(this->ReloadRate, exINI, section, "ReloadRate");
	detail::read(this->BuildupTime, exINI, section, "BuildupTime");
	detail::read(this->HarvesterDumpRate, exINI, section, "HarvesterDumpRate");
	detail::read(this->BuildSpeed, exINI, section, "BuildSpeed"); // field BuildSpeedBias, key "BuildSpeed"
	detail::read(this->DamageDelay, exINI, section, "DamageDelay");
	detail::read(this->GrowthRate, exINI, section, "GrowthRate");
	detail::read(this->RefundPercent, exINI, section, "RefundPercent");
	detail::read(this->RepairPercent, exINI, section, "RepairPercent");
	detail::read(this->RepairRate, exINI, section, "RepairRate");
	detail::read(this->URepairRate, exINI, section, "URepairRate");
	detail::read(this->IRepairRate, exINI, section, "IRepairRate");
	detail::read(this->RadarEventSpeed, exINI, section, "RadarEventSpeed");
	detail::read(this->RadarEventRotationSpeed, exINI, section, "RadarEventRotationSpeed");
	detail::read(this->RadarEventColorSpeed, exINI, section, "RadarEventColorSpeed");
	detail::read(this->MyEffectivenessCoefficientDefault, exINI, section, "MyEffectivenessCoefficientDefault");
	detail::read(this->TargetEffectivenessCoefficientDefault, exINI, section, "TargetEffectivenessCoefficientDefault");
	detail::read(this->TargetSpecialThreatCoefficientDefault, exINI, section, "TargetSpecialThreatCoefficientDefault");
	detail::read(this->TargetStrengthCoefficientDefault, exINI, section, "TargetStrengthCoefficientDefault");
	detail::read(this->TargetDistanceCoefficientDefault, exINI, section, "TargetDistanceCoefficientDefault");
	detail::read(this->DumbMyEffectivenessCoefficient, exINI, section, "DumbMyEffectivenessCoefficient");
	detail::read(this->DumbTargetEffectivenessCoefficient, exINI, section, "DumbTargetEffectivenessCoefficient");
	detail::read(this->DumbTargetSpecialThreatCoefficient, exINI, section, "DumbTargetSpecialThreatCoefficient");
	detail::read(this->DumbTargetStrengthCoefficient, exINI, section, "DumbTargetStrengthCoefficient");
	detail::read(this->DumbTargetDistanceCoefficient, exINI, section, "DumbTargetDistanceCoefficient");
	detail::read(this->EnemyHouseThreatBonus, exINI, section, "EnemyHouseThreatBonus");
	detail::read(this->AITriggerSuccessWeightDelta, exINI, section, "AITriggerSuccessWeightDelta");
	detail::read(this->AITriggerFailureWeightDelta, exINI, section, "AITriggerFailureWeightDelta");
	detail::read(this->AITriggerTrackRecordCoefficient, exINI, section, "AITriggerTrackRecordCoefficient");
	detail::read(this->ConditionYellowSparkingProbability, exINI, section, "ConditionYellowSparkingProbability");
	detail::read(this->ConditionRedSparkingProbability, exINI, section, "ConditionRedSparkingProbability");
	detail::read(this->WallBuildSpeedCoefficient, exINI, section, "WallBuildSpeedCoefficient");
	detail::read(this->ChargeToDrainRatio, exINI, section, "ChargeToDrainRatio");
	detail::read(this->SpotlightSpeed, exINI, section, "SpotlightSpeed");
	detail::read(this->SpotlightAcceleration, exINI, section, "SpotlightAcceleration");
	detail::read(this->SpotlightAngle, exINI, section, "SpotlightAngle");
	detail::read(this->EngineerCaptureLevel, exINI, section, "EngineerCaptureLevel");

	// SUSPECT: vanilla reads EngineerDamage using key "EngineerCaptureLevel" (same key as above — likely a copy-paste bug)
	// VERIFY: confirm intended key for EngineerDamage vs EngineerCaptureLevel
	detail::read(this->EngineerDamage, exINI, section, "EngineerDamage");
	auto ReadAsFloat = [](int& val, INI_EX& exINI , const char* pSection, const char* pKey){
		float buffer = val;
		if(detail::read<float>(buffer, exINI, pSection, pKey))
			val = buffer;
	};

	auto ReadAsDouble= [] (float& val, INI_EX& exINI, const char* pSection, const char* pKey , float* default_val = nullptr){
		double buffer = val;
		if(detail::read<double>(buffer, exINI , pSection, pKey))
			val = buffer;
		else if(default_val)
			val = *default_val;
	};


	// V3Rocket sub-struct
	ReadAsDouble(this->V3Rocket.PitchInitial, exINI, section, "V3RocketPitchInitial");
	ReadAsDouble(this->V3Rocket.PitchFinal, exINI, section, "V3RocketPitchFinal");
	ReadAsDouble(this->V3Rocket.TurnRate, exINI, section, "V3RocketTurnRate");
	ReadAsFloat(this->V3Rocket.RaiseRate, exINI, section, "V3RocketRaiseRate");
	ReadAsDouble(this->V3Rocket.Acceleration, exINI, section, "V3RocketAcceleration");

	// DMisl sub-struct
	ReadAsDouble(this->DMisl.PitchInitial, exINI, section, "DMislPitchInitial");
	ReadAsDouble(this->DMisl.PitchFinal, exINI, section, "DMislPitchFinal");
	ReadAsDouble(this->DMisl.TurnRate, exINI, section, "DMislTurnRate");
	ReadAsFloat(this->DMisl.RaiseRate, exINI, section, "DMislRaiseRate");
	ReadAsDouble(this->DMisl.Acceleration, exINI, section, "DMislAcceleration");

	// CMisl sub-struct
	ReadAsDouble(this->CMisl.PitchInitial, exINI, section, "CMislPitchInitial");
	ReadAsDouble(this->CMisl.PitchFinal, exINI, section, "CMislPitchFinal");
	ReadAsDouble(this->CMisl.TurnRate, exINI, section, "CMislTurnRate");
	ReadAsFloat(this->CMisl.RaiseRate, exINI, section, "CMislRaiseRate");
	// SUSPECT: vanilla reads CMisl.Acceleration with default this->DMisl.Acceleration (not CMisl)
	// Preserving vanilla behaviour; VERIFY if intentional
	ReadAsDouble(this->CMisl.Acceleration, exINI, section, "CMislAcceleration", &this->DMisl.Acceleration);

	// -------------------------------------------------------------------------
	// PrismSupportModifier — vanilla multiplies by 100 after read
	// VERIFY: confirm the stored value is ×100 of the INI value (percentage stored as integer × 100?)
	// -------------------------------------------------------------------------
	{
		double prismMod = this->PrismSupportModifier;
		detail::read(prismMod, exINI, section, "PrismSupportModifier");
		this->PrismSupportModifier = int(prismMod / 100.0);
	}

	// TalkBubbleTime — vanilla stores as frames (value × 60); INI is in seconds
	// VERIFY: confirm conversion direction matches vanilla
	{
		double talkBubbleSec = this->TalkBubbleTime * (1.0 / 60.0);
		detail::read(talkBubbleSec, exINI, section, "TalkBubbleTime");
		this->TalkBubbleTime = float(talkBubbleSec * 60.0);
	}

	// -------------------------------------------------------------------------
	// Scalar ints
	// -------------------------------------------------------------------------
	detail::read(this->SelfHealInfantryFrames, exINI, section, "SelfHealInfantryFrames");
	detail::read(this->SelfHealInfantryAmount, exINI, section, "SelfHealInfantryAmount");
	detail::read(this->SelfHealUnitFrames, exINI, section, "SelfHealUnitFrames");
	detail::read(this->SelfHealUnitAmount, exINI, section, "SelfHealUnitAmount");
	detail::read(this->DominatorFireAtPercentage, exINI, section, "DominatorFireAtPercentage");
	detail::read(this->DominatorCaptureRange, exINI, section, "DominatorCaptureRange");
	detail::read(this->DominatorDamage, exINI, section, "DominatorDamage");
	detail::read(this->MissileSafetyAltitude, exINI, section, "MissileSafetyAltitude");
	detail::read(this->DropPodHeight, exINI, section, "DropPodHeight");
	detail::read(this->DropPodSpeed, exINI, section, "DropPodSpeed");
	detail::read(this->HoverHeight, exINI, section, "HoverHeight");
	detail::read(this->BridgeVoxelMax, exINI, section, "BridgeVoxelMax");
	detail::read(this->CloakingStages, exINI, section, "CloakingStages");
	detail::read(this->CliffBackImpassability, exINI, section, "CliffBackImpassability");
	detail::read(this->WindDirection, exINI, section, "WindDirection");
	detail::read(this->FlightLevel, exINI, section, "FlightLevel");
	detail::read(this->ParachuteMaxFallRate, exINI, section, "ParachuteMaxFallRate");
	detail::read(this->NoParachuteMaxFallRate, exINI, section, "NoParachuteMaxFallRate");
	detail::read(this->MaximumCheerRate, exINI, section, "MaximumCheerRate");
	detail::read(this->SpyPowerBlackout, exINI, section, "SpyPowerBlackout");
	detail::read(this->AIAlternateProductionCreditCutoff, exINI, section, "AIAlternateProductionCreditCutoff");
	detail::read(this->DissolveUnfilledTeamDelay, exINI, section, "DissolveUnfilledTeamDelay");
	detail::read(this->AISafeDistance, exINI, section, "AISafeDistance");
	detail::read(this->HarvesterTooFarDistance, exINI, section, "HarvesterTooFarDistance");
	detail::read(this->ChronoHarvTooFarDistance, exINI, section, "ChronoHarvTooFarDistance");
	detail::read(this->AIRestrictReplaceTime, exINI, section, "AIRestrictReplaceTime");
	detail::read(this->ThreatPerOccupant, exINI, section, "ThreatPerOccupant");
	detail::read(this->ApproachTargetResetMultiplier, exINI, section, "ApproachTargetResetMultiplier");
	detail::read(this->CampaignMoneyDeltaEasy, exINI, section, "CampaignMoneyDeltaEasy");
	detail::read(this->CampaignMoneyDeltaHard, exINI, section, "CampaignMoneyDeltaHard");
	detail::read(this->GuardAreaTargetingDelay, exINI, section, "GuardAreaTargetingDelay");
	detail::read(this->NormalTargetingDelay, exINI, section, "NormalTargetingDelay");
	detail::read(this->AINavalYardAdjacency, exINI, section, "AINavalYardAdjacency");
	detail::read(this->MaximumBuildingPlacementFailures, exINI, section, "MaximumBuildingPlacementFailures");
	detail::read(this->SlaveMinerKickFrameDelay, exINI, section, "SlaveMinerKickFrameDelay");
	detail::read(this->AISuperDefenseFrames, exINI, section, "AISuperDefenseFrames");
	detail::read(this->AICaptureLowMoneyMark, exINI, section, "AICaptureLowMoneyMark");
	detail::read(this->ChronoDelay, exINI, section, "ChronoDelay");
	detail::read(this->ChronoReinfDelay, exINI, section, "ChronoReinfDelay");
	detail::read(this->ChronoDistanceFactor, exINI, section, "ChronoDistanceFactor");
	detail::read(this->ChronoMinimumDelay, exINI, section, "ChronoMinimumDelay");
	detail::read(this->ChronoRangeMinimum, exINI, section, "ChronoRangeMinimum");
	detail::read(this->SuspendPriority, exINI, section, "SuspendPriority");
	detail::read(this->AlliedSurvivorDivisor, exINI, section, "AlliedSurvivorDivisor");
	detail::read(this->SovietSurvivorDivisor, exINI, section, "SovietSurvivorDivisor");
	detail::read(this->ThirdSurvivorDivisor, exINI, section, "ThirdSurvivorDivisor");
	detail::read(this->HarvesterLoadRate, exINI, section, "HarvesterLoadRate");
	detail::read(this->RepairStep, exINI, section, "RepairStep");
	detail::read(this->IRepairStep, exINI, section, "IRepairStep");
	detail::read(this->AttackingAircraftSightRange, exINI, section, "AttackingAircraftSightRange");
	detail::read(this->LeptonsPerSightIncrease, exINI, section, "LeptonsPerSightIncrease");
	detail::read(this->TiberiumTransmogrify, exINI, section, "TiberiumTransmogrify");
	detail::read(this->LightningDeferment, exINI, section, "LightningDeferment");
	detail::read(this->LightningDamage, exINI, section, "LightningDamage");
	detail::read(this->LightningStormDuration, exINI, section, "LightningStormDuration");
	detail::read(this->LightningHitDelay, exINI, section, "LightningHitDelay");
	detail::read(this->LightningScatterDelay, exINI, section, "LightningScatterDelay");
	detail::read(this->LightningCellSpread, exINI, section, "LightningCellSpread");
	detail::read(this->LightningSeparation, exINI, section, "LightningSeparation");
	detail::read(this->ForceShieldRadius, exINI, section, "ForceShieldRadius");
	detail::read(this->ForceShieldDuration, exINI, section, "ForceShieldDuration");
	detail::read(this->ForceShieldBlackoutDuration, exINI, section, "ForceShieldBlackoutDuration");
	detail::read(this->ForceShieldPlayFadeSoundTime, exINI, section, "ForceShieldPlayFadeSoundTime");
	detail::read(this->PrismSupportMax, exINI, section, "PrismSupportMax");
	detail::read(this->PrismSupportDelay, exINI, section, "PrismSupportDelay");
	detail::read(this->PrismSupportDuration, exINI, section, "PrismSupportDuration");
	detail::read(this->PrismSupportHeight, exINI, section, "PrismSupportHeight");
	detail::read(this->ParadropRadius, exINI, section, "ParadropRadius");
	detail::read(this->SpotlightMovementRadius, exINI, section, "SpotlightMovementRadius");
	detail::read(this->SpotlightLocationRadius, exINI, section, "SpotlightLocationRadius");
	detail::read(this->SpotlightRadius, exINI, section, "SpotlightRadius");
	detail::read(this->RevealTriggerRadius, exINI, section, "RevealTriggerRadius");
	detail::read(this->WeedCapacity, exINI, section, "WeedCapacity");
	detail::read(this->FlashFrameTime, exINI, section, "FlashFrameTime");
	detail::read(this->RadarCombatFlashTime, exINI, section, "RadarCombatFlashTime");
	detail::read(this->RadarEventMinRadius, exINI, section, "RadarEventMinRadius");
	detail::read(this->VeinholeMonsterStrength, exINI, section, "VeinholeMonsterStrength");
	detail::read(this->MaxVeinholeGrowth, exINI, section, "MaxVeinholeGrowth");
	detail::read(this->VeinholeGrowthRate, exINI, section, "VeinholeGrowthRate");
	detail::read(this->VeinholeShrinkRate, exINI, section, "VeinholeShrinkRate");
	detail::read(this->VeinDamage, exINI, section, "VeinDamage");
	detail::read(this->InfantryBlinkDisguiseTime, exINI, section, "InfantryBlinkDisguiseTime");
	detail::read(this->MaximumQueuedObjects, exINI, section, "MaximumQueuedObjects");
	detail::read(this->MaxWaypointPathLength, exINI, section, "MaxWaypointPathLength");
	detail::read(this->TreeStrength, exINI, section, "TreeStrength");

	// V3Rocket / DMisl / CMisl int fields
	detail::read(this->V3Rocket.PauseFrames, exINI, section, "V3RocketPauseFrames");
	detail::read(this->V3Rocket.TiltFrames, exINI, section, "V3RocketTiltFrames");
	detail::read(this->V3Rocket.Altitude, exINI, section, "V3RocketAltitude");
	detail::read(this->V3Rocket.Damage, exINI, section, "V3RocketDamage");
	detail::read(this->V3Rocket.EliteDamage, exINI, section, "V3RocketEliteDamage");
	detail::read(this->V3Rocket.BodyLength, exINI, section, "V3RocketBodyLength");

	detail::read(this->DMisl.PauseFrames, exINI, section, "DMislPauseFrames");
	detail::read(this->DMisl.TiltFrames, exINI, section, "DMislTiltFrames");
	detail::read(this->DMisl.Altitude, exINI, section, "DMislAltitude");
	detail::read(this->DMisl.Damage, exINI, section, "DMislDamage");
	detail::read(this->DMisl.EliteDamage, exINI, section, "DMislEliteDamage");
	detail::read(this->DMisl.BodyLength, exINI, section, "DMislBodyLength");

	detail::read(this->CMisl.PauseFrames, exINI, section, "CMislPauseFrames");
	detail::read(this->CMisl.TiltFrames, exINI, section, "CMislTiltFrames");
	detail::read(this->CMisl.Altitude, exINI, section, "CMislAltitude");
	detail::read(this->CMisl.Damage, exINI, section, "CMislDamage");
	detail::read(this->CMisl.EliteDamage, exINI, section, "CMislEliteDamage");
	detail::read(this->CMisl.BodyLength, exINI, section, "CMislBodyLength");

	// -------------------------------------------------------------------------
	// Lepton fields (int, read via CCINIClass::Get_Lepton in vanilla)
	// VERIFY: confirm detail::read<int> handles leptons the same way (no implicit cell→lepton scaling)
	// -------------------------------------------------------------------------
	detail::read(this->CameraRange, exINI, section, "CameraRange");
	detail::read(this->Stray, exINI, section, "Stray");
	detail::read(this->RelaxedStray, exINI, section, "RelaxedStray");
	detail::read(this->GuardModeStray, exINI, section, "GuardModeStray");
	detail::read(this->GuardModeStray, exINI, section, "GuardModeStray");
	detail::read(this->TiberiumShortScan, exINI, section, "TiberiumShortScan");
	detail::read(this->TiberiumLongScan, exINI, section, "TiberiumLongScan");
	detail::read(this->SlaveMinerShortScan, exINI, section, "SlaveMinerShortScan");
	detail::read(this->SlaveMinerSlaveScan, exINI, section, "SlaveMinerSlaveScan");
	detail::read(this->SlaveMinerLongScan, exINI, section, "SlaveMinerLongScan");
	detail::read(this->SlaveMinerScanCorrection, exINI, section, "SlaveMinerScanCorrection");
	detail::read(this->AISuperDefenseDistance, exINI, section, "AISuperDefenseDistance");

	// -------------------------------------------------------------------------
	// Bool fields
	// -------------------------------------------------------------------------
	detail::read(this->RevealByHeight, exINI, section, "RevealByHeight");
	detail::read(this->AllowShroudedSubteranneanMoves, exINI, section, "AllowShroudedSubteranneanMoves");
	detail::read(this->NodAIBuildsWalls, exINI, section, "NodAIBuildsWalls");
	detail::read(this->AIBuildsWalls, exINI, section, "AIBuildsWalls");
	detail::read(this->UseMinDefenseRule, exINI, section, "UseMinDefenseRule");
	detail::read(this->CurleyShuffle, exINI, section, "CurleyShuffle");
	detail::read(this->FineDiffControl, exINI, section, "FineDiffControl");
	detail::read(this->AttackCursorOnDisguise, exINI, section, "AttackCursorOnDisguise");
	detail::read(this->ChronoTrigger, exINI, section, "ChronoTrigger");
	detail::read(this->BlendedFog, exINI, section, "BlendedFog");
	detail::read(this->SeparateAircraft, exINI, section, "SeparateAircraft");
	detail::read(this->MutateExplosion, exINI, section, "MutateExplosion");
	detail::read(this->LightningPrintText, exINI, section, "LightningPrintText");
	detail::read(this->AircraftFogReveal, exINI, section, "AircraftFogReveal"); // VERIFY: stored as int in vanilla but read via Get_Int
	detail::read(this->V3Rocket.LazyCurve, exINI, section, "V3RocketLazyCurve");
	detail::read(this->DMisl.LazyCurve, exINI, section, "DMislLazyCurve");
	detail::read(this->CMisl.LazyCurve, exINI, section, "CMislLazyCurve");

	// -------------------------------------------------------------------------
	// DropPodAngle — clamped after read [pi/8 .. 3pi/8]
	// -------------------------------------------------------------------------
	{
		double angle = this->DropPodAngle;
		detail::read(angle, exINI, section, "DropPodAngle");
		// vanilla constants: max = 1.178097245096172 (3π/8), min = 0.3926990816987241 (π/8)
		constexpr double kDropPodAngleMax = 1.178097245096172;
		constexpr double kDropPodAngleMin = 0.3926990816987241;
		this->DropPodAngle = std::clamp(angle, kDropPodAngleMin, kDropPodAngleMax);
	}

	if (pINI->ReadString(section, GameStrings::WallTower, nullptr, Phobos::readBuffer) > 0) {
		if (const auto pBuilding = BuildingTypeClass::FindOrAllocate(Phobos::readBuffer)) {
			this->WallTower = pBuilding;
		} else {
			Debug::LogInfo("WallTower Building readed as [{}] but it is nullptr ! ", Phobos::readBuffer);
		}
	}

	RocketTypeClass::ReadListFromINI(pINI);
}

void RulesExtData::InitializeAfterAllRulesLoaded()
{
	auto g_instance = PhobosGlobal::Instance();

	// tint color
	if (!g_instance->ColorDatas.Initialized) {
		g_instance->ColorDatas.Initialized = true;
		g_instance->ColorDatas.Forceshield_Color = GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->ForceShieldColor);
		g_instance->ColorDatas.IronCurtain_Color = GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->IronCurtainColor);
		g_instance->ColorDatas.LaserTarget_Color = GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->LaserTargetColor);
		g_instance->ColorDatas.Berserk_Color = GeneralUtils::GetColorFromColorAdd(RulesClass::Instance->BerserkColor);
	}
}

ASMJIT_PATCH(0x68684A, Game_ReadScenario_FinishReadingScenarioINI, 0x7) //9
{
	if (R->AL()) //ScenarioLoadSucceed
	{
		//pre iterate this important indexes
		//so we dont need to do lookups with name multiple times
		//these function only executed when ScenarioClass::ReadScenario return true (AL)
		if (const auto pRulesGlobal = RulesExtData::Instance())
		{
			pRulesGlobal->CivilianSideIndex = SideClass::FindIndexById(GameStrings::Civilian());
			//Debug::LogInfo("Finding Civilian Side Index[{}] ! " , pRulesGlobal->CivilianSideIndex);
			pRulesGlobal->NeutralCountryIndex = HouseTypeClass::FindIndexByIdAndName(GameStrings::Neutral());
			//Debug::LogInfo("Finding Neutral Country Index[{}] ! ", pRulesGlobal->NeutralCountryIndex);
			pRulesGlobal->SpecialCountryIndex = HouseTypeClass::FindIndexByIdAndName(GameStrings::Special());
			//Debug::LogInfo("Finding Special Country Index[{}] ! ", pRulesGlobal->SpecialCountryIndex);
		}
	}

	return 0x0;
}

ASMJIT_PATCH(0x683E21, ScenarioClass_StartScenario_LogHouses, 0x5)
{
	Debug::LogInfo("Scenario Map Name [{}] ", SessionClass::IsCampaign() || ScenarioExtData::Instance()->OriginalFilename->empty() ? SessionClass::Instance->ScenarioFilename : ScenarioExtData::Instance()->OriginalFilename->c_str());

	if (auto pPlayerSide = SideClass::Array->get_or_default(ScenarioClass::Instance->PlayerSideIndex)) {
		if (auto pSideMouse = SideExtContainer::Instance.Find(pPlayerSide)->MouseShape) {
			GameDelete<true, true>(std::exchange(MouseClass::ShapeData(), pSideMouse));
		}
	}

	HouseClass::Array->for_each([](HouseClass* it)
 {
	 const auto pType = HouseTypeClass::Array->get_or_default(it->Type->ArrayIndex);
	 Debug::LogInfo("Player Name: {} IsCurrentPlayer: {}; ColorScheme: {}({}); ID: {}; HouseType: {}; Edge: {}; StartingAllies: {}; Startspot: {},{}; Visionary: {}; MapIsClear: {}; Money: {}",
	 it->PlainName ? it->PlainName : GameStrings::NoneStr(),
	 it->IsHumanPlayer,
	 ColorScheme::Array->Items[it->ColorSchemeIndex]->ID,
	 it->ColorSchemeIndex,
	 it->ArrayIndex,
	 pType ? pType->Name : GameStrings::NoneStr(),
	 (int)it->Edge,
	 (int)it->StartingAllies.data,
	 it->StartingCell.X,
	 it->StartingCell.Y,
	 (bool)it->Visionary,
	 it->MapIsClear,
	 it->Available_Money()
	 );
	});

	//Debug::LogInfo(GameStrings::Init_Commands);
	//CommandClass::InitCommand();

	return 0x0;
}

ASMJIT_PATCH(0x685005, Game_InitData_GlobalParticleSystem, 0x5)
{

	GET(ParticleSystemClass*, pMem, ESI);

	const auto pGlobalType = RulesExtData::Instance()->DefaultGlobalParticleInstance;

	if (!pGlobalType)
		Debug::FatalErrorAndExit("Cannot Find DefaultGlobalParticleInstance it will crash the game !");

	if (pGlobalType->Lifetime != -1)
		Debug::FatalErrorAndExit("DefaultGlobalParticleInstance[{}] Lifetime must be -1 , otherwise it will crash the game !", pGlobalType->ID);

	COMPILETIMEEVAL CoordStruct dummycoord { 2688  , 2688  , 0 };
	pMem->ParticleSystemClass::ParticleSystemClass(pGlobalType.Get(), dummycoord, nullptr, nullptr, CoordStruct::Empty, nullptr);
	R->EAX(pMem);
	return 0x685040;
}

ASMJIT_PATCH(0x687C16, INIClass_ReadScenario_ValidateThings, 6)
{
	// create an array of crew for faster lookup
	std::vector<InfantryTypeClass*> Crews(SideClass::Array->Count, nullptr);
	for (int i = 0; i < SideClass::Array->Count; ++i)
	{
		auto pExt = SideExtContainer::Instance.Find(SideClass::Array->Items[i]);

		Crews[i] = pExt->GetCrew();
		// remove all types that cannot paradrop
		if (pExt->ParaDropTypes.HasValue())
			Helpers::Alex::remove_non_paradroppables(pExt->ParaDropTypes, SideClass::Array->Items[i]->ID, "ParaDrop.Types");
	}

	FillSecrets(RulesClass::Instance->SecretInfantry);
	FillSecrets(RulesClass::Instance->SecretUnits);
	FillSecrets(RulesClass::Instance->SecretBuildings);

	auto pINI = CCINIClass::INI_Rules();
	INI_EX exINI(pINI);

	for (auto pItem : *TechnoTypeClass::Array)
	{
		const auto what = pItem->WhatAmI();
		const auto isFoot = what != AbstractType::BuildingType;
		auto pExt = TechnoTypeExtContainer::Instance.Find(pItem);
		const auto myClassName = pItem->GetThisClassName();
		bool WeederAndHarvesterWarning = false;

		pExt->UpdateAdditionalAttributes(pINI);

		if (pExt->Image_Yellow && pExt->Image_Yellow->WhatAmI() != what)
		{
			Debug::LogInfo("[{} - {}] has Image.ConditionYellow [{} - {}] but it different ClassType from it!",
				pItem->ID, myClassName, pExt->Image_Yellow->ID, pExt->Image_Yellow->GetThisClassName());
			pExt->Image_Yellow = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->Image_Red && pExt->Image_Red->WhatAmI() != what)
		{
			Debug::LogInfo("[{} - {}] has Image.ConditionRed [{} - {}] but it different ClassType from it!",
				pItem->ID, myClassName, pExt->Image_Red->ID, pExt->Image_Red->GetThisClassName());
			pExt->Image_Red = nullptr;
			Debug::RegisterParserError();
		}

		if (pItem->Strength <= 0)
		{
			const bool IsUpgradeBld = what == BuildingTypeClass::AbsID && !BuildingTypeExtContainer::Instance.Find((BuildingTypeClass*)pItem)->PowersUp_Buildings.empty();

			if ((!IsVanillaDummy(pItem->ID) || !pExt->IsDummy) && !IsUpgradeBld)
			{
				Debug::LogInfo("TechnoType[{} - {}] , registered with 0 strength"
					", this most likely because this technotype has no rules entry"
					" or it is suppose to be an dummy", pItem->ID, myClassName);

				Debug::RegisterParserError();

				pExt->IsDummy = true;
			}
		}

		if (pItem->Sight < 0)
		{
			Debug::LogInfo("TechnoType[{} - {}] , registered with less than 0 Sight , Fixing.",
			pItem->ID, myClassName);
			Debug::RegisterParserError();
			pItem->Sight = 0;
		}

		// if (pExt->AIIonCannonValue.HasValue() && pExt->AIIonCannonValue.size() < 3) {
		// 	for(size_t i = 0; i < (3 - pExt->AIIonCannonValue.size()); ++i)
		// 		pExt->AIIonCannonValue.push_back(0);
		// }

		if (pExt->Promote_Vet_Type && pExt->Promote_Vet_Type->Strength <= 0)
		{
			Debug::LogInfo("TechnoType[{} - {}] , registered PromoteVet[{}] with 0 strength , Fixing.",
				pItem->ID, myClassName, pExt->Promote_Vet_Type->ID);

			pExt->Promote_Vet_Type = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->Promote_Elite_Type && pExt->Promote_Elite_Type->Strength <= 0)
		{
			Debug::LogInfo("TechnoType[{} - {}] , registered PromoteElite[{}] with 0 strength , Fixing.",
				pItem->ID, myClassName, pExt->Promote_Elite_Type->ID);

			pExt->Promote_Elite_Type = nullptr;
			Debug::RegisterParserError();
		}

		if (pItem->DebrisTypes.Count > 0 && pItem->DebrisMaximums.Count < pItem->DebrisTypes.Count)
		{
			Debug::LogInfo("TechnoType[{} - {}] DebrisMaximums items count is less than"
			" DebrisTypes items count it will fail when the index counter reached DebrisMaximus items count"
			, pItem->ID, myClassName
			);
			Debug::RegisterParserError();
		}

		if (pExt->Fake_Of && pExt->Fake_Of->WhatAmI() != what)
		{
			Debug::LogInfo("[{} - {}] has FakeOf [{} - {}] but it different ClassType from it!"
				, pItem->ID, myClassName, pExt->Fake_Of->ID, pExt->Fake_Of->GetThisClassName());
			pExt->Fake_Of = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->RecuitedAs.isset())
		{
			if (pExt->RecuitedAs && pExt->RecuitedAs->WhatAmI() != what)
			{
				Debug::LogInfo("[{} - {}] has ClonedAs [{} - {}] but it different ClassType from it!"
					, pItem->ID, myClassName, pExt->RecuitedAs->ID, pExt->RecuitedAs->GetThisClassName());
				Debug::RegisterParserError();
			}
			else if (!pExt->RecuitedAs || pExt->RecuitedAs == pItem)
			{
				pExt->RecuitedAs.Reset();
				Debug::RegisterParserError();
			}
		}

		if (!pExt->TeamMember_ConsideredAs.empty())
		{
			for (auto& cc : pExt->TeamMember_ConsideredAs)
			{
				if (cc && cc->WhatAmI() != what)
				{
					Debug::LogInfo("[{} - {}] has TeamMember.ConsideredAs [{} - {}] but it different ClassType from it!", pItem->ID, myClassName, cc->ID, cc->GetThisClassName());
					Debug::RegisterParserError();
					cc = nullptr;
				}
			}
		}

		if (pExt->ClonedAs && pExt->ClonedAs->WhatAmI() != what)
		{
			Debug::LogInfo("[{} - {}] has ClonedAs [{} - {}] but it different ClassType from it!", pItem->ID, myClassName, pExt->ClonedAs->ID, pExt->ClonedAs->GetThisClassName());
			pExt->ClonedAs = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->AI_ClonedAs && pExt->AI_ClonedAs->WhatAmI() != what)
		{
			Debug::LogInfo("[{} - {}] has AI.ClonedAs [{} - {}] but it different ClassType from it!", pItem->ID, myClassName, pExt->AI_ClonedAs->ID, pExt->AI_ClonedAs->GetThisClassName());
			pExt->AI_ClonedAs = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->ReversedAs.Get(nullptr) && pExt->ReversedAs->WhatAmI() != what)
		{
			Debug::LogInfo("[{} - {}] has ReversedAs [{} - {}] but it different ClassType from it!", pItem->ID, myClassName, pExt->ReversedAs->ID, pExt->ReversedAs->GetThisClassName());
			pExt->ReversedAs.Reset();
			Debug::RegisterParserError();
		}

		if (isFoot && !pExt->IsDummy)
		{
			if (pItem->SpeedType == SpeedType::None)
			{
				Debug::LogInfo("[{} - {}]SpeedType None is invalid!", pItem->ID, myClassName);
				Debug::RegisterParserError();
			}

			if (pItem->MovementZone == MovementZone::None)
			{
				Debug::LogInfo("[{} - {}]MovementZone None is invalid!", pItem->ID, myClassName);
				Debug::RegisterParserError();
			}
		}

		if (pItem->Passengers > 0 && (int)pItem->SizeLimit < 1)
		{
			Debug::LogInfo("[{} - {}]Passengers={} and SizeLimit={}!",
				pItem->ID, myClassName, pItem->Passengers, (int)pItem->SizeLimit);
			Debug::RegisterParserError();
		}

		auto ValidateVoxelStruct = [pItem, pExt, myClassName](VoxelStruct* pVxl, const char* ident)
			{
				std::string iident(pItem->ID);
				iident += " - ";
				iident += myClassName;
				iident += " - ";
				iident += ident;

				RulesExtData::Owners[pVxl] = std::move(iident);

				if (!pVxl->VXL->HeaderData || !pVxl->VXL->TailerData)
				{
					Debug::FatalError("Techno[%s - %s] Has %s VXL but has no HeaderData or TailerData wtf ?", myClassName, pItem->ID, ident);
					GameDelete(pVxl);
					pVxl = nullptr;
				}

				if (auto pHVA = pVxl->HVA)
				{
					auto shadowIdx = pItem->ShadowIndex;
					auto layerCount = pHVA->LayerCount;

					if (shadowIdx >= layerCount)
					{
						Debug::LogInfo("ShadowIndex on [{}]'s {} image is {}, but the HVA only has {} sections.",
							pItem->ID, ident, shadowIdx, layerCount);
						Debug::RegisterParserError();
					}
				}
				else
				{
					Debug::FatalError("Techno[%s - %s] Has %s VXL but has no HVA wtf ?", myClassName, pItem->ID, ident);
				}
			};

		if (pItem->MainVoxel.VXL)
		{
			ValidateVoxelStruct(&pItem->MainVoxel, "");
		}

		if (pItem->TurretVoxel.VXL)
		{
			ValidateVoxelStruct(&pItem->TurretVoxel, "TurretVoxel");
		}

		if (pItem->BarrelVoxel.VXL)
		{
			ValidateVoxelStruct(&pItem->BarrelVoxel, "BarrelVoxel");

			//if (IS_SAME_STR_("XTITAN", pItem->ID))
			//{
			//	WatchPointer(&pItem->BarrelVoxel.VXL->HeaderData, "XTITAN_BarrelVoxel_HeaderData");
			//	WatchPointer(&pItem->BarrelVoxel.VXL->BodyData, "XTITAN_BarrelVoxel_BodyData");
			//	WatchPointer(&pItem->BarrelVoxel.VXL->TailerData, "XTITAN_BarrelVoxel_TailerData");
			//}
		}

		if (pExt->SpawnAltData.VXL)
		{
			ValidateVoxelStruct(&pExt->SpawnAltData, "SpawnAltData");
		}

		for (size_t ia = 0; ia < pExt->BarrelImageData.size(); ++ia)
		{
			if (pExt->BarrelImageData[ia].VXL)
			{
				std::string ident_a("BarrelImageData ");
				ident_a += std::to_string(ia);
				ValidateVoxelStruct(&pExt->BarrelImageData[ia], ident_a.c_str());
			}
		}

		for (size_t ib = 0; ib < pExt->TurretImageData.size(); ++ib)
		{
			if (pExt->TurretImageData[ib].VXL)
			{
				std::string ident_b("TurretImageData ");
				ident_b += std::to_string(ib);
				ValidateVoxelStruct(&pExt->TurretImageData[ib], ident_b.c_str());
			}
		}

		if (pItem->PoweredUnit && !pExt->PoweredBy.empty())
		{
			Debug::LogInfo("[{} - {}] uses both PoweredUnit=yes and PoweredBy=!", pItem->ID, myClassName);
			pItem->PoweredUnit = false;
			Debug::RegisterParserError();
		}

		if (auto const pPowersUnit = pItem->PowersUnit)
		{
			if (!TechnoTypeExtContainer::Instance.Find(pPowersUnit)->PoweredBy.empty())
			{
				Debug::LogInfo("[{}]PowersUnit={}, but [{}] uses PoweredBy=!",
					pItem->ID, pPowersUnit->ID, pPowersUnit->ID);
				pItem->PowersUnit = nullptr;
				Debug::RegisterParserError();
			}
		}

		// if empty, set survivor pilots to the corresponding side's Crew
		{
			const size_t count = MinImpl(pExt->Survivors_Pilots.size(), Crews.size());

			for (size_t j = 0; j < count; ++j)
			{
				if (!pExt->Survivors_Pilots[j])
				{
					pExt->Survivors_Pilots[j] = Crews[j];
				}
			}
		}

		for (int k = int(pExt->ClonedAt.size()) - 1; k >= 0; --k)
		{
			auto const pCloner = pExt->ClonedAt[k];
			if (pCloner->Factory != AbstractType::None)
			{
				pExt->ClonedAt.erase(pExt->ClonedAt.begin() + k);
				Debug::LogInfo("[{}]ClonedAt includes {}, but {} has Factory= settings. "
					"This combination is not supported.(Protip: Factory= is "
					"not what controls unit exit behaviour, WeaponsFactory= "
					"and GDI/Nod/YuriBarracks= is.)", pItem->ID, pCloner->ID,
					pCloner->ID);
				Debug::RegisterParserError();
			}
		}

		if (!pExt->Harvester_Counted.isset() && pItem->Enslaves)
		{
			pExt->Harvester_Counted = true;
		}

		if (pExt->Spawn_LimitedExtraRange)
			pExt->CalculateSpawnerRange();

		if (isFoot)
		{
			if (what == UnitTypeClass::AbsID)
			{
				const auto pUnit = (UnitTypeClass*)pItem;

				if (!pExt->Harvester_Counted.isset() && pUnit->Harvester)
				{
					pExt->Harvester_Counted = true;
				}

				if (pUnit->Harvester && pUnit->Weeder)
				{
					WeederAndHarvesterWarning = true;
					pUnit->Weeder = false;
				}
			}
		}
		else
		{
			auto const pBType = (BuildingTypeClass*)pItem;

			if (pBType->Refinery && pBType->Weeder)
			{
				WeederAndHarvesterWarning = true;
				pBType->Weeder = false;
			}

			auto const pBExt = BuildingTypeExtContainer::Instance.Find(pBType);
			//pBExt->IsPrism = RulesClass::Instance->PrismType == pBType;

			if (pBExt->CloningFacility && pBType->Factory != AbstractType::None)
			{
				pBExt->CloningFacility = false;
				Debug::LogInfo("[{}] cannot have both CloningFacility= and Factory=.",
				pItem->ID);
			}

			const auto  techLevel = pItem->TechLevel;

			if (!(techLevel < 0 || techLevel > RulesClass::Instance->TechLevel))
			{
				if (pBType->BuildCat == BuildCat::DontCare)
				{
					pBType->BuildCat = ((pBType->SuperWeapon != -1) || pBType->IsBaseDefense || pBType->Wall)
						? BuildCat::Combat : BuildCat::Infrastructure;

					auto const catName = (pBType->BuildCat == BuildCat::Combat)
						? "Combat" : "Infrastructure";

					Debug::LogInfo("Building Type [{}] does not have a valid BuildCat set!"
							   "It was reset to {}, but you should really specify it "
							   "explicitly.", pBType->ID, catName);
					Debug::RegisterParserError();
				}
			}
		}

		if (WeederAndHarvesterWarning)
		{
			Debug::LogInfo("Please choose between Weeder or (Refinery / Harvester) for [{} - {}] both cant be used at same time", pItem->ID, myClassName);
			Debug::RegisterParserError();
		}
	}

	for (auto pItem : *WeaponTypeClass::Array)
	{
		if (!pItem->Warhead)
		{
			Debug::LogInfo("Weapon[{}] has no Warhead", pItem->ID);
			Debug::RegisterParserError();
		}

		if (!pItem->Projectile)
		{
			Debug::LogInfo("Weapon[{}] has no Projectile", pItem->ID);
			Debug::RegisterParserError();
		}

		const auto pExt = WeaponTypeExtContainer::Instance.Find(pItem);

		if ((pItem->IsRailgun || pExt->IsDetachedRailgun || pItem->UseSparkParticles || pItem->UseFireParticles)
				&& !pItem->AttachedParticleSystem)
		{

			Debug::LogInfo("Weapon[{}] is an Railgun/Detached Railgun/UseSparkParticles/UseFireParticles but it missing AttachedParticleSystem", pItem->ID);
			Debug::RegisterParserError();

			pItem->IsRailgun = false;
			pExt->IsDetachedRailgun = false;
			pItem->UseSparkParticles = false;
			pItem->UseFireParticles = false;
		}
	}

	for (auto& pConst : RulesClass::Instance->BuildConst)
	{
		if (!pConst->AIBuildThis)
		{
			Debug::LogInfo("[AI]BuildConst= includes [{}], which doesn't have "
				"AIBuildThis=yes!", pConst->ID);
		}
	}

	//if (OverlayTypeClass::Array->Count > 255) {
	//	Debug::LogInfo("Reaching over 255 OverlayTypes!.");
	//	Debug::RegisterParserError();
	//}

	for (auto pWH : *WarheadTypeClass::Array)
	{
		auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
		{
			const size_t versesSize = pWHExt->Verses.size();

			if (versesSize < ArmorTypeClass::Array.size())
			{
				Debug::LogInfo("Inconsistent verses size of [{} - {}] Warhead with ArmorType Array[{}]", pWH->ID, versesSize, ArmorTypeClass::Array.size());
				Debug::RegisterParserError();
			}

			if (pWHExt->Crit_AffectAbovePercent > pWHExt->Crit_AffectBelowPercent)
				Debug::Log("[Developer warning][%s] Crit.AffectsAbovePercent is bigger than Crit.AffectsBelowPercent, crit will never activate!\n", pWH->ID);

		}
	}

	for (size_t i = 1; i < ShieldTypeClass::Array.size(); ++i)
	{
		if (auto pShield = ShieldTypeClass::Array[i].get())
		{
			if (pShield->Strength <= 0)
			{
				Debug::LogInfo("[{}]ShieldType is not valid because Strength is 0.", pShield->Name.data());
				Debug::RegisterParserError();
			}
		}
	}

	for (auto pBullet : *BulletTypeClass::Array)
	{

		if (pBullet->Voxel)
		{
			if (pBullet->MainVoxel.VXL)
			{
				std::string iident(pBullet->ID);
				iident += " - ";
				iident += "BulletTypeClass";

				RulesExtData::Owners[&pBullet->MainVoxel] = std::move(iident);

				if (!pBullet->MainVoxel.VXL->HeaderData || !pBullet->MainVoxel.VXL->TailerData)
				{
					Debug::FatalError("Bullet[%s] Has VXL but has no HeaderData or TailerData wtf ?", pBullet->ID);
				}

				if (!pBullet->MainVoxel.HVA)
				{
					Debug::LogInfo("Bullet[{}] Has VXL but has no HVA wtf ?", pBullet->ID);
					Debug::RegisterParserError();
					GameDelete(pBullet->MainVoxel.VXL);
					pBullet->Voxel = false;
				}
			}
			else
			{
				Debug::LogInfo("Bullet[{}] Has no VXL but set as Voxel wtf ?", pBullet->ID);
				Debug::RegisterParserError();
				pBullet->Voxel = false;
			}
		}

		if (!pBullet->Voxel && !pBullet->GetImage())
		{
			Debug::LogInfo("Bullet[{}] has no valid SHP !", pBullet->ID);
			Debug::RegisterParserError();
		}

		//auto pExt = BulletTypeExtContainer::Instance.Find(pBullet);

		//if (pExt->AttachedSystem && pExt->AttachedSystem->BehavesLike != ParticleSystemTypeBehavesLike::Smoke) {
		//	Debug::LogInfo("Bullet[{}] With AttachedSystem[{}] is not BehavesLike=Smoke!", pBullet->ID, pExt->AttachedSystem->ID);
		//	Debug::RegisterParserError();
		//}
	}

	for (auto pVxlAnim : *VoxelAnimTypeClass::Array)
	{
		if (pVxlAnim->MainVoxel.VXL)
		{
			std::string iident(pVxlAnim->ID);
			iident += " - ";
			iident += "VoxelAnimTypeClass";

			RulesExtData::Owners[&pVxlAnim->MainVoxel] = std::move(iident);

			if (!pVxlAnim->MainVoxel.VXL->HeaderData || !pVxlAnim->MainVoxel.VXL->TailerData)
			{
				Debug::LogInfo("VoxelAnim[{}] Has VXL but has no HeaderData or TailerData wtf ?", pVxlAnim->ID);
				Debug::RegisterParserError();
				GameDelete(pVxlAnim->MainVoxel.VXL);
				continue;
			}

			if (!pVxlAnim->MainVoxel.HVA)
			{
				Debug::LogInfo("VoxelAnim[{}] Has VXL but has no HVA wtf ?", pVxlAnim->ID);
				Debug::RegisterParserError();
			}
		}
	}

	for (auto pHouse : *HouseTypeClass::Array)
	{
		auto pExt = HouseTypeExtContainer::Instance.Find(pHouse);

		// remove all types that cannot paradrop

		Helpers::Alex::remove_non_paradroppables(pExt->ParaDropTypes, pHouse->ID, "ParaDrop.Types");

		if (pExt->StartInMultiplayer_Types.HasValue())
			Helpers::Alex::remove_non_paradroppables(pExt->StartInMultiplayer_Types, pHouse->ID, "StartInMultiplayer.Types");
	}

	for (auto pSuper : *SuperWeaponTypeClass::Array)
	{
		const auto pSuperExt = SWTypeExtContainer::Instance.Find(pSuper);
		Nullable<MouseCursor> _Temp_MouseCursor {};

		{
			//if (auto pNew = pSuperExt->GetNewSWType()) {
			//	pNew->ValidateData(pSuperExt);
			//}
			//_Temp_MouseCursor.Read(exINI, pSuper->ID, "Cursor");
			//if (_Temp_MouseCursor.isset()) {
			//	std::string _name = pSuper->ID;
			//	_name += "Cursor";
			//
			//	CursorTypeClass::AllocateWithDefault(_name.c_str(), _Temp_MouseCursor);
			//}

			for (auto& pTech : pSuperExt->Aux_Techno) {
				TechnoTypeExtContainer::Instance.Find(pTech)->Linked_SW.emplace(pSuper);
			}

			fast_remove_if(pSuperExt->SW_AuxBuildings, [](BuildingTypeClass* pItem) { return !pItem; });
			fast_remove_if(pSuperExt->SW_NegBuildings, [](BuildingTypeClass* pItem) { return !pItem; });

			Helpers::Alex::remove_non_paradroppables(pSuperExt->DropPod_Types, pSuper->ID, "DropPod.Types");

			for (auto& para : pSuperExt->ParaDropDatas) {
				for (auto& pVec : para.second) {
					Helpers::Alex::remove_non_paradroppables(pVec.Types, pSuper->ID, "ParaDrop.Types");
				}
			}
		}
	}

	for (auto pAnim : *AnimTypeClass::Array)
	{

		if (!pAnim->ID || !strlen(pAnim->ID))
			Debug::FatalError("Empty name Anim [%x]!", pAnim);

		if (!pAnim->GetImage())
		{
			Debug::LogInfo("Anim[{}] Has no proper Image!", pAnim->ID);
			Debug::RegisterParserError();
		}
	}

	for (auto pTerrain : *TerrainTypeClass::Array)
	{

		if (!pTerrain->ID || !strlen(pTerrain->ID))
			Debug::FatalError("Empty name Terrain [%x]!", pTerrain);

		if (!pTerrain->GetImage())
		{
			Debug::LogInfo("Terrain [{} - {}] has no Image!", pTerrain->ID, (void*)pTerrain);
			Debug::RegisterParserError();
		}
	}

	if (Phobos::Otamaa::StrictParser && Phobos::Otamaa::ParserErrorDetected)
	{
		Debug::FatalErrorAndExit(
			"One or more errors were detected while parsing the INI files.\r"
			"Please review the contents of the debug log and correct them.");
	}

	for (auto& pArmor : ArmorTypeClass::Array)
	{
		pArmor->FreeTags();
	}

	return 0x0;
}

ASMJIT_PATCH(0x547043, IsometricTileTypeClass_ReadFromFile, 0x6)
{
	GET(int, FileSize, EBX);
	GET(IsometricTileTypeClass*, pTileType, ESI);
	LEA_STACK(CCFileClass*, pFile, 0xC);

	if (FileSize == 0)
	{
		auto what = (pTileType->ID + strlen(pTileType->ID) + 1 - pTileType->ID);
		auto pFileName = pFile->Filename;

		if (what > 9)
		{
			Debug::FatalErrorAndExit("Maximum allowed length for tile names, excluding the extension, is 9 characters.\n"
					"The tileset using filename '%s - %s' exceeds this limit - the game cannot proceed.", pTileType->ID, pFileName);
		}

		Debug::FatalErrorAndExit("The tileset '%s - %s' contains a file that could not be loaded for some reason - make sure the file exists."
			, pTileType->ID, pFileName);
	}

	return 0;
}

ASMJIT_PATCH(0x41088D, AbstractTypeClass_CTOR_IDTooLong, 0x6)
{
	GET(const char*, ID, EAX);

	if (strlen(ID) >= 25)
		Debug::FatalErrorAndExit("Tried to create a type with ID '%s' which is longer than the maximum length of 24 .", ID);

	return 0;
}

ASMJIT_PATCH(0x7272B5, TriggerTypeClass_LoadFromINI_House, 6)
{
	GET(int const, index, EAX);
	GET(TriggerTypeClass* const, pTrig, EBP);
	GET(const char*, pHouse, ESI);

	if (index < 0)
	{
		Debug::FatalError("TriggerType '%s' refers to a house named '%s', which does not exist. In case no house is needed, use '<none>' explicitly.", pTrig->ID, pHouse);
		R->EDX<HouseTypeClass*>(nullptr);
	}
	else
	{
		R->EDX<HouseTypeClass*>(HouseTypeClass::Array->Items[index]);
	}

	return 0x7272C1;
}

ASMJIT_PATCH(0x72749B, TriggerTypeClass_LoadFromINI_Buffers, 0x8)
{
	GET(TriggerTypeClass*, pThis, EBP);
	GET_STACK(CCINIClass*, pINI, 0x248);

	if (pINI->ReadString(GameStrings::Events(), pThis->ID, Phobos::readDefval, Phobos::readBuffer, Phobos::readLength))
	{
		if (auto v16 = CRT::atoi(CRT::strtok(Phobos::readBuffer, Phobos::readDelims)))
		{
			do
			{
				auto pEvent = GameCreate<TEventClass>();
				pEvent->LoadFromINI();
				--v16;
				pEvent->NextEvent = pThis->FirstEvent;
				pThis->FirstEvent = pEvent;
			}
			while (v16);
		}
	}

	if (pINI->ReadString(GameStrings::Actions(), pThis->ID, Phobos::readDefval, Phobos::readBuffer, Phobos::readLength))
	{
		pThis->FirstAction = nullptr;
		TActionClass* pTemp = nullptr;

		if (auto v16 = CRT::atoi(CRT::strtok(Phobos::readBuffer, Phobos::readDelims)))
		{
			do
			{
				auto pAction = GameCreate<TActionClass>();
				pAction->LoadFromINI();
				if (pThis->FirstAction)
				{
					pTemp->NextAction = pAction;
				}
				else
				{
					pThis->FirstAction = pAction;
				}
				--v16;
				pTemp = pAction;
			}
			while (v16);
		}
	}

	return 0x7275A5;
}

ASMJIT_PATCH(0x4750D0, CCINIClass_ReadHouseTypeList_Buffers, 0xA)
{
	GET(CCINIClass*, pThis, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(DWORD, _default, 0xC);

	if (pThis->ReadString(pSection, pKey, Phobos::readDefval, Phobos::readBuffer, Phobos::readLength))
	{
		_default = 0u;
		for (auto i = strtok(Phobos::readBuffer, Phobos::readDelims); i; i = strtok(0, Phobos::readDelims))
		{
			_default |= HouseTypeClass::FindIndexOfNameShiftToTheRightOnce(i);
		}
	}

	R->EAX(_default);
	return 0x475140;
}

ASMJIT_PATCH(0x475260, CCINIClass_ReadAlly_Buffers, 0xA)
{
	GET(CCINIClass*, pThis, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(DWORD, _default, 0xC);

	if (pThis->ReadString(pSection, pKey, Phobos::readDefval, Phobos::readBuffer, Phobos::readLength))
	{
		_default = 0u;
		for (auto i = strtok(Phobos::readBuffer, Phobos::readDelims); i; i = strtok(0, Phobos::readDelims))
		{
			_default |= (1 << HouseClass::FindIndexByName(i));
		}
	}

	R->EAX(_default);
	return 0x4752D9;
}

void FakeRulesClass::_ReadSpecialWeapons(CCINIClass* pINI)
{
	const char* section = GameStrings::SpecialWeapons;

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	// WarheadTypeClass* pointers
	detail::read(this->NukeWarhead, exINI, section, "NukeWarhead", true);
	detail::read(this->MutateWarhead, exINI, section, "MutateWarhead", true);
	detail::read(this->MutateExplosionWarhead, exINI, section, "MutateExplosionWarhead", true);
	detail::read(this->EMPulseWarhead, exINI, section, "EMPulseWarhead", true);

	// BulletTypeClass* pointers
	detail::read(this->NukeProjectile, exINI, section, "NukeProjectile", true);
	detail::read(this->NukeDown, exINI, section, "NukeDown", true);
	detail::read(this->EMPulseProjectile, exINI, section, "EMPulseProjectile", true);

	//Ext
	auto pData = RulesExtData::Instance();

	pData->HunterSeekerBuildings.Read(exINI, GameStrings::SpecialWeapons(), "HSBuilding");
	pData->NukeWarheadName.Read(exINI.GetINI(), GameStrings::SpecialWeapons(), "NukeWarhead");
}

DEFINE_FUNCTION_JUMP(LJMP, 0x668FB0, FakeRulesClass::_ReadSpecialWeapons)

void FakeRulesClass::_ReadElevationModel(CCINIClass* pINI)
{
	const char* section = "ElevationModel";

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);
	detail::read(this->ElevationIncrement, exINI, section, "ElevationIncrement", true);
	detail::read(this->ElevationIncrementBonus, exINI, section, "ElevationIncrementBonus", true);
	detail::read(this->ElevationBonusCap, exINI, section, "ElevationBonusCap", true);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x66D150, FakeRulesClass::_ReadElevationModel)

void FakeRulesClass::_ReadWallModel(CCINIClass* pINI)
{
	const char* section = "WallModel";

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);
	detail::read(this->AlliedWallTransparency, exINI, section, "AlliedWallTransparency", true);
	detail::read(this->WallPenetratorThreshold, exINI, section, "WallPenetratorThreshold", true);
}

DEFINE_FUNCTION_JUMP(LJMP, 0x66D1F0, FakeRulesClass::_ReadWallModel)

void FakeRulesClass::_ReadLandTypes(CCINIClass* pINI)
{
	static constexpr const char* LandTypeSections[] = {
	   "Clear",    // index 0  — 0x839D68
	   "Road",     // index 1  — 0x839D6C
	   "Water",    // index 2  — 0x839D70
	   "Rock",     // index 3  — 0x839D74
	   "Wall",     // index 4  — 0x839D78
	   "Tiberium", // index 5  — 0x839D7C
	   "Beach",    // index 6  — 0x839D80
	   "Rough",    // index 7  — 0x839D84
	   "Ice",      // index 8  — 0x839D88
	   "Railroad", // index 9  — 0x839D8C
	   "Tunnel",   // index 10 — 0x839D90
	   "Weeds",    // index 11 — 0x839D94
	};

	auto ReadSpeedCost = [](INI_EX& exINI, const char* section, const char* key) {
		double _buffer = 1.0;
		detail::read<double>(_buffer, exINI, section, key);
		return float((_buffer >= 1.0) ? 1.0 : _buffer);
	};

	for (int i = 0; i < GroundType::Array.size(); ++i) {
		const char* section = LandTypeSections[i];

		if (!pINI->GetSection(section))
			continue;

		INI_EX exINI(pINI);
		RulesExtData::Instance()->LandTypeConfigExts[i].Bounce_Elasticity.
			Read(exINI, section, "Bounce.Elasticity");

		auto& grnd = GroundType::Array[i];
		grnd.Cost[SpeedType::Foot] = ReadSpeedCost(exINI, section, "Foot");
		grnd.Cost[SpeedType::Track] = ReadSpeedCost(exINI, section, "Track");
		grnd.Cost[SpeedType::Wheel] = ReadSpeedCost(exINI, section, "Wheel");
		grnd.Cost[SpeedType::Hover] = ReadSpeedCost(exINI, section, "Hover");
		grnd.Cost[SpeedType::Float] = ReadSpeedCost(exINI, section, "Float");
		grnd.Cost[SpeedType::Amphibious] = ReadSpeedCost(exINI, section, "Amphibious");
		grnd.Cost[SpeedType::FloatBeach] = ReadSpeedCost(exINI, section, "FloatBeach");

		detail::read<bool>(grnd.Build, exINI, section, "Buildable");
		Debug::LogInfo("Reading LandTypeData of [{} - {}]", section, i);
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x674000, FakeRulesClass::_ReadLandTypes)

void FakeRulesClass::_ReadIQ(CCINIClass* pINI)
{
	static constexpr const char* section = "IQ";

	if (!pINI->GetSection(section))
		return;
	
	INI_EX exINI(pINI);

	detail::read(this->IQData.MaxLevels, exINI, section, "MaxIQLevels");
	detail::read(this->IQData.SuperWeapons, exINI, section, "SuperWeapons");
	detail::read(this->IQData.Production, exINI, section, "Production");
	detail::read(this->IQData.GuardArea, exINI, section, "GuardArea");
	detail::read(this->IQData.RepairSell, exINI, section, "RepairSell");
	detail::read(this->IQData.AutoCrush, exINI, section, "AutoCrush");   // field IQCrush, key "AutoCrush"
	detail::read(this->IQData.Scatter, exINI, section, "Scatter");
	detail::read(this->IQData.ContentScan, exINI, section, "ContentScan");
	detail::read(this->IQData.Aircraft, exINI, section, "Aircraft");
	detail::read(this->IQData.Harvester, exINI, section, "Harvester");
	detail::read(this->IQData.SellBack, exINI, section, "SellBack");

	//Ext
	auto pData = RulesExtData::Instance();

	pData->TogglePowerIQ.Read(exINI, section, "TogglePower");
}

DEFINE_FUNCTION_JUMP(LJMP, 0x674240, FakeRulesClass::_ReadIQ)

void FakeRulesClass::_ReadDifficulty(CCINIClass* pINI)
{
	auto DiffGet = [](DifficultyStruct& value, CCINIClass* pINI, const char* section)
		{
			if (pINI->GetSection(section)) {
				INI_EX exINI(pINI);

				// Doubles — hardcoded defaults from vanilla
				detail::read(value.Firepower, exINI, section, "FirePower");
				detail::read(value.GroundSpeed, exINI, section, "Groundspeed");
				detail::read(value.AirSpeed, exINI, section, "Airspeed");
				detail::read(value.Armor, exINI, section, "Armor");
				detail::read(value.ROF, exINI, section, "ROF");
				detail::read(value.Cost, exINI, section, "Cost");
				detail::read(value.RepairDelay, exINI, section, "RepairDelay");
				detail::read(value.BuildDelay, exINI, section, "BuildDelay");
				detail::read(value.BuildTime, exINI, section, "BuildTime");

				// Bools — hardcoded defaults (0 = false, 1 = true)
				detail::read(value.BuildSlowdown, exINI, section, "BuildSlowdown", false);
				detail::read(value.DestroyWalls, exINI, section, "DestroyWalls", true);
				detail::read(value.ContentScan, exINI, section, "ContentScan", false);
			}
		};

	DiffGet(this->DifficultyConfigs[ParsedDifficulty::Easy], pINI, "Easy");
	DiffGet(this->DifficultyConfigs[ParsedDifficulty::Normal], pINI, "Normal");
	DiffGet(this->DifficultyConfigs[ParsedDifficulty::Hard], pINI, "Difficult");
}

DEFINE_FUNCTION_JUMP(LJMP, 0x674500, FakeRulesClass::_ReadDifficulty)

void FakeRulesClass::_ReadAudioVisual(CCINIClass* pINI)
{
	const char* section = GameStrings::AudioVisual;

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	// -------------------------------------------------------------------------
	// Scalar ints
	// -------------------------------------------------------------------------
	detail::read(this->DetailMinFrameRateNormal, exINI, section, "DetailMinFrameRateNormal");
	detail::read(this->DetailMinFrameRateMovie, exINI, section, "DetailMinFrameRateMovie");
	detail::read(this->DetailBufferZoneWidth, exINI, section, "DetailBufferZoneWidth");
	detail::read(this->PoseDir, exINI, section, "PoseDir");
	detail::read(this->WaypointAnimationSpeed, exINI, section, "WaypointAnimationSpeed");
	detail::read(this->SpyPlaneCameraFrames, exINI, section, "SpyPlaneCameraFrames");
	detail::read(this->EliteFlashTimer, exINI, section, "EliteFlashTimer");
	detail::read(this->ShakeScreen, exINI, section, "ShakeScreen");
	detail::read(this->Gravity, exINI, section, "Gravity");
	detail::read(this->IceSolidifyFrameTime, exINI, section, "IceSolidifyFrameTime");
	detail::read(this->OreTwinkleChance, exINI, section, "OreTwinkleChance");
	detail::read(this->LaserTargetColor, exINI, section, "LaserTargetColor");
	detail::read(this->IronCurtainColor, exINI, section, "IronCurtainColor");
	detail::read(this->BerserkColor, exINI, section, "BerserkColor");
	detail::read(this->ForceShieldColor, exINI, section, "ForceShieldColor");

	// DeployDir — vanilla stores as (INI value << 5); reads back as (stored >> 5) for default
	// VERIFY: confirm the shift factor of 5 (i.e. stored value = INI_value * 32)
	{
		int deployDirRaw = this->DeployDir >> 5;
		detail::read(deployDirRaw, exINI, section, "DeployDir");
		this->DeployDir = deployDirRaw * 32;
	}

	// -------------------------------------------------------------------------
	// Scalar doubles
	// -------------------------------------------------------------------------
	detail::read(this->ScrollMultiplier, exINI, section, "ScrollMultiplier");
	detail::read(this->ConditionRed, exINI, section, "ConditionRed");
	detail::read(this->ConditionYellow, exINI, section, "ConditionYellow");
	detail::read(this->IdleActionFrequency, exINI, section, "IdleActionFrequency"); // field RandomAnimateTime, key "IdleActionFrequency"
	detail::read(this->MessageDelay, exINI, section, "MessageDelay");
	detail::read(this->MovieTime, exINI, section, "MovieTime");
	detail::read(this->SavourDelay, exINI, section, "SavourDelay");
	detail::read(this->ShroudRate, exINI, section, "ShroudRate");
	detail::read(this->FogRate, exINI, section, "FogRate");
	detail::read(this->VeinGrowthRate, exINI, section, "VeinGrowthRate");
	detail::read(this->IceGrowthRate, exINI, section, "IceGrowthRate");
	detail::read(this->AmbientChangeRate, exINI, section, "AmbientChangeRate");
	detail::read(this->AmbientChangeStep, exINI, section, "AmbientChangeStep");
	detail::read(this->SpeakDelay, exINI, section, "SpeakDelay");
	detail::read(this->TimerWarning, exINI, section, "TimerWarning");
	detail::read(this->DirectRockingCoefficient, exINI, section, "DirectRockingCoefficient");
	detail::read(this->FallBackCoefficient, exINI, section, "FallBackCoefficient");

	// ExtraUnitLight / ExtraInfantryLight / ExtraAircraftLight
	// Vanilla: default passed as (field / 1000.0), stored as (result * 1000.0)
	// VERIFY: confirm integer field scaled by 1000 (i.e. stored = INI_double * 1000, not a float field)
	{
		double v = this->ExtraUnitLight / 1000.0;
		detail::read(v, exINI, section, "ExtraUnitLight");
		this->ExtraUnitLight = float(v * 1000.0);
	}
	{
		double v = this->ExtraInfantryLight / 1000.0;
		detail::read(v, exINI, section, "ExtraInfantryLight");
		this->ExtraInfantryLight = float(v * 1000.0);
	}
	{
		double v = this->ExtraAircraftLight / 1000.0;
		detail::read(v, exINI, section, "ExtraAircraftLight");
		this->ExtraAircraftLight = float(v * 1000.0);
	}

	// -------------------------------------------------------------------------
	// Bool fields
	// -------------------------------------------------------------------------
	detail::read(this->ShroudGrow, exINI, section, "ShroudGrow");
	detail::read(this->AllyReveal, exINI, section, "AllyReveal");
	detail::read(this->EnemyHealth, exINI, section, "EnemyHealth");  // field IsHealthBar, key "EnemyHealth"
	detail::read(this->NamedCivilians, exINI, section, "NamedCivilians");

	// -------------------------------------------------------------------------
	// Lepton field
	// VERIFY: confirm detail::read<int> handles lepton scaling same as CCINIClass::Get_Lepton
	// -------------------------------------------------------------------------
	detail::read(this->DropZoneRadius, exINI, section, "DropZoneRadius");

	// -------------------------------------------------------------------------
	// ConditionGreen — hardcoded to 1.0 in vanilla, not read from INI
	// -------------------------------------------------------------------------
	this->ConditionGreen = 1.0; // 0x3FF0000000000000 IEEE 754 double

	// -------------------------------------------------------------------------
	// RGB color fields
	// VERIFY: confirm detail::read handles CCINIClass::Get_RGB equivalent for ColorStruct
	// -------------------------------------------------------------------------
	detail::read(this->LocalRadarColor, exINI, section, "LocalRadarColor");
	detail::read(this->LineTrailColorOverride, exINI, section, "LineTrailColorOverride");
	detail::read(this->ChronoBeamColor, exINI, section, "ChronoBeamColor");
	detail::read(this->MagnaBeamColor, exINI, section, "MagnaBeamColor");

	// -------------------------------------------------------------------------
	// AnimTypeClass* pointers — Find_Or_Make → pass true
	// -------------------------------------------------------------------------
	detail::read(this->DropPodPuff, exINI, section, "DropPodPuff", true);
	detail::read(this->VeinAttack, exINI, section, "VeinAttack", true);
	detail::read(this->Dig, exINI, section, "Dig", true);
	detail::read(this->AtmosphereEntry, exINI, section, "AtmosphereEntry", true);
	detail::read(this->SmallFire, exINI, section, "SmallFire", true);
	detail::read(this->LargeFire, exINI, section, "LargeFire", true);

	// SUSPECT: both Smoke1 and Smoke2 use the same INI key "Smoke" — likely vanilla reads same value twice
	// VERIFY: confirm this is intentional (two fields from one key) or if Smoke2 should use a different key
	detail::read(this->Smoke, exINI, section, "Smoke", true);
	detail::read(this->Smoke_, exINI, section, "Smoke", true);

	// -------------------------------------------------------------------------
	// TypeList<AnimTypeClass*>
	// -------------------------------------------------------------------------
	detail::ParseVector(this->TreeFire, exINI, section, "TreeFire");
	detail::ParseVector(this->OnFire, exINI, section, "OnFire");

	// -------------------------------------------------------------------------
	// TypeList<int> — sound ID lists
	// -------------------------------------------------------------------------
	if (exINI.ReadString(section, "CreditTicks"))
		detail::parse_indexes<VocClass>(this->CreditTicks, exINI, section, "CreditTicks");

	if (exINI.ReadString(section, "LightningSounds"))
		detail::parse_indexes<VocClass>(this->LightningSounds, exINI, section, "LightningSounds");

	if (exINI.ReadString(section, "IceCrackSounds"))
		detail::parse_indexes<VocClass>(this->IceCrackSounds, exINI, section, "IceCrackSounds");

	// -------------------------------------------------------------------------
	// Sound fields (VocType / int ID via VocClass::Get_ID_From_Name)
	// Vanilla falls back to existing value if key missing OR if name resolves to -1.
	// VERIFY: confirm detail::read<VocType> preserves the -1 fallback behaviour.
	// -------------------------------------------------------------------------
	detail::getindex<VocClass>(this->DigSound, exINI, section, "DigSound");
	detail::getindex<VocClass>(this->GUIMainButtonSound, exINI, section, "GUIMainButtonSound");
	detail::getindex<VocClass>(this->GUIBuildSound, exINI, section, "GUIBuildSound");
	detail::getindex<VocClass>(this->GUITabSound, exINI, section, "GUITabSound");
	detail::getindex<VocClass>(this->GUIOpenSound, exINI, section, "GUIOpenSound");
	detail::getindex<VocClass>(this->GUICloseSound, exINI, section, "GUICloseSound");
	detail::getindex<VocClass>(this->GUIMoveOutSound, exINI, section, "GUIMoveOutSound");
	detail::getindex<VocClass>(this->GUIMoveInSound, exINI, section, "GUIMoveInSound");
	detail::getindex<VocClass>(this->GUIComboOpenSound, exINI, section, "GUIComboOpenSound");
	detail::getindex<VocClass>(this->GUIComboCloseSound, exINI, section, "GUIComboCloseSound");
	detail::getindex<VocClass>(this->GUICheckboxSound, exINI, section, "GUICheckboxSound");
	detail::getindex<VocClass>(this->ScoreAnimSound, exINI, section, "ScoreAnimSound");
	detail::getindex<VocClass>(this->CheerSound, exINI, section, "CheerSound");
	detail::getindex<VocClass>(this->DefaultChronoSound, exINI, section, "DefaultChronoSound");
	detail::getindex<VocClass>(this->StartPlanningModeSound, exINI, section, "StartPlanningModeSound");
	detail::getindex<VocClass>(this->EndPlanningModeSound, exINI, section, "EndPlanningModeSound");
	detail::getindex<VocClass>(this->CrateMoneySound, exINI, section, "CrateMoneySound");
	detail::getindex<VocClass>(this->CrateRevealSound, exINI, section, "CrateRevealSound");
	detail::getindex<VocClass>(this->CrateFireSound, exINI, section, "CrateFireSound");
	detail::getindex<VocClass>(this->CrateArmourSound, exINI, section, "CrateArmourSound");
	detail::getindex<VocClass>(this->CrateSpeedSound, exINI, section, "CrateSpeedSound");
	detail::getindex<VocClass>(this->CrateUnitSound, exINI, section, "CrateUnitSound");
	detail::getindex<VocClass>(this->CratePromoteSound, exINI, section, "CratePromoteSound");
	detail::getindex<VocClass>(this->ImpactWaterSound, exINI, section, "ImpactWaterSound");
	detail::getindex<VocClass>(this->ImpactLandSound, exINI, section, "ImpactLandSound");
	detail::getindex<VocClass>(this->SinkingSound, exINI, section, "SinkingSound");
	detail::getindex<VocClass>(this->ChronoInSound, exINI, section, "ChronoInSound");
	detail::getindex<VocClass>(this->ChronoOutSound, exINI, section, "ChronoOutSound");
	detail::getindex<VocClass>(this->BombTickingSound, exINI, section, "BombTickingSound");
	detail::getindex<VocClass>(this->BombAttachSound, exINI, section, "BombAttachSound");
	detail::getindex<VocClass>(this->YuriMindControlSound, exINI, section, "YuriMindControlSound");
	detail::getindex<VocClass>(this->AddPlanningModeCommandSound, exINI, section, "AddPlanningModeCommandSound");
	detail::getindex<VocClass>(this->ExecutePlanSound, exINI, section, "ExecutePlanSound");
	detail::getindex<VocClass>(this->PlaceBeaconSound, exINI, section, "PlaceBeaconSound");
	detail::getindex<VocClass>(this->BuildingGarrisonedSound, exINI, section, "BuildingGarrisonedSound");
	detail::getindex<VocClass>(this->BuildingAbandonedSound, exINI, section, "BuildingAbandonedSound");
	detail::getindex<VocClass>(this->BuildingRepairedSound, exINI, section, "BuildingRepairedSound");
	detail::getindex<VocClass>(this->BaseUnderAttackSound, exINI, section, "BaseUnderAttackSound");
	detail::getindex<VocClass>(this->UpgradeVeteranSound, exINI, section, "UpgradeVeteranSound");
	detail::getindex<VocClass>(this->UpgradeEliteSound, exINI, section, "UpgradeEliteSound");
	detail::getindex<VocClass>(this->VoiceIFVRepair, exINI, section, "VoiceIFVRepair");
	detail::getindex<VocClass>(this->SlavesFreeSound, exINI, section, "SlavesFreeSound");
	detail::getindex<VocClass>(this->SlaveMinerDeploySound, exINI, section, "SlaveMinerDeploySound");
	detail::getindex<VocClass>(this->SlaveMinerUndeploySound, exINI, section, "SlaveMinerUndeploySound");
	detail::getindex<VocClass>(this->BunkerWallsUpSound, exINI, section, "BunkerWallsUpSound");
	detail::getindex<VocClass>(this->BunkerWallsDownSound, exINI, section, "BunkerWallsDownSound");
	detail::getindex<VocClass>(this->RepairBridgeSound, exINI, section, "RepairBridgeSound");
	detail::getindex<VocClass>(this->PsychicDominatorActivateSound, exINI, section, "PsychicDominatorActivateSound");
	detail::getindex<VocClass>(this->GeneticMutatorActivateSound, exINI, section, "GeneticMutatorActivateSound");
	detail::getindex<VocClass>(this->PsychicRevealActivateSound, exINI, section, "PsychicRevealActivateSound");
	detail::getindex<VocClass>(this->MasterMindOverloadDeathSound, exINI, section, "MasterMindOverloadDeathSound");
	detail::getindex<VocClass>(this->AirstrikeAbortSound, exINI, section, "AirstrikeAbortSound");
	detail::getindex<VocClass>(this->AirstrikeAttackVoice, exINI, section, "AirstrikeAttackVoice");
	detail::getindex<VocClass>(this->MindClearedSound, exINI, section, "MindClearedSound");
	detail::getindex<VocClass>(this->EnterGrinderSound, exINI, section, "EnterGrinderSound");
	detail::getindex<VocClass>(this->LeaveGrinderSound, exINI, section, "LeaveGrinderSound");
	detail::getindex<VocClass>(this->EnterBioReactorSound, exINI, section, "EnterBioReactorSound");
	detail::getindex<VocClass>(this->LeaveBioReactorSound, exINI, section, "LeaveBioReactorSound");
	detail::getindex<VocClass>(this->ActivateSound, exINI, section, "ActivateSound");
	detail::getindex<VocClass>(this->DeactivateSound, exINI, section, "DeactivateSound");
	detail::getindex<VocClass>(this->SpyPlaneCamera, exINI, section, "SpyPlaneCamera");
	detail::getindex<VocClass>(this->LetsDoTheTimeWarpOutAgain, exINI, section, "LetsDoTheTimeWarpOutAgain");
	detail::getindex<VocClass>(this->LetsDoTheTimeWarpInAgain, exINI, section, "LetsDoTheTimeWarpInAgain");
	detail::getindex<VocClass>(this->DiskLaserChargeUp, exINI, section, "DiskLaserChargeUp");
	detail::getindex<VocClass>(this->CreateUnitSound, exINI, section, "CreateUnitSound");
	detail::getindex<VocClass>(this->CreateInfantrySound, exINI, section, "CreateInfantrySound");
	detail::getindex<VocClass>(this->CreateAircraftSound, exINI, section, "CreateAircraftSound");
	detail::getindex<VocClass>(this->IFVTransformSound, exINI, section, "IFVTransformSound");
	detail::getindex<VocClass>(this->PsychicSensorDetectSound, exINI, section, "PsychicSensorDetectSound");
	detail::getindex<VocClass>(this->SpySatActivationSound, exINI, section, "SpySatActivationSound");
	detail::getindex<VocClass>(this->SpySatDeactivationSound, exINI, section, "SpySatDeactivationSound");
	detail::getindex<VocClass>(this->ShellButtonSlideSound, exINI, section, "ShellButtonSlideSound");
	detail::getindex<VocClass>(this->CloakSound, exINI, section, "CloakSound");
	detail::getindex<VocClass>(this->SellSound, exINI, section, "SellSound");
	detail::getindex<VocClass>(this->GameClosed, exINI, section, "GameClosed");
	detail::getindex<VocClass>(this->IncomingMessage, exINI, section, "IncomingMessage");
	detail::getindex<VocClass>(this->MessageCharTyped, exINI, section, "MessageCharTyped");
	detail::getindex<VocClass>(this->SystemError, exINI, section, "SystemError");
	detail::getindex<VocClass>(this->OptionsChanged, exINI, section, "OptionsChanged");
	detail::getindex<VocClass>(this->GameForming, exINI, section, "GameForming");
	detail::getindex<VocClass>(this->PlayerLeft, exINI, section, "PlayerLeft");
	detail::getindex<VocClass>(this->PlayerJoined, exINI, section, "PlayerJoined");
	detail::getindex<VocClass>(this->Construction, exINI, section, "Construction");
	detail::getindex<VocClass>(this->GateUp, exINI, section, "GateUp");
	detail::getindex<VocClass>(this->GateDown, exINI, section, "GateDown");
	detail::getindex<VocClass>(this->ScoldSound, exINI, section, "ScoldSound");
	detail::getindex<VocClass>(this->TeslaCharge, exINI, section, "TeslaCharge");
	detail::getindex<VocClass>(this->TeslaZap, exINI, section, "TeslaZap");
	detail::getindex<VocClass>(this->ChuteSound, exINI, section, "ChuteSound");
	detail::getindex<VocClass>(this->GenericClick, exINI, section, "GenericClick");
	detail::getindex<VocClass>(this->GenericBeep, exINI, section, "GenericBeep");
	detail::getindex<VocClass>(this->BuildingDrop, exINI, section, "BuildingDrop");
	detail::getindex<VocClass>(this->StopSound, exINI, section, "StopSound");
	detail::getindex<VocClass>(this->GuardSound, exINI, section, "GuardSound");
	detail::getindex<VocClass>(this->ScatterSound, exINI, section, "ScatterSound");
	detail::getindex<VocClass>(this->StormSound, exINI, section, "StormSound");

	// Field/key mismatches — preserved from vanilla
	detail::getindex<VocClass>(this->BuildingDieSound, exINI, section, "BuildingDieSound");    // field CrumbleSound, key "BuildingDieSound"
	detail::getindex<VocClass>(this->BuildingDamageSound, exINI, section, "BuildingDamageSound"); // field BlowupSound, key "BuildingDamageSound"

	auto pData = RulesExtData::Instance();

	auto Shield_ConditionGreen_d = Nullable<double>()(exINI, GameStrings::AudioVisual(), "Shield.ConditionGreen", false);
	auto Shield_ConditionYellow_d = Nullable<double>()(exINI, GameStrings::AudioVisual(), "Shield.ConditionYellow", false);
	auto Shield_ConditionRed_d = Nullable<double>()(exINI, GameStrings::AudioVisual(), "Shield.ConditionRed", false);
	auto ConditionYellow_Terrain_d = Nullable<double>()(exINI, GameStrings::AudioVisual(), "ConditionYellow.Terrain", false);

	pData->Shield_ConditionGreen = Shield_ConditionGreen_d.Get(this->ConditionGreen);
	pData->Shield_ConditionYellow = Shield_ConditionYellow_d.Get(this->ConditionYellow);
	pData->Shield_ConditionRed = Shield_ConditionRed_d.Get(this->ConditionRed);
	pData->ConditionYellow_Terrain = ConditionYellow_Terrain_d.Get(this->ConditionYellow);

	pData->DefaultSquidAnim.Read(exINI, section, "Parasite.GrappleAnim");
	pData->FactoryProgressDisplay.Read(exINI, section, "FactoryProgressDisplay");
	pData->MainSWProgressDisplay.Read(exINI, section, "MainSWProgressDisplay");
	pData->CombatAlert.Read(exINI, section, "CombatAlert");
	pData->CombatAlert_MakeAVoice.Read(exINI, section, "CombatAlert.MakeAVoice");
	pData->CombatAlert_IgnoreBuilding.Read(exINI, section, "CombatAlert.IgnoreBuilding");
	pData->CombatAlert_EVA.Read(exINI, section, "CombatAlert.EVA");
	pData->CombatAlert_UseFeedbackVoice.Read(exINI, section, "CombatAlert.UseFeedbackVoice");
	pData->CombatAlert_UseAttackVoice.Read(exINI, section, "CombatAlert.UseAttackVoice");
	pData->CombatAlert_SuppressIfInScreen.Read(exINI, section, "CombatAlert.SuppressIfInScreen");
	pData->CombatAlert_Interval.Read(exINI, section, "CombatAlert.Interval");
	pData->CombatAlert_SuppressIfAllyDamage.Read(exINI, section, "CombatAlert.SuppressIfAllyDamage");
	pData->SubterraneanHeight.Read(exINI, GameStrings::General, "SubterraneanHeight");

	pData->StartDistributionModeSound.Read(exINI, section, "StartDistributionModeSound");
	pData->EndDistributionModeSound.Read(exINI, section, "EndDistributionModeSound");
	pData->AddDistributionModeCommandSound.Read(exINI, section, "AddDistributionModeCommandSound");
	pData->FirestormActiveAnim.Read(exINI, section, "FirestormActiveAnim");
	pData->FirestormIdleAnim.Read(exINI, section, "FirestormIdleAnim");
	pData->FirestormGroundAnim.Read(exINI, section, "FirestormGroundAnim");
	pData->FirestormAirAnim.Read(exINI, section, "FirestormAirAnim");
	pData->Bounty_Display.Read(exINI, section, "BountyDisplay");
	pData->CloakAnim.Read(exINI, section, "CloakAnim");
	pData->DecloakAnim.Read(exINI, section, "DecloakAnim");

	pData->Promote_Vet_Anim.Read(exINI, section, "Promote.VeteranAnim");
	pData->Promote_Elite_Anim.Read(exINI, section, "Promote.EliteAnim");

	pData->Promote_Vet_PlaySpotlight.Read(exINI, section, "Promote.VeteranPlaySpotLight");
	pData->Promote_Elite_PlaySpotlight.Read(exINI, section, "Promote.ElitePlaySpotLight");

	pData->PrimaryFactoryIndicator.Read(exINI, section, "PrimaryFactoryIndicator");
	pData->PrimaryFactoryIndicator_Palette.Read(exINI, section, "PrimaryFactoryIndicator.Palette");

	pData->DefaultExplodeFireAnim.Read(exINI, section, "DefaultExplodeOverlayFireAnim");
	pData->SuperWeaponSidebar_AllowByDefault.Read(exINI, section, "SuperWeaponSidebar.AllowByDefault");

	pData->ColorAddUse8BitRGB.Read(exINI, section, "ColorAddUse8BitRGB");
	pData->IronCurtain_ExtraTintIntensity.Read(exINI, section, "IronCurtain.ExtraTintIntensity");
	pData->ForceShield_ExtraTintIntensity.Read(exINI, section, "ForceShield.ExtraTintIntensity");

	pData->DefaultInfantrySelectBox.Read(exINI, section, "DefaultInfantrySelectBox");
	pData->DefaultUnitSelectBox.Read(exINI, section, "DefaultUnitSelectBox");

	pData->VoxelLightSource.Read(exINI, section, "VoxelLightSource");
	pData->VoxelShadowLightSource.Read(exINI, section, "VoxelShadowLightSource");
	pData->UseFixedVoxelLighting.Read(exINI, section, "UseFixedVoxelLighting");
	pData->ShowPowerPlantEnhancerRange.Read(exINI, section, "ShowPowerPlantEnhancerRange");

	if (!pData->DefaultExplodeFireAnim)
		pData->DefaultExplodeFireAnim = AnimTypeClass::Find(GameStrings::Anim_FIRE3);

	pData->FlyNoWobbles.Read(exINI, section, "FlyNoWobbles");

	pData->DropShip_LandAnim.Read(exINI, section, "DefaultLandingAnim.Dropship");
	pData->CarryAll_LandAnim.Read(exINI, section, "DefaultLandingAnim.Carryall");
	pData->CarryAll_LandAnim.Read(exINI, section, "LandingAnim.Carryall", true);
	pData->DropShip_LandAnim.Read(exINI, section, "LandingAnim.Dropship", true);
	pData->Aircraft_LandAnim.Read(exINI, section, "LandingAnim.Aircraft", true);
	pData->LandingAnim.Read(exINI, section, "DefaultLandingAnim", true);


	detail::getindex<VocClass>(pData->AttachedToObject->DeploySound, exINI, section, "DeploySound");
	pData->RemoveMindControl_Silent.Read(exINI, section, "RemoveMindControl.Silent");
	pData->DisplayIncome_Delay.Read(exINI, section, "DisplayIncome.Delay");
	if (!pData->DisplayIncome_Delay)
	{
		Debug::Log("[Developer warning] [AudioVisual] DisplayIncome.Delay is set 0 which would cause a crash, set to 1 instead.\n");
		pData->DisplayIncome_Delay = 1;
	}
	pData->LaserZAdjust.Read(exINI, section, "LaserZAdjust");
	pData->EBoltZAdjust.Read(exINI, section, "EBoltZAdjust");
	pData->EBoltZAdjust_ClampInitialDepthForBuilding.Read(exINI, section, "EBoltZAdjust.ClampInitialDepthForBuilding");
	pData->AirstrikeLineZAdjust.Read(exINI, section, "AirstrikeLineZAdjust");
	pData->UseRetintFix.Read(exINI, section, "UseRetintFix");
	pData->WarheadAnimZAdjust.Read(exINI, section, "WarheadAnimZAdjust");
	pData->FiringAnim_Update.Read(exINI, section, "FiringAnim.Update");
	pData->WalkLocomotorMakesWake.Read(exINI, section, "WalkLocomotorMakesWake");
	pData->VisualScatter_Min.Read(exINI, section, "VisualScatter.Min");
	pData->VisualScatter_Max.Read(exINI, section, "VisualScatter.Max");
	pData->JumpjetTilt.Read(exINI, section, "JumpjetTilt");
	pData->AirstrikeLineColor.Read(exINI, section, "AirstrikeLineColor");
	pData->Cameo_AlwaysExist.Read(exINI, section, "Cameo.AlwaysExist");
	pData->Cameo_OverlayShapes.Read(exINI, section, "Cameo.OverlayShapes");
	pData->Cameo_OverlayFrames.Read(exINI, section, "Cameo.OverlayFrames");
	pData->Cameo_OverlayPalette.Read(exINI, section, "Cameo.OverlayPalette");
	pData->UnitIdleRotateTurret.Read(exINI, section, "UnitIdleRotateTurret");
	pData->UnitIdlePointToMouse.Read(exINI, section, "UnitIdlePointToMouse");
	pData->UnitIdleActionRestartMin.Read(exINI, section, "UnitIdleActionRestartMin");
	pData->UnitIdleActionRestartMax.Read(exINI, section, "UnitIdleActionRestartMax");
	pData->UnitIdleActionIntervalMin.Read(exINI, section, "UnitIdleActionIntervalMin");
	pData->UnitIdleActionIntervalMax.Read(exINI, section, "UnitIdleActionIntervalMax");
	pData->ShakeScreenUseTSCalculation.Read(exINI, section, "ShakeScreenUseTSCalculation");
	pData->CheckExpandPlaceGrid.Read(exINI, section, "CheckExpandPlaceGrid");
	pData->ExpandLandGridFrames.Read(exINI, section, "ExpandLandGridFrames");
	pData->ExpandWaterGridFrames.Read(exINI, section, "ExpandWaterGridFrames");
	pData->VeinsAttack_interval.Read(exINI, section, "VeinsAttackInterval");
	pData->BuildingFlameSpawnBlockFrames.Read(exINI, section, "BuildingFlameSpawnBlockFrames");
	pData->AircraftLevelLightMultiplier.Read(exINI, section, "AircraftLevelLightMultiplier");
	pData->AircraftCellLightLevelMultiplier.Read(exINI, section, "AircraftCellLightLevelMultiplier");
	pData->JumpjetLevelLightMultiplier.Read(exINI, section, "JumpjetLevelLightMultiplier");
	pData->JumpjetCellLightLevelMultiplier.Read(exINI, section, "JumpjetCellLightLevelMultiplier");
	pData->JumpjetCellLightApplyBridgeHeight.Read(exINI, section, "JumpjetCellLightApplyBridgeHeight");
	double AirShadowBaseScale = 0.0;
	if (detail::read<double>(AirShadowBaseScale, exINI, section, "AirShadowBaseScale") && AirShadowBaseScale > 0)
		pData->AirShadowBaseScale_log = -std::log(std::min(AirShadowBaseScale, 1.0));

	pData->HeightShadowScaling.Read(exINI, section, "HeightShadowScaling");

	if (AirShadowBaseScale > 0.98 && pData->HeightShadowScaling.Get())
		pData->HeightShadowScaling = false;

	pData->HeightShadowScaling_MinScale.Read(exINI, section, "HeightShadowScaling.MinScale");
	pData->Buildings_DefaultDigitalDisplayTypes.Read(exINI, section, "Buildings.DefaultDigitalDisplayTypes");
	pData->Infantry_DefaultDigitalDisplayTypes.Read(exINI, section, "Infantry.DefaultDigitalDisplayTypes");
	pData->Vehicles_DefaultDigitalDisplayTypes.Read(exINI, section, "Vehicles.DefaultDigitalDisplayTypes");
	pData->Aircraft_DefaultDigitalDisplayTypes.Read(exINI, section, "Aircraft.DefaultDigitalDisplayTypes");
	pData->DisplayIncome.Read(exINI, section, "DisplayIncome");
	pData->DisplayIncome_Houses.Read(exINI, section, "DisplayIncome.Houses");
	pData->DisplayIncome_AllowAI.Read(exINI, section, "DisplayIncome.AllowAI");
	pData->Droppod_ImageInfantry.Read(exINI, section, "DropPod.InfantryPodImage");
	pData->DrawInsigniaOnlyOnSelected.Read(exINI, section, "DrawInsigniaOnlyOnSelected");
	pData->DrawInsignia_AdjustPos_Infantry.Read(exINI, section, "DrawInsignia.AdjustPos.Infantry");
	pData->DrawInsignia_AdjustPos_Buildings.Read(exINI, section, "DrawInsignia.AdjustPos.Buildings");
	pData->DrawInsignia_AdjustPos_BuildingsAnchor.Read(exINI, section, "DrawInsignia.AdjustPos.BuildingsAnchor");
	pData->DrawInsignia_AdjustPos_Units.Read(exINI, section, "DrawInsignia.AdjustPos.Units");
	pData->DrawInsignia_UsePixelSelectionBracketDelta.Read(exINI, section, "DrawInsignia.UsePixelSelectionBracketDelta");
	pData->DisplayCreditsDelay.Read(exINI, section, "DisplayCreditsDelay");
	pData->VeinholeParticle.Read(exINI, section, "VeinholeSpawnParticleType", true);
	pData->Aircraft_TakeOffAnim.Read(exINI, section, "TakeOffAnim.Aircraft", true);
	pData->ElectricDeath.Read(exINI, section, "InfantryElectrocuted");
	pData->DrawTurretShadow.Read(exINI, section, "DrawTurretShadow");
	pData->AnimRemapDefaultColorScheme.Read(exINI, section, "AnimRemapDefaultColorScheme");
	pData->StealthSpeakDelay.Read(exINI, section, "StealthSpeakDelay");
	pData->SubterraneanSpeakDelay.Read(exINI, section, "SubterraneanSpeakDelay");
	pData->DeactivateDim_Powered.Read(exINI, section, "DeactivateDimPowered");
	pData->DeactivateDim_EMP.Read(exINI, section, "DeactivateDimEMP");
	pData->DeactivateDim_Operator.Read(exINI, section, "DeactivateDimOperator");
	pData->Building_PlacementPreview.Read(exINI, section, "ShowBuildingPlacementPreview");
	pData->Building_PlacementPreview.Read(exINI, section, "PlacementPreview");
	pData->PlacementGrid_TranslucencyWithPreview.Read(exINI, section, "PlacementGrid.TranslucencyWithPreview");
	pData->CreateSound_PlayerOnly.Read(exINI, section, "CreateSound.AffectOwner");
	pData->Pips_Shield.Read(exINI, section, "Pips.Shield");
	pData->Pips_Shield_Buildings.Read(exINI, section, "Pips.Shield.Building");
	pData->MissingCameo.Read(pINI, section, "MissingCameo");
	pData->PlacementGrid_TranslucentLevel.Read(exINI, section, !Phobos::Otamaa::CompatibilityMode ? "BuildingPlacementGrid.TranslucentLevel" : "PlacementGrid.Translucency");
	pData->BuildingPlacementPreview_TranslucentLevel.Read(exINI, section, !Phobos::Otamaa::CompatibilityMode ? "BuildingPlacementPreview.DefaultTranslucentLevel" : "PlacementPreview.Translucency");
	pData->Pips_Shield.Read(exINI, section, "Pips.Shield");
	pData->Pips_Shield_Background_SHP.Read(exINI, section, "Pips.Shield.Background");
	pData->Pips_Shield_Building.Read(exINI, section, "Pips.Shield.Building");
	pData->Pips_Shield_Building_Empty.Read(exINI, section, "Pips.Shield.Building.Empty");
	pData->Pips_SelfHeal_Infantry.Read(exINI, section, "Pips.SelfHeal.Infantry");
	pData->Pips_SelfHeal_Units.Read(exINI, section, "Pips.SelfHeal.Units");
	pData->Pips_SelfHeal_Buildings.Read(exINI, section, "Pips.SelfHeal.Buildings");
	pData->Pips_SelfHeal_Infantry_Offset.Read(exINI, section, "Pips.SelfHeal.Infantry.Offset");
	pData->Pips_SelfHeal_Units_Offset.Read(exINI, section, "Pips.SelfHeal.Units.Offset");
	pData->Pips_SelfHeal_Buildings_Offset.Read(exINI, section, "Pips.SelfHeal.Buildings.Offset");
	pData->Pips_Generic_Size.Read(exINI, section, "Pips.Generic.Size");
	pData->Pips_Generic_Buildings_Size.Read(exINI, section, "Pips.Generic.Buildings.Size");
	pData->Pips_Ammo_Size.Read(exINI, section, "Pips.Ammo.Size");
	pData->Pips_Ammo_Buildings_Size.Read(exINI, section, "Pips.Ammo.Buildings.Size");
	pData->Pips_Tiberiums_Frames.Read(exINI, section, "Pips.Tiberiums.Frames");
	pData->Pips_Tiberiums_DisplayOrder.Read(exINI, section, "Pips.Tiberiums.DisplayOrder");
	pData->ToolTip_Background_Color.Read(exINI, section, "ToolTip.Background.Color");
	pData->ToolTip_Background_Opacity.Read(exINI, section, "ToolTip.Background.Opacity");
	pData->ToolTip_Background_BlurSize.Read(exINI, section, "ToolTip.Background.BlurSize");
	pData->ToolTip_ExcludeSidebar.Read(exINI, section, "ToolTip.ExcludeSidebar");
	pData->UseSelectBrd.Read(exINI, section, "UseSelectBrd");
	pData->SHP_SelectBrdSHP_INF.Read(exINI, section, "SelectBrd.SHP.Infantry");
	pData->SHP_SelectBrdPAL_INF.Read(exINI, section, "SelectBrd.PAL.Infantry");
	pData->SelectBrd_Frame_Infantry.Read(exINI, section, "SelectBrd.Frame.Infantry");
	pData->SelectBrd_DrawOffset_Infantry.Read(exINI, section, "SelectBrd.DrawOffset.Infantry");
	pData->SHP_SelectBrdSHP_UNIT.Read(exINI, section, "SelectBrd.SHP.Unit");
	pData->SHP_SelectBrdPAL_UNIT.Read(exINI, section, "SelectBrd.PAL.Unit");
	pData->SelectBrd_Frame_Unit.Read(exINI, section, "SelectBrd.Frame.Unit");
	pData->SelectBrd_DrawOffset_Unit.Read(exINI, section, "SelectBrd.DrawOffset.Unit");
	pData->SelectBrd_DefaultTranslucentLevel.Read(exINI, section, "SelectBrd.DefaultTranslucentLevel");
	pData->SelectBrd_DefaultShowEnemy.Read(exINI, section, "SelectBrd.DefaultShowEnemy");
	pData->VeteranFlashTimer.Read(exINI, section, "VeteranFlashTimer");
	pData->Tiberium_ExplosiveAnim.Read(exINI, section, "TiberiumExplosiveAnim");
	pData->DecloakSound.Read(exINI, section, "DecloakSound");
	pData->IC_Flash.Read(exINI, section, "IronCurtainFlash");
	pData->DiskLaserAnimEnabled.Read(exINI, section, "DiskLaserAnimEnabled");
	pData->TimerBlinkColorScheme.Read(exINI, section, "TimerBlinkColorScheme");

	pData->SelectFlashTimer.Read(exINI, section, "SelectFlashTimer");
	pData->SelectFlashTimer.Read(exINI, section, "SelectionFlashDuration");

	pData->WarheadParticleAlphaImageIsLightFlash.Read(exINI, section, "WarheadParticleAlphaImageIsLightFlash");
	pData->CombatLightDetailLevel.Read(exINI, section, "CombatLightDetailLevel");
	pData->CombatLightDetailLevel_CheckColored.Read(exINI, section, "CombatLightDetailLevel.CheckColored");
	pData->LightFlashAlphaImageDetailLevel.Read(exINI, section, "LightFlashAlphaImageDetailLevel");

	pData->DrainMoneyDisplay.Read(exINI, section, "DrainMoneyDisplay");
	pData->DrainMoneyDisplay_Houses.Read(exINI, section, "DrainMoneyDisplay.Houses");
	pData->DrainMoneyDisplay_OnTarget.Read(exINI, section, "DrainMoneyDisplay.OnTarget");
	pData->DrainMoneyDisplay_OnTarget_UseDisplayIncome.Read(exINI, section, "DrainMoneyDisplay.OnTarget.UseDisplayIncome");

}

DEFINE_FUNCTION_JUMP(LJMP, 0x6691E0, FakeRulesClass::_ReadAudioVisual)

void FakeRulesClass::_ReadCrateRules(CCINIClass* pINI)
{
	static constexpr const char* section = "CrateRules";

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	// Bool
	detail::read(this->FreeMCV, exINI, section, "FreeMCV");

	// OverlayTypeClass* pointers — Find_Or_Make → pass true
	detail::read(this->WoodCrateImg, exINI, section, "WoodCrateImg", true);
	detail::read(this->CrateImg, exINI, section, "CrateImg", true);
	detail::read(this->WaterCrateImg, exINI, section, "WaterCrateImg", true);

	// Sound ID (VocClass::Get_ID_From_Name — no true needed)
	// VERIFY: confirm detail::read<VocType> preserves the -1 fallback behaviour
	detail::getindex<VocClass>(this->HealCrateSound, exINI, section, "HealCrateSound");

	// Scalar ints
	detail::read(this->CrateMinimum, exINI, section, "CrateMinimum");
	detail::read(this->CrateMaximum, exINI, section, "CrateMaximum");
	detail::read(this->SoloCrateMoney, exINI, section, "SoloCrateMoney");

	// Lepton
	detail::read(this->CrateRadius, exINI, section, "CrateRadius");

	// Double — key is "CrateRegen", field is CrateTime
	detail::read(this->CrateRegen, exINI, section, "CrateRegen");

	// UnitTypeClass* — Find_Or_Make → pass true
	detail::read(this->UnitCrateType, exINI, section, "UnitCrateType", true);

	// CrateType enum fields — SUSPECT: vanilla calls Put_CrateType (a write), then assigns result back.
	// Almost certainly IDA misidentified Get_CrateType as Put_CrateType. Cross-check assembly.
	// VERIFY: replace with correct Get_CrateType call once confirmed.
	detail::getindex<CrateTypeClass*>(this->SilverCrate_I, exINI, section, "SilverCrate"); // VERIFY: CrateType enum read
	detail::getindex<CrateTypeClass*>(this->WoodCrate_I, exINI, section, "WoodCrate");   // VERIFY: CrateType enum read
	detail::getindex<CrateTypeClass*>(this->WaterCrate_I, exINI, section, "WaterCrate");  // VERIFY: CrateType enum read

	// Ext 
	auto pData = RulesExtData::Instance();

	pData->RandomCrateMoney.Read(exINI, section, "RandomCrateMoney");
	pData->Crate_LandOnly.Read(exINI, section, "Crate.LandOnly");
	pData->UnitCrateVehicleCap.Read(exINI, section, "UnitCrateVehicleCap");
	pData->FreeMCV_CreditsThreshold.Read(exINI, section, "FreeMCV.CreditsThreshold");
}

DEFINE_FUNCTION_JUMP(LJMP, 0x66B900, FakeRulesClass::_ReadCrateRules)

void FakeRulesClass::_ReadRadiation(CCINIClass* pINI)
{
	static constexpr const char* section = "Radiation";

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	// Scalar ints
	detail::read(this->RadDurationMultiple, exINI, section, "RadDurationMultiple");
	detail::read(this->RadApplicationDelay, exINI, section, "RadApplicationDelay");
	detail::read(this->RadLevelMax, exINI, section, "RadLevelMax");
	detail::read(this->RadLevelDelay, exINI, section, "RadLevelDelay");
	detail::read(this->RadLightDelay, exINI, section, "RadLightDelay");

	// Scalar doubles
	detail::read(this->RadLevelFactor, exINI, section, "RadLevelFactor");
	detail::read(this->RadLightFactor, exINI, section, "RadLightFactor");
	detail::read(this->RadTintFactor, exINI, section, "RadTintFactor");

	// RGB color
	// VERIFY: confirm detail::read<ColorStruct/RGBClass> handles CCINIClass::Get_RGB equivalent
	detail::read(this->RadColor, exINI, section, "RadColor");

	// WarheadTypeClass* — Find_Or_Make_0 in vanilla
	detail::read(this->RadSiteWarhead, exINI, section, "RadSiteWarhead", true);

	// Ext 
	auto pData = RulesExtData::Instance();

	pData->RadApplicationDelay_Building.Read(exINI, section, "RadApplicationDelay.Building");
	pData->RadBuildingDamageMaxCount.Read(exINI, section, "RadBuildingDamageMaxCount");
	pData->RadWarhead_Detonate.Read(exINI, section, "RadSiteWarhead.Detonate");
	pData->RadHasOwner.Read(exINI, section, "RadHasOwner");
	pData->RadHasInvoker.Read(exINI, section, "RadHasInvoker");
	pData->UseGlobalRadApplicationDelay.Read(exINI, section, "UseGlobalRadApplicationDelay");

}

DEFINE_FUNCTION_JUMP(LJMP, 0x66CF70, FakeRulesClass::_ReadRadiation)

void FakeRulesClass::_ReadMPlayer(CCINIClass* pINI)
{
	const char* section = "MultiplayerDialogSettings";

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	// Scalar ints
	detail::read(this->MinMoney, exINI, section, "MinMoney");
	detail::read(this->Money, exINI, section, "Money");       // field MPDefaultMoney, key "Money"
	detail::read(this->MaxMoney, exINI, section, "MaxMoney");
	detail::read(this->MoneyIncrement, exINI, section, "MoneyIncrement");
	detail::read(this->MinUnitCount, exINI, section, "MinUnitCount");
	detail::read(this->UnitCount, exINI, section, "UnitCount");   // field MPUnitCount, key "UnitCount"
	detail::read(this->MaxUnitCount, exINI, section, "MaxUnitCount");
	detail::read(this->TechLevel, exINI, section, "TechLevel");   // field MPTechLevel, key "TechLevel"
	detail::read(this->GameSpeed, exINI, section, "GameSpeed");
	detail::read(this->AIDifficulty, exINI, section, "AIDifficulty");
	detail::read(this->AIPlayers, exINI, section, "AIPlayers");

	// Bool fields
	detail::read(this->BridgeDestruction, exINI, section, "BridgeDestruction");
	detail::read(this->ShadowGrow, exINI, section, "ShadowGrow");
	detail::read(this->Shroud, exINI, section, "Shroud");
	detail::read(this->Bases, exINI, section, "Bases");
	detail::read(this->TiberiumGrows, exINI, section, "TiberiumGrows");
	detail::read(this->Crates, exINI, section, "Crates");
	detail::read(this->CaptureTheFlag, exINI, section, "CaptureTheFlag");
	detail::read(this->HarvesterTruce, exINI, section, "HarvesterTruce");
	detail::read(this->MultiEngineer, exINI, section, "MultiEngineer");
	detail::read(this->AlliesAllowed, exINI, section, "AlliesAllowed");
	detail::read(this->AllyChangeAllowed, exINI, section, "AllyChangeAllowed");
	detail::read(this->ShortGame, exINI, section, "ShortGame");
	detail::read(this->SuperWeaponsAllowed, exINI, section, "SuperWeaponsAllowed");
	detail::read(this->BuildOffAlly, exINI, section, "BuildOffAlly");
	detail::read(this->FogOfWar, exINI, section, "FogOfWar");
	detail::read(this->MCVRedeploys, exINI, section, "MCVRedeploys");
	// Ext
	auto pData = RulesExtData::Instance();
}

DEFINE_FUNCTION_JUMP(LJMP, 0x671EA0, FakeRulesClass::_ReadMPlayer);

void FakeRulesClass::_ReadJumpjetControls(CCINIClass* pINI)
{
	const char* section = GameStrings::JumpjetControls;

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	// Scalar ints
	detail::read(this->TurnRate, exINI, section, "TurnRate");
	detail::read(this->Speed, exINI, section, "Speed");
	detail::read(this->CruiseHeight, exINI, section, "CruiseHeight");
	detail::read(this->WobbleDeviation, exINI, section, "WobbleDeviation");

	// Scalar doubles
	detail::read(this->Climb, exINI, section, "Climb");
	detail::read(this->Acceleration, exINI, section, "Acceleration");
	detail::read(this->WobblesPerSecond, exINI, section, "WobblesPerSecond");

	// Ext
	auto pData = RulesExtData::Instance();

	pData->JumpjetCrash.Read(exINI, GameStrings::JumpjetControls, "Crash");
	pData->JumpjetNoWobbles.Read(exINI, GameStrings::JumpjetControls, "NoWobbles");

}

DEFINE_FUNCTION_JUMP(LJMP, 0x6743D0, FakeRulesClass::_ReadJumpjetControls)

template<typename T>
void ReadArray(CCINIClass* pINI, const char* pSection) {

	Debug::Log("Processing %s.\n" , pSection);

	if (!pINI->GetSection(pSection)) {
		Debug::Log("Cannot Find %s section.\n", pSection);
		return;
	}

	for (int i = 0; i < pINI->GetKeyCount(pSection); ++i) {
		char _buffer[32];
		if (pINI->GetString(pSection, pINI->GetKeyName(pSection, i), _buffer) > 0) {
			T::FindOrAllocate(_buffer);
		}
	}
};

DEFINE_POINTER(RectangleStruct, _SomeRect , 0xB0FC68)

void FakeRulesClass::_Process(CCINIClass* pINI)
{
	RulesExtData::Instance()->Initialize(pINI);

	this->_ReadColors(pINI);
	this->_ReadJumpjetControls(pINI);
	this->_ReadColorAdd(pINI);

	{
		ReadArray<HouseTypeClass>(pINI, "Countries");
		//for (int i = 0; i < HouseTypeClass::Array->Count; ++i)
		//	Debug::Log("House [%s] At %d FP %f\n", HouseTypeClass::Array->Items[i]->ID, i, HouseTypeClass::Array->Items[i]->FirepowerMult);

		this->Read_Sides(pINI);
		ReadArray<OverlayTypeClass>(pINI, "OverlayTypes");
		ReadArray<SuperWeaponTypeClass>(pINI, "SuperWeaponTypes");

		//0x668D86 RulesData_Process_PreFillTypeListData

		{
			ReadArray<BulletTypeClass>(pINI, "Projectiles");
			ReadArray<TiberiumClass>(pINI, "Tiberiums");

			RulesExtData::Instance()->DefaultBulletType = BulletTypeClass::FindOrAllocate(DEFAULT_STR2);
			if (!RulesExtData::Instance()->DefaultBulletType)
				Debug::FatalError("Uneable to Allocate {} BulletType ! ", DEFAULT_STR2);

			ReadArray<WeaponTypeClass>(pINI,"WeaponTypes");
			ReadArray<WarheadTypeClass>(pINI,"Warheads");		
		}

		ReadArray<SmudgeTypeClass>(pINI,"SmudgeTypes");
		ReadArray<TerrainTypeClass>(pINI,"TerrainTypes");
		ReadArray<BuildingTypeClass>(pINI,"BuildingTypes");
		ReadArray<UnitTypeClass>(pINI,"VehicleTypes");
		ReadArray<AircraftTypeClass>(pINI,"AircraftTypes");
		ReadArray<InfantryTypeClass>(pINI,"InfantryTypes");
		ReadArray<AnimTypeClass>(pINI,"Animations");
		ReadArray<VoxelAnimTypeClass>(pINI,"VoxelAnims");
		ReadArray<ParticleTypeClass>(pINI,"Particles");
		ReadArray<ParticleSystemTypeClass>(pINI,"ParticleSystems");
	}

	this->_ReadMPlayer(pINI);
	this->_ReadAI(pINI);
	this->_ReadPowerups(pINI);
	this->_ReadLandTypes(pINI);
	this->_ReadIQ(pINI);
	this->_ReadGeneral(pINI);

	{	SideClass::Array->for_each([pINI](SideClass* pSide) {
			SideExtContainer::Instance.LoadFromINI(pSide, pINI, !pINI->GetSection(pSide->ID));
		});

	//
		HouseTypeClass::Array->for_each([pINI](HouseTypeClass* pHouse) {
			HouseTypeExtContainer::Instance.LoadFromINI(pHouse, pINI, !pINI->GetSection(pHouse->ID));
		});
	}

	{
		InsigniaTypeClass::LoadFromINIList(pINI);

		this->_ReadCrateRules(pINI);
		this->_ReadCombatDamage(pINI);
		this->_ReadRadiation(pINI);
		this->_ReadElevationModel(pINI);
		this->_ReadWallModel(pINI);
		this->_ReadAudioVisual(pINI);
		this->_ReadSpecialWeapons(pINI);

		RulesExtData::s_LoadBeforeTypeData(this, pINI);
		this->Read_Types(pINI);
		RulesExtData::LoadAfterTypeData(this, pINI);

		// Ensure entry not fail because of late instantiation
		// add more if needed , it will double the error log at some point
		// but it will take care some of missing stuffs that previously loaded late

		for (int i = 0; i < BuildingTypeClass::Array->Count; ++i) {
			BuildingTypeExtContainer::Instance.Find(BuildingTypeClass::Array->Items[i])
				->CompleteInitialization();
		}

		RulesExtData::Instance()->ReplaceVoxelLightSources();

		for (auto pWeapon : *WeaponTypeClass::Array) {
			pWeapon->LoadFromINI(pINI);
		}

		for (auto pBullet : *BulletTypeClass::Array) {
			pBullet->LoadFromINI(pINI);
		}

		for(int i = 0; i < WarheadTypeClass::Array->Count; ++i) {
			Debug::Log("WH [%s] At %d\n", WarheadTypeClass::Array->Items[i]->ID, i);

			WarheadTypeClass::Array->Items[i]->LoadFromINI(pINI);
		}

		for (auto pAnims : *AnimTypeClass::Array) {
			pAnims->LoadFromINI(pINI);
		}

		this->_ReadDifficulty(pINI);

		for (auto pTib : *TiberiumClass::Array) {
			//Debug::LogInfo("Reading Tiberium[{}] Configurations!", pTib->ID);
			pTib->LoadFromINI(pINI);
		}

		RulesExtData::InitializeAfterAllRulesLoaded();
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x668BF0, FakeRulesClass::_Process)

void FakeRulesClass::_ReadColorAdd(CCINIClass* pINI)
{
	if (!pINI->GetSection(GameStrings::ColorAdd))
		return;

	const int count = pINI->GetKeyCount(GameStrings::ColorAdd);

	if (count > 0)
	{
		struct temp_rgb
		{
			byte r, g, b;

			operator ColorStruct()
			{
				return *reinterpret_cast<ColorStruct*>(this);
			}

			operator byte* ()
			{
				return reinterpret_cast<byte*>(this);
			}
		};

		//this was for debugging purposes
		//the code below can be simplified
		RulesExtData::Instance()->ColorAdds.resize(count);

		for (int i = 0; i < count; ++i)
		{
			pINI->Read3Bytes(RulesExtData::Instance()->ColorAdds[i].asPointer()
				, GameStrings::ColorAdd
				, pINI->GetKeyName(GameStrings::ColorAdd, i)
				, RulesExtData::Instance()->ColorAdds[i].asPointer());
		}

		if (RulesExtData::Instance()->ColorAdds.size() >= RulesClass::Instance->ColorAdd.size())
		{
			Debug::LogInfo("Readed ColorAdd and the size is more than 16 max , parsed size {}", count);
			Debug::RegisterParserError();
		}

		for (size_t a = 0; a < RulesClass::Instance->ColorAdd.size(); ++a)
		{
			RulesClass::Instance->ColorAdd[a] = RulesExtData::Instance()->ColorAdds[a];
		}

	}
	else
	{
		Debug::FatalErrorAndExit("Empty ColorAdd\n");
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x66D480, FakeRulesClass::_ReadColorAdd)

void FakeRulesClass::_ReadColors(CCINIClass* pINI)
{
	const char* section = GameStrings::Colors;

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	const int count = pINI->GetKeyCount(section);
	for (int i = 0; i < count; ++i) {
		const char* pName = pINI->GetKeyName(section, i);
		auto _buffer = Valueable<HSVClass>()(exINI, section , pName, false).Get();
		Game::AddColor(pName, &_buffer);
		ColorScheme::FindOrAllocatePTR(pName, &_buffer, FileSystem::UNITPAL.operator->(), FileSystem::TEMPERAT_PAL.operator->(), 1);
		ColorScheme::FindOrAllocatePTR(pName, &_buffer, FileSystem::UNITPAL.operator->(), FileSystem::TEMPERAT_PAL.operator->(), 53);
	}
}

int __fastcall VQ_From_Name(void* a1) JMP_FAST(0x48DF30);

void FakeRulesClass::_ReadMovies(CCINIClass* pINI)
{
	const char* section = "Movies";

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);
	const int count = pINI->GetKeyCount(section);
	for (int i = 0; i < count; ++i) {
		auto pName = pINI->GetKeyName(section, i);
		
		char _buffer[256];
		if (pINI->ReadString(section, pName, GameStrings::NoneStr(), _buffer)) {	
			//does this mean <none> will still valid ? , i suppose
			if (VQ_From_Name(_buffer) != -1)
				continue;

			MovieInfoArray->emplace_back(_strdup(pName));
		}
	}
}

DEFINE_FUNCTION_JUMP(LJMP, 0x66D3A0, FakeRulesClass::_ReadColors)

void FakeRulesClass::_ReadAI(CCINIClass* pINI)
{
	const char* section = GameStrings::AI;

	if (!pINI->GetSection(section))
		return;

	INI_EX exINI(pINI);

	auto pData = RulesExtData::Instance();

	pData->AIAdjacentMax.Read(exINI, section, "AIAdjacentMax");
	pData->AIAdjacentMax_Campaign.Read(exINI, section, "AIAdjacentMax.Campaign");
	pData->AIAutoDeployMCV.Read(exINI, section, "AIAutoDeployMCV");
	pData->AISetBaseCenter.Read(exINI, section, "AISetBaseCenter");
	pData->AIBiasSpawnCell.Read(exINI, section, "AIBiasSpawnCell");
	pData->AIForbidConYard.Read(exINI, section, "AIForbidConYard");
	pData->AINodeWallsOnly.Read(exINI, section, "AINodeWallsOnly");
	pData->AICleanWallNode.Read(exINI, section, "AICleanWallNode");
	pData->EnablePowerSurplus.Read(exINI, section, "EnablePowerSurplus");
	pData->NewTeamsSelector.Read(exINI, section, "NewTeamsSelector");
	pData->NewTeamsSelector_SplitTriggersByCategory.Read(exINI, section, "NewTeamsSelector.SplitTriggersByCategory");
	pData->NewTeamsSelector_EnableFallback.Read(exINI, section, "NewTeamsSelector.EnableFallback");
	pData->NewTeamsSelector_MergeUnclassifiedCategoryWith.Read(exINI, section, "NewTeamsSelector.MergeUnclassifiedCategoryWith");
	pData->NewTeamsSelector_UnclassifiedCategoryPercentage.Read(exINI, section, "NewTeamsSelector.UnclassifiedCategoryPercentage");
	pData->NewTeamsSelector_GroundCategoryPercentage.Read(exINI, section, "NewTeamsSelector.GroundCategoryPercentage");
	pData->NewTeamsSelector_AirCategoryPercentage.Read(exINI, section, "NewTeamsSelector.AirCategoryPercentage");
	pData->NewTeamsSelector_NavalCategoryPercentage.Read(exINI, section, "NewTeamsSelector.NavalCategoryPercentage");
	pData->PowerSurplus_ScaleToDrainAmount.Read(exINI, section, "PowerSurplus.ScaleToDrainAmount");

	detail::ParseVector<BuildingTypeClass*>(this->BuildConst, exINI, section, GameStrings::BuildConst, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildPower, exINI, section, GameStrings::BuildPower, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildRefinery, exINI, section, GameStrings::BuildRefinery, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildBarracks, exINI, section, GameStrings::BuildBarracks, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildTech, exINI, section, GameStrings::BuildTech, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildWeapons, exINI, section, GameStrings::BuildWeapons, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->AlliedBaseDefenses, exINI, section, GameStrings::AlliedBaseDefenses, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->SovietBaseDefenses, exINI, section, GameStrings::SovietBaseDefenses, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->ThirdBaseDefenses, exINI, section, GameStrings::ThirdBaseDefenses, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildDefense, exINI, section, GameStrings::BuildDefense, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildPDefense, exINI, section, GameStrings::BuildPDefense, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildAA, exINI, section, GameStrings::BuildAA, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildHelipad, exINI, section, GameStrings::BuildHelipad, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildRadar, exINI, section, GameStrings::BuildRadar, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->ConcreteWalls, exINI, section, GameStrings::ConcreteWalls, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->NSGates, exINI, section, GameStrings::NSGates, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->EWGates, exINI, section, GameStrings::EWGates, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildNavalYard, exINI, section, GameStrings::BuildNavalYard, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->BuildDummy, exINI, section, GameStrings::BuildDummy, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*>(this->NeutralTechBuildings, exINI, section, GameStrings::NeutralTechBuildings, "Expect valid BuildingType");
	detail::ParseVector(this->AIForcePredictionFudge, exINI, section, GameStrings::AIForcePredictionFudge, "Expect valid number");

	detail::read<double>(this->AttackInterval, exINI, section, "AttackInterval");
	detail::read<double>(this->AttackDelay, exINI, section, "AttackDelay");
	detail::read<double>(this->PatrolScan, exINI, section, "PatrolScan");
	detail::read<int>(this->CreditReserve, exINI, section, "CreditReserve");
	detail::read<double>(this->PathDelay, exINI, section, "PathDelay");
	detail::read<int>(this->BlockagePathDelay, exINI, section, "BlockagePathDelay");
	detail::read<double>(this->AutocreateTime, exINI, section, "AutocreateTime");
	detail::read<int>(this->InfantryReserve, exINI, section, "InfantryReserve");
	detail::read<int>(this->InfantryBaseMult, exINI, section, "InfantryBaseMult");
	detail::read<int>(this->PowerSurplus, exINI, section, "PowerSurplus");
	detail::read<int>(this->BaseSizeAdd, exINI, section, "BaseSizeAdd");

	detail::read<double>(this->RefineryRatio, exINI, section, "RefineryRatio");
	detail::read<int>(this->RefineryLimit, exINI, section, "RefineryLimit");

	detail::read<double>(this->BarracksRatio, exINI, section, "BarracksRatio");
	detail::read<int>(this->BarracksLimit, exINI, section, "BarracksLimit");

	detail::read<double>(this->WarRatio, exINI, section, "WarRatio");
	detail::read<int>(this->WarLimit, exINI, section, "WarLimit");

	detail::read<double>(this->DefenseRatio, exINI, section, "DefenseRatio");
	detail::read<int>(this->DefenseLimit, exINI, section, "DefenseLimit");

	detail::read<double>(this->AARatio, exINI, section, "AARatio");
	detail::read<int>(this->AALimit, exINI, section, "AALimit");

	detail::read<double>(this->TeslaRatio, exINI, section, "TeslaRatio");
	detail::read<int>(this->TeslaLimit, exINI, section, "TeslaLimit");

	detail::read<double>(this->HelipadRatio, exINI, section, "HelipadRatio");
	detail::read<int>(this->HelipadLimit, exINI, section, "HelipadLimit");

	detail::read<double>(this->AirstripRatio, exINI, section, "AirstripRatio");
	detail::read<int>(this->AirstripLimit, exINI, section, "AirstripLimit");

	detail::read<bool>(this->CompEasyBonus, exINI, section, "CompEasyBonus");
	detail::read<bool>(this->Paranoid, exINI, section, "Paranoid");

	detail::read<double>(this->PowerEmergency, exINI, section, "PowerEmergency");
	detail::read<int>(this->AIBaseSpacing, exINI, section, "AIBaseSpacing");

	detail::read<double>(this->GDIWallDefense, exINI, section, "GDIWallDefense");
	detail::read<double>(this->GDIWallDefenseCoefficient, exINI, section, "GDIWallDefenseCoefficient");
	detail::read<double>(this->NodBaseDefenseCoefficient, exINI, section, "NodBaseDefenseCoefficient");
	detail::read<double>(this->GDIBaseDefenseCoefficient, exINI, section, "GDIBaseDefenseCoefficient");

	detail::read<int>(this->MaximumBaseDefenseValue, exINI, section, "MaximumBaseDefenseValue");
	detail::read<int>(this->ComputerBaseDefenseResponse, exINI, section, "ComputerBaseDefenseResponse");
}

DEFINE_FUNCTION_JUMP(LJMP, 0x672AE0, FakeRulesClass::_ReadAI)

void FakeRulesClass::_ReadCombatDamage(CCINIClass* pINI)
{
	INI_EX exINI(pINI);

	const char* section = GameStrings::CombatDamage;

	if (!pINI->GetSection(section))
		return;

	detail::ParseVector<SmudgeTypeClass*>(this->Scorches, exINI, section, GameStrings::Scorches, "Expect valid SmudgeType");
	detail::ParseVector<SmudgeTypeClass*>(this->Scorches1, exINI, section, GameStrings::Scorches1, "Expect valid SmudgeType");
	detail::ParseVector<SmudgeTypeClass*>(this->Scorches2, exINI, section, GameStrings::Scorches2, "Expect valid SmudgeType");
	detail::ParseVector<SmudgeTypeClass*>(this->Scorches3, exINI, section, GameStrings::Scorches3, "Expect valid SmudgeType");
	detail::ParseVector<SmudgeTypeClass*>(this->Scorches4, exINI, section, GameStrings::Scorches4, "Expect valid SmudgeType");
	detail::ParseVector<AnimTypeClass*>(this->SplashList, exINI, section, GameStrings::SplashList, "Expect valid AnimType");

	detail::read<int>(this->AmmoCrateDamage, exINI, section, "AmmoCrateDamage");
	detail::read<int>(this->IonCannonDamage, exINI, section, "IonCannonDamage");
	detail::read<int>(this->RailgunDamageRadius, exINI, section, "RailgunDamageRadius");
	detail::read<int>(this->TiberiumExplosionDamage, exINI, section, "TiberiumExplosionDamage");

	detail::read(this->FlameDamage, exINI, section, "FlameDamage", true);
	detail::read(this->FlameDamage2, exINI, section, "FlameDamage2", true);
	detail::read(this->C4Warhead, exINI, section, "C4Warhead", true);
	detail::read(this->CrushWarhead, exINI, section, "CrushWarhead", true);
	detail::read(this->V3Warhead, exINI, section, "V3Warhead", true);
	detail::read(this->DMislWarhead, exINI, section, "DMislWarhead", true);
	detail::read(this->V3EliteWarhead, exINI, section, "V3EliteWarhead", true);
	detail::read(this->DMislEliteWarhead, exINI, section, "DMislEliteWarhead", true);
	detail::read(this->CMislWarhead, exINI, section, "CMislWarhead", true);
	detail::read(this->CMislEliteWarhead, exINI, section, "CMislEliteWarhead", true);
	detail::read(this->IvanWarhead, exINI, section, "IvanWarhead", true);

	detail::read<bool>(this->CanDetonateTimeBomb, exINI, section, "CanDetonateTimeBomb");
	detail::read<bool>(this->CanDetonateDeathBomb, exINI, section, "CanDetonateDeathBomb");

	detail::read(this->DeathWeapon, exINI, section, "DeathWeapon", true);

	detail::read<int>(this->IvanDamage, exINI, section, "IvanDamage");
	detail::read<int>(this->IvanTimedDelay, exINI, section, "IvanTimedDelay");

	this->BOMBCURS_SHP = (SHPStruct*)FakeFileLoader::Retrieve("BOMBCURS.SHP", 0);
	this->CHRONOSK_SHP = (SHPStruct*)FakeFileLoader::Retrieve("CHRONOSK.SHP", 0);

	detail::read<int>(this->IvanIconFlickerRate, exINI, section, "IvanIconFlickerRate");
	detail::read<int>(this->IronCurtainDuration, exINI, section, "IronCurtainDuration");
	detail::read<int>(this->PsychicRevealRadius, exINI, section, "PsychicRevealRadius");

	auto ReadDoubleToFloat = [](float& value, INI_EX& exINI, const char* section, const char* key)
		{
			double _buffer = value;
			if (detail::read<double>(_buffer, exINI, section, key))
				value = (float)_buffer;
		};

	ReadDoubleToFloat(this->OccupyDamageMultiplier, exINI, section, "OccupyDamageMultiplier");
	ReadDoubleToFloat(this->OccupyROFMultiplier, exINI, section, "OccupyROFMultiplier");
	detail::read<int>(this->OccupyWeaponRange, exINI, section, "OccupyWeaponRange");

	ReadDoubleToFloat(this->BunkerDamageMultiplier, exINI, section, "BunkerDamageMultiplier");
	ReadDoubleToFloat(this->BunkerROFMultiplier, exINI, section, "BunkerROFMultiplier");
	detail::read<int>(this->BunkerWeaponRangeBonus, exINI, section, "BunkerWeaponRangeBonus");

	ReadDoubleToFloat(this->OpenToppedDamageMultiplier, exINI, section, "OpenToppedDamageMultiplier");
	detail::read<int>(this->OpenToppedRangeBonus, exINI, section, "OpenToppedRangeBonus");
	detail::read<int>(this->OpenToppedWarpDistance, exINI, section, "OpenToppedWarpDistance");

	detail::ParseVector<int>(this->OverloadCount, exINI, section, "OverloadCount");
	detail::ParseVector<int>(this->OverloadDamage, exINI, section, "OverloadDamage");
	detail::ParseVector<int>(this->OverloadFrames, exINI, section, "OverloadFrames");

	detail::read<int>(this->MindControlAttackLineFrames, exINI, section, "MindControlAttackLineFrames");

	detail::read(this->DrainAnimationType, exINI, section, "DrainAnimationType");

	detail::read<int>(this->DrainMoneyFrameDelay, exINI, section, "DrainMoneyFrameDelay");
	detail::read<int>(this->DrainMoneyAmount, exINI, section, "DrainMoneyAmount");

	ReadDoubleToFloat(this->FallingDamageMultiplier, exINI, section, "FallingDamageMultiplier");
	detail::read<bool>(this->CurrentStrengthDamage, exINI, section, "CurrentStrengthDamage");

	detail::read(this->ControlledAnimationType, exINI, section, "ControlledAnimationType", true);
	detail::read(this->PermaControlledAnimationType, exINI, section, "PermaControlledAnimationType", true);
	detail::read(this->IonCannonWarhead, exINI, section, "IonCannonWarhead");

	detail::read(this->DefaultLargeGreySmokeSystem, exINI, section, "DefaultLargeGreySmokeSystem", true);
	detail::read(this->DefaultSmallGreySmokeSystem, exINI, section, "DefaultSmallGreySmokeSystem", true);
	detail::read(this->DefaultSparkSystem, exINI, section, "DefaultSparkSystem", true);
	detail::read(this->DefaultLargeRedSmokeSystem, exINI, section, "DefaultLargeRedSmokeSystem", true);
	detail::read(this->DefaultSmallRedSmokeSystem, exINI, section, "DefaultSmallRedSmokeSystem", true);
	detail::read(this->DefaultDebrisSmokeSystem, exINI, section, "DefaultDebrisSmokeSystem", true);
	detail::read(this->DefaultFireStreamSystem, exINI, section, "DefaultFireStreamSystem", true);
	detail::read(this->DefaultTestParticleSystem, exINI, section, "DefaultTestParticleSystem", true);
	detail::read(this->DefaultRepairParticleSystem, exINI, section, "DefaultRepairParticleSystem", true);

	detail::read<bool>(this->BerzerkAllowed, exINI, section, "BerzerkAllowed");
	detail::read<double>(this->TurboBoost, exINI, section, "TurboBoost");
	detail::read<int>(this->AtomDamage, exINI, section, "AtomDamage");

	detail::read<Leptons>(this->BallisticScatter, exINI, section, "BallisticScatter");

	detail::read<int>(this->BridgeStrength, exINI, section, "BridgeStrength");
	detail::read<double>(this->C4Delay, exINI, section, "C4Delay");

	detail::read<Leptons>(this->Crush, exINI, section, "Crush");

	detail::read<double>(this->ExpSpread, exINI, section, "ExpSpread");

	detail::read(this->FireSupress, exINI, section, "FireSupress");
	detail::read(this->HomingScatter, exINI, section, "HomingScatter");

	detail::read<int>(this->MaxDamage, exINI, section, "MaxDamage");
	detail::read<int>(this->MinDamage, exINI, section, "MinDamage");

	detail::read<bool>(this->TiberiumExplosive, exINI, section, "TiberiumExplosive");
	detail::read<bool>(this->PlayerAutoCrush, exINI, section, "PlayerAutoCrush");
	detail::read<bool>(this->PlayerReturnFire, exINI, section, "PlayerReturnFire");
	detail::read<bool>(this->PlayerScatter, exINI, section, "PlayerScatter");
	detail::read<bool>(this->TreeTargeting, exINI, section, "TreeTargeting");

	auto ReadIncoming = [](int& value, INI_EX& exINI, const char* section, const char* key) -> void
		{

			int buffer_ = -1;
			detail::read<int>(buffer_, exINI, section, key);

			if (buffer_ != -1)
			{
				if (buffer_ >= 100)
				{
					buffer_ = 100;
				}

				buffer_ = ((buffer_ <= 0 ? 0 : buffer_) << 8) / 100;

				if (buffer_ >= 255)
				{
					buffer_ = 255;
				}

				value = buffer_;
			}
		};

	ReadIncoming(this->Incoming, exINI, section, "Incoming");

	detail::read<int>(this->CollapseChance, exINI, section, "CollapseChance");

	auto ReadStrictInitVector = [](TypeList<int>& value, INI_EX& exINI, const char* section, const char* key, int minCount, int maxCount)-> void
		{
			detail::ParseVector<int>(value, exINI, section, key, "Expect valid integer");
			if (value.Count < minCount)
			{
				Debug::Log("ReadStrictInitVector Vector[%s - %s] Count %d is less than %d fill it properly !", section, key, value.Count, minCount);
				return;
			}

			if (value.Count > maxCount)
			{
				Debug::Log("ReadStrictInitVector Vector[%s- %s] Count %d is more than %d remove the extra item !", section, key, value.Count, maxCount);
				return;
			}
		};

	ReadStrictInitVector(this->OverloadCount, exINI, section, "OverloadCount", 4, 4);
	ReadStrictInitVector(this->OverloadDamage, exINI, section, "OverloadDamage", 4, 4);
	ReadStrictInitVector(this->OverloadFrames, exINI, section, "OverloadFrames", 4, 4);

	auto pData = RulesExtData::Instance();
	pData->DamageOwnerMultiplier.Read(exINI, section, "DamageOwnerMultiplier");
	pData->DamageAlliesMultiplier.Read(exINI, section, "DamageAlliesMultiplier");
	pData->DamageEnemiesMultiplier.Read(exINI, section, "DamageEnemiesMultiplier");
	pData->DamageOwnerMultiplier_Berzerk.Read(exINI, section, "DamageOwnerMultiplier.Berzerk");
	pData->DamageAlliesMultiplier_Berzerk.Read(exINI, section, "DamageAlliesMultiplier.Berzerk");
	pData->DamageEnemiesMultiplier_Berzerk.Read(exINI, section, "DamageEnemiesMultiplier.Berzerk");
	pData->DamageOwnerMultiplier_NotAffectsEnemies.Read(exINI, section, "DamageOwnerMultiplier.NotAffectsEnemies");
	pData->DamageAlliesMultiplier_NotAffectsEnemies.Read(exINI, section, "DamageAlliesMultiplier.NotAffectsEnemies");
	pData->DriverKilled_KillPassengers.Read(exINI, section, "DriverKilled.KillPassengers");
	pData->Psychedelic_StackingMode.Read(exINI, section, "Psychedelic.StackingMode");
	pData->BerzerkMission.Read(exINI, section, "BerzerkMission");
	pData->ForceShield_KillOrganicsWarhead.Read(exINI, section, "ForceShield.KillOrganicsWarhead");

	if (!pData->ForceShield_KillOrganicsWarhead)
		pData->ForceShield_KillOrganicsWarhead = this->C4Warhead;

	pData->AllowWeaponSelectAgainstWalls.Read(exINI, section, "AllowWeaponSelectAgainstWalls");
	pData->IronCurtain_KillOrganicsWarhead.Read(exINI, section, "IronCurtain.KillOrganicsWarhead");

	if (!pData->IronCurtain_KillOrganicsWarhead)
		pData->IronCurtain_KillOrganicsWarhead = this->C4Warhead;

	pData->Temporal_ConsiderVersus.Read(exINI, section, "Temporal.ApplyVersus");
	pData->Temporal_ApplyMultiplier.Read(exINI, section, "Temporal.ApplyMultiplier");
	pData->Shrapnel_IgnoreHitBuildings.Read(exINI, section, "Shrapnel.IgnoreHitBuildings");
	pData->AffectsInvokerOnly_IgnoreInvokerState.Read(exINI, section, "AffectsInvokerOnly.IgnoreInvokerState");
	pData->Shrapnel_ObeyWarheadTriggerConditions.Read(exINI, section, "Shrapnel.ObeyWarheadTriggerConditions");
	pData->PenetratesTransport_Level.Read(exINI, section, "PenetratesTransport.Level");
	pData->DamageWallRecursivly.Read(exINI, section, "DamageWallRecursivly");
	pData->AdjacentWallDamage.Read(exINI, section, "AdjacentWallDamage");
	pData->IvanBombAttachToCenter.Read(exINI, section, "IvanBombAttachToCenter");
	pData->AllowBerzerkOnAllies.Read(exINI, section, "AllowBerzerkOnAllies");
	pData->ApplyPerTargetEffectsOnDetonate.Read(exINI, section, "ApplyPerTargetEffectsOnDetonate");
	pData->BerzerkTargeting.Read(exINI, section, "BerzerkTargeting");
	pData->Infantry_IgnoreBuildingSizeLimit.Read(exINI, section, "InfantryIgnoreBuildingSizeLimit");
	pData->MergeBuildingDamage.Read(exINI, section, "MergeBuildingDamage");
	pData->IronCurtain_KeptOnDeploy.Read(exINI, section, "IronCurtain.KeptOnDeploy");
	pData->ForceShield_KeptOnDeploy.Read(exINI, section, "ForceShield.KeptOnDeploy");
	pData->ForceShield_EffectOnOrganics.Read(exINI, section, "ForceShield.EffectOnOrganics");
	pData->IronCurtain_EffectOnOrganics.Read(exINI, section, "IronCurtain.EffectOnOrganics");
	pData->ROF_RandomDelay.Read(exINI, section, "ROF.RandomDelay");
	pData->Tiberium_ExplosiveWarhead.Read(exINI, section, "TiberiumExplosiveWarhead");
	pData->AlliedSolidTransparency.Read(exINI, section, "AlliedSolidTransparency");
	pData->ChainReact_Multiplier.Read(exINI, section, "ChainReact.Multiplier");
	pData->ChainReact_SpreadChance.Read(exINI, section, "ChainReact.SpreadChance");
	pData->ChainReact_MinDelay.Read(exINI, section, "ChainReact.MinDelay");
	pData->ChainReact_MaxDelay.Read(exINI, section, "ChainReact.MaxDelay");
	pData->DamageAirConsiderBridges.Read(exINI, section, "DamageAirConsiderBridges");
	pData->BerserkROFMultiplier.Read(exINI, section, "BerserkROFMultiplier");
	pData->DoggiePanicMax.Read(exINI, section, "DoggiePanicMax");
	pData->HunterSeeker_Damage.Read(exINI, section, "HunterSeekerDamage");
	pData->AutoRepelAI.Read(exINI, section, "AutoRepel");
	pData->AutoRepelPlayer.Read(exINI, section, "PlayerAutoRepel");
	pData->CanTargetAI_IronCurtained.Read(exINI, section, "CanTargetAI.IronCurtained");
	pData->CanTarget_IronCurtained.Read(exINI, section, "CanTarget.IronCurtained");
	pData->AutoTarget_IronCurtained.Read(exINI, section, "AutoTarget.IronCurtained");
	pData->EMPAIRecoverMission.Read(exINI, section, "EMPAIRecoverMission");
	pData->ShieldUseArmorplier.Read(exINI, section, "ShieldApplyArmorMult");
	pData->FirestormWarhead.Read(exINI, section, "FirestormWarhead");
	pData->Cloak_KickOutParasite.Read(exINI, section, "Cloak.KickOutParasite");
	pData->Veinhole_Warhead.Read(exINI, section, "VeinholeWarhead");
}

DEFINE_FUNCTION_JUMP(LJMP, 0x66BBB0, FakeRulesClass::_ReadCombatDamage)

#pragma region WeaponTypeBuffer

ASMJIT_PATCH(0x772462, WeaponTypeClass_LoadFromINI_ListLength, 0x9)
{
	GET(WeaponTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBX);
	GET(CCINIClass*, pINI, EDI);

	INI_EX exINI(pINI);
	detail::ParseVector<AnimTypeClass*>(pThis->Anim, exINI, pSection, GameStrings::Anim(), "Expect valid AnimType");

	return 0x77255F;
}
#pragma endregion

#pragma region WarheadTypeBuffer

// == WarheadType ==
ASMJIT_PATCH(0x75D660, WarheadTypeClass_LoadFromINI_ListLength, 9)
{
	GET(WarheadTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBP);
	GET(CCINIClass*, pINI, EDI);

	INI_EX exINI(pINI);
	detail::ParseVector<AnimTypeClass*>(pThis->AnimList, exINI, pSection, GameStrings::AnimList, "Expect valid AnimType");
	detail::ParseVector(pThis->DebrisMaximums, exINI, pSection, GameStrings::DebrisMaximums, "Expect valid number");
	detail::ParseVector<VoxelAnimTypeClass*>(pThis->DebrisTypes, exINI, pSection, GameStrings::DebrisTypes, "Expect valid VoxelAnimType");

	return 0x75D75D;
}

//WarheadTypeClass_LoadFromINI_SkipLists
DEFINE_JUMP(LJMP, 0x75DAE6, 0x75DDCC);

#pragma endregion

#pragma region TechnoTypeBuffer

ASMJIT_PATCH(0x713171, TechnoTypeClass_LoadFromINI_SkipLists1, 9)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(Category, category, EAX);
	pThis->Category = category;
	return 0x713264;
}

ASMJIT_PATCH(0x713C10, TechnoTypeClass_LoadFromINI_SkipLists2, 7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(const CoordStruct*, pResult, EAX);
	pThis->NaturalParticleSystemLocation = *pResult;
	return 0x713E1A;
}

// == TechnoType ==
ASMJIT_PATCH(0x7125DF, TechnoTypeClass_LoadFromINI_ListLength, 7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(const char*, pSection, EBX);
	GET(CCINIClass*, pINI, ESI);

	INI_EX exINI(pINI);

	detail::ParseVector<ParticleSystemTypeClass*>(pThis->DamageParticleSystems, exINI, pSection, GameStrings::DamageParticleSystems, "Expect valid ParticleSystemType");
	detail::ParseVector<ParticleSystemTypeClass*>(pThis->DestroyParticleSystems, exINI, pSection, GameStrings::DestroyParticleSystems, "Expect valid ParticleSystemType");

	detail::ParseVector<BuildingTypeClass*>(pThis->Dock, exINI, pSection, GameStrings::Dock, "Expect valid BuildingType");

	detail::ParseVector(pThis->DebrisMaximums, exINI, pSection, GameStrings::DebrisMaximums, "Expect valid number");
	detail::ParseVector<VoxelAnimTypeClass*>(pThis->DebrisTypes, exINI, pSection, GameStrings::DebrisTypes, "Expect valid VoxelAnimType");
	detail::ParseVector<AnimTypeClass*>(pThis->DebrisAnims, exINI, pSection, GameStrings::DebrisAnims, "Expect valid AnimType");

	return 0x712830;
}

#pragma endregion

#pragma region HouseTypeBuffer

ASMJIT_PATCH(0x511D16, HouseTypeClass_LoadFromINI_Buffer_CountryVeteran, 9)
{
	GET(HouseTypeClass*, pHouseType, EBX);
	GET(CCINIClass*, pINI, ESI);

	INI_EX exINI(pINI);
	detail::ParseVector<InfantryTypeClass*>(pHouseType->VeteranInfantry, exINI, pHouseType->ID, GameStrings::VeteranInfantry, "Expect valid InfantryType");
	detail::ParseVector<UnitTypeClass*>(pHouseType->VeteranUnits, exINI, pHouseType->ID, GameStrings::VeteranUnits, "Expect valid UnitType");
	detail::ParseVector<AircraftTypeClass*>(pHouseType->VeteranAircraft, exINI, pHouseType->ID, GameStrings::VeteranAircraft, "Expect valid AircraftType");

	return 0x51208C;
}

#pragma endregion

#pragma region GlobalHousePTR

ASMJIT_PATCH(0x581646, MapClass_CollapseCliffs_DefaultAnim, 0x5)
{
	R->Stack(0x1C, RulesExtData::Instance()->XGRYMED1_);//med1
	R->Stack(0x28, RulesExtData::Instance()->XGRYMED2_);//med2
	R->EDX(RulesExtData::Instance()->XGRYSML1_);//0x2C sml
	return 0x58168F;
}

ASMJIT_PATCH(0x581D4E, MapClass_CollapseCliffs_DefaultAnimB, 0x5)
{
	R->Stack(0x20, RulesExtData::Instance()->XGRYMED1_);//med1
	R->Stack(0x24, RulesExtData::Instance()->XGRYMED2_);//med2
	R->EDX(RulesExtData::Instance()->XGRYSML1_);//0x2C sml
	return 0x581D97;
}

ASMJIT_PATCH(0x42499C, AnimClass_AnimToInf_CivialHouse, 0x6)
{
	R->EAX(HouseExtData::FindFirstCivilianHouse());
	return 0x4249D8;
}

ASMJIT_PATCH(0x458230, BuildingClass_GarrisonAI_CivilianHouse, 0x6)
{
	R->EBX(HouseExtData::FindFirstCivilianHouse());
	return 0x45826E;
}

ASMJIT_PATCH(0x41ECB0, AITriggerClass_NeutralOwns_CivilianHouse, 0x5)
{
	R->EBX(HouseExtData::FindFirstCivilianHouse());
	return 0x41ECE8;
}

ASMJIT_PATCH(0x50157C, HouseClass_IsAllowedToAlly_CivilianHouse, 0x5)
{
	HouseExtData::FindFirstCivilianHouse();
	R->EAX(RulesExtData::Instance()->CivilianSideIndex);
	return 0x501586;
}

ASMJIT_PATCH(0x6B0AFE, SlaveManagerClass_FreeSlaves_ToCivilianHouse, 0x5)
{
	R->Stack(0x10, HouseExtData::FindFirstCivilianHouse());
	return 0x6B0B3C;
}

ASMJIT_PATCH(0x5A920D, galite_5A91E0_SpecialHouse, 0x5)
{
	R->EAX(HouseExtData::FindSpecial());
	return 0x5A921E;
}

#pragma endregion

#pragma region HouseUnlimit

// GDlgSupp_4E3690, remove country limit
DEFINE_JUMP(LJMP, 0x4E3792, 0x4E37AD);

//GDlgSupp_4E3A00, remove country limit
DEFINE_JUMP(LJMP, 0x4E3A9C, 0x4E3AA1);

//GDlgSupp_4E3CE0, remove country limit
DEFINE_JUMP(LJMP, 0x4E3F31, 0x4E3F4C);

//GDlgSupp_4E3F70, remove country limit
DEFINE_JUMP(LJMP, 0x4E412C, 0x4E4147);

//GDlgSupp_4E4170, remove country limit
DEFINE_JUMP(LJMP, 0x4E41A7, 0x4E41C3);

//OptionsDlg_WndProc_RemoveResLimit / MainOptions_Options_Dialog
DEFINE_JUMP(LJMP, 0x56017A, 0x560183);

//OptionsDlg_WndProc_RemoveHiResCheck / MainOptions_Options_Dialog
DEFINE_JUMP(LJMP, 0x5601E3, 0x5601FC);

#pragma endregion

ASMJIT_PATCH(0x74C8FB, VeinholeMonsterClass_CTOR_SetArmor, 0x6)
{
	GET(VeinholeMonsterClass*, pThis, ESI);
	GET(TerrainTypeClass* const, pThisTree, EDX);

	auto pType = pThis->GetType();
	if (pType && pThisTree)
		pType->Armor = pThisTree->Armor;

	return 0x0;
}

static	void __fastcall DrawShape_VeinHole
(Surface* Surface, ConvertClass* Pal, SHPStruct* SHP, int FrameIndex, const Point2D* const Position, const RectangleStruct* const Bounds,
 BlitterFlags Flags, int Remap, int ZAdjust, ZGradient ZGradientDescIndex, int Brightness, int TintColor, SHPStruct* ZShape,
 int ZShapeFrame, int XOffset, int YOffset
)
{
	if (auto pManager = RulesExtData::Instance()->VeinholePal.GetConvert())
		Pal = pManager;

	CC_Draw_Shape(Surface, Pal, SHP, FrameIndex, Position, Bounds, Flags, Remap, ZAdjust, ZGradientDescIndex, Brightness
	 , TintColor, ZShape, ZShapeFrame, XOffset, YOffset);
}

DEFINE_FUNCTION_JUMP(CALL, 0x74D5BC, DrawShape_VeinHole);

ASMJIT_PATCH(0x4AD097, DisplayClass_ReadINI_add, 0x6)
{
	const auto nTheater = ScenarioClass::Instance->Theater;
	SmudgeTypeClass::TheaterInit(nTheater);
	VeinholeMonsterClass::TheaterInit(nTheater);
	return 0x4AD0A8;
}

ASMJIT_PATCH(0x74D0D2, VeinholeMonsterClass_AI_SelectParticle, 0x5)
{
	//overriden instructions
	R->Stack(0x2C, R->EDX());
	R->Stack(0x30, R->EAX());
	LEA_STACK(CoordStruct*, pCoord, 0x28);
	const auto pRules = RulesExtData::Instance();
	const auto pParticle = pRules->VeinholeParticle.Get(pRules->DefaultVeinParticle.Get());
	R->EAX(ParticleSystemClass::Instance->SpawnParticle(pParticle, pCoord));
	return 0x74D100;
}

ASMJIT_PATCH(0x5D736E, MultiplayGameMode_GenerateInitForces, 0x6)
{
	return (R->EAX<int>() > 0) ? 0x0 : 0x5D743E;
}

ASMJIT_PATCH(0x5D3ADE, MessageListClass_Init_MessageMax, 0x6)
{
	if (Phobos::Otamaa::IsAdmin)
		R->EAX(14);

	return 0x0;
}

// Skip log spam "Unable to locate scenario %s - No digest info"
//MultiMission_CTOR
DEFINE_JUMP(LJMP, 0x69A797, 0x69A937);

//allow `VeinholeMonster` to be placed anywhere flat
//VeinholeClass_CTOR
DEFINE_JUMP(LJMP, 0x74C688, 0x74C697);

ASMJIT_PATCH(0x4A267D, CreditClass_AI_MissingCurPlayerPtr, 0x6)
{
	if (!HouseClass::CurrentPlayer())
		Debug::FatalError("CurrentPlayer ptr is Missing!");

	return 0x0;
}

ASMJIT_PATCH(0x5FF93F, SpotlightClass_Draw_OutOfboundSurfaceArrayFix, 0x7)
{
	//GET(SpotlightClass*, pThis, EBP);
	GET(int, idx, ECX);

	if (idx > 64)
	{
		//Debug::LogInfo("[0x{}]SpotlightClass with OutOfBoundSurfaceArrayIndex[{}] Fixing!", (void*)pThis, idx);
		idx = 64;
	}

	return 0x0;
}

enum class NewVHPScan : int
{
	None = 0,
	Normal = 1,
	Strong = 2,
	//Threat = 3,
	//Health = 4,
	//Damage = 5,
	//Value = 6,
	//Locked = 7,
	//Non_Infantry = 8,

	count
};

COMPILETIMEEVAL std::array<const char*, (size_t)NewVHPScan::count> NewVHPScanToString
{ {
	{ "None" }
	,{ "Normal" }
	,{ "Strong" }
	//,{ "Threat" }
	//,{ "Health" }
	//,{ "Damage" }
	//,{ "Value" }
	//,{ "Locked" }
	//,{ "Non_Infantry" }
	} };

ASMJIT_PATCH(0x477590, CCINIClass_ReadVHPScan_Replace, 0x6)
{
	GET(CCINIClass*, pThis, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(int, default_val, 0xC);

	INI_EX exINI(pThis);

	int vHp = default_val;

	if (exINI.ReadString(pSection, pKey) > 0)
	{
		for (int i = 0; i < (int)NewVHPScanToString.size(); ++i)
		{
			if (IS_SAME_STR_(exINI.value(), NewVHPScanToString[i]))
			{
				R->EAX(i);
				return 0x477613;
			}
		}

		Debug::INIParseFailed(pSection, pKey, exINI.value(), "Expected valid VHPScan value");
	}

	R->EAX(vHp);
	return 0x477613;
}

ASMJIT_PATCH(0x691A32, ReadScenarion_RemoveInline, 0x5)
{
	LEA_STACK(char*, pName, 0x18);
	R->ESI(GameCreate<ScriptTypeClass>(pName));
	return 0x691B01;
}

// TiberiumTransmogrify is never initialized explitly, thus do that here
ASMJIT_PATCH(0x66748A, RulesClass_CTOR_TiberiumTransmogrify, 6)
{
	GET(RulesClass*, pThis, ESI);
	pThis->TiberiumTransmogrify = 0;
	return 0;
}
