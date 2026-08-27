#include "Body.h"

#include <Ext/Rules/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>

#include <New/Entity/DropshipLoadoutClass.h>

std::unique_ptr<ScenarioExtData> ScenarioExtData::Data;
IStream* ScenarioExtData::g_pStm;
bool ScenarioExtData::CellParsed;
bool ScenarioExtData::UpdateLightSources;

template <typename T>
void ScenarioExtData::Serialize(T& Stm)
{
	//Debug::LogInfo("Processing ScenarioExtData ! ");
	Stm

		.Process(this->Initialized)
		.Process(this->OriginalFilename)
		.Process(this->Waypoints)
		.Process(this->Local_Variables)
		.Process(this->Global_Variables)
		.Process(this->TriggerTypePlayerAtXOwners)
		.Process(this->DefinedAudioWaypoints)
		.Process(this->ParTitle)
		.Process(this->ParMessage)
		.Process(this->ScoreCampaignTheme)
		.Process(this->NextMission)

		//.Process(this->DefaultNormalLighting)
		//.Process(this->DefaultAmbientOriginal)
		//.Process(this->DefaultAmbientCurrent)
		//.Process(this->DefaultAmbientTarget)
		//.Process(this->CurrentTint_Tiles)
		//.Process(this->CurrentTint_Schemes)
		//.Process(this->CurrentTint_Hashes)
		.Process(this->AdjustLightingFix)

		.Process(this->ShowBriefing)
		.Process(this->BriefingTheme)
		.Process(this->SWSidebar_Enable)
		.Process(this->IsHouseTypeVoiceNeedCheck)
		.Process(this->SWSidebar_Indices)

		.Process(this->RecordMessages)

		.Process(this->DefaultLS640BkgdName)
		.Process(this->DefaultLS800BkgdName)
		.Process(this->DefaultLS800BkgdPal)

		.Process(this->LimboLaunchers)
		.Process(this->UndergroundTracker)
		.Process(this->FallingDownTracker)
		.Process(this->OwnedUniqueTechnos)

		.Process(this->PrismRelayClaimFrame)
		.Process(this->PrismRelayClaimMaster)
		.Process(this->PrismRelayClaimWeaponIndex)

		.Process(this->DropshipLoadout_Theme)
		.Process(this->DropshipLoadout_Money)
		.Process(this->DropshipLoadout_StartEVA)
		.Process(this->DropshipLoadout_StartingDropships)
		.Process(this->DropshipLoadout_Carriers)
		.Process(this->DropshipLoadout_Carriers_SizeLimit)
		.Process(this->DropshipLoadout_AddUnusedMoneyToPlayer)
		.Process(this->DropshipLoadout_RememberPurchasedCargo)
		.Process(this->DropshipLoadout_Palette)
		.Process(this->DropshipLoadout_Background)
		.Process(this->DropshipLoadout_UpArrow)
		.Process(this->DropshipLoadout_DownArrow)
		.Process(this->DropshipLoadout_Loadout)
		.Process(this->DropshipLoadout_LoadoutLocation)
		.Process(this->DropshipLoadout_PilotLit)
		.Process(this->DropshipLoadout_PilotLitLocation)
		.Process(this->DropshipLoadout_DGreenList)
		.Process(this->DropshipLoadout_BackgroundPCX)
		.Process(this->DropshipLoadout_UpArrowPCX)
		.Process(this->DropshipLoadout_DownArrowPCX)
		.Process(this->DropshipLoadout_LoadoutPCX)
		.Process(this->DropshipLoadout_PilotLitPCX)
		.Process(this->DropshipLoadout_DGreenListPCX)
		.Process(this->DropshipLoadout_DGreenAnimationsCount)
		.Process(this->DropshipLoadout_DGreenLocations)
		.Process(this->DropshipLoadout_UpArrowLocation)
		.Process(this->DropshipLoadout_DownArrowLocation)
		.Process(this->DropshipLoadout_SidebarCameosCount)
		.Process(this->DropshipLoadout_SidebarCameoLocations)
		.Process(this->DropshipLoadout_DropshipCameosCount)
		.Process(this->DropshipLoadout_DropshipCameoLocations)
		.Process(this->DropshipLoadout_BuyClickSound)
		.Process(this->DropshipLoadout_SellClickSound)
		.Process(this->DropshipLoadout_ArrowsClickSound)
		.Process(this->DropshipLoadout_StartingDragDropSound)
		.Process(this->DropshipLoadout_EndingDragDropSound)
		.Process(this->DropshipLoadout_AllowableUnitsLists)
		.Process(this->DropshipLoadout_AllowableUnitMaximumsLists)
		.Process(this->DropshipLoadout_ActiveTeamSuffixes)
		.Process(this->Smudges)

		.Process(this->OwnerBitfield_BuildingType)
		.Process(this->OwnerBitfield_InfantryType)
		.Process(this->OwnerBitfield_VehicleType)
		.Process(this->OwnerBitfield_NavyType)
		.Process(this->OwnerBitfield_AircraftType)

		.Process(this->MissionTimer_Type)
		.Process(this->MissionTimer_Variable)
		.Process(this->MissionTimer_Reverse)
		;

}

