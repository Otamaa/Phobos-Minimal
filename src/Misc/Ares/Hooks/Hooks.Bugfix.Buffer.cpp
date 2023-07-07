#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <Utilities/TemplateDefB.h>

#include <WeaponTypeClass.h>
#include <CCINIClass.h>
#include <AnimTypeClass.h>
#include <TerrainTypeClass.h>
#include <SmudgeTypeClass.h>


DEFINE_OVERRIDE_HOOK(0x7274AF, TriggerTypeClass_LoadFromINI_Read_Events, 5)
{
	R->Stack(0x0, Phobos::readBuffer);
	R->Stack(0x4, Phobos::readLength);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x7274C8, TriggerTypeClass_LoadFromINI_Strtok_Events, 5)
{
	R->ECX(Phobos::readBuffer);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x727529, TriggerTypeClass_LoadFromINI_Read_Actions, 5)
{
	R->Stack(0x0, Phobos::readBuffer);
	R->Stack(0x4, Phobos::readLength);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x727544, TriggerTypeClass_LoadFromINI_Strtok_Actions, 5)
{
	R->EDX(Phobos::readBuffer);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4750EC, INIClass_ReadHouseTypesList, 7)
{
	R->Stack(0x0, Phobos::readBuffer);
	R->Stack(0x4, Phobos::readLength);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x475107, INIClass_ReadHouseTypesList_Strtok, 5)
{
	R->ECX(Phobos::readBuffer);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x47527C, INIClass_GetAlliesBitfield, 7)
{
	R->Stack(0x0, Phobos::readBuffer);
	R->Stack(0x4, Phobos::readLength);
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x475297, INIClass_GetAlliesBitfield_Strtok, 5)
{
	R->ECX(Phobos::readBuffer);
	return 0;
}

// == WeaponType ==
DEFINE_OVERRIDE_HOOK(0x772462, WeaponTypeClass_LoadFromINI_ListLength, 0x9)
{
	GET(WeaponTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBX);
	GET(CCINIClass*, pINI, EDI);

	INI_EX exINI(pINI);
	detail::ParseVector<AnimTypeClass*, true>(pThis->Anim, exINI, pSection, GameStrings::Anim(), "Expect valid AnimType");

	return 0x77255F;
}

// == WarheadType ==
DEFINE_OVERRIDE_HOOK(0x75D660, WarheadTypeClass_LoadFromINI_ListLength, 9)
{
	GET(WarheadTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBP);
	GET(CCINIClass*, pINI, EDI);

	INI_EX exINI(pINI);
	detail::ParseVector<AnimTypeClass*, true>(pThis->AnimList, exINI, pSection, GameStrings::AnimList, "Expect valid AnimType");
	detail::ParseVector(pThis->DebrisMaximums, exINI, pSection, GameStrings::DebrisMaximums, "Expect valid number");
	detail::ParseVector<VoxelAnimTypeClass*>(pThis->DebrisTypes, exINI, pSection, GameStrings::DebrisTypes, "Expect valid VoxelAnimType");

	return 0x75D75D;
}

DEFINE_OVERRIDE_HOOK(0x6A9348, StripClass_GetTip_FixLength, 9)
{
	DWORD HideObjectName = R->AL();

	GET(TechnoTypeClass*, Object, ESI);

	int Cost = Object->GetActualCost(HouseClass::CurrentPlayer);
	if (HideObjectName)
	{
		const wchar_t* Format = StringTable::LoadString(GameStrings::TXT_MONEY_FORMAT_1);
		_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, Cost);
	}
	else
	{
		const wchar_t* UIName = Object->UIName;
		const wchar_t* Format = StringTable::LoadString(GameStrings::TXT_MONEY_FORMAT_2);
		_snwprintf_s(SidebarClass::TooltipBuffer, SidebarClass::TooltipLength, SidebarClass::TooltipLength - 1, Format, UIName, Cost);
	}

	SidebarClass::TooltipBuffer[SidebarClass::TooltipBuffer.size() - 1] = 0;

	return 0x6A93B2;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x75DAE6, WarheadTypeClass_LoadFromINI_SkipLists, 9, 75DDCC)

DEFINE_OVERRIDE_HOOK(0x713171, TechnoTypeClass_LoadFromINI_SkipLists1, 9)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(Category, category, EAX);
	pThis->Category = category;
	return 0x713264;
}

DEFINE_OVERRIDE_HOOK(0x713C10, TechnoTypeClass_LoadFromINI_SkipLists2, 7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(const CoordStruct*, pResult, EAX);
	pThis->NaturalParticleSystemLocation = *pResult;
	return 0x713E1A;
}

