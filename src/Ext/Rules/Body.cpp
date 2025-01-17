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

	Nullable<double> Shield_ConditionGreen_d;
	Nullable<double> Shield_ConditionYellow_d;
	Nullable<double> Shield_ConditionRed_d;
	Nullable<double> ConditionYellow_Terrain_d;

	Shield_ConditionGreen_d.Read(iniEX, AUDIOVISUAL_SECTION, "Shield.ConditionGreen");// somewhat never used , man
	Shield_ConditionYellow_d.Read(iniEX, AUDIOVISUAL_SECTION, "Shield.ConditionYellow");
	Shield_ConditionRed_d.Read(iniEX, AUDIOVISUAL_SECTION, "Shield.ConditionRed");
	ConditionYellow_Terrain_d.Read(iniEX, AUDIOVISUAL_SECTION, "ConditionYellow.Terrain");

	pData->Shield_ConditionGreen = Shield_ConditionGreen_d.Get(pRules->ConditionGreen);
	pData->Shield_ConditionYellow = Shield_ConditionYellow_d.Get(pRules->ConditionYellow);
	pData->Shield_ConditionRed = Shield_ConditionRed_d.Get(pRules->ConditionRed);
	pData->ConditionYellow_Terrain = ConditionYellow_Terrain_d.Get(pRules->ConditionRed);
}

