#include "Body.h"
#include <Ext/Side/Body.h>
#include <Utilities/TemplateDef.h>
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

//#include <Ext/TechnoType/Body.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>

#include <Utilities/Macro.h>

#include <Misc/DynamicPatcher/Trails/TrailType.h>


std::unique_ptr<RulesExt::ExtData>  RulesExt::Data = nullptr;
IStream* RulesExt::g_pStm = nullptr;

void RulesExt::Allocate(RulesClass* pThis)
{
	Data = std::make_unique<RulesExt::ExtData>(pThis);
}

void RulesExt::Remove(RulesClass* pThis)
{
	Data = nullptr;
}

void RulesExt::ExtData::Initialize()
{
	AITargetTypesLists.reserve(5);
	AIScriptsLists.reserve(5);
	AIHateHousesLists.reserve(5);
	AIConditionsLists.reserve(5);
	AITriggersLists.reserve(5);
	AI_AutoSellHealthRatio.reserve(3);
	WallTowers.reserve(5);
}

void RulesExt::LoadVeryEarlyBeforeAnyData(RulesClass* pRules, CCINIClass* pINI)
{
}

void RulesExt::LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI)
{
	Data->LoadFromINIFile(pINI, false);
}

// do everything before `TypeData::ReadFromINI` executed
// to makesure everything is properly allocated from the list
void RulesExt::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	ArmorTypeClass::LoadFromINIList_New(pINI);
	ColorTypeClass::LoadFromINIList_New(pINI);
	CursorTypeClass::LoadFromINIList_New(pINI);

	TunnelTypeClass::LoadFromINIList(pINI);
	GenericPrerequisite::AddDefaults();

	// we override it , so it loaded before any type read happen , so all the properties will correcly readed
	//pThis->Read_CrateRules(pINI);
	//pThis->Read_CombatDamage(pINI);
	//pThis->Read_Radiation(pINI);
	//pThis->Read_ElevationModel(pINI);
	//pThis->Read_WallModel(pINI);
	//pThis->Read_AudioVisual(pINI);
	//pThis->Read_SpecialWeapons(pINI);

	ImmunityTypeClass::LoadFromINIList(pINI);
	ArmorTypeClass::EvaluateDefault();

	TrailType::LoadFromINIList(&CCINIClass::INI_Art.get());

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		if (RadTypeClass::Array.empty())
			RadTypeClass::AddDefaults();

		RadTypeClass::LoadFromINIList(pINI);
	}

	if (ShieldTypeClass::Array.empty())
		ShieldTypeClass::AddDefaults();

	ShieldTypeClass::LoadFromINIList(pINI);

	if (HoverTypeClass::Array.empty())
		HoverTypeClass::AddDefaults();

	HoverTypeClass::LoadFromINIList(pINI);

	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
	DigitalDisplayTypeClass::LoadFromINIList(pINI);

	Data->LoadBeforeTypeData(pThis, pINI);
}

