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

	detail::ParseVector<AnimTypeClass*>(pThis->Anim, pINI, pSection, GameStrings::Anim());

	return 0x77255F;
}

// == WarheadType ==
DEFINE_OVERRIDE_HOOK(0x75D660, WarheadTypeClass_LoadFromINI_ListLength, 9)
{
	GET(WarheadTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBP);
	GET(CCINIClass*, pINI, EDI);

	detail::ParseVector<AnimTypeClass*>(pThis->AnimList, pINI, pSection, GameStrings::AnimList);
	detail::ParseVector(pThis->DebrisMaximums, pINI, pSection, GameStrings::DebrisMaximums);
	detail::ParseVector<VoxelAnimTypeClass*>(pThis->DebrisTypes, pINI, pSection, GameStrings::DebrisTypes);

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

	detail::ParseVector<ParticleSystemTypeClass*>(pThis->DamageParticleSystems, pINI, pSection, GameStrings::DamageParticleSystems);
	detail::ParseVector<ParticleSystemTypeClass*>(pThis->DestroyParticleSystems, pINI, pSection, GameStrings::DestroyParticleSystems);

	detail::ParseVector<BuildingTypeClass*>(pThis->Dock, pINI, pSection, GameStrings::Dock);

	detail::ParseVector(pThis->DebrisMaximums, pINI, pSection, GameStrings::DebrisMaximums);
	detail::ParseVector<VoxelAnimTypeClass*>(pThis->DebrisTypes, pINI, pSection, GameStrings::DebrisTypes);
	detail::ParseVector<AnimTypeClass*,true>(pThis->DebrisAnims, pINI, pSection, GameStrings::DebrisAnims);

	return 0x712830;
}

// ============= [AI] =============

DEFINE_OVERRIDE_HOOK(0x672B0E, Buf_AI, 6)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	const char* section = GameStrings::AI;
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildConst, pINI, section, GameStrings::BuildConst);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildPower, pINI, section, GameStrings::BuildPower);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildRefinery, pINI, section, GameStrings::BuildRefinery);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildBarracks, pINI, section, GameStrings::BuildBarracks);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildTech, pINI, section, GameStrings::BuildTech);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildWeapons, pINI, section, GameStrings::BuildWeapons);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->AlliedBaseDefenses, pINI, section, GameStrings::AlliedBaseDefenses);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->SovietBaseDefenses, pINI, section, GameStrings::SovietBaseDefenses);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->ThirdBaseDefenses, pINI, section, GameStrings::ThirdBaseDefenses);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildDefense, pINI, section, GameStrings::BuildDefense);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildPDefense, pINI, section, GameStrings::BuildPDefense);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildAA, pINI, section, GameStrings::BuildAA);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildHelipad, pINI, section, GameStrings::BuildHelipad);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildRadar, pINI, section, GameStrings::BuildRadar);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->ConcreteWalls, pINI, section, GameStrings::ConcreteWalls);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->NSGates, pINI, section, GameStrings::NSGates);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->EWGates, pINI, section, GameStrings::EWGates);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildNavalYard, pINI, section, GameStrings::BuildNavalYard);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->BuildDummy, pINI, section, GameStrings::BuildDummy);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->NeutralTechBuildings, pINI, section, GameStrings::NeutralTechBuildings);

	detail::ParseVector(pRules->AIForcePredictionFudge, pINI, section, GameStrings::AIForcePredictionFudge);

	return 0x673950;
}

// ============= [CombatDamage] =============
DEFINE_OVERRIDE_HOOK(0x66BC71, Buf_CombatDamage, 9)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	pRules->TiberiumStrength = R->EAX<int>();

	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches);
	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches1, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches1);
	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches2, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches2);
	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches3, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches3);
	detail::ParseVector<SmudgeTypeClass*, true>(pRules->Scorches4, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches4);

	detail::ParseVector<AnimTypeClass*, true>(pRules->SplashList, pINI, COMBATDAMAGE_SECTION, GameStrings::SplashList);
	return 0x66C287;
}