DEFINE_HOOK(0x66B8E2, RulesClass_ReadAudioVisual_End, 0x5)
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

	ImmunityTypeClass::LoadFromINIList(pINI);
	ArmorTypeClass::EvaluateDefault();

	TrailType::LoadFromINIList(&CCINIClass::INI_Art.get());

	if (!Phobos::Otamaa::DisableCustomRadSite)
	{
		RadTypeClass::LoadFromINIOnlyTheList(pINI);
	}

	ShieldTypeClass::LoadFromINIOnlyTheList(pINI);
	HoverTypeClass::LoadFromINIOnlyTheList(pINI);

	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
	DigitalDisplayTypeClass::LoadFromINIList(pINI);

	PhobosAttachEffectTypeClass::LoadFromINIOnlyTheList(pINI);
	TechTreeTypeClass::LoadFromINIOnlyTheList(pINI);

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
	INI_EX iniEX(pINI);
	auto pData = RulesExtData::Instance();

	CrateTypeClass::ReadListFromINI(pINI);
	HoverTypeClass::ReadListFromINI(pINI);
	ShieldTypeClass::ReadListFromINI(pINI);
	RadTypeClass::ReadListFromINI(pINI);
	PhobosAttachEffectTypeClass::ReadListFromINI(pINI);
	TechTreeTypeClass::ReadListFromINI(pINI);

	pData->VoxelLightSource.Read(iniEX, GameStrings::AudioVisual, "VoxelLightSource");
	pData->VoxelShadowLightSource.Read(iniEX, GameStrings::AudioVisual, "VoxelShadowLightSource");
	pData->UseFixedVoxelLighting.Read(iniEX, GameStrings::AudioVisual, "UseFixedVoxelLighting");

	pData->ReplaceVoxelLightSources();

	//got invalidated early , so parse it again
	detail::ParseVector(iniEX, pData->AITargetTypesLists, "AITargetTypes");
	detail::ParseVector<ScriptTypeClass*, true>(iniEX, pData->AIScriptsLists, "AIScriptsList");
	detail::ParseVector<HouseTypeClass*>(iniEX, pData->AIHateHousesLists, "AIHateHousesList");
	detail::ParseVector<HouseTypeClass*>(iniEX, pData->AIHousesLists, "AIHousesList");
	detail::ParseVector(iniEX, pData->AIConditionsLists, "AIConditionsList", true, false, "/");
	detail::ParseVector<AITriggerTypeClass*>(iniEX, pData->AITriggersLists, "AITriggersList");

	pData->AIChronoSphereSW.Read(iniEX, GameStrings::General, "AIChronoSphereSW");
	pData->AIChronoWarpSW.Read(iniEX, GameStrings::General, "AIChronoWarpSW");

	pData->DamageOwnerMultiplier.Read(iniEX, GameStrings::CombatDamage, "DamageOwnerMultiplier");
	pData->DamageAlliesMultiplier.Read(iniEX, GameStrings::CombatDamage, "DamageAlliesMultiplier");
	pData->DamageEnemiesMultiplier.Read(iniEX, GameStrings::CombatDamage, "DamageEnemiesMultiplier");

	pData->FactoryProgressDisplay.Read(iniEX, GameStrings::AudioVisual, "FactoryProgressDisplay");
	pData->MainSWProgressDisplay.Read(iniEX, GameStrings::AudioVisual, "MainSWProgressDisplay");
	pData->CombatAlert.Read(iniEX, GameStrings::AudioVisual, "CombatAlert");
	pData->CombatAlert_IgnoreBuilding.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.IgnoreBuilding");
	pData->CombatAlert_EVA.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.EVA");
	pData->CombatAlert_SuppressIfInScreen.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.SuppressIfInScreen");
	pData->CombatAlert_Interval.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.Interval");
	pData->CombatAlert_SuppressIfAllyDamage.Read(iniEX, GameStrings::AudioVisual, "CombatAlert.SuppressIfAllyDamage");
	pData->SubterraneanHeight.Read(iniEX, GameStrings::General, "SubterraneanHeight");

	pData->ForceShield_KillOrganicsWarhead.Read(iniEX, COMBATDAMAGE_SECTION, "ForceShield.KillOrganicsWarhead");

	if (!pData->ForceShield_KillOrganicsWarhead)
		pData->ForceShield_KillOrganicsWarhead = pThis->C4Warhead;

	pData->IronCurtain_KillOrganicsWarhead.Read(iniEX, COMBATDAMAGE_SECTION, "IronCurtain.KillOrganicsWarhead");

	if (!pData->IronCurtain_KillOrganicsWarhead)
		pData->IronCurtain_KillOrganicsWarhead = pThis->C4Warhead;

	pData->DefaultAircraftDamagedSmoke = AnimTypeClass::Find(GameStrings::SGRYSMK1());
	pData->FirestormActiveAnim.Read(iniEX, AUDIOVISUAL_SECTION, "FirestormActiveAnim");
	pData->FirestormIdleAnim.Read(iniEX, AUDIOVISUAL_SECTION, "FirestormIdleAnim");
	pData->FirestormGroundAnim.Read(iniEX, AUDIOVISUAL_SECTION, "FirestormGroundAnim");
	pData->FirestormAirAnim.Read(iniEX, AUDIOVISUAL_SECTION, "FirestormAirAnim");
	pData->FirestormWarhead.Read(iniEX, COMBATDAMAGE_SECTION, "FirestormWarhead");
	pData->DamageToFirestormDamageCoefficient.Read(iniEX, GENERAL_SECTION, "DamageToFirestormDamageCoefficient");

	pData->Bounty_Enablers.Read(iniEX, GENERAL_SECTION, "BountyEnablers");
	pData->Bounty_Display.Read(iniEX, AUDIOVISUAL_SECTION, "BountyDisplay");
	pData->CloakAnim.Read(iniEX, AUDIOVISUAL_SECTION, "CloakAnim");
	pData->DecloakAnim.Read(iniEX, AUDIOVISUAL_SECTION, "DecloakAnim");
	pData->Cloak_KickOutParasite.Read(iniEX, GameStrings::CombatDamage, "Cloak.KickOutParasite");

	pData->Veinhole_Warhead.Read(iniEX, COMBATDAMAGE_SECTION, "VeinholeWarhead");

	pData->WallTowers.Read(iniEX, GENERAL_SECTION, "WallTowers");
	if (pThis->WallTower && !pData->WallTowers.Contains(pThis->WallTower))
		pData->WallTowers.push_back(pThis->WallTower);

	pData->Promote_Vet_Anim.Read(iniEX, AUDIOVISUAL_SECTION, "Promote.VeteranAnim");
	pData->Promote_Elite_Anim.Read(iniEX, AUDIOVISUAL_SECTION, "Promote.EliteAnim");
	pData->PrimaryFactoryIndicator.Read(iniEX, AUDIOVISUAL_SECTION, "PrimaryFactoryIndicator");
	pData->PrimaryFactoryIndicator_Palette.Read(iniEX, AUDIOVISUAL_SECTION, "PrimaryFactoryIndicator.Palette");

	pData->DefaultExplodeFireAnim.Read(iniEX, AUDIOVISUAL_SECTION, "DefaultExplodeOverlayFireAnim");

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
static COMPILETIMEEVAL void FillSecrets(DynamicVectorClass<T>& secrets) {

	for(auto Option : secrets){
		RulesExtData::Instance()->Secrets.emplace_back(Option);
		Debug::Log("Adding [%s - %s] onto Global Secrets pool\n" , Option->ID, Option->GetThisClassName());
	}
}