#include <Ext/SWType/Body.h>

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	INI_EX iniEX(pINI);
	auto pData = RulesExt::Global();

	if (!Data->ElectricDeath)
		Data->ElectricDeath = AnimTypeClass::Find("ELECTRO");

	if (!Data->DefaultParaPlane)
		Data->DefaultParaPlane = AircraftTypeClass::Find(GameStrings::PDPLANE());

	Data->DefaultVeinParticle = ParticleTypeClass::Find(GameStrings::GASCLUDM1());
	Data->DefaultSquidAnim = AnimTypeClass::Find(GameStrings::SQDG());

	if (!Data->CarryAll_LandAnim)
		Data->CarryAll_LandAnim = AnimTypeClass::Find(GameStrings::CARYLAND());

	if (!Data->DropShip_LandAnim)
		Data->DropShip_LandAnim = AnimTypeClass::Find(GameStrings::DROPLAND());

	if (!Data->DropPodTrailer)
		Data->DropPodTrailer = AnimTypeClass::Find(GameStrings::SMOKEY());

	pData->Bounty_Enablers.Read(iniEX, GENERAL_SECTION, "BountyEnablers");
	pData->Bounty_Display.Read(iniEX, AUDIOVISUAL_SECTION, "BountyDisplay");
	pData->CloakAnim.Read(iniEX, AUDIOVISUAL_SECTION, "CloakAnim");
	pData->DecloakAnim.Read(iniEX, AUDIOVISUAL_SECTION, "DecloakAnim");
	pData->Cloak_KickOutParasite.Read(iniEX, GameStrings::CombatDamage, "Cloak.KickOutParasite");

	pData->Buildings_DefaultDigitalDisplayTypes.Read(iniEX, GameStrings::AudioVisual, "Buildings.DefaultDigitalDisplayTypes");
	pData->Infantry_DefaultDigitalDisplayTypes.Read(iniEX, GameStrings::AudioVisual, "Infantry.DefaultDigitalDisplayTypes");
	pData->Vehicles_DefaultDigitalDisplayTypes.Read(iniEX, GameStrings::AudioVisual, "Vehicles.DefaultDigitalDisplayTypes");
	pData->Aircraft_DefaultDigitalDisplayTypes.Read(iniEX, GameStrings::AudioVisual, "Aircraft.DefaultDigitalDisplayTypes");

	// TODO : move this to its main place , not modifiable thru map ,..
	if (pINI == CCINIClass::INI_Rules)
	{
		char buffer[0x30];

		InfantryTypeClass::Array->for_each([&](InfantryTypeClass* pInfType) {
			WarheadTypeClass::Array->for_each([&](WarheadTypeClass* pWarhead) {
				if (auto const pExt = WarheadTypeExt::ExtMap.TryFind(pWarhead))
				{
					Nullable<AnimTypeClass*> nBuffer {};
					IMPL_SNPRNINTF(buffer, sizeof(buffer), "%s.InfDeathAnim", pInfType->ID);
					nBuffer.Read(iniEX, pWarhead->ID, buffer);
		
					if (!nBuffer.isset() || !nBuffer.Get())
						return;
		
					//Debug::Log("Found specific InfDeathAnim for [WH : %s Inf : %s Anim %s]\n", pWarhead->ID, pInfType->ID, nBuffer->ID);
					pExt->InfDeathAnims[pInfType->ArrayIndex] = nBuffer;
				}
			});
		});

		SuperWeaponTypeClass::Array->for_each([&](SuperWeaponTypeClass* pSWType) {
			if(const auto pSuperExt = SWTypeExt::ExtMap.TryFind(pSWType)) {
		
				if (pSuperExt->Aux_Techno.empty())
					return;
		
				TechnoTypeClass::Array->for_each([&](TechnoTypeClass* pType) {
					if (pType->WhatAmI() == AbstractType::BuildingType)
						return;
		
					if(auto pTypeExt = TechnoTypeExt::ExtMap.TryFind(pType)) {
						if (pSuperExt->Aux_Techno.Contains(pType) && !pTypeExt->Linked_SW.Contains(pSWType))
								pTypeExt->Linked_SW.push_back(pSWType);
					}
				});
			}
		});

		//for (auto pType : *TechnoTypeClass::Array)
		//{
		//	if (auto pTypeExt = TechnoTypeExt::ExtMap.TryFind(pType)){
		//
		//		char Temp_[0x80];
		//		pTypeExt->SpecificExpFactor.clear();
		//		for (int i = 0;; ++i)
		//		{
		//			Nullable<TechnoTypeClass*> Temp {};
		//			IMPL_SNPRNINTF(Temp_, sizeof(Temp_), "SpecificExperience%d.Type", i);
		//				Temp.Read(iniEX, pType->ID, Temp_);
		//
		//			if (!Temp)
		//				break;
		//
		//			IMPL_SNPRNINTF(Temp_, sizeof(Temp_), "SpecificExperience%d.Factor", i);
		//			pTypeExt->SpecificExpFactor[Temp.Get()].Read(iniEX, pType->ID, Temp_);
		//		}
		//	}
		//}
	}

}

// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
void RulesExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{

}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	//Allocate Default bullet
	//int len = pINI->GetKeyCount("Projectiles");
	//for (int i = 0; i < len; ++i) {
	//	const char* key = pINI->GetKeyName("Projectiles", i);
	//	if (pINI->ReadString("Projectiles", key, "", Phobos::readBuffer)) {
	//		BulletTypeClass::FindOrAllocate(Phobos::readBuffer);
	//	}
	//}

	BulletTypeClass::FindOrAllocate(DEFAULT_STR2);
	GenericPrerequisite::LoadFromINIList_New(pINI);

	INI_EX exINI(pINI);

