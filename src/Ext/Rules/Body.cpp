#include "Body.h"
#include <Ext/Side/Body.h>
#include <Utilities/TemplateDef.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/ArmorTypeClass.h>
#include <New/Type/HoverTypeClass.h>

//#include <Ext/TechnoType/Body.h>

#include <Ext/BuildingType/Body.h>

#include <Utilities/Macro.h>
#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailType.h>
#endif

std::unique_ptr<RulesExt::ExtData> RulesExt::Data = nullptr;

void RulesExt::Allocate(RulesClass* pThis)
{
	Data = std::make_unique<RulesExt::ExtData>(pThis);
	if (Data)
		Data->EnsureConstanted();
}

void RulesExt::Remove(RulesClass* pThis)
{
	if (Data)
		Data->Uninitialize();

	Data = nullptr;
}

void RulesExt::ExtData::InitializeConstants()
{
	AITargetTypesLists.reserve(5);
	AIScriptsLists.reserve(5);
	AIHousesLists.reserve(5);
	AIConditionsLists.reserve(5);
	AITriggersLists.reserve(5);
	GenericPrerequisitesData.reserve(20);
	AI_AutoSellHealthRatio.reserve(3);
	WallTowers.reserve(5);
}

void RulesExt::LoadVeryEarlyBeforeAnyData(RulesClass* pRules, CCINIClass* pINI)
{
	ArmorTypeClass::LoadFromINIList_New(pINI, false);
	ColorTypeClass::LoadFromINIList_New(pINI, false);
}

void RulesExt::LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI)
{
	//Debug::Log(__FUNCTION__" Reading ArmorType ! \n");
	ArmorTypeClass::LoadFromINIList_New(pINI, false);
	//Debug::Log(__FUNCTION__" Reading ColorType ! \n");
	ColorTypeClass::LoadFromINIList_New(pINI, false);

		//Debug::Log(__FUNCTION__" Called ! \n");
	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		//Debug::Log(__FUNCTION__" AddDefault RadType ! \n");
		RadTypeClass::AddDefaults();
	}

	//Debug::Log(__FUNCTION__" AddDefault ShieldType ! \n");
	ShieldTypeClass::AddDefaults();

	//Debug::Log(__FUNCTION__" AddDefault HoverType ! \n");
	HoverTypeClass::AddDefaults();

	Data->LoadFromINI(pINI);
}

void RulesExt::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	for (auto& pArmor : ArmorTypeClass::Array) {
		pArmor->EvaluateDefault();
	}

#ifdef COMPILE_PORTED_DP_FEATURES
	TrailType::LoadFromINIList(&CCINIClass::INI_Art.get(), false);
#endif

	if (!Phobos::Otamaa::DisableCustomRadSite)
		RadTypeClass::LoadFromINIList(pINI);

	ShieldTypeClass::LoadFromINIList(pINI);
	HoverTypeClass::LoadFromINIList(pINI);
	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	if (pINI == CCINIClass::INI_Rules)
		Data->InitializeAfterTypeData(pThis);


	Data->LoadAfterTypeData(pThis, pINI);
}

// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
void RulesExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{

}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	//Allocate Default bullet
	if(!BulletTypeClass::Find(DEFAULT_STR2))
	GameCreate<BulletTypeClass>(DEFAULT_STR2);

	INI_EX exINI(pINI);

	this->StealthSpeakDelay.Read(exINI, AUDIOVISUAL_SECTION, "StealthSpeakDelay");
	this->SubterraneanSpeakDelay.Read(exINI, AUDIOVISUAL_SECTION, "SubterraneanSpeakDelay");

	this->BerserkROFMultiplier.Read(exINI, COMBATDAMAGE_SECTION, "BerserkROFMultiplier");
	this->TeamRetaliate.Read(exINI, GENERAL_SECTION, "TeamRetaliate");

