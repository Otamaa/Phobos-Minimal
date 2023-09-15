#include "Body.h"

IStream* ScenarioExt::g_pStm = nullptr;
bool ScenarioExt::CellParsed = false;
std::unique_ptr< ScenarioExt::ExtData>  ScenarioExt::Data = nullptr;

void ScenarioExt::SaveVariablesToFile(bool isGlobal)
{
	const auto fileName = isGlobal ? "globals.ini" : "locals.ini";

	auto pINI = GameCreate<CCINIClass>();
	auto pFile = GameCreate<CCFileClass>(fileName);

	if (pFile->Exists())
		pINI->ReadCCFile(pFile);
	else
		pFile->CreateFileA();

	for (const auto& variable : *ScenarioExt::GetVariables(isGlobal))
		pINI->WriteInteger(ScenarioClass::Instance()->FileName, variable.second.Name, variable.second.Value, false);

	pINI->WriteCCFile(pFile);
	pFile->Close();
}

std::map<int, ExtendedVariable>* ScenarioExt::GetVariables(bool IsGlobal)
{
	if (IsGlobal)
		return &ScenarioExt::Global()->Global_Variables;

	return &ScenarioExt::Global()->Local_Variables;
}

void ScenarioExt::ExtData::SetVariableToByID(const bool IsGlobal, int nIndex, char bState)
{
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	const auto dict = ScenarioExt::GetVariables(IsGlobal);
	const auto itr = dict->find(nIndex);

	if (itr != dict->end() && itr->second.Value != bState)
	{
		itr->second.Value = bState;
		ScenarioClass::Instance->VariablesChanged = true;
		if (!IsGlobal)
			TagClass::NotifyLocalChanged(nIndex);
		else
			TagClass::NotifyGlobalChanged(nIndex);
	}
}

void ScenarioExt::ExtData::GetVariableStateByID(const bool IsGlobal,int nIndex, char* pOut)
{
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	const auto dict = ScenarioExt::GetVariables(IsGlobal);
	const auto itr = dict->find(nIndex);

	if (itr != dict->end())
		*pOut = static_cast<char>(itr->second.Value);
	else
		Debug::Log("Failed When Trying to Get [%d]Variables with Indx [%d] \n", (int)IsGlobal, nIndex);

}

void ScenarioExt::ExtData::ReadVariables(const bool IsGlobal, CCINIClass* pINI)
{
	//auto const pString = IsGlobal ? "Global" : "Local";
	//Debug::Log("%s , Executed For %s Variables !\n", __FUNCTION__, pString);

	if (!IsGlobal) // Local variables need to be read again
		ScenarioExt::GetVariables(false)->clear();
	else if (!ScenarioExt::GetVariables(true)->empty()) // Global variables had been loaded, DO NOT CHANGE THEM
		return;

	const char* const pVariableNames = GameStrings::VariableNames();
	const int nCount = pINI->GetKeyCount(pVariableNames);

	for (int i = 0; i < nCount; ++i)
	{
		const auto pKey = pINI->GetKeyName(pVariableNames, i);
		int nIndex = -1;

		if (sscanf_s(pKey, "%d", &nIndex) == 1)
		{
			auto& var = (*ScenarioExt::GetVariables(IsGlobal))[nIndex];
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
}

void ScenarioExt::Allocate(ScenarioClass* pThis)
{
	Data = std::make_unique<ScenarioExt::ExtData>(pThis);
}

void ScenarioExt::Remove(ScenarioClass* pThis)
{
	Data = nullptr;
}

void ScenarioExt::LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI)
{
	Data->LoadFromINIFile(pINI , false);
}

void ScenarioExt::ExtData::LoadBasicFromINIFile(CCINIClass* pINI)
{
	AdjustLightingFix = pINI->ReadBool(GameStrings::Basic(), "AdjustLightingFix", false);
}

void ScenarioExt::ExtData::FetchVariables(ScenarioClass* pScen)
{
	// Initialize
	DefaultAmbientOriginal = pScen->AmbientOriginal;
	DefaultAmbientCurrent = pScen->AmbientCurrent;
	DefaultAmbientTarget = pScen->AmbientTarget;
	DefaultNormalLighting = pScen->NormalLighting;

	CurrentTint_Tiles = pScen->NormalLighting.Tint;
}

void ScenarioExt::ExtData::LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr)
{
	auto pThis = this->Get();

	 INI_EX exINI(pINI);

	 CCINIClass* pINI_MISSIONMD = CCINIClass::LoadINIFile(GameStrings::MISSIONMD_INI);
	 auto const scenarioName = pThis->FileName;

	 // Override rankings
	 pThis->ParTimeEasy = pINI_MISSIONMD->ReadTime(scenarioName, "Ranking.ParTimeEasy", pThis->ParTimeEasy);
	 pThis->ParTimeMedium = pINI_MISSIONMD->ReadTime(scenarioName, "Ranking.ParTimeMedium", pThis->ParTimeMedium);
	 pThis->ParTimeDifficult = pINI_MISSIONMD->ReadTime(scenarioName, "Ranking.ParTimeHard", pThis->ParTimeDifficult);

	 pINI_MISSIONMD->ReadString(scenarioName, "Ranking.UnderParTitle", pThis->UnderParTitle, pThis->UnderParTitle);
	 pINI_MISSIONMD->ReadString(scenarioName, "Ranking.UnderParMessage", pThis->UnderParMessage, pThis->UnderParMessage);
	 pINI_MISSIONMD->ReadString(scenarioName, "Ranking.OverParTitle", pThis->OverParTitle, pThis->OverParTitle);
	 pINI_MISSIONMD->ReadString(scenarioName, "Ranking.OverParMessage", pThis->OverParMessage, pThis->OverParMessage);

	 this->ShowBriefing = pINI_MISSIONMD->ReadBool(scenarioName, "ShowBriefing", pINI->ReadBool(GameStrings::Basic, "ShowBriefing", this->ShowBriefing));
	 this->BriefingTheme = pINI_MISSIONMD->ReadTheme(scenarioName, "BriefingTheme", pINI->ReadTheme(GameStrings::Basic, "BriefingTheme", this->BriefingTheme));

	 CCINIClass::UnloadINIFile(pINI_MISSIONMD);
}