#pragma region Otamaa

	this->AllowParallelAIQueues.Read(exINI, GLOBALCONTROLS_SECTION, "AllowParallelAIQueues");
	this->ForbidParallelAIQueues_Infantry.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Infantry");
	this->ForbidParallelAIQueues_Vehicle.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Vehicle");
	this->ForbidParallelAIQueues_Navy.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Navy");
	this->ForbidParallelAIQueues_Aircraft.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Aircraft");
	this->ForbidParallelAIQueues_Building.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Building");

	this->EngineerDamage.Read(exINI, GENERAL_SECTION, "EngineerDamage");
	this->EngineerAlwaysCaptureTech.Read(exINI, GENERAL_SECTION, "EngineerAlwaysCaptureTech");
	this->EngineerDamageCursor.Read(exINI, GENERAL_SECTION, "EngineerDamageCursor");

	this->DefaultParaPlane.Read(exINI, GENERAL_SECTION, "ParadropPlane", true);
	this->VeinholeParticle.Read(exINI, AUDIOVISUAL_SECTION, "VeinholeSpawnParticleType", true);
	this->CarryAll_LandAnim.Read(exINI, AUDIOVISUAL_SECTION, "LandingAnim.Carryall", true);
	this->DropShip_LandAnim.Read(exINI, AUDIOVISUAL_SECTION, "LandingAnim.Dropship", true);
	this->Aircraft_LandAnim.Read(exINI, AUDIOVISUAL_SECTION, "LandingAnim.Aircraft", true);
	this->Aircraft_TakeOffAnim.Read(exINI, AUDIOVISUAL_SECTION, "TakeOffAnim.Aircraft", true);

	this->DropPodTrailer.Read(exINI, GENERAL_SECTION, "DropPodTrailer", true);
	this->ElectricDeath.Read(exINI, AUDIOVISUAL_SECTION, "InfantryElectrocuted");

	this->DropPodTypes.Read(exINI, GENERAL_SECTION, "DropPodTypes");
	this->DropPodMinimum.Read(exINI, GENERAL_SECTION, "DropPodMinimum");
	this->DropPodMaximum.Read(exINI, GENERAL_SECTION, "DropPodMaximum");
	this->ReturnStructures.Read(exINI, GENERAL_SECTION, "ReturnStructures");

	this->MessageSilosNeeded.Read(exINI, GENERAL_SECTION, "Message.SilosNeeded");

	this->HunterSeekerBuildings.Read(exINI, SPECIALWEAPON_SECTION, "HSBuilding");
	this->HunterSeekerDetonateProximity.Read(exINI, GENERAL_SECTION, "HunterSeekerDetonateProximity");
	this->HunterSeekerDescendProximity.Read(exINI, GENERAL_SECTION, "HunterSeekerDescendProximity");
	this->HunterSeekerAscentSpeed.Read(exINI, GENERAL_SECTION, "HunterSeekerAscentSpeed");
	this->HunterSeekerDescentSpeed.Read(exINI, GENERAL_SECTION, "HunterSeekerDescentSpeed");
	this->HunterSeekerEmergeSpeed.Read(exINI, GENERAL_SECTION, "HunterSeekerEmergeSpeed");
	this->Units_UnSellable.Read(exINI, GENERAL_SECTION, "UnitsUnsellable");
	this->DrawTurretShadow.Read(exINI, AUDIOVISUAL_SECTION, "DrawTurretShadow");
	this->AnimRemapDefaultColorScheme.Read(exINI, AUDIOVISUAL_SECTION, "AnimRemapDefaultColorScheme");

	if (pThis->WallTower)
		this->WallTowers.push_back(pThis->WallTower);

