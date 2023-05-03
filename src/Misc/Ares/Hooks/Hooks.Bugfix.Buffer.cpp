#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <Utilities/TemplateDefB.h>

#include <WeaponTypeClass.h>
#include <CCINIClass.h>
#include <AnimTypeClass.h>
#include <TerrainTypeClass.h>
#include <SmudgeTypeClass.h>

template<typename T, bool Allocate = false, bool Unique = false>
static void ParseList(DynamicVectorClass<T>& List, CCINIClass* pINI, const char* section, const char* key) {
	if (pINI->ReadString(section, key, Phobos::readDefval, Phobos::readBuffer)) {
		List.Clear();
		char* context = nullptr;

		if constexpr (std::is_pointer<T>())
		{
			using BaseType = std::remove_pointer_t<T>;

			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur;
				 cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				BaseType* buffer = nullptr;
				if constexpr (Allocate)
				{
					buffer = BaseType::FindOrAllocate(cur);
				} else {
					buffer = BaseType::Find(cur);
				}

				if (buffer) {

					//if (VTable::Get(buffer) != BaseType::vtable)
					//	Debug::FatalError("%s possibly using wrong function , please check ! \n", AbstractClass::GetAbstractClassName(BaseType::AbsID));

					if constexpr (!Unique) {
						List.AddItem(buffer);
					} else {
						List.AddUnique(buffer);
					}
				}
				else if (!INIClass::IsBlank(cur) && BaseType::Array->IsAllocated && BaseType::Array->Count > 0)
				{
					Debug::INIParseFailed(section, key, cur);
				}
			}
		}
		else
		{
			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur;
				 cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				T buffer = T();
				if (Parser<T>::TryParse(cur, &buffer)) {
					if constexpr (!Unique) {
						List.AddItem(buffer);
					} else {
						List.AddUnique(buffer);
					}
				} else if (!INIClass::IsBlank(cur)) {
					Debug::INIParseFailed(section, key, cur);
				}
			}
		}
	}
};

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
	
	ParseList<AnimTypeClass*, true>(pThis->Anim, pINI, pSection, GameStrings::Anim());

	return 0x77255F;
}

// == WarheadType ==
DEFINE_OVERRIDE_HOOK(0x75D660, WarheadTypeClass_LoadFromINI_ListLength, 9)
{
	GET(WarheadTypeClass*, pThis, ESI);
	GET(const char*, pSection, EBP);
	GET(CCINIClass*, pINI, EDI);

	ParseList<AnimTypeClass*, true>(pThis->AnimList, pINI, pSection, GameStrings::AnimList);
	ParseList(pThis->DebrisMaximums, pINI, pSection, GameStrings::DebrisMaximums);
	ParseList<VoxelAnimTypeClass*, true>(pThis->DebrisTypes, pINI, pSection, GameStrings::DebrisTypes);

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

	ParseList<ParticleSystemTypeClass*, true>(pThis->DamageParticleSystems, pINI, pSection, GameStrings::DamageParticleSystems);
	ParseList<ParticleSystemTypeClass*, true>(pThis->DestroyParticleSystems, pINI, pSection, GameStrings::DestroyParticleSystems);

	ParseList<BuildingTypeClass*, true>(pThis->Dock, pINI, pSection, GameStrings::Dock);

	ParseList(pThis->DebrisMaximums, pINI, pSection, GameStrings::DebrisMaximums);
	ParseList<VoxelAnimTypeClass*, true>(pThis->DebrisTypes, pINI, pSection, GameStrings::DebrisTypes);
	ParseList<AnimTypeClass*, true>(pThis->DebrisAnims, pINI, pSection, GameStrings::DebrisAnims);

	return 0x712830;
}

// ============= [AI] =============

DEFINE_OVERRIDE_HOOK(0x672B0E, Buf_AI, 6)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	const char* section = GameStrings::AI;
	ParseList<BuildingTypeClass*, true>(pRules->BuildConst, pINI, section, GameStrings::BuildConst);
	ParseList<BuildingTypeClass*, true>(pRules->BuildPower, pINI, section, GameStrings::BuildPower);
	ParseList<BuildingTypeClass*, true>(pRules->BuildRefinery, pINI, section, GameStrings::BuildRefinery);
	ParseList<BuildingTypeClass*, true>(pRules->BuildBarracks, pINI, section, GameStrings::BuildBarracks);
	ParseList<BuildingTypeClass*, true>(pRules->BuildTech, pINI, section, GameStrings::BuildTech);
	ParseList<BuildingTypeClass*, true>(pRules->BuildWeapons, pINI, section, GameStrings::BuildWeapons);
	ParseList<BuildingTypeClass*, true>(pRules->AlliedBaseDefenses, pINI, section, GameStrings::AlliedBaseDefenses);
	ParseList<BuildingTypeClass*, true>(pRules->SovietBaseDefenses, pINI, section, GameStrings::SovietBaseDefenses);
	ParseList<BuildingTypeClass*, true>(pRules->ThirdBaseDefenses, pINI, section, GameStrings::ThirdBaseDefenses);
	ParseList<BuildingTypeClass*, true>(pRules->BuildDefense, pINI, section, GameStrings::BuildDefense);
	ParseList<BuildingTypeClass*, true>(pRules->BuildPDefense, pINI, section, GameStrings::BuildPDefense);
	ParseList<BuildingTypeClass*, true>(pRules->BuildAA, pINI, section, GameStrings::BuildAA);
	ParseList<BuildingTypeClass*, true>(pRules->BuildHelipad, pINI, section, GameStrings::BuildHelipad);
	ParseList<BuildingTypeClass*, true>(pRules->BuildRadar, pINI, section, GameStrings::BuildRadar);
	ParseList<BuildingTypeClass*, true>(pRules->ConcreteWalls, pINI, section, GameStrings::ConcreteWalls);
	ParseList<BuildingTypeClass*, true>(pRules->NSGates, pINI, section, GameStrings::NSGates);
	ParseList<BuildingTypeClass*, true>(pRules->EWGates, pINI, section, GameStrings::EWGates);
	ParseList<BuildingTypeClass*, true>(pRules->BuildNavalYard, pINI, section, GameStrings::BuildNavalYard);
	ParseList<BuildingTypeClass*, true>(pRules->BuildDummy, pINI, section, GameStrings::BuildDummy);
	ParseList<BuildingTypeClass*, true>(pRules->NeutralTechBuildings, pINI, section, GameStrings::NeutralTechBuildings);

	ParseList(pRules->AIForcePredictionFudge, pINI, section, GameStrings::AIForcePredictionFudge);

	return 0x673950;
}

