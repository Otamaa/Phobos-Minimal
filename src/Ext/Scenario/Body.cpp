#include "Body.h"

std::unique_ptr<ScenarioExt::ExtData> ScenarioExt::Data = nullptr;

bool ScenarioExt::CellParsed = false;

std::map<int, ExtendedVariable>& ScenarioExt::ScenarioExt::GetVariables(bool IsGlobal)
{
	if (IsGlobal)
		return ScenarioExt::Global()->Global_Variables;
	
	return ScenarioExt::Global()->Local_Variables;
}

void ScenarioExt::ExtData::SetVariableToByID(const bool IsGlobal, int nIndex, char bState)
{
	//Debug::Log("%s , Executed !\n", __FUNCTION__);

	auto& dict = ScenarioExt::GetVariables(IsGlobal);
	const auto itr = dict.find(nIndex);

	if (itr != dict.end() && itr->second.Value != bState)
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

	const auto& dict = ScenarioExt::GetVariables(IsGlobal);
	const auto itr = dict.find(nIndex);

	if (itr != dict.end())
		*pOut = static_cast<char>(itr->second.Value);
	else
		Debug::Log("Failed When Trying to Get [%d]Variables with Indx [%d] \n", (int)IsGlobal, nIndex);

}

void ScenarioExt::ExtData::ReadVariables(const bool IsGlobal, CCINIClass* pINI)
{
	//auto const pString = IsGlobal ? "Global" : "Local";
	//Debug::Log("%s , Executed For %s Variables !\n", __FUNCTION__, pString);

	if (!IsGlobal) // Local variables need to be read again
		ScenarioExt::GetVariables(false).clear();
	else if (!ScenarioExt::GetVariables(true).empty()) // Global variables had been loaded, DO NOT CHANGE THEM
		return;

	const char* const pVariableNames = GameStrings::VariableNames();
	const int nCount = pINI->GetKeyCount(pVariableNames);

	for (int i = 0; i < nCount; ++i)
	{
		const auto pKey = pINI->GetKeyName(pVariableNames, i);
		int nIndex = -1;

		if (sscanf_s(pKey, "%d", &nIndex) == 1)
		{
			auto& var = ScenarioExt::GetVariables(IsGlobal)[nIndex];
			pINI->ReadString(pVariableNames, pKey, pKey, Phobos::readBuffer);
			char* buffer = nullptr;
			strcpy_s(var.Name, strtok_s(Phobos::readBuffer, Phobos::readDelims, &buffer));
			if (auto pState = strtok_s(nullptr, Phobos::readDelims, &buffer))
				var.Value = CRT::atoi(pState);
			else
				var.Value = 0;

			//Debug::Log("ReadVariables [%s] result %s ! \n", var.Name, var.Value ? "True" : "False");
		}
	}
}

void ScenarioExt::Allocate(ScenarioClass* pThis)
{
	Data = std::make_unique<ScenarioExt::ExtData>(pThis);

	//if (Data)
	//	Data->EnsureConstanted();
}

void ScenarioExt::Remove(ScenarioClass* pThis)
{
	if (Data)
		Data->Uninitialize();

	Data = nullptr;
}

void ScenarioExt::LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI)
{
	Data->LoadFromINI(pINI);
}

void ScenarioExt::ExtData::LoadBasicFromINIFile(CCINIClass* pINI)
{
	AdjustLightingFix = pINI->ReadBool(GameStrings::Basic(), "AdjustLightingFix", false);
}

void ScenarioExt::ExtData::FetchVariables(ScenarioClass* pScen)
{
	ParTitle = pScen->OverParTitle;
	ParMessage = pScen->OverParMessage;

	// Initialize
	DefaultAmbientOriginal = pScen->AmbientOriginal;
	DefaultAmbientCurrent = pScen->AmbientCurrent;
	DefaultAmbientTarget = pScen->AmbientTarget;
	DefaultNormalLighting = pScen->NormalLighting;

	CurrentTint_Tiles = pScen->NormalLighting.Tint;
}

void ScenarioExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	// auto pThis = this->Get();

	// INI_EX exINI(pINI);
}

// =============================
// load / save

template <typename T>
void ScenarioExt::ExtData::Serialize(T& Stm)
{

	Debug::Log("Processing ScenarioExt ! \n");
	Stm

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

		;


}

void ScenarioExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	// Extra datas
	Debug::Log("Loading Scenario Configs ! \n");
	Stm.Process(SessionClass::Instance->Config);

	Extension<ScenarioClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void ScenarioExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	// Extra datas
	Debug::Log("Saving Scenario Configs ! \n");
	Stm.Process(SessionClass::Instance->Config);

	Extension<ScenarioClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool ScenarioExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool ScenarioExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}


// =============================
// container hooks

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

IStream* ScenarioExt::g_pStm = nullptr;

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

		if (Reader.Expect(ScenarioExt::Canary) && Reader.RegisterChange(buffer))
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

	writer.Expect(ScenarioExt::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	if (!saver.WriteBlockToStream(ScenarioExt::g_pStm))
		Debug::Log("Faild To Write ScenarioExt to the Stream ! ");

	return 0;
}

DEFINE_HOOK(0x689FC0, ScenarioClass_LoadFromINI_ReadBasic, 0x8)
{
	GET(CCINIClass*, pINI, EDI);

	//read the "Basic" section
	ScenarioExt::Global()->LoadBasicFromINIFile(pINI);
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