DEFINE_HOOK(0x687C16, INIClass_ReadScenario_ValidateThings, 6)
{	// create an array of crew for faster lookup
	std::vector<InfantryTypeClass*> Crews(SideClass::Array->Count, nullptr);
	for (int i = 0; i < SideClass::Array->Count; ++i)
	{
		auto pExt = SideExtContainer::Instance.Find(SideClass::Array->Items[i]);

		Crews[i] = pExt->GetCrew();
		// remove all types that cannot paradrop
		if (pExt->ParaDropTypes.HasValue() && !pExt->ParaDropTypes.empty())
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

		if (pItem->Strength <= 0)
		{
			const bool IsUpgradeBld = what == BuildingTypeClass::AbsID && *((BuildingTypeClass*)pItem)->PowersUpBuilding && strlen(((BuildingTypeClass*)pItem)->PowersUpBuilding) > 0;

			if ((!IsVanillaDummy(pItem->ID) || !pExt->IsDummy) && !IsUpgradeBld)
			{
				Debug::Log("TechnoType[%s - %s] , registered with 0 strength"
					", this most likely because this technotype has no rules entry"
					" or it is suppose to be an dummy\n", pItem->ID, myClassName);

				Debug::RegisterParserError();

				pExt->IsDummy = true;
			}
		}

		if (pItem->Sight < 0)
		{
			Debug::Log("TechnoType[%s - %s] , registered with less than 0 Sight , Fixing.\n",
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
			Debug::Log("TechnoType[%s - %s] , registered PromoteVet[%s] with 0 strength , Fixing.\n",
				pItem->ID, myClassName, pExt->Promote_Vet_Type->ID);

			pExt->Promote_Vet_Type = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->Promote_Elite_Type && pExt->Promote_Elite_Type->Strength <= 0)
		{
			Debug::Log("TechnoType[%s - %s] , registered PromoteElite[%s] with 0 strength , Fixing.\n",
				pItem->ID, myClassName, pExt->Promote_Elite_Type->ID);

			pExt->Promote_Elite_Type = nullptr;
			Debug::RegisterParserError();
		}

		if (pItem->DebrisTypes.Count > 0 && pItem->DebrisMaximums.Count < pItem->DebrisTypes.Count)
		{
			Debug::Log("TechnoType[%s - %s] DebrisMaximums items count is less than"
			" DebrisTypes items count it will fail when the index counter reached DebrisMaximus items count\n"
			, pItem->ID, myClassName
			);
			Debug::RegisterParserError();
		}

		if (pExt->Fake_Of && pExt->Fake_Of->WhatAmI() != what)
		{
			Debug::Log("[%s - %s] has fake of but it different ClassType from it!\n", pItem->ID, myClassName);
			pExt->Fake_Of = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->ClonedAs && pExt->ClonedAs->WhatAmI() != what)
		{
			Debug::Log("[%s - %s] has ClonedAs but it different ClassType from it!\n", pItem->ID, myClassName);
			pExt->ClonedAs = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->AI_ClonedAs && pExt->AI_ClonedAs->WhatAmI() != what)
		{
			Debug::Log("[%s - %s] has AI.ClonedAs but it different ClassType from it!\n", pItem->ID, myClassName);
			pExt->AI_ClonedAs = nullptr;
			Debug::RegisterParserError();
		}

		if (pExt->ReversedAs.Get(nullptr) && pExt->ReversedAs->WhatAmI() != what)
		{
			Debug::Log("[%s - %s] has ReversedAs but it different ClassType from it!\n", pItem->ID, pItem->ID, myClassName);
			pExt->ReversedAs.Reset();
			Debug::RegisterParserError();
		}

		if (isFoot && !pExt->IsDummy)
		{
			if (pItem->SpeedType == SpeedType::None)
			{
				Debug::Log("[%s - %s]SpeedType is invalid!\n", pItem->ID, myClassName);
				Debug::RegisterParserError();
			}

			if (pItem->MovementZone == MovementZone::None)
			{
				Debug::Log("[%s - %s]MovementZone is invalid!\n", pItem->ID, myClassName);
				Debug::RegisterParserError();
			}
		}

		if (pItem->Passengers > 0 && pItem->SizeLimit < 1)
		{
			Debug::Log("[%s - %s]Passengers=%d and SizeLimit=%d!\n",
				pItem->ID, myClassName, pItem->Passengers, pItem->SizeLimit);
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
					Debug::Log("ShadowIndex on [%s]'s image is %d, but the HVA only has %d sections.\n",
						pItem->ID, shadowIdx, layerCount);
					Debug::RegisterParserError();
				}
			}
			else
			{
				Debug::FatalError("Techno[%s - %s] Has VXL but has no HVA wtf ?\n", myClassName, pItem->ID);
			}
		}

		if (pItem->PoweredUnit && !pExt->PoweredBy.empty())
		{
			Debug::Log("[%s - %s] uses both PoweredUnit=yes and PoweredBy=!\n", pItem->ID, myClassName);
			pItem->PoweredUnit = false;
			Debug::RegisterParserError();
		}

		if (auto const pPowersUnit = pItem->PowersUnit)
		{
			if (!TechnoTypeExtContainer::Instance.Find(pPowersUnit)->PoweredBy.empty())
			{
				Debug::Log("[%s]PowersUnit=%s, but [%s] uses PoweredBy=!\n",
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
				Debug::Log("[%s]ClonedAt includes %s, but %s has Factory= settings. "
					"This combination is not supported.\n(Protip: Factory= is "
					"not what controls unit exit behaviour, WeaponsFactory= "
					"and GDI/Nod/YuriBarracks= is.)\n", pItem->ID, pCloner->ID,
					pCloner->ID);
				Debug::RegisterParserError();
			}
		}

		if (isFoot)
		{
			if (what == UnitTypeClass::AbsID)
			{
				const auto pUnit = (UnitTypeClass*)pItem;

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

			if (pBExt->CloningFacility && pBType->Factory != AbstractType::None)
			{
				pBExt->CloningFacility = false;
				Debug::Log("[%s] cannot have both CloningFacility= and Factory=.\n",
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

					Debug::Log("Building Type [%s] does not have a valid BuildCat set!\n"
							   "It was reset to %s, but you should really specify it "
							   "explicitly.\n", pBType->ID, catName);
					Debug::RegisterParserError();
				}
			}
		}

		if (WeederAndHarvesterWarning)
		{
			Debug::Log("Please choose between Weeder or (Refinery / Harvester) for [%s - %s] both cant be used at same time\n", pItem->ID, myClassName);
			Debug::RegisterParserError();
		}
	}

	COMPILETIMEEVAL auto const Msg =
		"Weapon[%s] has no %s! This usually indicates one of two things:\n"
		"- The weapon was created too late and its rules weren't read "
		"(see WEEDGUY hack);\n- The weapon's name was misspelled.\n";

	for (auto pItem : *WeaponTypeClass::Array)
	{
		if (!pItem->Warhead)
		{
			Debug::Log(Msg, pItem->ID, "Warhead");
			Debug::RegisterParserError();
		}

		if (!pItem->Projectile)
		{
			Debug::Log(Msg, pItem->ID, "Projectile");
			Debug::RegisterParserError();
		}

		const auto pExt = WeaponTypeExtContainer::Instance.Find(pItem);

		if ((pItem->IsRailgun || pExt->IsDetachedRailgun || pItem->UseSparkParticles || pItem->UseFireParticles)
				&& !pItem->AttachedParticleSystem)
		{

			Debug::Log("Weapon[%s] is an Railgun/Detached Railgun/UseSparkParticles/UseFireParticles but it missing AttachedParticleSystem", pItem->ID);
			Debug::RegisterParserError();

			pItem->IsRailgun = false;
			pExt->IsDetachedRailgun = false;
			pItem->UseSparkParticles = false;
			pItem->UseFireParticles = false;
		}
	}

	for (auto const& pConst : RulesClass::Instance->BuildConst)
	{
		if (!pConst->AIBuildThis)
		{
			Debug::Log("[AI]BuildConst= includes [%s], which doesn't have "
				"AIBuildThis=yes!\n", pConst->ID);
		}
	}

	//if (OverlayTypeClass::Array->Count > 255) {
	//	Debug::Log("Reaching over 255 OverlayTypes!.\n");
	//	Debug::RegisterParserError();
	//}

	for (auto pWH : *WarheadTypeClass::Array)
	{
		auto pWHExt = WarheadTypeExtContainer::Instance.Find(pWH);
		{
			const size_t versesSize = pWHExt->Verses.size();

			if (versesSize < ArmorTypeClass::Array.size())
			{
				Debug::Log("Inconsistent verses size of [%s - %d] Warhead with ArmorType Array[%d]\n", pWH->ID, versesSize, ArmorTypeClass::Array.size());
				Debug::RegisterParserError();
			}
		}
	}

	for (size_t i = 1; i < ShieldTypeClass::Array.size(); ++i)
	{
		if (auto pShield = ShieldTypeClass::Array[i].get())
		{
			if (pShield->Strength <= 0)
			{
				Debug::Log("[%s]ShieldType is not valid because Strength is 0.\n", pShield->Name.data());
				Debug::RegisterParserError();
			}
		}
	}

	for (auto pBullet : *BulletTypeClass::Array)
	{

		//if(PhobosTrajectoryType::TrajectoryValidation(pBullet))
		//	Debug::RegisterParserError();

		auto pExt = BulletTypeExtContainer::Instance.Find(pBullet);
		{
			if (pExt->AttachedSystem && pExt->AttachedSystem->BehavesLike != ParticleSystemTypeBehavesLike::Smoke)
			{
				Debug::Log("Bullet[%s] With AttachedSystem[%s] is not BehavesLike=Smoke!\n", pBullet->ID, pExt->AttachedSystem->ID);
				Debug::RegisterParserError();
			}
		}
	}

	for (auto pHouse : *HouseTypeClass::Array)
	{
		auto pExt = HouseTypeExtContainer::Instance.Find(pHouse);

		{
			// remove all types that cannot paradrop

			if (!pExt->ParaDropTypes.empty())
				Helpers::Alex::remove_non_paradroppables(pExt->ParaDropTypes, pHouse->ID, "ParaDrop.Types");

			if (pExt->StartInMultiplayer_Types.HasValue() && !pExt->StartInMultiplayer_Types.empty())
				Helpers::Alex::remove_non_paradroppables(pExt->StartInMultiplayer_Types, pHouse->ID, "StartInMultiplayer.Types");
		}
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

			if (!pSuperExt->DropPod_Types.empty())
				Helpers::Alex::remove_non_paradroppables(pSuperExt->DropPod_Types, pSuper->ID, "DropPod.Types");

			for (auto& para : pSuperExt->ParaDropDatas)
			{
				for (auto& pVec : para.second)
				{
					if (!pVec.Types.empty())
						Helpers::Alex::remove_non_paradroppables(pVec.Types, pSuper->ID, "ParaDrop.Types");
				}
			}
		}
	}

	//for (auto pAnim : *AnimTypeClass::Array) {
	//	if (!pAnim->GetImage()) {
	//		Debug::Log("Anim[%s] Has no proper Image!\n", pAnim->ID);
	//		Debug::RegisterParserError();
	//	}
	//}

	if (Phobos::Otamaa::StrictParser && Phobos::Otamaa::ParserErrorDetected)
	{
		Debug::FatalErrorAndExit(
			"One or more errors were detected while parsing the INI files.\r\n"
			"Please review the contents of the debug log and correct them.");
	}

	return 0x0;
}

// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
void RulesExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	CursorTypeClass::AddDefaults();
	CrateTypeClass::AddDefaults();
	ArmorTypeClass::AddDefaults();

	if (!Phobos::Otamaa::DisableCustomRadSite)
		RadTypeClass::AddDefaults();

	GenericPrerequisite::AddDefaults();
	HoverTypeClass::AddDefaults();
	ShieldTypeClass::AddDefaults();
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
	}

	GenericPrerequisite::LoadFromINIList_New(pINI);

	INI_EX exINI(pINI);

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

	this->ExpandAircraftMission.Read(exINI, GameStrings::General, "ExpandAircraftMission");

	this->NoQueueUpToEnter.Read(exINI, GameStrings::General, "NoQueueUpToEnter");
	this->NoQueueUpToUnload.Read(exINI, GameStrings::General, "NoQueueUpToUnload");

	this->NoRearmInEMPState.Read(exINI, GameStrings::General, "NoRearmInEMPState");
	this->NoRearmInTemporal.Read(exINI, GameStrings::General, "NoRearmInTemporal");

	this->AttackMindControlledDelay.Read(exINI, GameStrings::General, "AttackMindControlledDelay");

	this->EnablePowerSurplus.Read(exINI, GameStrings::AI, "EnablePowerSurplus");
	this->ShakeScreenUseTSCalculation.Read(exINI, GameStrings::AudioVisual, "ShakeScreenUseTSCalculation");
	exINI.ReadSpeed(GameStrings::General, "SubterraneanSpeed", &this->SubterraneanSpeed);

	this->CheckUnitBaseNormal.Read(exINI, GameStrings::General, "CheckUnitBaseNormal");
	this->ExpandBuildingPlace.Read(exINI, GameStrings::General, "ExpandBuildingPlace");
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

	this->StartInMultiplayerUnitCost.Read(exINI, GENERAL_SECTION, "StartInMultiplayerUnitCost");
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

