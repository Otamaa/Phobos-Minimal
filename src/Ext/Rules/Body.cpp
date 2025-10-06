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
#include <New/Type/CrateTypeClass.h>
#include <New/Type/TechTreeTypeClass.h>
#include <New/Type/RocketTypeClass.h>
#include <New/Type/InsigniaTypeClass.h>
#include <New/Type/SelectBoxTypeClass.h>
#include <New/Type/BannerTypeClass.h>

#include <New/PhobosAttachedAffect/PhobosAttachEffectTypeClass.h>

//#include <Ext/TechnoType/Body.h>

#include <Ext/WarheadType/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/Helpers.h>

#include <Misc/DynamicPatcher/Trails/TrailType.h>

#include <GameStrings.h>

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

void RulesExtData::Initialize()
{
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

void RulesExtData::LoadVeryEarlyBeforeAnyData(RulesClass* pRules, CCINIClass* pINI)
{
}

void RulesExtData::LoadEndOfAudioVisual(RulesClass* pRules, CCINIClass* pINI)
{
	INI_EX iniEX(pINI);
	auto pData = RulesExtData::Instance();

	Nullable<double> Shield_ConditionGreen_d {};
	Nullable<double> Shield_ConditionYellow_d {};
	Nullable<double> Shield_ConditionRed_d {};
	Nullable<double> ConditionYellow_Terrain_d {};

	Shield_ConditionGreen_d.Read(iniEX, GameStrings::AudioVisual(), "Shield.ConditionGreen");// somewhat never used , man
	Shield_ConditionYellow_d.Read(iniEX, GameStrings::AudioVisual(), "Shield.ConditionYellow");
	Shield_ConditionRed_d.Read(iniEX, GameStrings::AudioVisual(), "Shield.ConditionRed");
	ConditionYellow_Terrain_d.Read(iniEX, GameStrings::AudioVisual(), "ConditionYellow.Terrain");

	pData->Shield_ConditionGreen = Shield_ConditionGreen_d.Get(pRules->ConditionGreen);
	pData->Shield_ConditionYellow = Shield_ConditionYellow_d.Get(pRules->ConditionYellow);
	pData->Shield_ConditionRed = Shield_ConditionRed_d.Get(pRules->ConditionRed);
	pData->ConditionYellow_Terrain = ConditionYellow_Terrain_d.Get(pRules->ConditionYellow);
}

ASMJIT_PATCH(0x66B8E2, RulesClass_ReadAudioVisual_End, 0x5)
{
	GET(DWORD, ptr, ESI);
	GET(CCINIClass*, pINI, EDI);

	RulesClass* pRules = reinterpret_cast<RulesClass*>(ptr - 0x18B8);
	RulesExtData::LoadEndOfAudioVisual(pRules, pINI);
	return 0x0;
}

void RulesExtData::s_LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI)
{
	Data->Initialize();
	Data->LoadFromINIFile(pINI, false);
}

// do everything before `TypeData::ReadFromINI` executed
// to makesure everything is properly allocated from the list
void RulesExtData::s_LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	ArmorTypeClass::LoadFromINIList_New(pINI);
	ColorTypeClass::LoadFromINIList_New(pINI);
	CursorTypeClass::LoadFromINIList_New(pINI);
	InsigniaTypeClass::LoadFromINIList(pINI);

	TunnelTypeClass::LoadFromINIList(pINI);

	CrateTypeClass::ReadFromINIList(pINI); //yeah ,..

	// we override it , so it loaded before any type read happen , so all the properties will correcly readed
	pThis->Read_CrateRules(pINI);
	pThis->Read_CombatDamage(pINI);
	pThis->Read_Radiation(pINI);
	pThis->Read_ElevationModel(pINI);
	pThis->Read_WallModel(pINI);
	pThis->Read_AudioVisual(pINI);
	pThis->Read_SpecialWeapons(pINI);

	CrateTypeClass::AddDefaults();
	RadTypeClass::AddDefaults();
	GenericPrerequisite::AddDefaults();
	HoverTypeClass::AddDefaults();
	ShieldTypeClass::AddDefaults();

	ImmunityTypeClass::LoadFromINIList(pINI);
	ArmorTypeClass::EvaluateDefault();

	TrailType::LoadFromINIList(&CCINIClass::INI_Art.get());

	RadTypeClass::LoadFromINIOnlyTheList(pINI);

	ShieldTypeClass::LoadFromINIOnlyTheList(pINI);
	HoverTypeClass::LoadFromINIOnlyTheList(pINI);

	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
	DigitalDisplayTypeClass::LoadFromINIList(pINI);
	SelectBoxTypeClass::LoadFromINIList(pINI);

	PhobosAttachEffectTypeClass::LoadFromINIOnlyTheList(pINI);
	TechTreeTypeClass::LoadFromINIOnlyTheList(pINI);

	BannerTypeClass::LoadFromINIList(pINI);

	if (Data->HugeBar_Config.empty())
	{
		Data->HugeBar_Config.push_back(std::move(std::make_unique<HugeBar>(DisplayInfoType::Health)));
		Data->HugeBar_Config.push_back(std::move(std::make_unique<HugeBar>(DisplayInfoType::Shield)));
	}

	for (auto& huge_bar : Data->HugeBar_Config) {
		huge_bar->LoadFromINI(pINI);
	}

	Data->LoadBeforeTypeData(pThis, pINI);
}