// == TechnoType ==
DEFINE_OVERRIDE_HOOK(0x7125DF, TechnoTypeClass_LoadFromINI_ListLength, 7)
{
	GET(TechnoTypeClass*, pThis, EBP);
	GET(const char*, pSection, EBX);
	GET(CCINIClass*, pINI, ESI);

	INI_EX exINI(pINI);

	detail::ParseVector<ParticleSystemTypeClass*>(pThis->DamageParticleSystems, exINI, pSection, GameStrings::DamageParticleSystems, "Expect valid ParticleSystemType");
	detail::ParseVector<ParticleSystemTypeClass*>(pThis->DestroyParticleSystems, exINI, pSection, GameStrings::DestroyParticleSystems, "Expect valid ParticleSystemType");

	detail::ParseVector<BuildingTypeClass*>(pThis->Dock, exINI, pSection, GameStrings::Dock , "Expect valid BuildingType");

	detail::ParseVector(pThis->DebrisMaximums, exINI, pSection, GameStrings::DebrisMaximums, "Expect valid number");
	detail::ParseVector<VoxelAnimTypeClass*>(pThis->DebrisTypes, exINI, pSection, GameStrings::DebrisTypes, "Expect valid VoxelAnimType");
	detail::ParseVector<AnimTypeClass*>(pThis->DebrisAnims, exINI, pSection, GameStrings::DebrisAnims, "Expect valid AnimType");

	return 0x712830;
}

// ============= [AI] =============

DEFINE_OVERRIDE_HOOK(0x672B0E, Buf_AI, 6)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	INI_EX exINI(pINI);
	const char* section = GameStrings::AI;
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildConst, exINI, section, GameStrings::BuildConst, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildPower, exINI, section, GameStrings::BuildPower, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildRefinery, exINI, section, GameStrings::BuildRefinery, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildBarracks, exINI, section, GameStrings::BuildBarracks, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildTech, exINI, section, GameStrings::BuildTech, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildWeapons, exINI, section, GameStrings::BuildWeapons, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->AlliedBaseDefenses, exINI, section, GameStrings::AlliedBaseDefenses, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->SovietBaseDefenses, exINI, section, GameStrings::SovietBaseDefenses, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->ThirdBaseDefenses, exINI, section, GameStrings::ThirdBaseDefenses, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildDefense, exINI, section, GameStrings::BuildDefense, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildPDefense, exINI, section, GameStrings::BuildPDefense, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildAA, exINI, section, GameStrings::BuildAA, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildHelipad, exINI, section, GameStrings::BuildHelipad, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildRadar, exINI, section, GameStrings::BuildRadar, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->ConcreteWalls, exINI, section, GameStrings::ConcreteWalls, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->NSGates, exINI, section, GameStrings::NSGates, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->EWGates, exINI, section, GameStrings::EWGates, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildNavalYard, exINI, section, GameStrings::BuildNavalYard, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildDummy, exINI, section, GameStrings::BuildDummy, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->NeutralTechBuildings, exINI, section, GameStrings::NeutralTechBuildings, "Expect valid BuildingType");

	detail::ParseVector(pRules->AIForcePredictionFudge, exINI, section, GameStrings::AIForcePredictionFudge, "Expect valid number");

	return 0x673950;
}

// ============= [CombatDamage] =============
DEFINE_OVERRIDE_HOOK(0x66BC71, Buf_CombatDamage, 9)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	pRules->TiberiumStrength = R->EAX<int>();

	INI_EX exINI(pINI);
	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches, exINI, COMBATDAMAGE_SECTION, GameStrings::Scorches, "Expect valid SmudgeType");
	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches1, exINI, COMBATDAMAGE_SECTION, GameStrings::Scorches1, "Expect valid SmudgeType");
	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches2, exINI, COMBATDAMAGE_SECTION, GameStrings::Scorches2, "Expect valid SmudgeType");
	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches3, exINI, COMBATDAMAGE_SECTION, GameStrings::Scorches3, "Expect valid SmudgeType");
	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches4, exINI, COMBATDAMAGE_SECTION, GameStrings::Scorches4, "Expect valid SmudgeType");

	detail::ParseVector<AnimTypeClass*, true>(pRules->SplashList, exINI, COMBATDAMAGE_SECTION, GameStrings::SplashList, "Expect valid AnimType");
	return 0x66C287;
}