#pragma region Otamaa
	this->AutoAttackICedTarget.Read(exINI, COMBATDAMAGE_SECTION, "Firing.AllowICedTargetForAI");
	this->NukeWarheadName.Read(exINI.GetINI(), "SpecialWeapons", "NukeWarhead");
	this->AI_AutoSellHealthRatio.Read(exINI, GENERAL_SECTION, "AI.AutoSellHealthRatio");
	this->Building_PlacementPreview.Read(exINI, AUDIOVISUAL_SECTION, "ShowBuildingPlacementPreview");
	this->DisablePathfindFailureLog.Read(exINI, GENERAL_SECTION, "DisablePathfindFailureLog");
	this->CreateSound_PlayerOnly.Read(exINI, AUDIOVISUAL_SECTION, "CreateSound.PlayerOnly");
	this->DoggiePanicMax.Read(exINI, COMBATDAMAGE_SECTION, "DoggiePanicMax");
	this->HunterSeeker_Damage.Read(exINI, COMBATDAMAGE_SECTION, "HunterSeekerDamage");
	this->AutoRepelAI.Read(exINI, COMBATDAMAGE_SECTION, "AutoRepel");
	this->AutoRepelPlayer.Read(exINI, COMBATDAMAGE_SECTION, "PlayerAutoRepel");
	this->AIFriendlyDistance.Read(exINI, GENERAL_SECTION, "AIFriendlyDistance");

#ifdef COMPILE_PORTED_DP_FEATURES
	this->MyPutData.Read(exINI, GENERAL_SECTION);
#endif
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

	this->ToolTip_Background_Color.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.Background.BlurSize");

	this->Crate_LandOnly.Read(exINI, GameStrings::CrateRules(), "Crate.LandOnly");

	this->InfantryGainSelfHealCap.Read(exINI, GENERAL_SECTION, "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GENERAL_SECTION, "UnitsGainSelfHealCap");

	this->EnemyInsignia.Read(exINI, GENERAL_SECTION, "EnemyInsignia");
	this->ShowAllyDisguiseBlinking.Read(exINI, GENERAL_SECTION, "ShowAllyDisguiseBlinking");

	this->UseSelectBrd.Read(exINI, AUDIOVISUAL_SECTION, "UseSelectBrd");
	this->SHP_SelectBrdSHP_INF.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.SHP.Infantry");
	this->SHP_SelectBrdPAL_INF.Read(pINI, AUDIOVISUAL_SECTION, "SelectBrd.PAL.Infantry");
	this->SelectBrd_Frame_Infantry.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.Frame.Infantry");
	this->SelectBrd_DrawOffset_Infantry.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.DrawOffset.Infantry");
	this->SHP_SelectBrdSHP_UNIT.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.SHP.Unit");
	this->SHP_SelectBrdPAL_UNIT.Read(pINI, AUDIOVISUAL_SECTION, "SelectBrd.PAL.Unit");
	this->SelectBrd_Frame_Unit.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.Frame.Unit");
	this->SelectBrd_DrawOffset_Unit.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.DrawOffset.Unit");
	this->SelectBrd_DefaultTranslucentLevel.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.DefaultTranslucentLevel");
	this->SelectBrd_DefaultShowEnemy.Read(exINI, AUDIOVISUAL_SECTION, "SelectBrd.DefaultShowEnemy");

	this->VeteranFlashTimer.Read(exINI, AUDIOVISUAL_SECTION, "VeteranFlashTimer");

	this->Tiberium_DamageEnabled.Read(exINI, GENERAL_SECTION, "TiberiumDamageEnabled");
	this->Tiberium_HealEnabled.Read(exINI, GENERAL_SECTION, "TiberiumHealEnabled");
	this->Tiberium_ExplosiveWarhead.Read(exINI, COMBATDAMAGE_SECTION, "TiberiumExplosiveWarhead");

	this->OverlayExplodeThreshold.Read(exINI, GENERAL_SECTION, "OverlayExplodeThreshold");
	this->AlliedSolidTransparency.Read(exINI, COMBATDAMAGE_SECTION, "AlliedSolidTransparency");
	this->DecloakSound.Read(exINI, AUDIOVISUAL_SECTION, "DecloakSound");

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

	this->IC_Flash.Read(exINI, AUDIOVISUAL_SECTION, "IronCurtainFlash");

	// Section Generic Prerequisites
	//FillDefaultPrerequisites(pINI);

}

