#include "Body.h"

IStream* ScenarioExtData::g_pStm = nullptr;
bool ScenarioExtData::CellParsed = false;
std::unique_ptr<ScenarioExtData>  ScenarioExtData::Data = nullptr;
bool ScenarioExtData::UpdateLightSources = false;

void ScenarioExtData::SaveVariablesToFile(bool isGlobal)
{
	const auto fileName = isGlobal ? "globals.ini" : "locals.ini";
	UniqueGamePtr<CCFileClass> pFile { GameCreate<CCFileClass>(fileName) };

	bool is_newFile = false;

	if(!pFile->Exists()){
		if(!pFile->CreateFileA()) {
			return;
		}

		is_newFile = true;
		if (!pFile->Exists())
			return;
	}

	if(!pFile->Open(FileAccessMode::Write)) {
		Debug::Log(" %s Failed to Open file %s for\n" , __FUNCTION__ , fileName);
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

	if (!file.Exists())
	{
		return;
	}

	if (!file.Open(FileAccessMode::ReadWrite))
	{
		Debug::Log(" %s Failed to Open file %s for\n", __FUNCTION__, fileName);
		return;
	}

	CCINIClass ini {};
	ini.ReadCCFile(&file);

	const auto variables = ScenarioExtData::GetVariables(isGlobal);
	std::for_each(variables->begin(), variables->end(), [&](const auto& variable) {
		ini.ReadInteger(isGlobal ? "GlobalVariables" : ScenarioClass::Instance()->FileName, variable.second.Name, variable.second.Value);
	});
}

PhobosMap<int, ExtendedVariable>* ScenarioExtData::GetVariables(bool IsGlobal)
{
	if (IsGlobal)
		return &ScenarioExtData::Instance()->Global_Variables;

	return &ScenarioExtData::Instance()->Local_Variables;
}

void ScenarioExtData::SetVariableToByID(const bool IsGlobal, int nIndex, char bState)
{
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	const auto dict = ScenarioExtData::GetVariables(IsGlobal);
	auto itr = dict->tryfind(nIndex);

	if (itr && itr->Value != bState)
	{
		//Debug::Log("[%d]SetVariableToByID %s - %d from [%d] to [%d]\n", (int)IsGlobal ,itr->Name , nIndex, itr->Value , bState);
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
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	const auto dict = ScenarioExtData::GetVariables(IsGlobal);

	if (const auto itr = dict->tryfind(nIndex))
		*pOut = static_cast<char>(itr->Value);
	else
		Debug::Log("Failed When Trying to Get [%d]Variables with Indx [%d] \n", (int)IsGlobal, nIndex);

}

void ScenarioExtData::ReadVariables(const bool IsGlobal, CCINIClass* pINI)
{
	//auto const pString = IsGlobal ? "Global" : "Local";
	//Debug::Log("%s , Executed For %s Variables !\n", __FUNCTION__, pString);

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

		if (sscanf_s(pKey, "%d", &nIndex) == 1)
		{
			auto& var = (*ScenarioExtData::GetVariables(IsGlobal))[nIndex];
			pINI->ReadString(pVariableNames, pKey, pKey, Phobos::readBuffer);
			char* buffer = nullptr;
			strcpy_s(var.Name, strtok_s(Phobos::readBuffer, Phobos::readDelims, &buffer));
			if (auto pState = strtok_s(nullptr, Phobos::readDelims, &buffer))
				var.Value = atoi(pState);
			else
				var.Value = 0;

			//Debug::Log("ReadVariables [%s] result %s ! \n", var.Name, var.Value ? "True" : "False");
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

void ScenarioExtData::ReadMissionMDINI()
{
	CCFileClass file { GameStrings::MISSIONMD_INI };

	if (!file.Exists()) {
		Debug::Log(" %s Failed to Find file %s for\n", __FUNCTION__, file.FileName);
		return;
	}

	if (!file.Open(FileAccessMode::ReadWrite)) {
		Debug::Log(" %s Failed to Open file %s for\n", __FUNCTION__, file.FileName);
		return;
	}

	CCINIClass ini {};
	ini.ReadCCFile(&file);
	auto pThis = this->AttachedToObject;
	auto const scenarioName = pThis->FileName;

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

}

void ScenarioExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	//auto pThis = this->AttachedToObject;

	 INI_EX exINI(pINI);

	this->ShowBriefing.Read(exINI, GameStrings::Basic, "ShowBriefing");
	this->BriefingTheme = pINI->ReadTheme(GameStrings::Basic, "BriefingTheme", this->BriefingTheme);
	this->OriginalFilename.Read(exINI, GameStrings::Basic, "OriginalFilename");
	this->ReadMissionMDINI();

}

// =============================
// load / save

template <typename T>
void ScenarioExtData::Serialize(T& Stm)
{
	//Debug::Log("Processing ScenarioExtData ! \n");
	Stm

		.Process(SessionClass::Instance->Config)
		.Process(this->Initialized)
		.Process(this->OriginalFilename)
		.Process(this->Waypoints)
		.Process(this->Local_Variables)
		.Process(this->Global_Variables)
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

		.Process(ShowBriefing)
		.Process(BriefingTheme)
		;


}

// =============================
// container hooks
//
DEFINE_HOOK(0x683549, ScenarioClass_CTOR, 0x9)
{
	GET(ScenarioClass*, pItem, EAX);

	ScenarioExtData::Allocate(pItem);
	ScenarioExtData::Instance()->Waypoints.clear();
	ScenarioExtData::Instance()->Local_Variables.clear();
	ScenarioExtData::Instance()->Global_Variables.clear();

	return 0;
}

DEFINE_HOOK(0x6BEB7D, ScenarioClass_DTOR, 0x6)
{
	GET(ScenarioClass*, pItem, ESI);

	ScenarioExtData::Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x689470, ScenarioClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x689310, ScenarioClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IStream*, pStm, 0x4);

	ScenarioExtData::g_pStm = pStm;

	return 0;
}

#include <Misc/Spawner/Main.h>

DEFINE_HOOK(0x689669, ScenarioClass_Load_Suffix, 0x6)
{
	// Clear UIGameMode on game load
	if (SpawnerMain::Configs::Enabled)
		SpawnerMain::GameConfigs::m_Ptr->UIGameMode[0] = 0;

	auto buffer = ScenarioExtData::Instance();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(ScenarioExtData::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(ScenarioExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x68945B, ScenarioClass_Save_Suffix, 0x8)
{
	auto buffer = ScenarioExtData::Instance();
	// negative 4 for the AttachedToObjectPointer , it doesnot get S/L
	PhobosByteStream saver(sizeof(GameModeOptionsClass) + (sizeof(ScenarioExtData) - 4u));
	PhobosStreamWriter writer(saver);

	writer.Save(ScenarioExtData::Canary);
	writer.Save(buffer);

	buffer->SaveToStream(writer);
	//if (!
	saver.WriteBlockToStream(ScenarioExtData::g_pStm)
	//) Debug::Log("Faild To Write ScenarioExtData to the Stream ! ")
		;

	return 0;
}

DEFINE_HOOK(0x689FC0, ScenarioClass_LoadFromINI_ReadBasic, 0x8)
{
	GET(CCINIClass*, pINI, EDI);

	//read the "Basic" section
	ScenarioExtData::Instance()->LoadBasicFromINIFile(pINI);
	return 0x0;
}

DEFINE_HOOK(0x68AD2F, ScenarioClass_LoadFromINI_AfterPlayerDataInit, 0x5)
{
	//GET(ScenarioClass*, pItem, ESI);
	GET(CCINIClass*, pINI, EDI);

	INI_EX exINI(pINI);

	if (SessionClass::IsCampaign()) {
		GameModeOptionsClass::Instance->MCVRedeploy = pINI->ReadBool(GameStrings::Basic(), GameStrings::MCVRedeploys(), false);
	}

	return 0x0;
}

//DEFINE_HOOK(0x689EA8, ScenarioClass_LoadFromINI_Early, 0x8)
//{
//	GET(ScenarioClass*, pItem, ECX);
//	GET(CCINIClass*, pINI, EDI);
//
//	//init the Ext
//	//ScenarioExtData::s_LoadFromINIFile(pItem, pINI);
//	return 0;
//}

DEFINE_HOOK(0x68AD62, ScenarioClass_LoadFromINI, 0x6)
{
	ScenarioExtData::Instance()->FetchVariables(ScenarioClass::Instance());
	return 0;
}