// ============= [General] =============
DEFINE_OVERRIDE_HOOK(0x66D55E, Buf_General, 6)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	INI_EX exINI(pINI);
	const char* section = GENERAL_SECTION;
	detail::ParseVector<InfantryTypeClass*, true>(pRules->AmerParaDropInf, exINI, section, GameStrings::AmerParaDropInf, "Expect valid InfantryType");
	detail::ParseVector<InfantryTypeClass*, true>(pRules->AllyParaDropInf, exINI, section, GameStrings::AllyParaDropInf, "Expect valid InfantryType");
	detail::ParseVector<InfantryTypeClass*, true>(pRules->SovParaDropInf, exINI, section, GameStrings::SovParaDropInf, "Expect valid InfantryType");
	detail::ParseVector<InfantryTypeClass*, true>(pRules->YuriParaDropInf, exINI, section, GameStrings::YuriParaDropInf, "Expect valid InfantryType");

	detail::ParseVector(pRules->AmerParaDropNum, exINI, section, GameStrings::AmerParaDropNum, "Expect valid number");
	detail::ParseVector(pRules->AllyParaDropNum, exINI, section, GameStrings::AllyParaDropNum, "Expect valid number");
	detail::ParseVector(pRules->SovParaDropNum, exINI, section, GameStrings::SovParaDropNum, "Expect valid number");
	detail::ParseVector(pRules->YuriParaDropNum, exINI, section, GameStrings::YuriParaDropNum, "Expect valid number");

	detail::ParseVector<InfantryTypeClass*, true>(pRules->AnimToInfantry, exINI, section, GameStrings::AnimToInfantry, "Expect valid InfantryType");

	detail::ParseVector<InfantryTypeClass*, true>(pRules->SecretInfantry, exINI, section, GameStrings::SecretInfantry, "Expect valid InfantryType");
	detail::ParseVector<UnitTypeClass*, true>(pRules->SecretUnits, exINI, section, GameStrings::SecretUnits, "Expect valid UnitType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->SecretBuildings, exINI, section, GameStrings::SecretBuildings, "Expect valid BuildingType");

	pRules->SecretSum = pRules->SecretInfantry.Count
		+ pRules->SecretUnits.Count
		+ pRules->SecretBuildings.Count;

	detail::ParseVector<UnitTypeClass*, true>(pRules->HarvesterUnit, exINI, section, GameStrings::HarvesterUnit, "Expect valid UnitType");
	detail::ParseVector<UnitTypeClass*, true>(pRules->BaseUnit, exINI, section, GameStrings::BaseUnit, "Expect valid UnitType");
	detail::ParseVector<AircraftTypeClass*, true>(pRules->PadAircraft, exINI, section, GameStrings::PadAircraft, "Expect valid AircraftType");

	detail::ParseVector<BuildingTypeClass*, true>(pRules->Shipyard, exINI, section, GameStrings::Shipyard, "Expect valid BuildingType");
	detail::ParseVector<BuildingTypeClass*, true>(pRules->RepairBay, exINI, section, GameStrings::RepairBay, "Expect valid BuildingType");

	detail::ParseVector<AnimTypeClass*, true>(pRules->WeatherConClouds, exINI, section, GameStrings::WeatherConClouds, "Expect valid AnimType");
	detail::ParseVector<AnimTypeClass*, true>(pRules->WeatherConBolts, exINI, section, GameStrings::WeatherConBolts, "Expect valid AnimType");
	detail::ParseVector<AnimTypeClass*, true>(pRules->BridgeExplosions, exINI, section, GameStrings::BridgeExplosions, "Expect valid AnimType");

	detail::ParseVector<TerrainTypeClass*, true>(pRules->DefaultMirageDisguises, exINI, section, GameStrings::DefaultMirageDisguises, "Expect valid TerrainType");

	if (pINI->ReadString(section, GameStrings::WallTower, nullptr, Phobos::readBuffer))
	{
		if (const auto pBuilding = BuildingTypeClass::FindOrAllocate(Phobos::readBuffer))
		{
			pRules->WallTower = pBuilding;
		}
		else
		{
			Debug::Log("WallTower Building is nullptr ! \n");
		}
	}

	return 0;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x67062F, Buf_AnimToInf_Paradrop, 6, 6707FE)
DEFINE_OVERRIDE_SKIP_HOOK(0x66FA13, Buf_SecretBoons, 6, 66FAD6)
DEFINE_OVERRIDE_SKIP_HOOK(0x66f589, Buf_Shipyard, 6, 66F68C)

DEFINE_OVERRIDE_HOOK(0x66F7C0, Buf_PPA, 9)
{
	GET(RulesClass*, Rules, ESI);
	GET(UnitTypeClass*, Pt, EAX); // recreating overwritten bits
	Rules->PrerequisiteProcAlternate = Pt;
	return 0x66F9FA;
}

DEFINE_OVERRIDE_HOOK(0x66F34B, Buf_RepairBay, 5)
{
	GET(RulesClass*, Rules, ESI);
	Rules->NoParachuteMaxFallRate = R->EAX<int>();
	return 0x66F450;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x66DD13, Buf_WeatherArt, 6, 66DF19)
DEFINE_OVERRIDE_SKIP_HOOK(0x66DB93, Buf_BridgeExplosions, 6, 66DC96)

DEFINE_OVERRIDE_HOOK(0x511D16, HouseTypeClass_LoadFromINI_Buffer_CountryVeteran, 9)
{
	GET(HouseTypeClass*, pHouseType, EBX);
	GET(CCINIClass*, pINI, ESI);

	INI_EX exINI(pINI);
	detail::ParseVector<InfantryTypeClass*,true>(pHouseType->VeteranInfantry, exINI, pHouseType->ID, GameStrings::VeteranInfantry, "Expect valid InfantryType");
	detail::ParseVector<UnitTypeClass*,true>(pHouseType->VeteranUnits, exINI, pHouseType->ID, GameStrings::VeteranUnits, "Expect valid UnitType");
	detail::ParseVector<AircraftTypeClass*,true>(pHouseType->VeteranAircraft, exINI, pHouseType->ID, GameStrings::VeteranAircraft, "Expect valid AircraftType");

	return 0x51208C;
}