#pragma region Otamaa
	this->DisplayCreditsDelay.Read(exINI, AUDIOVISUAL_SECTION, "DisplayCreditsDelay");
	this->TypeSelectUseDeploy.Read(exINI, GENERAL_SECTION, "TypeSelectUseDeploy");
	this->AIDetectDisguise_Percent.Read(exINI, GENERAL_SECTION, "AIDisguiseDetectionPercent");
	this->CanDrive.Read(exINI, GENERAL_SECTION, "EveryoneCanDrive");
	this->TogglePowerAllowed.Read(exINI, GENERAL_SECTION, "TogglePowerAllowed");
	this->TogglePowerDelay.Read(exINI, GENERAL_SECTION, "TogglePowerDelay");
	this->TogglePowerIQ.Read(exINI, "IQ", "TogglePower");
	this->GainSelfHealAllowMultiplayPassive.Read(exINI, GENERAL_SECTION, "GainSelfHealAllowMultiplayPassive");
	this->VeinsDamagingWeightTreshold.Read(exINI, GENERAL_SECTION, "VeinsDamagingWeightTreshold");
	this->VeinholePal.Read(exINI, GENERAL_SECTION, "VeinholePalette");
	this->DegradeEnabled.Read(exINI, GENERAL_SECTION, "Degrade.Enabled");
	this->DegradePercentage.Read(exINI, GENERAL_SECTION, "Degrade.Percentage");
	this->DegradeAmountNormal.Read(exINI, GENERAL_SECTION, "Degrade.AmountNormal");
	this->DegradeAmountConsumer.Read(exINI, GENERAL_SECTION, "Degrade.AmountConsumer");

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
	this->DroppodTrailerSpawnDelay.Read(exINI, GENERAL_SECTION, "DropPodTrailerSpawnDelay");

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

	this->Veins_PerCellAmount.Read(exINI, GENERAL_SECTION, "VeinsPerCellStorageAmount");
	this->MultipleFactoryCap.Read(exINI, GENERAL_SECTION);