void ScenarioExtData::SaveVariablesToFile(bool isGlobal)
{
	const auto fileName = isGlobal ? "globals.ini" : "locals.ini";
	UniqueGamePtr<CCFileClass> pFile { GameCreate<CCFileClass>(fileName) };

	bool is_newFile = false;

	if(!pFile->IsAvaible()){
		if(!pFile->Create()) {
			return;
		}

		is_newFile = true;
		if (!pFile->IsAvaible())
			return;
	}

	if(!pFile->Open1(FileAccessMode::Write)) {
		Debug::LogInfo(__FUNCTION__" Failed to Open file {} for" , fileName);
		return;
	}

	UniqueGamePtr<CCINIClass> pINI { GameCreate<CCINIClass>() };

	if(!is_newFile)
		pINI->ReadCCFile(pFile.get());

	const auto variables = ScenarioExtData::GetVariables(isGlobal);
	for (auto& [idx, var] : *variables) {
		pINI->WriteInteger(isGlobal ? "GlobalVariables" : ScenarioClass::Instance()->FileName, var.Name, var.Value, false);
	}

	pINI->WriteCCFile(pFile.get());
	pFile->Close();
}

void ScenarioExtData::LoadVariablesToFile(bool isGlobal)
{
	const auto fileName = isGlobal ? "globals.ini" : "locals.ini";
	CCFileClass file { fileName };

	if (!file.IsAvaible())
	{
		return;
	}

	if (!file.Open1(FileAccessMode::ReadWrite))
	{
		Debug::LogInfo(" {} Failed to Open file {} for", __FUNCTION__, fileName);
		return;
	}

	CCINIClass ini {};
	ini.ReadCCFile(&file);

	const auto variables = ScenarioExtData::GetVariables(isGlobal);
	std::ranges::for_each(*variables, [&](auto& variable) {
		variable.second.Value = ini.ReadInteger(isGlobal ? "GlobalVariables" : ScenarioClass::Instance()->FileName, variable.second.Name, variable.second.Value);
	});
}

// PhobosMap<int, ExtendedVariable>* ScenarioExtData::GetVariables(bool IsGlobal)
// {
// 	if (IsGlobal)
// 		return &ScenarioExtData::Instance()->Global_Variables;
//
// 	return &ScenarioExtData::Instance()->Local_Variables;
// }

void ScenarioExtData::SetVariableToByID(const bool IsGlobal, int nIndex, char bState)
{
	//Debug::LogInfo("{} , Executed !", __FUNCTION__);

	const auto dict = ScenarioExtData::GetVariables(IsGlobal);
	auto itr = dict->tryfind(nIndex);

	if (itr && itr->Value != bState)
	{
		//Debug::LogInfo("[{}]SetVariableToByID {} - {} from [{}] to [{}]", (int)IsGlobal ,itr->Name , nIndex, itr->Value , bState);
		itr->Value = bState;
		ScenarioClass::Instance->VariablesChanged = true;
		if (!IsGlobal)
			TagClass::NotifyLocalChanged(nIndex);
		else
			TagClass::NotifyGlobalChanged(nIndex);
	}
}