void RulesExt::FillDefaultPrerequisites(CCINIClass* pRules)
{
	auto& nPrepreqNames = RulesExt::Global()->GenericPrerequisitesData;

	if (nPrepreqNames.size() != 0) //everything was initiated
		return;

	if (pRules != CCINIClass::INI_Rules()) //it is not RulesMD , skip
		return;

	auto ReadPreReQs = [pRules, &nPrepreqNames](const char* pKey, const char* CatagoryName)
	{
		pRules->ReadString(GameStrings::General(), pKey, "", Phobos::readBuffer);

		char* context = nullptr;
		std::vector<int> typelist;
		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); 
			cur; 
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			const int idx = BuildingTypeClass::FindIndexById(cur);
			if (idx >= 0)
				typelist.push_back(idx);
			else
				Debug::Log("%s Failed To Find BuildingTypeIndex for %s !  \n", pKey, cur);
		}

		nPrepreqNames.emplace_back(CatagoryName, typelist);
		typelist.clear();
	};

	ReadPreReQs(GameStrings::PrerequisitePower(), "POWER");
	ReadPreReQs(GameStrings::PrerequisiteFactory(), "FACTORY");
	ReadPreReQs(GameStrings::PrerequisiteBarracks(), "BARRACKS");
	ReadPreReQs(GameStrings::PrerequisiteRadar(), "RADAR");
	ReadPreReQs(GameStrings::PrerequisiteTech(), "TECH");
	ReadPreReQs(GameStrings::PrerequisiteProc(), "PROC");

	// If [GenericPrerequisites] is present will be added after these.
	// Also the originals can be replaced by new ones
	auto const pGenPrereq = "GenericPrerequisites";
	for (int i = 0; i < pRules->GetKeyCount(pGenPrereq); ++i)
	{
		char* context = nullptr;
		auto const pKeyName = pRules->GetKeyName(pGenPrereq, i);
		pRules->ReadString(pGenPrereq, pKeyName, "", Phobos::readBuffer);

		std::vector<int> objectsList;
		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); 
			cur; 
			cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			const int idx = BuildingTypeClass::FindIndexById(cur);
			if (idx >= 0)
				objectsList.push_back(idx);
		}

		bool bFound = false;
		for (auto& [Catagory, Lists] : nPrepreqNames)
		{
			if (IS_SAME_STR_(Catagory.c_str(), pKeyName))
			{
				bFound = true;
				Lists.clear();
				Lists = objectsList;
			}
		}

		if (!bFound)
		{
			// New generic prerequisite
			nPrepreqNames.emplace_back(pKeyName, objectsList);
		}

		objectsList.clear();
	}
}

void RulesExt::LoadEarlyOptios(RulesClass* pThis, CCINIClass* pINI)
{}

void RulesExt::LoadEarlyBeforeColor(RulesClass* pThis, CCINIClass* pINI)
{}

// this runs between the before and after type data loading methods for rules ini
void RulesExt::ExtData::InitializeAfterTypeData(RulesClass* const pThis) { }

namespace ObjectTypeParser
{
	template<typename T, bool Alloc = false>
	void Exec(CCINIClass* pINI, DynamicVectorClass<DynamicVectorClass<T*>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims)
	{
		if (!pINI->GetSection(pSection))
			return;

		for (int i = 0; i < pINI->GetKeyCount(pSection); ++i)
		{
			DynamicVectorClass<T*> _Buffer;
			char* context = nullptr;
			pINI->ReadString(pSection, pINI->GetKeyName(pSection, i), "", Phobos::readBuffer);

			for (char* cur = strtok_s(Phobos::readBuffer, Delims, &context);
				cur; cur = strtok_s(nullptr, Delims, &context))
			{
				cur = CRT::strtrim(cur);

				T* buffer;
				Parser<T*>::TryParse(cur, &buffer);

				if (buffer)
				{
					if (_Buffer.AddItem(buffer))
						if (bVerbose)
							Debug::Log("ObjectTypeParser DEBUG: [%s][%d]: Verose parsing [%s]\n", pSection, nVecDest.Count, cur);
				}
				else
				{

					if constexpr (!Alloc)
					{
						if (bDebug)
						{
							Debug::Log("ObjectTypeParser DEBUG: [%s][%d]: Error parsing [%s]\n", pSection, nVecDest.Count, cur);
						}
					}
					else
					{
						if (_Buffer.AddItem(GameCreate<T>(cur)))
							if (bVerbose)
								Debug::Log("ObjectTypeParser DEBUG: Allocated [%s][%d]: Verose parsing [%s]\n", pSection, nVecDest.Count, cur);
					}
				}
			}

			nVecDest.AddItem(_Buffer);
			_Buffer.Clear();
		}
	}