#pragma endregion

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

	if(Phobos::Config::EnableBuildingPlacementPreview) {
		this->Building_PlacementPreview.Read(exINI, AUDIOVISUAL_SECTION, "ShowBuildingPlacementPreview");
		this->Building_PlacementPreview.Read(exINI, AUDIOVISUAL_SECTION, "PlacementPreview");

		if (this->Building_PlacementPreview != Phobos::Config::EnableBuildingPlacementPreview)
			this->Building_PlacementPreview = Phobos::Config::EnableBuildingPlacementPreview;
	}

	this->PlacementGrid_TranslucencyWithPreview.Read(exINI, GameStrings::AudioVisual, "PlacementGrid.TranslucencyWithPreview");
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
	this->RadBuildingDamageMaxCount.Read(exINI, GameStrings::Radiation, "RadBuildingDamageMaxCount");
	this->RadWarhead_Detonate.Read(exINI, RADIATION_SECTION, "RadSiteWarhead.Detonate");
	this->RadHasOwner.Read(exINI, RADIATION_SECTION, "RadHasOwner");
	this->RadHasInvoker.Read(exINI, RADIATION_SECTION, "RadHasInvoker");
	this->UseGlobalRadApplicationDelay.Read(exINI, GameStrings::Radiation, "UseGlobalRadApplicationDelay");

	this->IronCurtain_KeptOnDeploy.Read(exINI, COMBATDAMAGE_SECTION, "IronCurtain.KeptOnDeploy");
	this->ForceShield_KeptOnDeploy.Read(exINI, COMBATDAMAGE_SECTION, "ForceShield.KeptOnDeploy");

	this->ForceShield_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "ForceShield.EffectOnOrganics");

	this->IronCurtain_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "IronCurtain.EffectOnOrganics");

	this->ROF_RandomDelay.Read(exINI, GameStrings::CombatDamage, "ROF.RandomDelay");

	this->Pips_Shield.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield");
	this->Pips_Shield_Buildings.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield.Building");
	this->MissingCameo.Read(pINI, AUDIOVISUAL_SECTION, "MissingCameo");
	this->JumpjetAllowLayerDeviation.Read(exINI, JUMPJET_SECTION, "AllowLayerDeviation");
	this->JumpjetTurnToTarget.Read(exINI, JUMPJET_SECTION, "TurnToTarget");
	this->JumpjetCrash_Rotate.Read(exINI, JUMPJET_SECTION, "CrashRotate");

	this->JumpjetClimbPredictHeight.Read(exINI, GameStrings::General, "JumpjetClimbPredictHeight");
	this->JumpjetClimbWithoutCutOut.Read(exINI, GameStrings::General, "JumpjetClimbWithoutCutOut");

	this->PlacementGrid_TranslucentLevel.Read(exINI, AUDIOVISUAL_SECTION, !Phobos::Otamaa::CompatibilityMode ? "BuildingPlacementGrid.TranslucentLevel" : "PlacementGrid.Translucency");
	this->BuildingPlacementPreview_TranslucentLevel.Read(exINI, AUDIOVISUAL_SECTION, !Phobos::Otamaa::CompatibilityMode ? "BuildingPlacementPreview.DefaultTranslucentLevel" : "PlacementPreview.Translucency");
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

	this->Pips_Tiberiums_Frames.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.Frames");
	this->Pips_Tiberiums_DisplayOrder.Read(exINI, GameStrings::AudioVisual, "Pips.Tiberiums.DisplayOrder");

	this->ToolTip_Background_Color.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.Background.BlurSize");
	this->ToolTip_ExcludeSidebar.Read(exINI, AUDIOVISUAL_SECTION, "ToolTip.ExcludeSidebar");

	this->Crate_LandOnly.Read(exINI, GameStrings::CrateRules(), "Crate.LandOnly");
	this->UnitCrateVehicleCap.Read(exINI, GameStrings::CrateRules, "UnitCrateVehicleCap");
	this->FreeMCV_CreditsThreshold.Read(exINI, GameStrings::CrateRules, "FreeMCV.CreditsThreshold");

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

	this->CloakHeight.Read(exINI, GENERAL_SECTION, "CloakHeight");

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
	//Debug::Log(__FUNCTION__" Called ! \n");
}