void ScenarioExtData::GetVariableStateByID(const bool IsGlobal,int nIndex, char* pOut)
{
	//Debug::LogInfo("{} , Executed !", __FUNCTION__);

	const auto dict = ScenarioExtData::GetVariables(IsGlobal);

	if (const auto itr = dict->tryfind(nIndex))
		*pOut = static_cast<char>(itr->Value);
	else
		Debug::LogInfo("Failed When Trying to Get [{}]Variables with Indx [{}] ", (int)IsGlobal, nIndex);

}

void ScenarioExtData::ReadVariables(const bool IsGlobal, CCINIClass* pINI)
{
	//auto const pString = IsGlobal ? "Global" : "Local";
	//Debug::LogInfo("{} , Executed For {} Variables !", __FUNCTION__, pString);

	if (!IsGlobal) // Local variables need to be read again
		ScenarioExtData::GetVariables(false)->clear();
	else if (!ScenarioExtData::GetVariables(true)->empty()) // Global variables had been loaded, DO NOT CHANGE THEM
		return;

	const char* const pVariableNames = GameStrings::VariableNames();
	const int nCount = pINI->GetKeyCount(pVariableNames);

	for (int i = 0; i < nCount; ++i)
	{
		const auto pKey = pINI->GetKeyName(pVariableNames, i);
		int nIndex = -1;

		if (sscanf_s(pKey, "%d", &nIndex) == 1 && nIndex >= 0)  // Added validation for non-negative index
		{
			auto& var = (*ScenarioExtData::GetVariables(IsGlobal))[nIndex];
			pINI->ReadString(pVariableNames, pKey, pKey, Phobos::readBuffer);
			char* buffer = nullptr;
			strcpy_s(var.Name, strtok_s(Phobos::readBuffer, Phobos::readDelims, &buffer));
			if (auto pState = strtok_s(nullptr, Phobos::readDelims, &buffer))
				var.Value = atoi(pState);
			else
				var.Value = 0;

			//Debug::LogInfo("ReadVariables [{}] result {} ! ", var.Name, var.Value ? "True" : "False");
		}
	}

	if (IsGlobal) {
		ScenarioExtData::Instance()->LoadVariablesToFile(true);

		if (!Phobos::Config::SaveVariablesOnScenarioEnd)
		{
			// Is it better not to delete the file?
			DeleteFileA("globals.ini");
		}
	}
}

void ScenarioExtData::Allocate(ScenarioClass* pThis)
{
	Data = std::make_unique<ScenarioExtData>();
	Data->AttachedToObject = pThis;
}

void ScenarioExtData::Remove(ScenarioClass* pThis)
{
	Data = nullptr;
}

void ScenarioExtData::s_LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI)
{
	//Data->Initialize();
	Data->LoadFromINIFile(pINI , false);
}

void ScenarioExtData::LoadBasicFromINIFile(CCINIClass* pINI)
{
	AdjustLightingFix = pINI->ReadBool(GameStrings::Basic(), "AdjustLightingFix", false);
}

void ScenarioExtData::FetchVariables(ScenarioClass* pScen)
{
	// Initialize
	//DefaultAmbientOriginal = pScen->AmbientOriginal;
	//DefaultAmbientCurrent = pScen->AmbientCurrent;
	//DefaultAmbientTarget = pScen->AmbientTarget;
	//DefaultNormalLighting = pScen->NormalLighting;

	//CurrentTint_Tiles = pScen->NormalLighting.Tint;
}

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WarheadType/Body.h>