	template<typename T, bool Alloc = false>
	void Exec(CCINIClass* pINI, std::vector<std::vector<T*>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims)
	{
		if (!pINI->GetSection(pSection))
			return;

		auto const nKeyCount = pINI->GetKeyCount(pSection);
		nVecDest.reserve(nKeyCount);

		for (int i = 0; i < nKeyCount; ++i)
		{
			std::vector<T*> _Buffer;
			char* context = nullptr;
			pINI->ReadString(pSection, pINI->GetKeyName(pSection, i), "", Phobos::readBuffer);

			for (char* cur = strtok_s(Phobos::readBuffer, Delims, &context);
				cur; cur = strtok_s(nullptr, Delims, &context))
			{
				cur = CRT::strtrim(cur);

				T* buffer = nullptr;
				if constexpr (!Alloc)
					buffer = T::Find(cur);
				else
					buffer = T::FindOrAllocate(cur);

				if (buffer)
				{
					_Buffer.push_back(buffer);
					if (bVerbose)
						Debug::Log("ObjectTypeParser DEBUG: [%s][%d]: Verose parsing [%s]\n", pSection, nVecDest.size(), cur);
				}
				else
				{
					if (bDebug)
					{
						Debug::Log("ObjectTypeParser DEBUG: [%s][%d]: Error parsing [%s]\n", pSection, nVecDest.size(), cur);
					}
				}
			}

			nVecDest.push_back(_Buffer);
		}
	}

	void Exec(CCINIClass* pINI, std::vector<std::vector<std::string>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false, const char* Delims = Phobos::readDelims)
	{
		if (!pINI->GetSection(pSection))
			return;

		auto const nKeyCount = pINI->GetKeyCount(pSection);
		nVecDest.reserve(nKeyCount);

		for (int i = 0; i < nKeyCount; ++i)
		{
			std::vector<std::string> objectsList;

			char* context = nullptr;
			pINI->ReadString(pSection, pINI->GetKeyName(pSection, i), "", Phobos::readBuffer);

			for (char* cur = strtok_s(Phobos::readBuffer, Delims, &context);
				cur;
				cur = strtok_s(nullptr, Delims, &context))
			{
				cur = CRT::strtrim(cur);

				objectsList.emplace_back(cur);

				if (bVerbose)
					Debug::Log("ObjectTypeParser DEBUG: [%s][%d]: Verose parsing [%s]\n", pSection, nVecDest.size(), cur);
			}

			nVecDest.push_back(objectsList);
		}
	}
};

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::ExtData::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData /*|| pINI != CCINIClass::INI_Rules()*/)
		return;

	INI_EX exINI(pINI);

	//for (auto pTTYpeArray : *TechnoTypeClass::Array) {
	//	if (auto pTTYpeExt = TechnoTypeExt::ExtMap.Find(pTTYpeArray))
	//		pTTYpeExt->LoadFromINIFile_EvaluateSomeVariables(pINI);
	//}

#pragma region Otamaa

	
		;

	this->VeinholeParticle.Read(exINI, AUDIOVISUAL_SECTION, "VeinholeSpawnParticleType");

	this->DefaultVeinParticle = ParticleTypeClass::FindOrAllocate(GameStrings::GASCLUDM1());
	this->DefaultSquidAnim = AnimTypeClass::FindOrAllocate(GameStrings::SQDG());
	this->CarryAll_LandAnim.Read(exINI, AUDIOVISUAL_SECTION, "LandingAnim.Carryall");

	if (!this->CarryAll_LandAnim)
		this->CarryAll_LandAnim = AnimTypeClass::Find(GameStrings::CARYLAND());

	this->DropShip_LandAnim.Read(exINI, AUDIOVISUAL_SECTION, "LandingAnim.Dropship");

	if (!this->DropShip_LandAnim)
		this->DropShip_LandAnim = AnimTypeClass::Find(GameStrings::DROPLAND());

	this->Aircraft_LandAnim.Read(exINI, AUDIOVISUAL_SECTION, "LandingAnim.Aircraft");
	this->Aircraft_TakeOffAnim.Read(exINI, AUDIOVISUAL_SECTION, "TakeOffAnim.Aircraft");

	this->WallTowers.Read(exINI, GENERAL_SECTION, "WallTowers");

	if (pThis->WallTower)
		this->WallTowers.push_back(pThis->WallTower);

	this->AI_SpyMoneyStealPercent.Read(exINI, GENERAL_SECTION, "AI.SpyMoneyStealPercent");