#include <Ext/SWType/Body.h>

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExtData::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	INI_EX iniEX(pINI);
	auto pData = RulesExtData::Instance();

	CrateTypeClass::ReadListFromINI(pINI);
	HoverTypeClass::ReadListFromINI(pINI);
	ShieldTypeClass::ReadListFromINI(pINI);
	RadTypeClass::ReadListFromINI(pINI);
	PhobosAttachEffectTypeClass::ReadListFromINI(pINI);
	TechTreeTypeClass::ReadListFromINI(pINI);

	pData->BattlePoints.Read(iniEX, GameStrings::General, "BattlePoints");
	pData->BattlePoints_DefaultValue.Read(iniEX, GameStrings::General, "BattlePoints.DefaultValue");
	pData->BattlePoints_DefaultFriendlyValue.Read(iniEX, GameStrings::General, "BattlePoints.DefaultFriendlyValue");

	pData->SuperWeaponSidebar_AllowByDefault.Read(iniEX, GameStrings::AudioVisual, "SuperWeaponSidebar.AllowByDefault");

	pData->DamagedSpeed.Read(iniEX, GameStrings::General, "DamagedSpeed");
	pData->ColorAddUse8BitRGB.Read(iniEX, GameStrings::AudioVisual, "ColorAddUse8BitRGB");
	pData->IronCurtain_ExtraTintIntensity.Read(iniEX, GameStrings::AudioVisual, "IronCurtain.ExtraTintIntensity");
	pData->ForceShield_ExtraTintIntensity.Read(iniEX, GameStrings::AudioVisual, "ForceShield.ExtraTintIntensity");

	pData->DefaultInfantrySelectBox.Read(iniEX, GameStrings::AudioVisual, "DefaultInfantrySelectBox");
	pData->DefaultUnitSelectBox.Read(iniEX, GameStrings::AudioVisual, "DefaultUnitSelectBox");

	pData->InfantrySpeedData.Crawls.Read(iniEX, GameStrings::General, "ProneSpeed.Crawls");
	pData->InfantrySpeedData.NoCrawls.Read(iniEX, GameStrings::General, "ProneSpeed.NoCrawls");

	pData->VoxelLightSource.Read(iniEX, GameStrings::AudioVisual, "VoxelLightSource");
	pData->VoxelShadowLightSource.Read(iniEX, GameStrings::AudioVisual, "VoxelShadowLightSource");
	pData->UseFixedVoxelLighting.Read(iniEX, GameStrings::AudioVisual, "UseFixedVoxelLighting");

	//got invalidated early , so parse it again
	detail::ParseVector(iniEX, pData->AITargetTypesLists, "AITargetTypes");
	detail::ParseVector<ScriptTypeClass*, true>(iniEX, pData->AIScriptsLists, "AIScriptsList");
	detail::ParseVector<HouseTypeClass*>(iniEX, pData->AIHateHousesLists, "AIHateHousesList");
	detail::ParseVector<HouseTypeClass*>(iniEX, pData->AIHousesLists, "AIHousesList");
	detail::ParseVector(iniEX, pData->AIConditionsLists, "AIConditionsList", true, false, "/");
	detail::ParseVector<AITriggerTypeClass*, true>(iniEX, pData->AITriggersLists, "AITriggersList");

	pData->AIChronoSphereSW.Read(iniEX, GameStrings::General, "AIChronoSphereSW");
	pData->AIChronoWarpSW.Read(iniEX, GameStrings::General, "AIChronoWarpSW");

	pData->DamageOwnerMultiplier.Read(iniEX, GameStrings::CombatDamage, "DamageOwnerMultiplier");
	pData->DamageAlliesMultiplier.Read(iniEX, GameStrings::CombatDamage, "DamageAlliesMultiplier");
	pData->DamageEnemiesMultiplier.Read(iniEX, GameStrings::CombatDamage, "DamageEnemiesMultiplier");
	pData->DamageEnemiesMultiplier_UsedForAllTargetInBerzerk.Read(iniEX, GameStrings::CombatDamage, "DamageEnemiesMultiplier.UsedForAllTargetInBerzerk");
	pData->DamageOwnerMultiplier_NotAffectsEnemies.Read(iniEX, GameStrings::CombatDamage, "DamageOwnerMultiplier.NotAffectsEnemies");
	pData->DamageAlliesMultiplier_NotAffectsEnemies.Read(iniEX, GameStrings::CombatDamage, "DamageAlliesMultiplier.NotAffectsEnemies");

	pData->FactoryProgressDisplay.Read(iniEX, GameStrings::AudioVisual, "FactoryProgressDisplay");
	pData->MainSWProgressDisplay.Read(iniEX, GameStrings::AudioVisual, "MainSWProgressDisplay");
	pData->CombatAlert.Read(iniEX, GameStrings::AudioVisual, "CombatAlert");
	pData->CombatAlert_MakeAVoice.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.MakeAVoice");
	pData->CombatAlert_IgnoreBuilding.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.IgnoreBuilding");
	pData->CombatAlert_EVA.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.EVA");
	pData->CombatAlert_UseFeedbackVoice.Read(iniEX,GameStrings::AudioVisual, "CombatAlert.UseFeedbackVoice");
	pData->CombatAlert_UseAttackVoice.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.UseAttackVoice");
	pData->CombatAlert_SuppressIfInScreen.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.SuppressIfInScreen");
	pData->CombatAlert_Interval.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.Interval");
	pData->CombatAlert_SuppressIfAllyDamage.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.SuppressIfAllyDamage");
	pData->SubterraneanHeight.Read(iniEX, GameStrings::General, "SubterraneanHeight");

	pData->ForceShield_KillOrganicsWarhead.Read(iniEX, GameStrings::CombatDamage(), "ForceShield.KillOrganicsWarhead");

	if (!pData->ForceShield_KillOrganicsWarhead)
		pData->ForceShield_KillOrganicsWarhead = pThis->C4Warhead;

	pData->IronCurtain_KillOrganicsWarhead.Read(iniEX, GameStrings::CombatDamage(), "IronCurtain.KillOrganicsWarhead");

	if (!pData->IronCurtain_KillOrganicsWarhead)
		pData->IronCurtain_KillOrganicsWarhead = pThis->C4Warhead;

	pData->DefaultAircraftDamagedSmoke = AnimTypeClass::Find(GameStrings::SGRYSMK1());
	pData->FirestormActiveAnim.Read(iniEX, GameStrings::AudioVisual(), "FirestormActiveAnim");
	pData->FirestormIdleAnim.Read(iniEX, GameStrings::AudioVisual(), "FirestormIdleAnim");
	pData->FirestormGroundAnim.Read(iniEX, GameStrings::AudioVisual(), "FirestormGroundAnim");
	pData->FirestormAirAnim.Read(iniEX, GameStrings::AudioVisual(), "FirestormAirAnim");
	pData->FirestormWarhead.Read(iniEX, GameStrings::CombatDamage(), "FirestormWarhead");
	pData->DamageToFirestormDamageCoefficient.Read(iniEX, GameStrings::General(), "DamageToFirestormDamageCoefficient");

	pData->Bounty_Enablers.Read(iniEX, GameStrings::General(), "BountyEnablers");
	pData->Bounty_Display.Read(iniEX, GameStrings::AudioVisual(), "BountyDisplay");
	pData->CloakAnim.Read(iniEX, GameStrings::AudioVisual(), "CloakAnim");
	pData->DecloakAnim.Read(iniEX, GameStrings::AudioVisual(), "DecloakAnim");
	pData->Cloak_KickOutParasite.Read(iniEX, GameStrings::CombatDamage, "Cloak.KickOutParasite");

	pData->Veinhole_Warhead.Read(iniEX, GameStrings::CombatDamage(), "VeinholeWarhead");

	pData->WallTowers.Read(iniEX, GameStrings::General(), "WallTowers");
	if (pThis->WallTower && !pData->WallTowers.Contains(pThis->WallTower))
		pData->WallTowers.push_back(pThis->WallTower);

	pData->Promote_Vet_Anim.Read(iniEX, GameStrings::AudioVisual(), "Promote.VeteranAnim");
	pData->Promote_Elite_Anim.Read(iniEX, GameStrings::AudioVisual(), "Promote.EliteAnim");
	pData->PrimaryFactoryIndicator.Read(iniEX, GameStrings::AudioVisual(), "PrimaryFactoryIndicator");
	pData->PrimaryFactoryIndicator_Palette.Read(iniEX, GameStrings::AudioVisual(), "PrimaryFactoryIndicator.Palette");

	pData->DefaultExplodeFireAnim.Read(iniEX, GameStrings::AudioVisual(), "DefaultExplodeOverlayFireAnim");

	if (!pData->DefaultExplodeFireAnim)
		pData->DefaultExplodeFireAnim = AnimTypeClass::Find(GameStrings::Anim_FIRE3);

	for (int i = 0; i < WeaponTypeClass::Array->Count; ++i)
	{
		WeaponTypeClass::Array->Items[i]->LoadFromINI(pINI);
	}

	for (int i = 0; i < BuildingTypeClass::Array->Count; ++i)
	{
		BuildingTypeExtContainer::Instance.Find(BuildingTypeClass::Array->Items[i])
			->CompleteInitialization();
	}

	pData->ReplaceVoxelLightSources();
}

static bool NOINLINE IsVanillaDummy(const char* ID)
{
	static COMPILETIMEEVAL const char* exception[] = { "DeathDummy" , "WEEDGUY" , "YDUM" };

	for (auto const& gameDummy : exception)
	{
		if (IS_SAME_STR_(ID, gameDummy))
			return true;
	}

	return false;
}

#include <Ext/SWType/NewSuperWeaponType/NewSWType.h>

