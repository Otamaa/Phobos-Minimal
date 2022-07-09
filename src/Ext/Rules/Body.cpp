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

#include <Ext/BuildingType/Body.h>

#ifdef COMPILE_PORTED_DP_FEATURES
#include <Misc/DynamicPatcher/Trails/TrailType.h>
#endif
template<> const DWORD Extension<RulesClass>::Canary = 0x12341234;
std::unique_ptr<RulesExt::ExtData> RulesExt::Data = nullptr;

void RulesExt::Allocate(RulesClass* pThis)
{
	Data = std::make_unique<RulesExt::ExtData>(pThis);
}

void RulesExt::Remove(RulesClass* pThis)
{
	Data = nullptr;
}

void RulesExt::LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI)
{
	Data->LoadFromINI(pINI);
}

void RulesExt::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	ArmorTypeClass::LoadFromINIList_New(pINI, false);
#ifdef COMPILE_PORTED_DP_FEATURES
	TrailType::LoadFromINIList(&CCINIClass::INI_Art.get(), true);
#endif
	if (!Phobos::Otamaa::DisableCustomRadSite)
		RadTypeClass::LoadFromINIList(pINI);

	ShieldTypeClass::LoadFromINIList(pINI);
	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
	HoverTypeClass::LoadFromINIList(pINI);

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	if (pINI == CCINIClass::INI_Rules)
		Data->InitializeAfterTypeData(pThis);

	Data->LoadAfterTypeData(pThis, pINI);
}

void RulesExt::ExtData::InitializeConstants()
{

}

// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
void RulesExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{

}

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData)
		return;

	INI_EX exINI(pINI);
#pragma region Otamaa
	this->NukeWarheadName.Read(exINI.GetINI(), "SpecialWeapons", "NukeWarhead");
	this->AI_AutoSellHealthRatio.Read(exINI, GENERAL_SECTION, "AI.AutoSellHealthRatio");
	this->Building_PlacementPreview.Read(exINI, AUDIOVISUAL_SECTION, "ShowBuildingPlacementPreview");
	this->DisablePathfindFailureLog.Read(exINI, GENERAL_SECTION, "DisablePathfindFailureLog");
	this->CreateSound_PlayerOnly.Read(exINI, AUDIOVISUAL_SECTION, "CreateSound.PlayerOnly");
#ifdef COMPILE_PORTED_DP_FEATURES
	this->MyPutData.Read(exINI, GENERAL_SECTION);
#endif
#pragma endregion
	this->Storage_TiberiumIndex.Read(exINI, GENERAL_SECTION, "Storage.TiberiumIndex");

	this->RadApplicationDelay_Building.Read(exINI, RADIATION_SECTION, "RadApplicationDelay.Building");
	this->RadWarhead_Detonate.Read(exINI, RADIATION_SECTION, "RadSiteWarhead.Detonate");
	this->RadHasOwner.Read(exINI, RADIATION_SECTION, "RadHasOwner");
	this->RadHasInvoker.Read(exINI, RADIATION_SECTION, "RadHasInvoker");

	this->Pips_Shield.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield");
	this->Pips_Shield_Buildings.Read(exINI, AUDIOVISUAL_SECTION, "Pips.Shield.Building");
	this->MissingCameo.Read(pINI, AUDIOVISUAL_SECTION, "MissingCameo");
	this->JumpjetAllowLayerDeviation.Read(exINI, JUMPJET_SECTION, "AllowLayerDeviation");
	this->JumpjetTurnToTarget.Read(exINI, JUMPJET_SECTION, "TurnToTarget");
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
}