// =============================
// load / save

template <typename T>
void ScenarioExt::ExtData::Serialize(T& Stm)
{

	//Debug::Log("Processing ScenarioExt ! \n");
	Stm

		.Process(SessionClass::Instance->Config)
		.Process(this->Initialized)
		.Process(this->Waypoints)
		.Process(this->Local_Variables)
		.Process(this->Global_Variables)

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

	ScenarioExt::Allocate(pItem);
	ScenarioExt::Global()->Waypoints.clear();
	ScenarioExt::Global()->Local_Variables.clear();
	ScenarioExt::Global()->Global_Variables.clear();

	return 0;
}

DEFINE_HOOK(0x6BEB7D, ScenarioClass_DTOR, 0x6)
{
	GET(ScenarioClass*, pItem, ESI);

	ScenarioExt::Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x689470, ScenarioClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x689310, ScenarioClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IStream*, pStm, 0x4);

	ScenarioExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x689669, ScenarioClass_Load_Suffix, 0x6)
{
	auto buffer = ScenarioExt::Global();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(ScenarioExt::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(ScenarioExt::ExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x68945B, ScenarioClass_Save_Suffix, 0x8)
{
	auto buffer = ScenarioExt::Global();
	PhobosByteStream saver(//sizeof(GameModeOptionsClass)
		+ sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(ScenarioExt::ExtData::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	//if (!
	saver.WriteBlockToStream(ScenarioExt::g_pStm)
	//) Debug::Log("Faild To Write ScenarioExt to the Stream ! ")
		;

	return 0;
}

DEFINE_HOOK(0x689FC0, ScenarioClass_LoadFromINI_ReadBasic, 0x8)
{
	GET(CCINIClass*, pINI, EDI);

	//read the "Basic" section
	ScenarioExt::Global()->LoadBasicFromINIFile(pINI);
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

DEFINE_HOOK(0x689E90, ScenarioClass_LoadFromINI_Early, 0x6)
{
	GET(ScenarioClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x8);

	//init the Ext
	ScenarioExt::LoadFromINIFile(pItem, pINI);
	return 0;
}

DEFINE_HOOK(0x68AD62, ScenarioClass_LoadFromINI, 0x6)
{
	ScenarioExt::Global()->FetchVariables(ScenarioClass::Instance());
	return 0;
}