template<typename T>
static COMPILETIMEEVAL FORCEDINLINE void FillSecrets(DynamicVectorClass<T>& secrets) {

	for(auto opt : secrets){
		RulesExtData::Instance()->Secrets.emplace_back(opt);
		//Debug::LogInfo("Adding [{} - {}] onto Global Secrets pool" , Option->ID, Option->GetThisClassName());
	}
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
	INI_EX iniEX(pINI);

	for (auto pItem : *TechnoTypeClass::Array)
	{
		const auto what = pItem->WhatAmI();
		const auto isFoot = what != AbstractType::BuildingType;
		auto pExt = TechnoTypeExtContainer::Instance.Find(pItem);
		const auto myClassName = pItem->GetThisClassName();
		bool WeederAndHarvesterWarning = false;

		if (pExt->Image_Yellow && pExt->Image_Yellow->WhatAmI() != what) {
			Debug::LogInfo("[{} - {}] has Image.ConditionYellow [{} - {}] but it different ClassType from it!",
				pItem->ID, myClassName, pExt->Image_Yellow->ID, pExt->Image_Yellow->GetThisClassName());
			pExt->Image_Yellow = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->Image_Red && pExt->Image_Red->WhatAmI() != what) {
			Debug::LogInfo("[{} - {}] has Image.ConditionRed [{} - {}] but it different ClassType from it!",
				pItem->ID, myClassName, pExt->Image_Red->ID, pExt->Image_Red->GetThisClassName());
			pExt->Image_Red = nullptr;
			Debug::RegisterParserError();
		}

		if (pItem->Strength <= 0)
		{
			const bool IsUpgradeBld = what == BuildingTypeClass::AbsID && *((BuildingTypeClass*)pItem)->PowersUpBuilding && strlen(((BuildingTypeClass*)pItem)->PowersUpBuilding) > 0;

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
			Debug::LogInfo("[{} - {}] has FakeOf [{} - {}] but it different ClassType from it!", pItem->ID, myClassName , pExt->Fake_Of->ID , pExt->Fake_Of->GetThisClassName());
			pExt->Fake_Of = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->RecuitedAs.isset()) {
			if (pExt->RecuitedAs && pExt->RecuitedAs->WhatAmI() != what) {
				Debug::LogInfo("[{} - {}] has ClonedAs [{} - {}] but it different ClassType from it!", pItem->ID, myClassName, pExt->ClonedAs->ID, pExt->ClonedAs->GetThisClassName());
				Debug::RegisterParserError();
			}
			else if (!pExt->RecuitedAs || pExt->RecuitedAs == pItem) {
				pExt->RecuitedAs.Reset();
			}
		}

		if (pExt->ClonedAs && pExt->ClonedAs->WhatAmI() != what)
		{
			Debug::LogInfo("[{} - {}] has ClonedAs [{} - {}] but it different ClassType from it!", pItem->ID, myClassName , pExt->ClonedAs->ID , pExt->ClonedAs->GetThisClassName());
			pExt->ClonedAs = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->AI_ClonedAs && pExt->AI_ClonedAs->WhatAmI() != what)
		{
			Debug::LogInfo("[{} - {}] has AI.ClonedAs [{} - {}] but it different ClassType from it!", pItem->ID, myClassName , pExt->AI_ClonedAs->ID , pExt->AI_ClonedAs->GetThisClassName());
			pExt->AI_ClonedAs = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->ReversedAs.Get(nullptr) && pExt->ReversedAs->WhatAmI() != what)
		{
			Debug::LogInfo("[{} - {}] has ReversedAs [{} - {}] but it different ClassType from it!", pItem->ID, myClassName , pExt->ReversedAs->ID , pExt->ReversedAs->GetThisClassName());
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
		if (pItem->MainVoxel.VXL)
		{
			if (auto pHVA = pItem->MainVoxel.HVA)
			{

				auto shadowIdx = pItem->ShadowIndex;
				auto layerCount = pHVA->LayerCount;

				if (shadowIdx >= layerCount)
				{
					Debug::LogInfo("ShadowIndex on [{}]'s image is {}, but the HVA only has {} sections.",
						pItem->ID, shadowIdx, layerCount);
					Debug::RegisterParserError();
				}
			}
			else
			{
				Debug::FatalError("Techno[{} - {}] Has VXL but has no HVA wtf ?", myClassName, pItem->ID);
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

		if (!pExt->Harvester_Counted.isset() && pItem->Enslaves) {
			pExt->Harvester_Counted = true;
		}

		if (pExt->Spawn_LimitedExtraRange)
			pExt->CalculateSpawnerRange();

		if (isFoot)
		{
			if (what == UnitTypeClass::AbsID)
			{
				const auto pUnit = (UnitTypeClass*)pItem;

				if (!pExt->Harvester_Counted.isset() && pUnit->Harvester) {
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

	 for (auto pBullet : *BulletTypeClass::Array) {

		 if(pBullet->Voxel && !pBullet->MainVoxel.VXL) {
			 Debug::LogInfo("Bullet[{}] has no valid VXL !", pBullet->ID);
			 pBullet->Voxel = false;//shp bullet has image checking
			 Debug::RegisterParserError();
		 }
		 else if (pBullet->Voxel && pBullet->MainVoxel.VXL && !pBullet->MainVoxel.HVA) {
			 Debug::LogInfo("Bullet[{}] has no valid HVA !", pBullet->ID);
			 Debug::RegisterParserError();
		 }
		 else if (!pBullet->Voxel && !pBullet->GetImage()) {
			 Debug::LogInfo("Bullet[{}] has no valid SHP !", pBullet->ID);
			 Debug::RegisterParserError();
		 }

	 	//auto pExt = BulletTypeExtContainer::Instance.Find(pBullet);

	 	//if (pExt->AttachedSystem && pExt->AttachedSystem->BehavesLike != ParticleSystemTypeBehavesLike::Smoke) {
	 	//	Debug::LogInfo("Bullet[{}] With AttachedSystem[{}] is not BehavesLike=Smoke!", pBullet->ID, pExt->AttachedSystem->ID);
	 	//	Debug::RegisterParserError();
	 	//}
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
			//_Temp_MouseCursor.Read(iniEX, pSuper->ID, "Cursor");
			//if (_Temp_MouseCursor.isset()) {
			//	std::string _name = pSuper->ID;
			//	_name += "Cursor";
			//
			//	CursorTypeClass::AllocateWithDefault(_name.c_str(), _Temp_MouseCursor);
			//}

			for (auto& pTech : pSuperExt->Aux_Techno)
			{
				TechnoTypeExtContainer::Instance.Find(pTech)->Linked_SW.push_back(pSuper);
			}

			fast_remove_if(pSuperExt->SW_AuxBuildings ,[](BuildingTypeClass* pItem)	{ return !pItem; } );
			fast_remove_if(pSuperExt->SW_NegBuildings ,[](BuildingTypeClass* pItem)	{ return !pItem; } );

			Helpers::Alex::remove_non_paradroppables(pSuperExt->DropPod_Types, pSuper->ID, "DropPod.Types");

			for (auto& para : pSuperExt->ParaDropDatas) {
				for (auto& pVec : para.second) {
						Helpers::Alex::remove_non_paradroppables(pVec.Types, pSuper->ID, "ParaDrop.Types");
				}
			}
		}
	}

	for (auto pAnim : *AnimTypeClass::Array) {

		if (!pAnim->ID || !strlen(pAnim->ID))
			Debug::FatalError("Empty name Anim [%x]! " , pAnim);

		if (!pAnim->GetImage()) {
			Debug::LogInfo("Anim[{}] Has no proper Image!", pAnim->ID);
			Debug::RegisterParserError();
		}
	}

	if (Phobos::Otamaa::StrictParser && Phobos::Otamaa::ParserErrorDetected)
	{
		Debug::FatalErrorAndExit(
			"One or more errors were detected while parsing the INI files.\r"
			"Please review the contents of the debug log and correct them.");
	}

	return 0x0;
}

// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
void RulesExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{

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

	GenericPrerequisite::LoadFromINIList_New(pINI);

	INI_EX exINI(pINI);

	exINI.Read3Bool(GameStrings::General,"CampaignAllowHarvesterScanUnderShroud", this->CampaignAllowHarvesterScanUnderShroud);

	this->AttackMove_IgnoreWeaponCheck.Read(exINI, GameStrings::General, "AttackMove.IgnoreWeaponCheck");
	this->AttackMove_StopWhenTargetAcquired.Read(exINI, GameStrings::General, "AttackMove.StopWhenTargetAcquired");
	this->PenetratesTransport_Level.Read(exINI, GameStrings::CombatDamage, "PenetratesTransport.Level");
	this->DamageWallRecursivly.Read(exINI, GameStrings::CombatDamage, "DamageWallRecursivly");
	this->AirstrikeLineZAdjust.Read(exINI, GameStrings::AudioVisual, "AirstrikeLineZAdjust");
	this->AdjacentWallDamage.Read(exINI, GameStrings::CombatDamage, "AdjacentWallDamage");
	this->AISellCapturedBuilding.Read(exINI, GameStrings::General, "AISellCapturedBuilding");
	this->InfantryAutoDeploy.Read(exINI, GameStrings::General, "InfantryAutoDeploy");
	this->EnablePassiveAcquireMode.Read(exINI, GameStrings::General, "EnablePassiveAcquireMode");
	this->UseRetintFix.Read(exINI, GameStrings::AudioVisual, "UseRetintFix");
	this->AIAdjacentMax.Read(exINI, GameStrings::AI, "AIAdjacentMax");
	this->AIAdjacentMax_Campaign.Read(exINI, GameStrings::AI, "AIAdjacentMax.Campaign");

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

	this->BerzerkTargeting.Read(exINI, GameStrings::CombatDamage, "BerzerkTargeting");
	this->Infantry_IgnoreBuildingSizeLimit.Read(exINI, GameStrings::CombatDamage, "InfantryIgnoreBuildingSizeLimit");
	this->HarvesterDumpAmount.Read(exINI, GameStrings::General, "HarvesterDumpAmount");
	this->AttackMove_Aggressive.Read(exINI, GameStrings::General, "AttackMove.Aggressive");
	this->AttackMove_UpdateTarget.Read(exINI, GameStrings::General, "AttackMove.UpdateTarget");
	this->HarvesterScanAfterUnload.Read(exINI, GameStrings::General, "HarvesterScanAfterUnload");
	this->VisualScatter_Min.Read(exINI, GameStrings::AudioVisual, "VisualScatter.Min");
	this->VisualScatter_Max.Read(exINI, GameStrings::AudioVisual, "VisualScatter.Max");

	this->GiveMoneyIfStorageFull.Read(exINI, GameStrings::General, "Storage.GiveMoneyIfFull");
	this->AutoBuilding.Read(exINI, GameStrings::General, "AutoBuilding");
	this->AIAngerOnAlly.Read(exINI, GameStrings::General, "AIAngerOnAlly");
	this->BuildingTypeSelectable.Read(exINI, GameStrings::General, "BuildingTypeSelectable");
	this->BuildingWaypoint.Read(exINI, GameStrings::General, "BuildingWaypoint");

	this->AIAutoDeployMCV.Read(exINI, GameStrings::AI, "AIAutoDeployMCV");
	this->AISetBaseCenter.Read(exINI, GameStrings::AI, "AISetBaseCenter");
	this->AIBiasSpawnCell.Read(exINI, GameStrings::AI, "AIBiasSpawnCell");
	this->AIForbidConYard.Read(exINI, GameStrings::AI, "AIForbidConYard");
	this->AINodeWallsOnly.Read(exINI, GameStrings::AI, "AINodeWallsOnly");
	this->AICleanWallNode.Read(exINI, GameStrings::AI, "AICleanWallNode");

	this->JumpjetTilt.Read(exINI, GameStrings::AudioVisual, "JumpjetTilt");
	this->NoTurret_TrackTarget.Read(exINI, GameStrings::General, "NoTurret.TrackTarget");

	this->RecountBurst.Read(exINI, GameStrings::General, "RecountBurst");
	this->AirstrikeLineColor.Read(exINI, GameStrings::AudioVisual, "AirstrikeLineColor");

	this->AmphibiousEnter.Read(exINI, GameStrings::General, "AmphibiousEnter");
	this->AmphibiousUnload.Read(exINI, GameStrings::General, "AmphibiousUnload");

	this->Cameo_AlwaysExist.Read(exINI, GameStrings::AudioVisual, "Cameo.AlwaysExist");
	this->Cameo_OverlayShapes.Read(exINI, GameStrings::AudioVisual, "Cameo.OverlayShapes");
	this->Cameo_OverlayFrames.Read(exINI, GameStrings::AudioVisual, "Cameo.OverlayFrames");
	this->Cameo_OverlayPalette.Read(exINI, GameStrings::AudioVisual, "Cameo.OverlayPalette");

	this->UnitIdleRotateTurret.Read(exINI, GameStrings::AudioVisual, "UnitIdleRotateTurret");
	this->UnitIdlePointToMouse.Read(exINI, GameStrings::AudioVisual, "UnitIdlePointToMouse");
	this->UnitIdleActionRestartMin.Read(exINI, GameStrings::AudioVisual, "UnitIdleActionRestartMin");
	this->UnitIdleActionRestartMax.Read(exINI, GameStrings::AudioVisual, "UnitIdleActionRestartMax");
	this->UnitIdleActionIntervalMin.Read(exINI, GameStrings::AudioVisual, "UnitIdleActionIntervalMin");
	this->UnitIdleActionIntervalMax.Read(exINI, GameStrings::AudioVisual, "UnitIdleActionIntervalMax");

	this->ExpandAircraftMission.Read(exINI, GameStrings::General, "ExtendedAircraftMissions");
	this->ExtendedAircraftMissions_UnlandDamage.Read(exINI, GameStrings::General, "ExtendedAircraftMissions.UnlandDamage");

	this->AssignUnitMissionAfterParadropped.Read(exINI, GameStrings::General, "AssignUnitMissionAfterParadropped");
	this->NoQueueUpToEnter.Read(exINI, GameStrings::General, "NoQueueUpToEnter");
	this->NoQueueUpToUnload.Read(exINI, GameStrings::General, "NoQueueUpToUnload");

	this->NoRearm_UnderEMP.Read(exINI, GameStrings::General, "NoRearm.UnderEMP");
	this->NoRearm_Temporal.Read(exINI, GameStrings::General, "NoRearm.Temporal");
	this->NoReload_UnderEMP.Read(exINI, GameStrings::General, "NoReload.UnderEMP");
	this->NoReload_Temporal.Read(exINI, GameStrings::General, "NoReload.Temporal");

	this->AttackMindControlledDelay.Read(exINI, GameStrings::General, "AttackMindControlledDelay");

	this->MergeBuildingDamage.Read(exINI, GameStrings::CombatDamage, "MergeBuildingDamage");
	this->ExpandBuildingQueue.Read(exINI, GameStrings::General, "BuildingProductionQueue");
	this->EnablePowerSurplus.Read(exINI, GameStrings::AI, "EnablePowerSurplus");
	this->ShakeScreenUseTSCalculation.Read(exINI, GameStrings::AudioVisual, "ShakeScreenUseTSCalculation");
	exINI.ReadSpeed(GameStrings::General, "SubterraneanSpeed", &this->SubterraneanSpeed);

	this->CheckUnitBaseNormal.Read(exINI, GameStrings::General, "CheckUnitBaseNormal");

	//TODO : fuck this break AI
	this->ExtendedBuildingPlacing.Read(exINI, GameStrings::General, "ExtendedBuildingPlacing");

	this->CheckExpandPlaceGrid.Read(exINI, GameStrings::AudioVisual, "CheckExpandPlaceGrid");
	this->ExpandLandGridFrames.Read(exINI, GameStrings::AudioVisual, "ExpandLandGridFrames");
	this->ExpandWaterGridFrames.Read(exINI, GameStrings::AudioVisual, "ExpandWaterGridFrames");

	this->AISuperWeaponDelay.Read(exINI, GameStrings::General, "AISuperWeaponDelay");
	this->ChronoSpherePreDelay.Read(exINI, GameStrings::General, "ChronoSpherePreDelay");
	this->ChronoSphereDelay.Read(exINI, GameStrings::General, "ChronoSphereDelay");

	this->VeinsAttack_interval.Read(exINI, GameStrings::AudioVisual, "VeinsAttackInterval");
	this->BuildingFlameSpawnBlockFrames.Read(exINI, GameStrings::AudioVisual, "BuildingFlameSpawnBlockFrames");

	this->AINormalTargetingDelay.Read(exINI, GameStrings::General, "AINormalTargetingDelay");
	this->PlayerNormalTargetingDelay.Read(exINI, GameStrings::General, "PlayerNormalTargetingDelay");
	this->AIGuardAreaTargetingDelay.Read(exINI, GameStrings::General, "AIGuardAreaTargetingDelay");
	this->PlayerGuardAreaTargetingDelay.Read(exINI, GameStrings::General, "PlayerGuardAreaTargetingDelay");
	this->AIAttackMoveTargetingDelay.Read(exINI, GameStrings::General, "AIAttackMoveTargetingDelay");
	this->PlayerAttackMoveTargetingDelay.Read(exINI, GameStrings::General, "PlayerAttackMoveTargetingDelay");
	this->DistributeTargetingFrame.Read(exINI, GameStrings::General, "DistributeTargetingFrame");
	this->DistributeTargetingFrame_AIOnly.Read(exINI, GameStrings::General, "DistributeTargetingFrame.AIOnly");

	this->AircraftLevelLightMultiplier.Read(exINI, GameStrings::AudioVisual, "AircraftLevelLightMultiplier");
	this->AircraftCellLightLevelMultiplier.Read(exINI, GameStrings::AudioVisual, "AircraftCellLightLevelMultiplier");
	this->JumpjetLevelLightMultiplier.Read(exINI, GameStrings::AudioVisual, "JumpjetLevelLightMultiplier");
	this->JumpjetCellLightLevelMultiplier.Read(exINI, GameStrings::AudioVisual, "JumpjetCellLightLevelMultiplier");
	this->JumpjetCellLightApplyBridgeHeight.Read(exINI, GameStrings::AudioVisual, "JumpjetCellLightApplyBridgeHeight");

	double AirShadowBaseScale = 0.0;
	if (detail::read<double>(AirShadowBaseScale, exINI, GameStrings::AudioVisual, "AirShadowBaseScale") && AirShadowBaseScale > 0)
		this->AirShadowBaseScale_log = -std::log(std::min(AirShadowBaseScale, 1.0));

	this->HeightShadowScaling.Read(exINI, GameStrings::AudioVisual, "HeightShadowScaling");
	if (AirShadowBaseScale > 0.98 && this->HeightShadowScaling.Get())
		this->HeightShadowScaling = false;

	this->HeightShadowScaling_MinScale.Read(exINI, GameStrings::AudioVisual, "HeightShadowScaling.MinScale");

	this->StartInMultiplayerUnitCost.Read(exINI, GameStrings::General(), "StartInMultiplayerUnitCost");
	this->Buildings_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Buildings.DefaultDigitalDisplayTypes");
	this->Infantry_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Infantry.DefaultDigitalDisplayTypes");
	this->Vehicles_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Vehicles.DefaultDigitalDisplayTypes");
	this->Aircraft_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Aircraft.DefaultDigitalDisplayTypes");

	if (pINI->ReadString("GlobalControls", "AllowBypassBuildLimit", "", Phobos::readBuffer) > 0)
	{
		bool temp[3] {};
		for (int i = 0; i < (int)Parser<bool, 3>::Parse(Phobos::readBuffer, temp); ++i)
		{
			int diffIdx = 2 - i; // remapping so that HouseClass::AIDifficulty can be used as an index
			this->AllowBypassBuildLimit[diffIdx] = temp[i];
		}
	}

	this->DisplayIncome.Read(exINI, GameStrings::AudioVisual, "DisplayIncome");
	this->DisplayIncome_Houses.Read(exINI, GameStrings::AudioVisual, "DisplayIncome.Houses");
	this->DisplayIncome_AllowAI.Read(exINI, GameStrings::AudioVisual, "DisplayIncome.AllowAI");
	this->Droppod_ImageInfantry.Read(exINI, GameStrings::AudioVisual, "DropPod.InfantryPodImage");
	this->DrawInsigniaOnlyOnSelected.Read(exINI, GameStrings::AudioVisual, "DrawInsigniaOnlyOnSelected");
	this->DrawInsignia_AdjustPos_Infantry.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.Infantry");
	this->DrawInsignia_AdjustPos_Buildings.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.Buildings");
	this->DrawInsignia_AdjustPos_BuildingsAnchor.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.BuildingsAnchor");
	this->DrawInsignia_AdjustPos_Units.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.AdjustPos.Units");
	this->DrawInsignia_UsePixelSelectionBracketDelta.Read(exINI, GameStrings::AudioVisual, "DrawInsignia.UsePixelSelectionBracketDelta");

#pragma region Otamaa
	this->DisplayCreditsDelay.Read(exINI, GameStrings::AudioVisual(), "DisplayCreditsDelay");
	this->TypeSelectUseDeploy.Read(exINI, GameStrings::General(), "TypeSelectUseDeploy");
	this->AIDetectDisguise_Percent.Read(exINI, GameStrings::General(), "AIDisguiseDetectionPercent");
	this->CanDrive.Read(exINI, GameStrings::General(), "EveryoneCanDrive");
	this->TogglePowerAllowed.Read(exINI, GameStrings::General(), "TogglePowerAllowed");
	this->TogglePowerDelay.Read(exINI, GameStrings::General(), "TogglePowerDelay");
	this->TogglePowerIQ.Read(exINI, "IQ", "TogglePower");

	this->GainSelfHealAllowMultiplayPassive.Read(exINI, GameStrings::General(), "GainSelfHealAllowMultiplayPassive");
	this->GainSelfHealFromPlayerControl.Read(exINI, GameStrings::General, "GainSelfHealFromPlayerControl");
	this->GainSelfHealFromAllies.Read(exINI, GameStrings::General, "GainSelfHealFromAllies");

	this->VeinsDamagingWeightTreshold.Read(exINI, GameStrings::General(), "VeinsDamagingWeightTreshold");
	this->VeinholePal.Read(exINI, GameStrings::General(), "VeinholePalette");
	this->DegradeEnabled.Read(exINI, GameStrings::General(), "Degrade.Enabled");
	this->DegradePercentage.Read(exINI, GameStrings::General(), "Degrade.Percentage");
	this->DegradeAmountNormal.Read(exINI, GameStrings::General(), "Degrade.AmountNormal");
	this->DegradeAmountConsumer.Read(exINI, GameStrings::General(), "Degrade.AmountConsumer");

	this->AllowParallelAIQueues.Read(exINI, GLOBALCONTROLS_SECTION, "AllowParallelAIQueues");
	this->ForbidParallelAIQueues_Infantry.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Infantry");
	this->ForbidParallelAIQueues_Vehicle.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Vehicle");
	this->ForbidParallelAIQueues_Navy.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Navy");
	this->ForbidParallelAIQueues_Aircraft.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Aircraft");
	this->ForbidParallelAIQueues_Building.Read(exINI, GLOBALCONTROLS_SECTION, "ForbidParallelAIQueues.Building");

	this->EngineerDamage.Read(exINI, GameStrings::General(), "EngineerDamage");
	this->EngineerAlwaysCaptureTech.Read(exINI, GameStrings::General(), "EngineerAlwaysCaptureTech");
	this->EngineerDamageCursor.Read(exINI, GameStrings::General(), "EngineerDamageCursor");

	this->DefaultParaPlane.Read(exINI, GameStrings::General(), "ParadropPlane", true);
	this->VeinholeParticle.Read(exINI, GameStrings::AudioVisual(), "VeinholeSpawnParticleType", true);
	this->CarryAll_LandAnim.Read(exINI, GameStrings::AudioVisual(), "LandingAnim.Carryall", true);
	this->DropShip_LandAnim.Read(exINI, GameStrings::AudioVisual(), "LandingAnim.Dropship", true);
	this->Aircraft_LandAnim.Read(exINI, GameStrings::AudioVisual(), "LandingAnim.Aircraft", true);
	this->Aircraft_TakeOffAnim.Read(exINI, GameStrings::AudioVisual(), "TakeOffAnim.Aircraft", true);

	this->DropPodTrailer.Read(exINI, GameStrings::General(), "DropPodTrailer", true);
	this->ElectricDeath.Read(exINI, GameStrings::AudioVisual(), "InfantryElectrocuted");
	this->DroppodTrailerSpawnDelay.Read(exINI, GameStrings::General(), "DropPodTrailerSpawnDelay");

	this->DropPodTypes.Read(exINI, GameStrings::General(), "DropPodTypes");
	this->DropPodMinimum.Read(exINI, GameStrings::General(), "DropPodMinimum");
	this->DropPodMaximum.Read(exINI, GameStrings::General(), "DropPodMaximum");
	this->ReturnStructures.Read(exINI, GameStrings::General(), "ReturnStructures");

	this->MessageSilosNeeded.Read(exINI, GameStrings::General(), "Message.SilosNeeded");

	this->HunterSeekerBuildings.Read(exINI, GameStrings::SpecialWeapons(), "HSBuilding");
	this->HunterSeekerDetonateProximity.Read(exINI, GameStrings::General(), "HunterSeekerDetonateProximity");
	this->HunterSeekerDescendProximity.Read(exINI, GameStrings::General(), "HunterSeekerDescendProximity");
	this->HunterSeekerAscentSpeed.Read(exINI, GameStrings::General(), "HunterSeekerAscentSpeed");
	this->HunterSeekerDescentSpeed.Read(exINI, GameStrings::General(), "HunterSeekerDescentSpeed");
	this->HunterSeekerEmergeSpeed.Read(exINI, GameStrings::General(), "HunterSeekerEmergeSpeed");
	this->Units_UnSellable.Read(exINI, GameStrings::General(), "UnitsUnsellable");
	this->DrawTurretShadow.Read(exINI, GameStrings::AudioVisual(), "DrawTurretShadow");
	this->AnimRemapDefaultColorScheme.Read(exINI, GameStrings::AudioVisual(), "AnimRemapDefaultColorScheme");

	this->Veins_PerCellAmount.Read(exINI, GameStrings::General(), "VeinsPerCellStorageAmount");
	this->MultipleFactoryCap.Read(exINI, GameStrings::General());

#pragma endregion

	this->StealthSpeakDelay.Read(exINI, GameStrings::AudioVisual(), "StealthSpeakDelay");
	this->SubterraneanSpeakDelay.Read(exINI, GameStrings::AudioVisual(), "SubterraneanSpeakDelay");
	this->RandomCrateMoney.Read(exINI, GameStrings::CrateRules, "RandomCrateMoney");
	this->ChronoSparkleDisplayDelay.Read(exINI, GameStrings::General(), "ChronoSparkleDisplayDelay");
	this->ChronoSparkleBuildingDisplayPositions.Read(exINI, GameStrings::General(), "ChronoSparkleBuildingDisplayPositions");
	this->RepairStopOnInsufficientFunds.Read(exINI, GameStrings::General(), "RepairStopOnInsufficientFunds");

	this->BerserkROFMultiplier.Read(exINI, GameStrings::CombatDamage(), "BerserkROFMultiplier");
	this->TeamRetaliate.Read(exINI, GameStrings::General(), "TeamRetaliate");
	this->AI_CostMult.Read(exINI, GameStrings::General(), "AICostMult");

	this->DeactivateDim_Powered.Read(exINI, GameStrings::AudioVisual(), "DeactivateDimPowered");
	this->DeactivateDim_EMP.Read(exINI, GameStrings::AudioVisual(), "DeactivateDimEMP");
	this->DeactivateDim_Operator.Read(exINI, GameStrings::AudioVisual(), "DeactivateDimOperator");

#pragma region Otamaa
	this->AI_SpyMoneyStealPercent.Read(exINI, GameStrings::General(), "AI.SpyMoneyStealPercent");
	this->AutoAttackICedTarget.Read(exINI, GameStrings::CombatDamage(), "Firing.AllowICedTargetForAI");
	this->NukeWarheadName.Read(exINI.GetINI(), GameStrings::SpecialWeapons(), "NukeWarhead");
	this->AI_AutoSellHealthRatio.Read(exINI, GameStrings::General(), "AI.AutoSellHealthRatio");

	this->Building_PlacementPreview.Read(exINI, GameStrings::AudioVisual(), "ShowBuildingPlacementPreview");
	this->Building_PlacementPreview.Read(exINI, GameStrings::AudioVisual(), "PlacementPreview");

	this->PlacementGrid_TranslucencyWithPreview.Read(exINI, GameStrings::AudioVisual, "PlacementGrid.TranslucencyWithPreview");
	this->DisablePathfindFailureLog.Read(exINI, GameStrings::General(), "DisablePathfindFailureLog");
	this->CreateSound_PlayerOnly.Read(exINI, GameStrings::AudioVisual(), "CreateSound.AffectOwner");
	this->DoggiePanicMax.Read(exINI, GameStrings::CombatDamage(), "DoggiePanicMax");
	this->HunterSeeker_Damage.Read(exINI, GameStrings::CombatDamage(), "HunterSeekerDamage");
	this->AutoRepelAI.Read(exINI, GameStrings::CombatDamage(), "AutoRepel");
	this->AutoRepelPlayer.Read(exINI, GameStrings::CombatDamage(), "PlayerAutoRepel");
	this->AIFriendlyDistance.Read(exINI, GameStrings::General(), "AIFriendlyDistance");

	this->MyPutData.Read(exINI, GameStrings::General());

#pragma endregion
	this->Storage_TiberiumIndex.Read(exINI, GameStrings::General(), "Storage.TiberiumIndex");

	this->RadApplicationDelay_Building.Read(exINI, GameStrings::Radiation(), "RadApplicationDelay.Building");
	this->RadBuildingDamageMaxCount.Read(exINI, GameStrings::Radiation, "RadBuildingDamageMaxCount");
	this->RadWarhead_Detonate.Read(exINI, GameStrings::Radiation(), "RadSiteWarhead.Detonate");
	this->RadHasOwner.Read(exINI, GameStrings::Radiation(), "RadHasOwner");
	this->RadHasInvoker.Read(exINI, GameStrings::Radiation(), "RadHasInvoker");
	this->UseGlobalRadApplicationDelay.Read(exINI, GameStrings::Radiation, "UseGlobalRadApplicationDelay");

	this->IronCurtain_KeptOnDeploy.Read(exINI, GameStrings::CombatDamage(), "IronCurtain.KeptOnDeploy");
	this->ForceShield_KeptOnDeploy.Read(exINI, GameStrings::CombatDamage(), "ForceShield.KeptOnDeploy");

	this->ForceShield_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "ForceShield.EffectOnOrganics");

	this->IronCurtain_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "IronCurtain.EffectOnOrganics");

	this->ROF_RandomDelay.Read(exINI, GameStrings::CombatDamage, "ROF.RandomDelay");

	this->Pips_Shield.Read(exINI, GameStrings::AudioVisual(), "Pips.Shield");
	this->Pips_Shield_Buildings.Read(exINI, GameStrings::AudioVisual(), "Pips.Shield.Building");
	this->MissingCameo.Read(pINI, GameStrings::AudioVisual(), "MissingCameo");
	this->JumpjetAllowLayerDeviation.Read(exINI, GameStrings::JumpjetControls(), "AllowLayerDeviation");
	this->JumpjetTurnToTarget.Read(exINI, GameStrings::JumpjetControls(), "TurnToTarget");
	this->JumpjetCrash_Rotate.Read(exINI, GameStrings::JumpjetControls(), "CrashRotate");

	this->JumpjetClimbPredictHeight.Read(exINI, GameStrings::General, "JumpjetClimbPredictHeight");
	this->JumpjetClimbWithoutCutOut.Read(exINI, GameStrings::General, "JumpjetClimbWithoutCutOut");

	this->PlacementGrid_TranslucentLevel.Read(exINI, GameStrings::AudioVisual(), !Phobos::Otamaa::CompatibilityMode ? "BuildingPlacementGrid.TranslucentLevel" : "PlacementGrid.Translucency");
	this->BuildingPlacementPreview_TranslucentLevel.Read(exINI, GameStrings::AudioVisual(), !Phobos::Otamaa::CompatibilityMode ? "BuildingPlacementPreview.DefaultTranslucentLevel" : "PlacementPreview.Translucency");
	this->Pips_Shield.Read(exINI, GameStrings::AudioVisual(), "Pips.Shield");
	this->Pips_Shield_Background_SHP.Read(exINI, GameStrings::AudioVisual(), "Pips.Shield.Background");
	this->Pips_Shield_Building.Read(exINI, GameStrings::AudioVisual(), "Pips.Shield.Building");
	this->Pips_Shield_Building_Empty.Read(exINI, GameStrings::AudioVisual(), "Pips.Shield.Building.Empty");

	this->Pips_SelfHeal_Infantry.Read(exINI, GameStrings::AudioVisual(), "Pips.SelfHeal.Infantry");
	this->Pips_SelfHeal_Units.Read(exINI, GameStrings::AudioVisual(), "Pips.SelfHeal.Units");
	this->Pips_SelfHeal_Buildings.Read(exINI, GameStrings::AudioVisual(), "Pips.SelfHeal.Buildings");
	this->Pips_SelfHeal_Infantry_Offset.Read(exINI, GameStrings::AudioVisual(), "Pips.SelfHeal.Infantry.Offset");
	this->Pips_SelfHeal_Units_Offset.Read(exINI, GameStrings::AudioVisual(), "Pips.SelfHeal.Units.Offset");
	this->Pips_SelfHeal_Buildings_Offset.Read(exINI, GameStrings::AudioVisual(), "Pips.SelfHeal.Buildings.Offset");

	this->Pips_Generic_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Generic.Size");
	this->Pips_Generic_Buildings_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Generic.Buildings.Size");
	this->Pips_Ammo_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Ammo.Size");
	this->Pips_Ammo_Buildings_Size.Read(exINI, GameStrings::AudioVisual, "Pips.Ammo.Buildings.Size");

	this->Pips_Tiberiums_Frames.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.Frames");
	this->Pips_Tiberiums_DisplayOrder.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.DisplayOrder");

	this->ToolTip_Background_Color.Read(exINI, GameStrings::AudioVisual(), "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, GameStrings::AudioVisual(), "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, GameStrings::AudioVisual(), "ToolTip.Background.BlurSize");
	this->ToolTip_ExcludeSidebar.Read(exINI, GameStrings::AudioVisual(), "ToolTip.ExcludeSidebar");

	this->Crate_LandOnly.Read(exINI, GameStrings::CrateRules(), "Crate.LandOnly");
	this->UnitCrateVehicleCap.Read(exINI, GameStrings::CrateRules, "UnitCrateVehicleCap");
	this->FreeMCV_CreditsThreshold.Read(exINI, GameStrings::CrateRules, "FreeMCV.CreditsThreshold");

	this->InfantryGainSelfHealCap.Read(exINI, GameStrings::General(), "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GameStrings::General(), "UnitsGainSelfHealCap");

	this->EnemyInsignia.Read(exINI, GameStrings::General(), "EnemyInsignia");
	this->DisguiseBlinkingVisibility.Read(exINI, GameStrings::General(), "DisguiseBlinkingVisibility");

	this->UseSelectBrd.Read(exINI, GameStrings::AudioVisual(), "UseSelectBrd");
	this->SHP_SelectBrdSHP_INF.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.SHP.Infantry");
	this->SHP_SelectBrdPAL_INF.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.PAL.Infantry");
	this->SelectBrd_Frame_Infantry.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.Frame.Infantry");
	this->SelectBrd_DrawOffset_Infantry.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.DrawOffset.Infantry");
	this->SHP_SelectBrdSHP_UNIT.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.SHP.Unit");
	this->SHP_SelectBrdPAL_UNIT.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.PAL.Unit");
	this->SelectBrd_Frame_Unit.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.Frame.Unit");
	this->SelectBrd_DrawOffset_Unit.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.DrawOffset.Unit");
	this->SelectBrd_DefaultTranslucentLevel.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.DefaultTranslucentLevel");
	this->SelectBrd_DefaultShowEnemy.Read(exINI, GameStrings::AudioVisual(), "SelectBrd.DefaultShowEnemy");

	this->VeteranFlashTimer.Read(exINI, GameStrings::AudioVisual(), "VeteranFlashTimer");

	this->Tiberium_DamageEnabled.Read(exINI, GameStrings::General(), "TiberiumDamageEnabled");
	this->Tiberium_HealEnabled.Read(exINI, GameStrings::General(), "TiberiumHealEnabled");
	this->Tiberium_ExplosiveWarhead.Read(exINI, GameStrings::CombatDamage(), "TiberiumExplosiveWarhead");
	this->Tiberium_ExplosiveAnim.Read(exINI, GameStrings::AudioVisual(), "TiberiumExplosiveAnim");
	this->OverlayExplodeThreshold.Read(exINI, GameStrings::General(), "OverlayExplodeThreshold");
	this->AlliedSolidTransparency.Read(exINI, GameStrings::CombatDamage(), "AlliedSolidTransparency");
	this->DecloakSound.Read(exINI, GameStrings::AudioVisual(), "DecloakSound");
	this->IC_Flash.Read(exINI, GameStrings::AudioVisual(), "IronCurtainFlash");

	this->ChainReact_Multiplier.Read(exINI, GameStrings::CombatDamage(), "ChainReact.Multiplier");
	this->ChainReact_SpreadChance.Read(exINI, GameStrings::CombatDamage(), "ChainReact.SpreadChance");
	this->ChainReact_MinDelay.Read(exINI, GameStrings::CombatDamage(), "ChainReact.MinDelay");
	this->ChainReact_MaxDelay.Read(exINI, GameStrings::CombatDamage(), "ChainReact.MaxDelay");

	this->ChronoInfantryCrush.Read(exINI, GameStrings::General(), "ChronoInfantryCrush");

	this->DamageAirConsiderBridges.Read(exINI, GameStrings::CombatDamage(), "DamageAirConsiderBridges");
	this->DiskLaserAnimEnabled.Read(exINI, GameStrings::AudioVisual(), "DiskLaserAnimEnabled");

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

	this->EnemyWrench.Read(exINI, GameStrings::General(), "EnemyWrench");
	this->Bounty_Value_Option.Read(exINI, GameStrings::General(), "BountyRewardOption");
	this->EMPAIRecoverMission.Read(exINI, GameStrings::CombatDamage(), "EMPAIRecoverMission");
	this->TimerBlinkColorScheme.Read(exINI, GameStrings::AudioVisual, "TimerBlinkColorScheme");

	this->CloakHeight.Read(exINI, GameStrings::General(), "CloakHeight");

	if(Phobos::Config::ShowFlashOnSelecting){
		this->SelectFlashTimer.Read(exINI, GameStrings::AudioVisual, "SelectFlashTimer");
		this->SelectFlashTimer.Read(exINI, GameStrings::AudioVisual , "SelectionFlashDuration");
	}

	this->WarheadParticleAlphaImageIsLightFlash.Read(exINI, GameStrings::AudioVisual, "WarheadParticleAlphaImageIsLightFlash");
	this->CombatLightDetailLevel.Read(exINI, GameStrings::AudioVisual, "CombatLightDetailLevel");
	this->LightFlashAlphaImageDetailLevel.Read(exINI, GameStrings::AudioVisual, "LightFlashAlphaImageDetailLevel");

	this->RegroupWhenMCVDeploy.Read(exINI, GameStrings::General, "GatherWhenMCVDeploy");
	this->AISellAllOnLastLegs.Read(exINI, GameStrings::General, "AIFireSale");
	this->AISellAllDelay.Read(exINI, GameStrings::General, "AIFireSaleDelay");
	this->AIAllInOnLastLegs.Read(exINI, GameStrings::General, "AIAllToHunt");
	this->RepairBaseNodes.Read(exINI, GameStrings::General, "RepairBaseNodes");
	this->MCVRedeploysInCampaign.Read(exINI, GameStrings::General, "MCVRedeploysInCampaign");
}

void RulesExtData::LoadEarlyOptios(RulesClass* pThis, CCINIClass* pINI)
{ }

void RulesExtData::LoadEarlyBeforeColor(RulesClass* pThis, CCINIClass* pINI)
{
	ArmorTypeClass::AddDefaults();
	CursorTypeClass::AddDefaults();
	SelectBoxTypeClass::AddDefaults();
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

void RulesExtData::LoadBeforeGeneralData(RulesClass* pThis, CCINIClass* pINI)
{
}

void RulesExtData::LoadAfterAllLogicData(RulesClass* pThis, CCINIClass* pINI)
{
}

// =============================
// load / save

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
		.Process(this->UseGlobalRadApplicationDelay)
		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->ForceShield_KeptOnDeploy)
		.Process(this->ForceShield_EffectOnOrganics)
		.Process(this->ForceShield_KillOrganicsWarhead)
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
		.Process(this->LightFlashAlphaImageDetailLevel)

		.Process(this->Promote_Vet_Anim)
		.Process(this->Promote_Elite_Anim)

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
		.Process(this->DefautBulletType)
		.Process(this->AIChronoSphereSW)
		.Process(this->AIChronoWarpSW)
		.Process(this->DamageOwnerMultiplier)
		.Process(this->DamageAlliesMultiplier)
		.Process(this->DamageEnemiesMultiplier)
		.Process(this->DamageEnemiesMultiplier_UsedForAllTargetInBerzerk)
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

		.Process(this->AttackMove_IgnoreWeaponCheck)
		.Process(this->AttackMove_StopWhenTargetAcquired)
		.Process(this->PenetratesTransport_Level)
		.Process(this->DamageWallRecursivly)
		.Process(this->AirstrikeLineZAdjust)
		.Process(this->AdjacentWallDamage)
		.Process(this->AISellCapturedBuilding)
		.Process(this->InfantryAutoDeploy)
		.Process(this->EnablePassiveAcquireMode)
		.Process(this->UseRetintFix)
		.Process(this->MyPutData)

		.Process(this->AIAdjacentMax)
		.Process(this->AIAdjacentMax_Campaign)

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
		;
}

// =============================
// container hooks

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

	if (R->Origin() == 0x675210)
	{
		pItem->BarrelDebris.Clear();
		pItem->DeadBodies.Clear();
		pItem->DropPod.Clear();
		pItem->MetallicDebris.Clear();
		pItem->BridgeExplosions.Clear();
		pItem->DamageFireTypes.Clear();
		pItem->WeatherConClouds.Clear();
		pItem->WeatherConBolts.Clear();
	}

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

DEFINE_JUMP(LJMP, 0x52C9C4, 0x52CA37);

// Read on very first RulesClass::Process function
ASMJIT_PATCH(0x668BF0, RulesClass_Process_Addition, 0x5)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	RulesExtData::s_LoadFromINIFile(pItem, pINI);

	return 0;
}

ASMJIT_PATCH(0x668D86, RulesData_Process_PreFillTypeListData, 0x6)
{
	GET(CCINIClass*, pINI, ESI);

	for (int nn = 0; nn < pINI->GetKeyCount("Projectiles"); ++nn)
	{
		if (pINI->GetString("Projectiles", pINI->GetKeyName("Projectiles", nn), Phobos::readBuffer))
		{
			BulletTypeClass::FindOrAllocate(Phobos::readBuffer);
		}
	}

	for (int i = 0; i < pINI->GetKeyCount(GameStrings::Tiberiums()); ++i)
	{
		if (pINI->ReadString(GameStrings::Tiberiums(), pINI->GetKeyName(GameStrings::Tiberiums(), i), Phobos::readDefval, Phobos::readBuffer) > 0)
		{
			TiberiumClass::FindOrAllocate(Phobos::readBuffer);
		}
	}

	RulesExtData::Instance()->DefautBulletType = BulletTypeClass::FindOrAllocate(DEFAULT_STR2);
	if(!RulesExtData::Instance()->DefautBulletType)
		Debug::FatalError("Uneable to Allocate {} BulletType ! " , DEFAULT_STR2);

	for (int nn = 0; nn < pINI->GetKeyCount("WeaponTypes"); ++nn)
	{
		if (pINI->GetString("WeaponTypes", pINI->GetKeyName("WeaponTypes", nn), Phobos::readBuffer))
		{
			WeaponTypeClass::FindOrAllocate(Phobos::readBuffer);
		}
	}

	for (int nn = 0; nn < pINI->GetKeyCount("Warheads"); ++nn)
	{
		if (pINI->GetString("Warheads", pINI->GetKeyName("Warheads", nn), Phobos::readBuffer))
		{
			WarheadTypeClass::FindOrAllocate(Phobos::readBuffer);
		}
	}

	return 0x668DD2;
}

void FakeRulesClass::_ReadColors(CCINIClass* pINI)
{
	RulesExtData::LoadEarlyBeforeColor(this, pINI);

	this->Read_JumpjetControls(pINI);
	this->Read_Colors(pINI);

	RocketTypeClass::LoadFromINIList(pINI);
}

void FakeRulesClass::_ReadGeneral(CCINIClass* pINI)
{
	RulesExtData::LoadBeforeGeneralData(this, pINI);
	this->Read_General(pINI);


	RocketTypeClass::AddDefaults();
	RocketTypeClass::ReadListFromINI(pINI);

	SideClass::Array->for_each([pINI](SideClass* pSide) {
		SideExtContainer::Instance.LoadFromINI(pSide, pINI, !pINI->GetSection(pSide->ID));
	});

	HouseTypeClass::Array->for_each([pINI](HouseTypeClass* pHouse) {
		HouseTypeExtContainer::Instance.LoadFromINI(pHouse, pINI, !pINI->GetSection(pHouse->ID));
	});

	// All TypeClass Created but not yet read INI
	//	RulesClass::Initialized = true;

	RulesExtData::s_LoadBeforeTypeData(this, pINI);
	this->Read_Types(pINI);

	// Ensure entry not fail because of late instantiation
	// add more if needed , it will double the error log at some point
	// but it will take care some of missing stuffs that previously loaded late

	for (auto pWeapon : *WeaponTypeClass::Array) {
		pWeapon->LoadFromINI(pINI);
	}

	for (auto pBullet : *BulletTypeClass::Array) {
		pBullet->LoadFromINI(pINI);
	}

	for (auto pWarhead : *WarheadTypeClass::Array) {
		pWarhead->LoadFromINI(pINI);
	}

	for (auto pAnims : *AnimTypeClass::Array) {
		pAnims->LoadFromINI(pINI);
	}

	RulesExtData::LoadAfterTypeData(this, pINI);
	this->Read_Difficulties(pINI);
	RulesExtData::LoadAfterAllLogicData(this, pINI);

	for (auto pTib : *TiberiumClass::Array) {
		//Debug::LogInfo("Reading Tiberium[{}] Configurations!", pTib->ID);
		pTib->LoadFromINI(pINI);
	}
}

DEFINE_FUNCTION_JUMP(CALL, 0x668BFE, FakeRulesClass::_ReadColors);
DEFINE_FUNCTION_JUMP(CALL, 0x668EE8, FakeRulesClass::_ReadGeneral);

#include <Misc/PhobosGlobal.h>

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

	auto g_scenario_instance = ScenarioExtData::Instance();

	// Init master bullet
	g_scenario_instance->MasterDetonationBullet = BulletTypeExtData::GetDefaultBulletType()->CreateBullet(nullptr, nullptr, 0, nullptr, 0, false);
}

ASMJIT_PATCH(0x668EED, RulesData_InitializeAfterAllLoaded, 0x8)
{
	RulesExtData::InitializeAfterAllRulesLoaded();
	return 0x668F6A;
}

ASMJIT_PATCH(0x6744E4, RulesClass_ReadJumpjetControls_Extra, 0x7)
{
	GET(CCINIClass*, pINI, EDI);
	INI_EX exINI(pINI);

	RulesExtData::Instance()->JumpjetCrash.Read(exINI, GameStrings::JumpjetControls, "Crash");
	RulesExtData::Instance()->JumpjetNoWobbles.Read(exINI, GameStrings::JumpjetControls, "NoWobbles");

	return 0;
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

	if (auto pPlayerSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex)) {
		if (auto pSideMouse = SideExtContainer::Instance.Find(pPlayerSide)->MouseShape) {
			GameDelete<true, true>(std::exchange(MouseClass::ShapeData(), pSideMouse));
		}
	}

	HouseClass::Array->for_each([](HouseClass* it)
 {
	 const auto pType = HouseTypeClass::Array->GetItemOrDefault(it->Type->ArrayIndex);
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

// remove reading warhead from `SpecialWeapon`
DEFINE_PATCH_TYPED(BYTE, 0x669193
	, 0x5B //pop EBX
	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
	, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
	, 0x90, 0x90, 0x90, 0x90, 0x90
);

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