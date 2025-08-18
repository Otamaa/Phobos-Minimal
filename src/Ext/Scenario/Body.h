#pragma once

#include <ScenarioClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/VectorHelper.h>

#include <Utilities/PhobosFixedString.h>

struct ExtendedVariable
{
	char Name[0x100];
	int Value;

	OPTIONALINLINE bool Load(PhobosStreamReader& stm, bool registerForChange)
	{
		return
			stm
			.Process(Name , registerForChange)
			.Process(Value, registerForChange)
			.Success()
			;
	}

	OPTIONALINLINE bool Save(PhobosStreamWriter& stm) const
	{
		return
			stm
			.Process(Name)
			.Process(Value)
			.Success()
			;
	}
};

class ScenarioExtData final
{
private:
	static std::unique_ptr<ScenarioExtData> Data;
public:
	static COMPILETIMEEVAL size_t Canary = 0xABCD1595;
	using base_type = ScenarioClass;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:
	Valueable<std::string> OriginalFilename {};
	PhobosMap<int, CellStruct> Waypoints { };
	PhobosMap<int, ExtendedVariable> Local_Variables { }; // 0 for local, 1 for global
	PhobosMap<int, ExtendedVariable> Global_Variables { };
	std::vector<CellStruct> DefinedAudioWaypoints {};

	Nullable<FixedString<0x1F>> ParTitle { };
	Nullable<FixedString<0x1F>> ParMessage { };

	Nullable<FixedString<0x20>> ScoreCampaignTheme { };
	Nullable<FixedString<0x104>> NextMission { };

	//LightingStruct DefaultNormalLighting { {1000,1000,1000},0,0 };
	//int DefaultAmbientOriginal { 0 };
	//int DefaultAmbientCurrent { 0 };
	//int DefaultAmbientTarget { 0 };
	//TintStruct CurrentTint_Tiles { -1,-1,-1 };
	//TintStruct CurrentTint_Schemes { -1,-1,-1 };
	//TintStruct CurrentTint_Hashes { -1,-1,-1 };
	bool AdjustLightingFix { false };

	Valueable<bool> ShowBriefing { false };
	int BriefingTheme { -1 };

	HelperedVector<TechnoTypeClass*> OwnedExistCameoTechnoTypes {};
	bool SWSidebar_Enable { true };
	std::set<int> SWSidebar_Indices {};

	std::vector<std::wstring> RecordMessages {};

	PhobosFixedString<64u> DefaultLS640BkgdName {};
	PhobosFixedString<64u> DefaultLS800BkgdName {};
	PhobosFixedString<64u> DefaultLS800BkgdPal {};

	void SetVariableToByID(const bool IsGlobal, int nIndex, char bState);
	void GetVariableStateByID(const bool IsGlobal, int nIndex, char* pOut);
	void ReadVariables(const bool IsGlobal, CCINIClass* pINI);


	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void ReadMissionMDINI();

	void LoadBasicFromINIFile(CCINIClass* pINI);
	void FetchVariables(ScenarioClass* pScen);

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

private:
	template <typename T>
	void Serialize(T& Stm);
public:
	static IStream* g_pStm;
	static bool CellParsed;
	static bool UpdateLightSources;

	static void Allocate(ScenarioClass* pThis);
	static void Remove(ScenarioClass* pThis);

	static void s_LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI);

	static ScenarioExtData* Instance()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(ScenarioClass::Instance);
	}

	static void SaveVariablesToFile(bool isGlobal);
	static void LoadVariablesToFile(bool isGlobal);

	static COMPILETIMEEVAL PhobosMap<int, ExtendedVariable>* GetVariables(bool IsGlobal) {
		if (IsGlobal)
			return &ScenarioExtData::Instance()->Global_Variables;

		return &ScenarioExtData::Instance()->Local_Variables;
	}

};

class NOVTABLE FakeScenarioClass : public ScenarioClass
{
public:
	CellStruct _Get_Waypoint_Location(int idx);
};