void RulesExt::LoadEarlyBeforeColor(RulesClass* pThis, CCINIClass* pINI)
{
	if (!Phobos::Otamaa::IsAdmin)
	{
		std::string_view ModNameTemp;
		pINI->ReadString(GENERAL_SECTION, "Name", "", Phobos::readBuffer);
		ModNameTemp = Phobos::readBuffer;

		if (!ModNameTemp.empty())
		{
			std::size_t nFInd = ModNameTemp.find("Rise of the East");

			if (nFInd == std::string::npos)
				nFInd = ModNameTemp.find("Ion Shock");

			if (nFInd == std::string::npos)
				nFInd = ModNameTemp.find("New War");

			if (nFInd == std::string::npos)
				nFInd = ModNameTemp.find("NewWar");

			if (nFInd == std::string::npos)
			{
				MessageBoxW(NULL,
					L"This is not Official Phobos Build.\n\n"
					L"Please contact original Mod Author for Assistance !.",
					L"Warning !", MB_OK);
			}
		}
	}

	if (!Phobos::Otamaa::DisableCustomRadSite)
		RadTypeClass::AddDefaults();

	ArmorTypeClass::AddDefaults();
	ShieldTypeClass::AddDefaults();
	HoverTypeClass::AddDefaults();

	if (!Phobos::Otamaa::IsAdmin)
		Phobos::Config::DevelopmentCommands = pINI->ReadBool(GLOBALCONTROLS_SECTION, "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);

	Phobos::Otamaa::DisableCustomRadSite = pINI->ReadBool(PHOBOS_STR, "DisableCustomRadSite", false);
	Phobos::Config::ArtImageSwap = pINI->ReadBool(GENERAL_SECTION, "ArtImageSwap", false);
	Phobos::Config::AllowParallelAIQueues = pINI->ReadBool(GLOBALCONTROLS_SECTION, "AllowParallelAIQueues", false);

}

// this runs between the before and after type data loading methods for rules ini
void RulesExt::ExtData::InitializeAfterTypeData(RulesClass* const pThis) { }

namespace ObjectTypeParser
{
	template<typename T>
	void Exec(CCINIClass* pINI, DynamicVectorClass<DynamicVectorClass<T*>>& nVecDest, const char* pSection, bool bDebug = true, bool bVerbose = false)
	{
		if (!pINI->GetSection(pSection))
			return;

		for (int i = 0; i < pINI->GetKeyCount(pSection); ++i)
		{
			DynamicVectorClass<T*> _Buffer;
			char* context = nullptr;
			pINI->ReadString(pSection, pINI->GetKeyName(pSection, i), "", Phobos::readBuffer);

			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
				cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
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
					if (bDebug)
						Debug::Log("ObjectTypeParser DEBUG: [%s][%d]: Error parsing [%s]\n", pSection, nVecDest.Count, cur);
				}
			}

			nVecDest.AddItem(_Buffer);
			_Buffer.Clear();
		}
	}
};

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::ExtData::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData)
		return;

	INI_EX exINI(pINI);
#pragma region Otamaa
	this->VeinholeParticle.Read(exINI, AUDIOVISUAL_SECTION, "VeinholeSpawnParticleType");

	this->CarryAll_LandAnim.Read(exINI, AUDIOVISUAL_SECTION, "LandingAnim.Carryall");
	if (!this->CarryAll_LandAnim)
		this->CarryAll_LandAnim = AnimTypeClass::Find("CARYLAND");

	this->DropShip_LandAnim.Read(exINI, AUDIOVISUAL_SECTION, "LandingAnim.Dropship");
	if (!this->DropShip_LandAnim)
		this->DropShip_LandAnim = AnimTypeClass::Find("DROPLAND");

	this->Aircraft_LandAnim.Read(exINI, AUDIOVISUAL_SECTION, "LandingAnim.Aircraft");
	this->Aircraft_TakeOffAnim.Read(exINI, AUDIOVISUAL_SECTION, "TakeOffAnim.Aircraft");