void RulesExtData::LoadAfterAllLogicData(RulesClass* pThis, CCINIClass* pINI)
{
}

// =============================
// load / save

template <typename T>
void RulesExtData::Serialize(T& Stm)
{
	//Debug::Log("Processing RulesExt ! \n");

	Stm
		.Process(this->Initialized)

		.Process(Phobos::Config::ArtImageSwap)
		.Process(Phobos::Otamaa::DisableCustomRadSite)
		.Process(Phobos::Config::ShowTechnoNamesIsActive)
		.Process(Phobos::Misc::CustomGS)
		.Process(Phobos::Config::ApplyShadeCountFix)
		.Process(Phobos::Otamaa::CompatibilityMode)

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
		.Process(this->FactoryProgressDisplay)
		.Process(this->MainSWProgressDisplay)
		.Process(this->CombatAlert)
		.Process(this->CombatAlert_IgnoreBuilding)
		.Process(this->CombatAlert_EVA)
		.Process(this->CombatAlert_SuppressIfInScreen)
		.Process(this->CombatAlert_Interval)
		.Process(this->CombatAlert_SuppressIfAllyDamage)
		.Process(this->SubterraneanSpeed)

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

		.Process(this->UseFixedVoxelLighting)
		.Process(this->AINormalTargetingDelay)
		.Process(this->PlayerNormalTargetingDelay)
		.Process(this->AIGuardAreaTargetingDelay)
		.Process(this->PlayerGuardAreaTargetingDelay)
		.Process(this->DistributeTargetingFrame)
		.Process(this->DistributeTargetingFrame_AIOnly)
		.Process(this->CheckUnitBaseNormal)
		.Process(this->ExpandBuildingPlace)
		.Process(this->DefaultExplodeFireAnim)
		.Process(this->CheckExpandPlaceGrid)
		.Process(this->ExpandLandGridFrames)
		.Process(this->ExpandWaterGridFrames)
		.Process(this->AISuperWeaponDelay)
		.Process(this->ChronoSpherePreDelay)
		.Process(this->ChronoSphereDelay)
		.Process(this->EnablePowerSurplus)
		.Process(this->ShakeScreenUseTSCalculation)
		.Process(this->SubterraneanSpeed)
		.Process(this->UnitIdleRotateTurret)
		.Process(this->UnitIdlePointToMouse)
		.Process(this->UnitIdleActionRestartMin)
		.Process(this->UnitIdleActionRestartMax)
		.Process(this->UnitIdleActionIntervalMin)
		.Process(this->UnitIdleActionIntervalMax)
		.Process(this->ExpandAircraftMission)

		.Process(this->LandTypeConfigExts)
		.Process(this->Secrets)

		.Process(this->NoQueueUpToEnter)
		.Process(this->NoQueueUpToUnload)

		.Process(this->NoRearmInEMPState)
		.Process(this->NoRearmInTemporal)

		.Process(this->AttackMindControlledDelay)

		.Process(this->Cameo_AlwaysExist)
		.Process(this->Cameo_OverlayShapes)
		.Process(this->Cameo_OverlayFrames)
		.Process(this->Cameo_OverlayPalette)
		;

	MyPutData.Serialize(Stm);

}

// =============================
// container hooks

DEFINE_HOOK(0x667A1D, RulesClass_CTOR, 0x5)
{
	GET(RulesClass*, pItem, ESI);

	RulesExtData::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x667A30, RulesClass_DTOR, 0x5)
{
	GET(RulesClass*, pItem, ECX);

	RulesExtData::Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x674730, RulesClass_SaveLoad_Prefix, 0x6)
DEFINE_HOOK(0x675210, RulesClass_SaveLoad_Prefix, 0x5)
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
}