#pragma endregion

	detail::ParseVector(exINI, this->AITargetTypesLists, "AITargetTypes");
	detail::ParseVector<ScriptTypeClass*, true>(exINI, this->AIScriptsLists, "AIScriptsList");
	detail::ParseVector<HouseTypeClass*, true>(exINI, this->AIHateHousesLists, "AIHateHousesList");
	detail::ParseVector<HouseTypeClass*, true>(exINI, this->AIHousesLists, "AIHousesList");
	detail::ParseVector(exINI, this->AIConditionsLists, "AIConditionsList", true, false, "/");
	detail::ParseVector<AITriggerTypeClass*, true>(exINI, this->AITriggersLists, "AITriggersList");

	this->StealthSpeakDelay.Read(exINI, AUDIOVISUAL_SECTION, "StealthSpeakDelay");
	this->SubterraneanSpeakDelay.Read(exINI, AUDIOVISUAL_SECTION, "SubterraneanSpeakDelay");
	this->RandomCrateMoney.Read(exINI, GameStrings::CrateRules, "RandomCrateMoney");
	this->ChronoSparkleDisplayDelay.Read(exINI, GENERAL_SECTION, "ChronoSparkleDisplayDelay");
	this->ChronoSparkleBuildingDisplayPositions.Read(exINI, GENERAL_SECTION, "ChronoSparkleBuildingDisplayPositions");
	this->RepairStopOnInsufficientFunds.Read(exINI, GENERAL_SECTION, "RepairStopOnInsufficientFunds");

	this->BerserkROFMultiplier.Read(exINI, COMBATDAMAGE_SECTION, "BerserkROFMultiplier");
	this->TeamRetaliate.Read(exINI, GENERAL_SECTION, "TeamRetaliate");
	this->AI_CostMult.Read(exINI, GENERAL_SECTION, "AICostMult");

	this->DeactivateDim_Powered.Read(exINI, AUDIOVISUAL_SECTION, "DeactivateDimPowered");
	this->DeactivateDim_EMP.Read(exINI, AUDIOVISUAL_SECTION, "DeactivateDimEMP");
	this->DeactivateDim_Operator.Read(exINI, AUDIOVISUAL_SECTION, "DeactivateDimOperator");

#pragma region Otamaa
	this->AI_SpyMoneyStealPercent.Read(exINI, GENERAL_SECTION, "AI.SpyMoneyStealPercent");
	this->AutoAttackICedTarget.Read(exINI, COMBATDAMAGE_SECTION, "Firing.AllowICedTargetForAI");
	this->NukeWarheadName.Read(exINI.GetINI(), GameStrings::SpecialWeapons(), "NukeWarhead");
	this->AI_AutoSellHealthRatio.Read(exINI, GENERAL_SECTION, "AI.AutoSellHealthRatio");
	this->Building_PlacementPreview.Read(exINI, AUDIOVISUAL_SECTION, "ShowBuildingPlacementPreview");
	this->DisablePathfindFailureLog.Read(exINI, GENERAL_SECTION, "DisablePathfindFailureLog");
	this->CreateSound_PlayerOnly.Read(exINI, AUDIOVISUAL_SECTION, "CreateSound.AffectOwner");
	this->DoggiePanicMax.Read(exINI, COMBATDAMAGE_SECTION, "DoggiePanicMax");
	this->HunterSeeker_Damage.Read(exINI, COMBATDAMAGE_SECTION, "HunterSeekerDamage");
	this->AutoRepelAI.Read(exINI, COMBATDAMAGE_SECTION, "AutoRepel");
	this->AutoRepelPlayer.Read(exINI, COMBATDAMAGE_SECTION, "PlayerAutoRepel");
	this->AIFriendlyDistance.Read(exINI, GENERAL_SECTION, "AIFriendlyDistance");

	this->MyPutData.Read(exINI, GENERAL_SECTION);