void ScenarioExtData::DetonateMasterBullet(const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget, bool isBright, WeaponTypeClass* pWeapon, WarheadTypeClass* pWarhead)
{
	BulletTypeClass* pType = pWeapon ? pWeapon->Projectile : BulletTypeExtData::GetDefaultBulletType();
	auto pBullet = pType->CreateBullet(nullptr, nullptr, 0 , nullptr, 0, false);
	const int speed = WarheadTypeExtContainer::Instance.Find(pWarhead)->DetonateOnAllMapObjects  ? 100 : 0;

	pBullet->Construct(pType, pTarget, pOwner, damage, pWarhead, speed, isBright);

	if (pWeapon) {
		pBullet->SetWeaponType(pWeapon);
	} else {
		pBullet->SetWeaponType(nullptr);
	}

	CoordStruct detonateCoord = coords;
	if (!coords.IsValid() && pTarget)
		detonateCoord = pTarget->GetCoords();

	auto pBulletExt = BulletExtContainer::Instance.Find(pBullet);

	if (pFiringHouse) {
		pBulletExt->Owner = pFiringHouse;
	}

	pBulletExt->IsInstantDetonation = true;
	pBullet->SetLocation(detonateCoord);
	pBullet->Explode(true);
	pBullet->UnInit();
}

void ScenarioExtData::ReadMissionMDINI()
{
	const char* _requested = //SpawnerMain::Configs::Active ? "SPAWN.INI" :
		GameStrings::MISSIONMD_INI;
	CCFileClass file { _requested };

	if (!file.IsAvaible()) {
		Debug::LogInfo(__FUNCTION__ " Failed to Find file {} - {} for", _requested, file.Filename);
		return;
	}

	if (!file.Open1(FileAccessMode::ReadWrite)) {
		Debug::LogInfo(__FUNCTION__ " Failed to Open file {} - {} for", _requested , file.Filename);
		return;
	}

	CCINIClass ini {};
	ini.ReadCCFile(&file);

	auto pThis = this->AttachedToObject;
	auto const scenarioName = pThis->FileName;
	auto const defaultsSection = "Defaults";

	INI_EX exINI(&ini);

	// Override rankings
	pThis->ParTimeEasy = ini.ReadTime(scenarioName, "Ranking.ParTimeEasy", pThis->ParTimeEasy);
	pThis->ParTimeMedium = ini.ReadTime(scenarioName, "Ranking.ParTimeMedium", pThis->ParTimeMedium);
	pThis->ParTimeDifficult = ini.ReadTime(scenarioName, "Ranking.ParTimeHard", pThis->ParTimeDifficult);

	ini.ReadString(scenarioName, "Ranking.UnderParTitle", pThis->UnderParTitle, pThis->UnderParTitle);
	ini.ReadString(scenarioName, "Ranking.UnderParMessage", pThis->UnderParMessage, pThis->UnderParMessage);
	ini.ReadString(scenarioName, "Ranking.OverParTitle", pThis->OverParTitle, pThis->OverParTitle);
	ini.ReadString(scenarioName, "Ranking.OverParMessage", pThis->OverParMessage, pThis->OverParMessage);

	this->ShowBriefing.Read(exINI, scenarioName, "ShowBriefing");
	this->BriefingTheme = ini.ReadTheme(scenarioName, "BriefingTheme", this->BriefingTheme);

	this->DefaultLS640BkgdName.Read(&ini, defaultsSection, "DefaultLS640BkgdName");
	this->DefaultLS800BkgdName.Read(&ini, defaultsSection, "DefaultLS800BkgdName");
	this->DefaultLS800BkgdPal.Read(&ini, defaultsSection, "DefaultLS800BkgdPal");
}

void ScenarioExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	//auto pThis = this->AttachedToObject;

	 INI_EX exINI(pINI);

	this->ShowBriefing.Read(exINI, GameStrings::Basic, "ShowBriefing");
	this->BriefingTheme = pINI->ReadTheme(GameStrings::Basic, "BriefingTheme", this->BriefingTheme);
	this->OriginalFilename.Read(exINI, GameStrings::Basic, "OriginalFilename");

	DropshipLoadoutClass::ParseScenario(exINI, GameStrings::Basic, this);

	this->ReadMissionMDINI();

}