// ============= [General] =============
DEFINE_OVERRIDE_HOOK(0x66D55E, Buf_General, 6)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	const char* section = GENERAL_SECTION;
	detail::ParseVector<InfantryTypeClass*, true>(pRules->AmerParaDropInf, pINI, section, GameStrings::AmerParaDropInf);
	detail::ParseVector<InfantryTypeClass*, true>(pRules->AllyParaDropInf, pINI, section, GameStrings::AllyParaDropInf);
	detail::ParseVector<InfantryTypeClass*, true>(pRules->SovParaDropInf, pINI, section, GameStrings::SovParaDropInf);
	detail::ParseVector<InfantryTypeClass*, true>(pRules->YuriParaDropInf, pINI, section, GameStrings::YuriParaDropInf);

	detail::ParseVector(pRules->AmerParaDropNum, pINI, section, GameStrings::AmerParaDropNum);
	detail::ParseVector(pRules->AllyParaDropNum, pINI, section, GameStrings::AllyParaDropNum);
	detail::ParseVector(pRules->SovParaDropNum, pINI, section, GameStrings::SovParaDropNum);
	detail::ParseVector(pRules->YuriParaDropNum, pINI, section, GameStrings::YuriParaDropNum);

	detail::ParseVector<InfantryTypeClass*, true>(pRules->AnimToInfantry, pINI, section, GameStrings::AnimToInfantry);

	detail::ParseVector<InfantryTypeClass*, true>(pRules->SecretInfantry, pINI, section, GameStrings::SecretInfantry);
	detail::ParseVector<UnitTypeClass*, true>(pRules->SecretUnits, pINI, section, GameStrings::SecretUnits);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->SecretBuildings, pINI, section, GameStrings::SecretBuildings);

	pRules->SecretSum = pRules->SecretInfantry.Count
		+ pRules->SecretUnits.Count
		+ pRules->SecretBuildings.Count;

	detail::ParseVector<UnitTypeClass*, true>(pRules->HarvesterUnit, pINI, section, GameStrings::HarvesterUnit);
	detail::ParseVector<UnitTypeClass*, true>(pRules->BaseUnit, pINI, section, GameStrings::BaseUnit);
	detail::ParseVector<AircraftTypeClass*, true>(pRules->PadAircraft, pINI, section, GameStrings::PadAircraft);

	detail::ParseVector<BuildingTypeClass*, true>(pRules->Shipyard, pINI, section, GameStrings::Shipyard);
	detail::ParseVector<BuildingTypeClass*, true>(pRules->RepairBay, pINI, section, GameStrings::RepairBay);

	detail::ParseVector<AnimTypeClass*, true>(pRules->WeatherConClouds, pINI, section, GameStrings::WeatherConClouds);
	detail::ParseVector<AnimTypeClass*, true>(pRules->WeatherConBolts, pINI, section, GameStrings::WeatherConBolts);
	detail::ParseVector<AnimTypeClass*, true>(pRules->BridgeExplosions, pINI, section, GameStrings::BridgeExplosions);

	detail::ParseVector<TerrainTypeClass*, true>(pRules->DefaultMirageDisguises, pINI, section, GameStrings::DefaultMirageDisguises);

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

DEFINE_OVERRIDE_HOOK(0x66F7C0, Buf_PPA, 9)
{
	GET(RulesClass*, Rules, ESI);
	GET(UnitTypeClass*, Pt, EAX); // recreating overwritten bits
	Rules->PrerequisiteProcAlternate = Pt;
	return 0x66F9FA;
}

DEFINE_OVERRIDE_SKIP_HOOK(0x66F553, Buf_Shipyard, 6, 66F68C)

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

	detail::ParseVector<InfantryTypeClass*,true>(pHouseType->VeteranInfantry, pINI, pHouseType->ID, GameStrings::VeteranInfantry);
	detail::ParseVector<UnitTypeClass*,true>(pHouseType->VeteranUnits, pINI, pHouseType->ID, GameStrings::VeteranUnits);
	detail::ParseVector<AircraftTypeClass*,true>(pHouseType->VeteranAircraft, pINI, pHouseType->ID, GameStrings::VeteranAircraft);

	return 0x51208C;
}