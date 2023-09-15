#pragma once

#include <ScenarioClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <map>

struct ExtendedVariable
{
	char Name[0x100];
	int Value;
};

class ScenarioExt
{
public:

	static IStream* g_pStm;
	static bool CellParsed;

	class ExtData final : public Extension<ScenarioClass>
	{
	public:
		static constexpr size_t Canary = 0xABCD1595;
		using base_type = ScenarioClass;

	public:
		std::map<int, CellStruct> Waypoints { };
		std::map<int, ExtendedVariable> Local_Variables { }; // 0 for local, 1 for global
		std::map<int, ExtendedVariable> Global_Variables { };

		Nullable<FixedString<0x1F>> ParTitle { };
		Nullable<FixedString<0x1F>> ParMessage { };

		Nullable<FixedString<0x20>> ScoreCampaignTheme { };
		Nullable<FixedString<0x104>> NextMission { };

		LightingStruct DefaultNormalLighting { {1000,1000,1000},0,0 };
		int DefaultAmbientOriginal { 0 };
		int DefaultAmbientCurrent { 0 };
		int DefaultAmbientTarget { 0 };
		TintStruct CurrentTint_Tiles { -1,-1,-1 };
		TintStruct CurrentTint_Schemes { -1,-1,-1 };
		TintStruct CurrentTint_Hashes { -1,-1,-1 };
		bool AdjustLightingFix { false };

		bool ShowBriefing { false };
		int BriefingTheme { -1 };

		ExtData(ScenarioClass* OwnerObject) : Extension<ScenarioClass>(OwnerObject)
		{ }

		virtual ~ExtData() override = default;

		void SetVariableToByID(const bool IsGlobal, int nIndex, char bState);
		void GetVariableStateByID(const bool IsGlobal, int nIndex, char* pOut);
		void ReadVariables(const bool IsGlobal, CCINIClass* pINI);

		void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);

		void LoadBasicFromINIFile(CCINIClass* pINI);
		void FetchVariables(ScenarioClass* pScen);

		void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
		void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;
public:

	static void Allocate(ScenarioClass* pThis);
	static void Remove(ScenarioClass* pThis);

	static void LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(ScenarioClass::Instance);
	}

	static void SaveVariablesToFile(bool isGlobal);

	static std::map<int, ExtendedVariable>* GetVariables(bool IsGlobal);
};