// =============================
// container hooks
//
ASMJIT_PATCH(0x683549, ScenarioClass_CTOR, 0x9)
{
	GET(ScenarioClass*, pItem, EAX);

	ScenarioExtData::Allocate(pItem);
	//ScenarioExtData::Instance()->Waypoints.clear();
	//ScenarioExtData::Instance()->Local_Variables.clear();
	//ScenarioExtData::Instance()->Global_Variables.clear();

	return 0;
}

ASMJIT_PATCH(0x6BEB7D, ScenarioClass_DTOR, 0x6)
{
	GET(ScenarioClass*, pItem, ESI);

	ScenarioExtData::Remove(pItem);
	return 0;
}


ASMJIT_PATCH(0x689310, ScenarioClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IStream*, pStm, 0x4);

	ScenarioExtData::g_pStm = pStm;

	return 0;
}ASMJIT_PATCH_AGAIN(0x689470, ScenarioClass_SaveLoad_Prefix, 0x5)

#include <Misc/Spawner/Main.h>

ASMJIT_PATCH(0x689669, ScenarioClass_Load_Suffix, 0x6)
{
	// Clear UIGameMode on game load
	if (SpawnerMain::Configs::Enabled)
		SpawnerMain::GameConfigs::m_Ptr.UIGameMode[0] = 0;

	auto buffer = ScenarioExtData::Instance();

	PhobosByteStream Stm(0);
	if (Stm.ReadFromStream(ScenarioExtData::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(ScenarioExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

ASMJIT_PATCH(0x68945B, ScenarioClass_Save_Suffix, 0x8)
{
	auto buffer = ScenarioExtData::Instance();
	// negative 4 for the AttachedToObjectPointer , it doesnot get S/L
	PhobosByteStream saver((sizeof(ScenarioExtData) - 4u));
	PhobosStreamWriter writer(saver);

	writer.Save(ScenarioExtData::Canary);
	writer.Save(buffer);

	buffer->SaveToStream(writer);
	//if (!
	saver.WriteToStream(ScenarioExtData::g_pStm)
	//) Debug::LogInfo("Faild To Write ScenarioExtData to the Stream ! ")
		;

	return 0;
}

ASMJIT_PATCH(0x689FC0, ScenarioClass_LoadFromINI_ReadBasic, 0x8)
{
	GET(CCINIClass*, pINI, EDI);

	//read the "Basic" section
	ScenarioExtData::Instance()->LoadBasicFromINIFile(pINI);
	return 0x0;
}

ASMJIT_PATCH(0x68AD2F, ScenarioClass_LoadFromINI_AfterPlayerDataInit, 0x5)
{
	//GET(ScenarioClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EDI);

	INI_EX exINI(pINI);

	if (SessionClass::IsCampaign()) {
		GameModeOptionsClass::Instance->MCVRedeploy = pINI->ReadBool(GameStrings::Basic(), GameStrings::MCVRedeploys(), FakeRulesClass::Instance()->MCVRedeploysInCampaign);
	}

	HouseClass::Array->for_each([](HouseClass* pHouse){
		HouseExtContainer::Instance.Find(pHouse)->FreeRadar = ScenarioClass::Instance->FreeRadar;
	});

	return 0x0;
}

//ASMJIT_PATCH(0x689EA8, ScenarioClass_LoadFromINI_Early, 0x8)
//{
//	GET(ScenarioClass*, pItem, ECX);
//	GET(CCINIClass*, pINI, EDI);
//
//	//init the Ext
//	//ScenarioExtData::s_LoadFromINIFile(pItem, pINI);
//	return 0;
//}

ASMJIT_PATCH(0x68AD62, ScenarioClass_LoadFromINI, 0x6)
{
	ScenarioExtData::Instance()->FetchVariables(ScenarioClass::Instance());
	return 0;
}