#pragma endregion

	ObjectTypeParser::Exec(pINI, AITargetTypesLists, "AITargetTypes", true);
	ObjectTypeParser::Exec<ScriptTypeClass, true>(pINI, AIScriptsLists, "AIScriptsList", true);
	ObjectTypeParser::Exec(pINI, AIHousesLists, "AIHousesList", true);
	ObjectTypeParser::Exec(pINI, AIConditionsLists, "AIConditionsList", true, false, "/");
	ObjectTypeParser::Exec<AITriggerTypeClass, true>(pINI, AITriggersLists, "AITriggersList", true);
}

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
		.Process(this->AIHousesLists)
		.Process(this->AIConditionsLists)
		.Process(this->AITriggersLists)

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

		.Process(this->InfantryGainSelfHealCap)
		.Process(this->UnitsGainSelfHealCap)
		.Process(this->EnemyInsignia)
		.Process(this->ShowAllyDisguiseBlinking)

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

		.Process(this->GenericPrerequisitesData)

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

		.Process(this->BerserkROFMultiplier)
		.Process(this->TeamRetaliate)
		;
#ifdef COMPILE_PORTED_DP_FEATURES
	MyPutData.Serialize(Stm);
#endif
}

void RulesExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RulesClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void RulesExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<RulesClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool RulesExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool RulesExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
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

IStream* RulesExt::g_pStm = nullptr;

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

		if (Reader.Expect(RulesExt::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x675205, RulesClass_Save_Suffix, 0x8)
{
	auto buffer = RulesExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(RulesExt::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(RulesExt::g_pStm);

	return 0;
}

DEFINE_HOOK(0x52C9C4, GameInit_VeryEarlyRulesInit, 0x6)
{
	//RulesExt::LoadVeryEarlyBeforeAnyData(RulesClass::Instance(), CCINIClass::INI_Rules());
	return 0x52CA37;
}

// DEFINE_HOOK(0x52D149, InitRules_PostInit, 0x5)
// {
// 	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
// 	return 0;
// }

DEFINE_HOOK(0x668BF0, RulesClass_Addition, 0x5)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	//	RulesClass::Initialized = false;
	RulesExt::LoadFromINIFile(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679A15, RulesData_LoadBeforeTypeData, 0x6)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

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

	//for (auto const& pUnparsedData : UnParsedThing::UnparsedList)
	//{
	//	if (pUnparsedData.empty())
	//		continue;

	//	Debug::Log("Loading Unparsed WH[%s] from INI Files ! \n" , pUnparsedData.c_str());
	//	if (auto const pWH = WarheadTypeClass::Find(pUnparsedData.c_str()))
	//		pWH->LoadFromINI(pINI);
	//}

	//UnParsedThing::UnparsedList.clear();

	return 0;
}

DEFINE_HOOK(0x66D530, RulesData_LoadBeforeGeneralData, 0x6)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	RulesExt::LoadBeforeGeneralData(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x668F6A, RulesData_LoadAfterAllLogicData, 0x5)
{
	GET(RulesClass*, pItem, EDI);
	GET(CCINIClass*, pINI, ESI);

	RulesExt::LoadAfterAllLogicData(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679CAF, RulesClass_LoadAfterTypeData_CompleteInitialization, 0x5)
{
	//Debug::Log(__FUNCTION__" Called ! \n");
	// std::for_each(BuildingTypeClass::Array->begin(), BuildingTypeClass::Array->end(), [](BuildingTypeClass* pType) {
	// 	if (auto const pExt = BuildingTypeExt::ExtMap.Find(pType))
	// 	pExt->CompleteInitialization();
	// });

	// TODO :
	//std::for_each(TechnoTypeClass::Array->begin(), TechnoTypeClass::Array->end(), [](TechnoTypeClass* pType) {
	//if (auto const pExt = TechnoTypeExt::ExtMap.Find(pType))
	//	pExt->Initialize();
	//});

	return 0;
}

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

//Skip game WallTower read
//DEFINE_JUMP(LJMP,0x66F553, 0x66F589);