#pragma endregion
	this->Storage_TiberiumIndex.Read(exINI, GENERAL_SECTION, "Storage.TiberiumIndex");

	this->RadApplicationDelay_Building.Read(exINI, RADIATION_SECTION, "RadApplicationDelay.Building");
	this->RadWarhead_Detonate.Read(exINI, RADIATION_SECTION, "RadSiteWarhead.Detonate");
	this->RadHasOwner.Read(exINI, RADIATION_SECTION, "RadHasOwner");
	this->RadHasInvoker.Read(exINI, RADIATION_SECTION, "RadHasInvoker");
	this->IronCurtain_SyncDeploysInto.Read(exINI, COMBATDAMAGE_SECTION, "IronCurtain.KeptOnDeploy");
	this->ROF_RandomDelay.Read(exINI, GameStrings::CombatDamage, "ROF.RandomDelay");

	this->Pips_Shield.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield");
	this->Pips_Shield_Buildings.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield.Building");
	this->MissingCameo.Read(pINI, AUDIOVISUAL_SECTION, "MissingCameo");
	this->JumpjetAllowLayerDeviation.Read(exINI, JUMPJET_SECTION, "AllowLayerDeviation");
	this->JumpjetTurnToTarget.Read(exINI, JUMPJET_SECTION, "TurnToTarget");
	this->JumpjetCrash_Rotate.Read(exINI, JUMPJET_SECTION, "CrashRotate");

	this->PlacementGrid_TranslucentLevel.Read(exINI, AUDIOVISUAL_SECTION, "BuildingPlacementGrid.TranslucentLevel");
	this->BuildingPlacementPreview_TranslucentLevel.Read(exINI, AUDIOVISUAL_SECTION, "BuildingPlacementPreview.DefaultTranslucentLevel");
	this->Pips_Shield.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield");
	this->Pips_Shield_Background_SHP.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield.Background");
	this->Pips_Shield_Building.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield.Building");
	this->Pips_Shield_Building_Empty.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield.Building.Empty");

	this->Pips_SelfHeal_Infantry.Read(exINI, AUDIOVISUAL_SECTION, "Pips.SelfHeal.Infantry");
	this->Pips_SelfHeal_Units.Read(exINI, AUDIOVISUAL_SECTION, "Pips.SelfHeal.Units");
	this->Pips_SelfHeal_Buildings.Read(exINI, AUDIOVISUAL_SECTION, "Pips.SelfHeal.Buildings");
	this->Pips_SelfHeal_Infantry_Offset.Read(exINI, AUDIOVISUAL_SECTION, "Pips.SelfHeal.Infantry.Offset");
	this->Pips_SelfHeal_Units_Offset.Read(exINI, AUDIOVISUAL_SECTION, "Pips.SelfHeal.Units.Offset");
	this->Pips_SelfHeal_Buildings_Offset.Read(exINI, AUDIOVISUAL_SECTION, "Pips.SelfHeal.Buildings.Offset");

	this->Pips_Generic_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Generic.Size");
	this->Pips_Generic_Buildings_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Generic.Buildings.Size");
	this->Pips_Ammo_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Ammo.Size");
	this->Pips_Ammo_Buildings_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Ammo.Buildings.Size");

	this->ToolTip_Background_Color.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.Background.BlurSize");

	this->Crate_LandOnly.Read(exINI, GameStrings::CrateRules(), "Crate.LandOnly");

	this->InfantryGainSelfHealCap.Read(exINI, GENERAL_SECTION, "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GENERAL_SECTION, "UnitsGainSelfHealCap");

	this->EnemyInsignia.Read(exINI, GENERAL_SECTION, "EnemyInsignia");
	this->DisguiseBlinkingVisibility.Read(exINI, GENERAL_SECTION, "DisguiseBlinkingVisibility");

	this->UseSelectBrd.Read(exINI, AUDIOVISUAL_SECTION, "UseSelectBrd");
	this->SHP_SelectBrdSHP_INF.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.SHP.Infantry");
	this->SHP_SelectBrdPAL_INF.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.PAL.Infantry");
	this->SelectBrd_Frame_Infantry.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.Frame.Infantry");
	this->SelectBrd_DrawOffset_Infantry.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.DrawOffset.Infantry");
	this->SHP_SelectBrdSHP_UNIT.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.SHP.Unit");
	this->SHP_SelectBrdPAL_UNIT.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.PAL.Unit");
	this->SelectBrd_Frame_Unit.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.Frame.Unit");
	this->SelectBrd_DrawOffset_Unit.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.DrawOffset.Unit");
	this->SelectBrd_DefaultTranslucentLevel.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.DefaultTranslucentLevel");
	this->SelectBrd_DefaultShowEnemy.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.DefaultShowEnemy");

	this->VeteranFlashTimer.Read(exINI, AUDIOVISUAL_SECTION, "VeteranFlashTimer");

	this->Tiberium_DamageEnabled.Read(exINI, GENERAL_SECTION, "TiberiumDamageEnabled");
	this->Tiberium_HealEnabled.Read(exINI, GENERAL_SECTION, "TiberiumHealEnabled");
	this->Tiberium_ExplosiveWarhead.Read(exINI, COMBATDAMAGE_SECTION, "TiberiumExplosiveWarhead");
	this->Tiberium_ExplosiveAnim.Read(exINI, AUDIOVISUAL_SECTION, "TiberiumExplosiveAnim");
	this->OverlayExplodeThreshold.Read(exINI, GENERAL_SECTION, "OverlayExplodeThreshold");
	this->AlliedSolidTransparency.Read(exINI, COMBATDAMAGE_SECTION, "AlliedSolidTransparency");
	this->DecloakSound.Read(exINI, AUDIOVISUAL_SECTION, "DecloakSound");
	this->IC_Flash.Read(exINI, AUDIOVISUAL_SECTION, "IronCurtainFlash");

	this->ChainReact_Multiplier.Read(exINI, COMBATDAMAGE_SECTION, "ChainReact.Multiplier");
	this->ChainReact_SpreadChance.Read(exINI, COMBATDAMAGE_SECTION, "ChainReact.SpreadChance");
	this->ChainReact_MinDelay.Read(exINI, COMBATDAMAGE_SECTION, "ChainReact.MinDelay");
	this->ChainReact_MaxDelay.Read(exINI, COMBATDAMAGE_SECTION, "ChainReact.MaxDelay");

	this->ChronoInfantryCrush.Read(exINI, GENERAL_SECTION, "ChronoInfantryCrush");

	this->DamageAirConsiderBridges.Read(exINI, COMBATDAMAGE_SECTION, "DamageAirConsiderBridges");
	this->DiskLaserAnimEnabled.Read(exINI, AUDIOVISUAL_SECTION, "DiskLaserAnimEnabled");

	//TODO :Disabled atm
	this->NewTeamsSelector.Read(exINI, "AI", "NewTeamsSelector");
	this->NewTeamsSelector_SplitTriggersByCategory.Read(exINI, "AI", "NewTeamsSelector.SplitTriggersByCategory");
	this->NewTeamsSelector_EnableFallback.Read(exINI, "AI", "NewTeamsSelector.EnableFallback");
	this->NewTeamsSelector_MergeUnclassifiedCategoryWith.Read(exINI, "AI", "NewTeamsSelector.MergeUnclassifiedCategoryWith");
	this->NewTeamsSelector_UnclassifiedCategoryPercentage.Read(exINI, "AI", "NewTeamsSelector.UnclassifiedCategoryPercentage");
	this->NewTeamsSelector_GroundCategoryPercentage.Read(exINI, "AI", "NewTeamsSelector.GroundCategoryPercentage");
	this->NewTeamsSelector_AirCategoryPercentage.Read(exINI, "AI", "NewTeamsSelector.AirCategoryPercentage");
	this->NewTeamsSelector_NavalCategoryPercentage.Read(exINI, "AI", "NewTeamsSelector.NavalCategoryPercentage");
	//

	this->EnemyWrench.Read(exINI, GENERAL_SECTION, "EnemyWrench");
	this->Bounty_Value_Option.Read(exINI, GENERAL_SECTION, "BountyRewardOption");
	this->EMPAIRecoverMission.Read(exINI, COMBATDAMAGE_SECTION, "EMPAIRecoverMission");
	this->TimerBlinkColorScheme.Read(exINI, GameStrings::AudioVisual, "TimerBlinkColorScheme");
}