#pragma endregion

	const char* const sectionAIConditionsList  = "AIConditionsList";

	ObjectTypeParser::Exec(pINI, AITargetTypesLists, "AITargetTypes", true);
	ObjectTypeParser::Exec(pINI, AIScriptsLists, "AIScriptsList", true);

	// Section AIConditionsList
	for (int i = 0; i < pINI->GetKeyCount(sectionAIConditionsList); ++i)
	{
		DynamicVectorClass<std::string> objectsList;

		char* context = nullptr;
		pINI->ReadString(sectionAIConditionsList, pINI->GetKeyName(sectionAIConditionsList, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, "/", &context);
			cur;
			cur = strtok_s(nullptr, "/", &context)) {
			objectsList.AddItem(cur);
		}

		AIConditionsLists.AddItem(objectsList);
		objectsList.Clear();
	}
}

bool RulesExt::DetailsCurrentlyEnabled()
{
	// not only checks for the min frame rate from the rules, but also whether
	// the low frame rate is actually desired. in that case, don't reduce.
	auto const current = FPSCounter::CurrentFrameRate();
	auto const wanted = static_cast<unsigned int>(
		60 / Math::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));

	return current >= wanted || current >= Detail::GetMinFrameRate();
}

bool RulesExt::DetailsCurrentlyEnabled(int const minDetailLevel)
{
	return GameOptionsClass::Instance->DetailLevel >= minDetailLevel
		&& DetailsCurrentlyEnabled();
}

void RulesExt::LoadBeforeGeneralData(RulesClass* pThis, CCINIClass* pINI) { Debug::Log(__FUNCTION__" Called ! \n"); }

void RulesExt::LoadAfterAllLogicData(RulesClass* pThis, CCINIClass* pINI) { Debug::Log(__FUNCTION__" Called ! \n"); }

// =============================
// load / save

template <typename T>
void RulesExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Pips_Shield)
		.Process(this->Pips_Shield_Buildings)
		.Process(this->RadApplicationDelay_Building)
		.Process(this->MissingCameo)
		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->JumpjetTurnToTarget)
		.Process(this->AITargetTypesLists)
		.Process(this->AIScriptsLists)
		.Process(this->AIConditionsLists)
		.Process(this->Storage_TiberiumIndex)
		.Process(this->PlacementGrid_TranslucentLevel)
		.Process(this->BuildingPlacementPreview_TranslucentLevel)
		.Process(this->Pips_Shield)
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

		.Process(this->VeinholeParticle)
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

	//	RulesClass::Initialized = true;
	RulesExt::LoadBeforeTypeData(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679CAF, RulesData_LoadAfterTypeData, 0x5)
{
	RulesClass* pItem = RulesClass::Instance();
	GET(CCINIClass*, pINI, ESI);

	RulesExt::LoadAfterTypeData(pItem, pINI);

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
	//GET(CCINIClass*, pINI, ESI);

	for (auto const& pType : *BuildingTypeClass::Array)
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pType);
		pExt->CompleteInitialization();
	}

	return 0;
}

DEFINE_HOOK(0x68684A, Game_ReadScenario_FinishReadingScenarioINI, 0x9)
{
	if (R->AL()) //ScenarioLoadSucceed
	{
		//pre iterate this important indexes
		//so we dont need to do lookups with name multiple times
		//these function only executed when ScenarioClass::ReadScenario return true (AL)
		if (auto pRulesGlobal = RulesExt::Global()) {
			pRulesGlobal->CivilianSideIndex = SideClass::FindIndex("Civilian");
			Debug::Log("Finding Civilian Side Index[%d] ! \n" , pRulesGlobal->CivilianSideIndex);
			pRulesGlobal->NeutralCountryIndex = HouseTypeClass::FindIndexOfName("Neutral");
			Debug::Log("Finding Neutral Country Index[%d] ! \n", pRulesGlobal->NeutralCountryIndex);
			pRulesGlobal->SpecialCountryIndex = HouseTypeClass::FindIndexOfName("Special");
			Debug::Log("Finding Special Country Index[%d] ! \n", pRulesGlobal->SpecialCountryIndex);
		}
	}

	return 0x0;
}