DEFINE_HOOK(0x678841, RulesClass_Load_Suffix, 0x7)
{
	auto buffer = RulesExtData::Instance();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(RulesExtData::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(RulesExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x675205, RulesClass_Save_Suffix, 0x8)
{
	auto buffer = RulesExtData::Instance();
	/* 5 extra boolean that added to the save
		.Process(Phobos::Config::ArtImageSwap)
		.Process(Phobos::Otamaa::DisableCustomRadSite)
		.Process(Phobos::Config::ShowTechnoNamesIsActive)
		.Process(Phobos::Misc::CustomGS)
		.Process(Phobos::Config::ApplyShadeCountFix)
		.Process(Phobos::Otamaa::CompatibilityMode)
	*/
	// negative 4 for the AttachedToObjectPointer , it doesnot get S/L
	PhobosByteStream saver((sizeof(RulesExtData) - 4u) + (6 * (sizeof(bool))));
	PhobosStreamWriter writer(saver);

	writer.Save(RulesExtData::Canary);
	writer.Save(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(RulesExtData::g_pStm);

	return 0;
}

DEFINE_JUMP(LJMP, 0x52C9C4, 0x52CA37);

// Read on very first RulesClass::Process function
DEFINE_HOOK(0x668BF0, RulesClass_Process_Addition, 0x5)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	RulesExtData::s_LoadFromINIFile(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x668D86, RulesData_Process_PreFillTypeListData, 0x6)
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
		Debug::FatalError("Uneable to Allocate %s BulletType ! \n" , DEFAULT_STR2);

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

	RocketTypeClass::AddDefaults();
	RocketTypeClass::LoadFromINIList(pINI);
}

void FakeRulesClass::_ReadGeneral(CCINIClass* pINI)
{

	RulesExtData::LoadBeforeGeneralData(this, pINI);
	this->Read_General(pINI);
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
		//Debug::Log("Reading Tiberium[%s] Configurations!\n", pTib->ID);
		pTib->LoadFromINI(pINI);
	}
}

DEFINE_JUMP(CALL, 0x668BFE, MiscTools::to_DWORD(&FakeRulesClass::_ReadColors));
DEFINE_JUMP(CALL, 0x668EE8, MiscTools::to_DWORD(&FakeRulesClass::_ReadGeneral));
DEFINE_JUMP(LJMP, 0x668EED, 0x668F6A);

DEFINE_HOOK(0x6744E4, RulesClass_ReadJumpjetControls_Extra, 0x7)
{
	GET(CCINIClass*, pINI, EDI);
	INI_EX exINI(pINI);

	RulesExtData::Instance()->JumpjetCrash.Read(exINI, GameStrings::JumpjetControls, "Crash");
	RulesExtData::Instance()->JumpjetNoWobbles.Read(exINI, GameStrings::JumpjetControls, "NoWobbles");

	return 0;
}

DEFINE_HOOK(0x68684A, Game_ReadScenario_FinishReadingScenarioINI, 0x7) //9
{
	if (R->AL()) //ScenarioLoadSucceed
	{
		//pre iterate this important indexes
		//so we dont need to do lookups with name multiple times
		//these function only executed when ScenarioClass::ReadScenario return true (AL)
		if (const auto pRulesGlobal = RulesExtData::Instance())
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
	Debug::Log("Scenario Map Name [%s] \n", SessionClass::IsCampaign() || ScenarioExtData::Instance()->OriginalFilename->empty() ? SessionClass::Instance->ScenarioFilename : ScenarioExtData::Instance()->OriginalFilename->c_str());

	if (auto pPlayerSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex)) {
		if (auto pSideMouse = SideExtContainer::Instance.Find(pPlayerSide)->MouseShape) {
			GameDelete<true, true>(std::exchange(MouseClass::ShapeData(), pSideMouse));
		}
	}

	HouseClass::Array->for_each([](HouseClass* it)
 {
	 const auto pType = HouseTypeClass::Array->GetItemOrDefault(it->Type->ArrayIndex);
	 Debug::Log("Player Name: %s IsCurrentPlayer: %u; ColorScheme: %s(%d); ID: %d; HouseType: %s; Edge: %d; StartingAllies: %d; Startspot: %d,%d; Visionary: %d; MapIsClear: %u; Money: %d\n",
	 it->PlainName ? it->PlainName : NONE_STR,
	 it->IsHumanPlayer,
	 ColorScheme::Array->Items[it->ColorSchemeIndex]->ID,
	 it->ColorSchemeIndex,
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

	Debug::Log(GameStrings::Init_Commands);
	CommandClass::InitCommand();

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

DEFINE_HOOK(0x685005, Game_InitData_GlobalParticleSystem, 0x5)
{

	GET(ParticleSystemClass*, pMem, ESI);

	const auto pGlobalType = RulesExtData::Instance()->DefaultGlobalParticleInstance;

	if (!pGlobalType)
		Debug::FatalErrorAndExit("Cannot Find DefaultGlobalParticleInstance it will crash the game !\n");

	if (pGlobalType->Lifetime != -1)
		Debug::FatalErrorAndExit("DefaultGlobalParticleInstance[%s] Lifetime must be -1 , otherwise it will crash the game !\n", pGlobalType->ID);

	COMPILETIMEEVAL CoordStruct dummycoord { 2688  , 2688  , 0 };
	pMem->ParticleSystemClass::ParticleSystemClass(pGlobalType.Get(), dummycoord, nullptr, nullptr, CoordStruct::Empty, nullptr);
	R->EAX(pMem);
	return 0x685040;
}