void RulesExt::LoadEarlyOptios(RulesClass* pThis, CCINIClass* pINI)
{ }

void RulesExt::LoadEarlyBeforeColor(RulesClass* pThis, CCINIClass* pINI)
{ }

bool RulesExt::DetailsCurrentlyEnabled()
{
	// not only checks for the min frame rate from the rules, but also whether
	// the low frame rate is actually desired. in that case, don't reduce.
	auto const current = FPSCounter::CurrentFrameRate();
	auto const wanted = static_cast<unsigned int>(
		60 / std::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));

	return current >= wanted || current >= Detail::GetMinFrameRate();
}

bool RulesExt::DetailsCurrentlyEnabled(int const minDetailLevel)
{
	return GameOptionsClass::Instance->DetailLevel >= minDetailLevel
		&& DetailsCurrentlyEnabled();
}

void RulesExt::LoadBeforeGeneralData(RulesClass* pThis, CCINIClass* pINI)
{
	//Debug::Log(__FUNCTION__" Called ! \n");
}

void RulesExt::LoadAfterAllLogicData(RulesClass* pThis, CCINIClass* pINI)
{
	//Debug::Log(__FUNCTION__" Called ! \n");
}

// =============================
// load / save

template <typename T>
void RulesExt::ExtData::Serialize(T& Stm)
{
	//Debug::Log("Processing RulesExt ! \n");

	Stm
		.Process(this->Initialized)

		.Process(Phobos::Config::ArtImageSwap)
		.Process(Phobos::Otamaa::DisableCustomRadSite)
		.Process(Phobos::Config::ShowTechnoNamesIsActive)
		.Process(Phobos::Misc::CustomGS)

		.Process(this->Pips_Shield)
		.Process(this->Pips_Shield_Buildings)

		.Process(this->RadApplicationDelay_Building)
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

		.Process(this->InfantryGainSelfHealCap)
		.Process(this->UnitsGainSelfHealCap)
		.Process(this->EnemyInsignia)
		.Process(this->DisguiseBlinkingVisibility)

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
		.Process(this->IronCurtain_SyncDeploysInto)
		.Process(this->ROF_RandomDelay)

		.Process(this->ToolTip_Background_Color)
		.Process(this->ToolTip_Background_Opacity)
		.Process(this->ToolTip_Background_BlurSize)

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
		.Process(this->AI_AutoSellHealthRatio)

		.Process(this->CarryAll_LandAnim)
		.Process(this->DropShip_LandAnim)
		.Process(this->Aircraft_LandAnim)
		.Process(this->Aircraft_TakeOffAnim)

		.Process(this->DisablePathfindFailureLog)
		.Process(this->CreateSound_PlayerOnly)

		.Process(this->CivilianSideIndex)
		.Process(this->SpecialCountryIndex)
		.Process(this->NeutralCountryIndex)

		.Process(this->WallTowers)
		.Process(this->AutoAttackICedTarget)
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
		;

	MyPutData.Serialize(Stm);

}