// ============= [CombatDamage] =============
DEFINE_OVERRIDE_HOOK(0x66BC71, Buf_CombatDamage, 9)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	pRules->TiberiumStrength = R->EAX<int>();

	ParseList<SmudgeTypeClass*, true>(pRules->Scorches, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches);
	ParseList<SmudgeTypeClass*, true>(pRules->Scorches1, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches1);
	ParseList<SmudgeTypeClass*, true>(pRules->Scorches2, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches2);
	ParseList<SmudgeTypeClass*, true>(pRules->Scorches3, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches3);
	ParseList<SmudgeTypeClass*, true>(pRules->Scorches4, pINI, COMBATDAMAGE_SECTION, GameStrings::Scorches4);

	ParseList<AnimTypeClass*, true>(pRules->SplashList, pINI, COMBATDAMAGE_SECTION, GameStrings::SplashList);
	return 0x66C287;
}

// ============= [General] =============
DEFINE_OVERRIDE_HOOK(0x66D55E, Buf_General, 6)
{
	GET(RulesClass*, pRules, ESI);
	GET(CCINIClass*, pINI, EDI);

	const char* section = GENERAL_SECTION;
	ParseList<InfantryTypeClass*, true>(pRules->AmerParaDropInf, pINI, section, GameStrings::AmerParaDropInf);
	ParseList<InfantryTypeClass*, true>(pRules->AllyParaDropInf, pINI, section, GameStrings::AllyParaDropInf);
	ParseList<InfantryTypeClass*, true>(pRules->SovParaDropInf, pINI, section, GameStrings::SovParaDropInf);
	ParseList<InfantryTypeClass*, true>(pRules->YuriParaDropInf, pINI, section, GameStrings::YuriParaDropInf);

	ParseList(pRules->AmerParaDropNum, pINI, section, GameStrings::AmerParaDropNum);
	ParseList(pRules->AllyParaDropNum, pINI, section, GameStrings::AllyParaDropNum);
	ParseList(pRules->SovParaDropNum, pINI, section, GameStrings::SovParaDropNum);
	ParseList(pRules->YuriParaDropNum, pINI, section, GameStrings::YuriParaDropNum);

	ParseList<InfantryTypeClass*, true>(pRules->AnimToInfantry, pINI, section, GameStrings::AnimToInfantry);

	ParseList<InfantryTypeClass*, true>(pRules->SecretInfantry, pINI, section, GameStrings::SecretInfantry);
	ParseList<UnitTypeClass*, true>(pRules->SecretUnits, pINI, section, GameStrings::SecretUnits);
	ParseList<BuildingTypeClass*, true>(pRules->SecretBuildings, pINI, section, GameStrings::SecretBuildings);

	pRules->SecretSum = pRules->SecretInfantry.Count
		+ pRules->SecretUnits.Count
		+ pRules->SecretBuildings.Count;

	ParseList<UnitTypeClass*, true>(pRules->HarvesterUnit, pINI, section, GameStrings::HarvesterUnit);
	ParseList<UnitTypeClass*, true>(pRules->BaseUnit, pINI, section, GameStrings::BaseUnit);
	ParseList<AircraftTypeClass*, true>(pRules->PadAircraft, pINI, section, GameStrings::PadAircraft);

	ParseList<BuildingTypeClass*, true>(pRules->Shipyard, pINI, section, GameStrings::Shipyard);
	ParseList<BuildingTypeClass*, true>(pRules->RepairBay, pINI, section, GameStrings::RepairBay);

	ParseList<AnimTypeClass*, true>(pRules->WeatherConClouds, pINI, section, GameStrings::WeatherConClouds);
	ParseList<AnimTypeClass*, true>(pRules->WeatherConBolts, pINI, section, GameStrings::WeatherConBolts);
	ParseList<AnimTypeClass*, true>(pRules->BridgeExplosions, pINI, section, GameStrings::BridgeExplosions);

	ParseList<TerrainTypeClass*, true>(pRules->DefaultMirageDisguises, pINI, section, GameStrings::DefaultMirageDisguises);

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

	ParseList<InfantryTypeClass*, true>(pHouseType->VeteranInfantry, pINI, pHouseType->ID, GameStrings::VeteranInfantry);
	ParseList<UnitTypeClass*, true>(pHouseType->VeteranUnits, pINI, pHouseType->ID, GameStrings::VeteranUnits);
	ParseList<AircraftTypeClass*, true>(pHouseType->VeteranAircraft, pINI, pHouseType->ID, GameStrings::VeteranAircraft);

	return 0x51208C;
}