// =============================
// container hooks

DEFINE_HOOK(0x667A1D, RulesClass_CTOR, 0x5)
{
	GET(RulesClass*, pItem, ESI);

	RulesExt::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x667A30, RulesClass_DTOR, 0x5)
{
	GET(RulesClass*, pItem, ECX);

	RulesExt::Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x674730, RulesClass_SaveLoad_Prefix, 0x6)
DEFINE_HOOK(0x675210, RulesClass_SaveLoad_Prefix, 0x5)
{
	//GET(RulesClass*, pItem, ECX);
	GET_STACK(IStream*, pStm, 0x4);

	RulesExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x678841, RulesClass_Load_Suffix, 0x7)
{
	auto buffer = RulesExt::Global();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(RulesExt::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(RulesExt::ExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x675205, RulesClass_Save_Suffix, 0x8)
{
	auto buffer = RulesExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(RulesExt::ExtData::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(RulesExt::g_pStm);

	return 0;
}

DEFINE_HOOK(0x52C9C4, GameInit_ReReadStuffs, 0x6)
{
	//AnimTypeClass::Array->ForEach([](AnimTypeClass* pType) {
	//	pType->LoadFromINI(&CCINIClass::INI_Art());
	//});

	//BuildingTypeClass::Array->ForEach([](BuildingTypeClass* pType) {
	//	pType->LoadFromINI(CCINIClass::INI_Rules());
	//});

	//SuperWeaponTypeClass::Array->ForEach([](SuperWeaponTypeClass* pType) {
	//	pType->LoadFromINI(CCINIClass::INI_Rules());
	//});

	//WeaponTypeClass::Array->ForEach([](WeaponTypeClass* pType) {
	//	pType->LoadFromINI(CCINIClass::INI_Rules());
	//});

	//WarheadTypeClass::Array->ForEach([](WarheadTypeClass* pType) {
	//	pType->LoadFromINI(CCINIClass::INI_Rules());
	//});

	return 0x52CA37;
}

// DEFINE_HOOK(0x52D149, InitRules_PostInit, 0x5)
// {
// 	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
// 	return 0;
// }

//DEFINE_HOOK(0x668BF0, RulesClass_Addition, 0x5)
//{
//	GET(RulesClass*, pItem, ECX);
//	GET_STACK(CCINIClass*, pINI, 0x4);
//
//	//	RulesClass::Initialized = false;
//	RulesExt::LoadFromINIFile(pItem, pINI);
//
//	return 0;
//}

DEFINE_HOOK(0x679A15, RulesData_LoadBeforeTypeData, 0x6)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	SideClass::Array->for_each([pINI](SideClass* pSide) {
		SideExt::ExtMap.LoadFromINI(pSide, pINI, !pINI->GetSection(pSide->ID));
	});

	// All TypeClass Created but not yet read INI
	//	RulesClass::Initialized = true;
	RulesExt::LoadBeforeTypeData(pItem, pINI);

	return 0;
}

#include <Ext/WarheadType/Body.h>

DEFINE_HOOK(0x679CAF, RulesData_LoadAfterTypeData, 0x5)
{
	RulesClass* pItem = RulesClass::Instance();
	GET(CCINIClass*, pINI, ESI);

	RulesExt::LoadAfterTypeData(pItem, pINI);

	return 0;
}

//DEFINE_HOOK(0x66D530, RulesData_LoadBeforeGeneralData, 0x6)
//{
//	GET(RulesClass*, pItem, ECX);
//	GET_STACK(CCINIClass*, pINI, 0x4);
//
//	RulesExt::LoadBeforeGeneralData(pItem, pINI);
//
//	return 0;
//}

//DEFINE_HOOK(0x668F6A, RulesData_LoadAfterAllLogicData, 0x5)
//{
//	GET(RulesClass*, pItem, EDI);
//	GET(CCINIClass*, pINI, ESI);
//
//	RulesExt::LoadAfterAllLogicData(pItem, pINI);
//
//	return 0;
//}

DEFINE_HOOK(0x68684A, Game_ReadScenario_FinishReadingScenarioINI, 0x7) //9
{
	if (R->AL()) //ScenarioLoadSucceed
	{
		//pre iterate this important indexes
		//so we dont need to do lookups with name multiple times
		//these function only executed when ScenarioClass::ReadScenario return true (AL)
		if (const auto pRulesGlobal = RulesExt::Global())
		{
			pRulesGlobal->CivilianSideIndex = SideClass::FindIndexById(GameStrings::Civilian());
			//Debug::Log("Finding Civilian Side Index[%d] ! \n" , pRulesGlobal->CivilianSideIndex);
			pRulesGlobal->NeutralCountryIndex = HouseTypeClass::FindIndexByIdAndName(GameStrings::Neutral());
			//Debug::Log("Finding Neutral Country Index[%d] ! \n", pRulesGlobal->NeutralCountryIndex);
			pRulesGlobal->SpecialCountryIndex = HouseTypeClass::FindIndexByIdAndName(GameStrings::Special());
			//Debug::Log("Finding Special Country Index[%d] ! \n", pRulesGlobal->SpecialCountryIndex);
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x683E21, ScenarioClass_StartScenario_LogHouses, 0x5)
{
	Debug::Log("Scenario Name [%s] , Map Name [%s] \n", ScenarioClass::Instance->FileName, SessionClass::Instance->ScenarioFilename);

	HouseClass::Array->for_each([](HouseClass* it) {
		const auto pType = HouseTypeClass::Array->GetItemOrDefault(it->Type->ArrayIndex);
		Debug::Log("Player Name: %s IsCurrentPlayer: %u; ColorScheme: %s; ID: %d; HouseType: %s; Edge: %d; StartingAllies: %d; Startspot: %d,%d; Visionary: %d; MapIsClear: %u; Money: %d\n",
		it->PlainName ? it->PlainName : NONE_STR,
		it->IsHumanPlayer,
		ColorScheme::Array->GetItem(it->ColorSchemeIndex)->ID,
		it->ArrayIndex,
		pType ? pType->Name : NONE_STR,
		it->Edge,
		(int)it->StartingAllies.data,
		it->StartingCell.X,
		it->StartingCell.Y,
		it->Visionary,
		it->MapIsClear,
		it->Available_Money()
		);
	});

	return 0x0;
}

//DEFINE_HOOK(0x679C92, RulesClass_ReadAllFromINI_ReIterate, 0x7)
//{
//	GET(CCINIClass*, pINI, ESI);
//
//	for (auto pAnim : *AnimTypeClass::Array) {
//		pAnim->LoadFromINI(pINI);
//	}
//
//	return 0x0;
//}

//DEFINE_SKIP_HOOK(0x668F2B, RulesClass_Process_RemoveThese, 0x8, 668F63);
//DEFINE_JUMP(LJMP, 0x668F2B ,0x668F63); // move all these reading before type reading

//void NAKED RulesClass_Process_SpecialWeapon_RemoveWHReadingDuplicate_RET() {
//	POP_REG(ebx);
//	JMP(0x6691B7);
//}
//
//DEFINE_HOOK(0x669193, RulesClass_Process_SpecialWeapon_RemoveWHReadingDuplicate, 0x9) {
//	return (int)RulesClass_Process_SpecialWeapon_RemoveWHReadingDuplicate_RET;
//}
//DEFINE_JUMP(LJMP, 0x66919B, 0x6691B7); // remove reading warhead from `SpecialWeapon`
//DEFINE_HOOK(0x687C16, INIClass_ReadScenario_ValidateThings, 6)
//{
//	return 0x0;
//}