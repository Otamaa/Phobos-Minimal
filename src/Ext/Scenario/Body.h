#pragma once

#include <ScenarioClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/VectorHelper.h>

#include <Utilities/PhobosFixedString.h>
#include <Utilities/VectorSet.h>

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
#pragma region ClassMembers
	// ============================================================
	// Large aggregates (fixed-size strings, Nullable<FixedString>)
	// ============================================================
	Nullable<FixedString<0x104>> NextMission {};
	Nullable<FixedString<0x20>> ScoreCampaignTheme {};
	Nullable<FixedString<0x1F>> ParTitle {};
	Nullable<FixedString<0x1F>> ParMessage {};
	PhobosFixedString<64u> DefaultLS640BkgdName {};
	PhobosFixedString<64u> DefaultLS800BkgdName {};
	PhobosFixedString<64u> DefaultLS800BkgdPal {};
	Valueable<std::string> OriginalFilename {};

	// ============================================================
	// Maps (large containers)
	// ============================================================
	PhobosMap<int, CellStruct> Waypoints {};
	PhobosMap<int, ExtendedVariable> Local_Variables {};
	PhobosMap<int, ExtendedVariable> Global_Variables {};
	PhobosMap<int, int> TriggerTypePlayerAtXOwners {};

	// ============================================================
	// Vectors / VectorSets (24+ bytes each)
	// ============================================================
	std::vector<CellStruct> DefinedAudioWaypoints {};
	HelperedVector<TechnoTypeClass*> OwnedExistCameoTechnoTypes {};
	VectorSet<int> SWSidebar_Indices {};
	std::vector<std::wstring> RecordMessages {};
	VectorSet<TechnoClass*> LimboLaunchers {};
	VectorSet<TechnoClass*> UndergroundTracker {};
	VectorSet<TechnoClass*> FallingDownTracker {};

	// ============================================================
	// 4-byte aligned: int
	// ============================================================
	int BriefingTheme { -1 };

	// ============================================================
	// 1-byte aligned: Valueable<bool> and plain bool (packed at the end)
	// ============================================================
	Valueable<bool> ShowBriefing { false };
	bool AdjustLightingFix { false };
	bool SWSidebar_Enable { true };
	// 3 bools = 3 bytes, pads to 4 for alignment

#pragma endregion

	void SetVariableToByID(const bool IsGlobal, int nIndex, char bState);
	void GetVariableStateByID(const bool IsGlobal, int nIndex, char* pOut);
	void ReadVariables(const bool IsGlobal, CCINIClass* pINI);


	void LoadFromINIFile(CCINIClass* pINI, bool parseFailAddr);
	void ReadMissionMDINI();

	void LoadBasicFromINIFile(CCINIClass* pINI);
	void FetchVariables(ScenarioClass* pScen);

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	static void DetonateMasterBullet(const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse, AbstractClass* pTarget, bool isBright, WeaponTypeClass* pWeapon, WarheadTypeClass* pWarhead);

private:
	template <typename T>
	void Serialize(T& Stm)
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
			.Process(this->OwnedExistCameoTechnoTypes)
			.Process(this->SWSidebar_Enable)
			.Process(this->SWSidebar_Indices)

			.Process(this->RecordMessages)

			.Process(this->DefaultLS640BkgdName)
			.Process(this->DefaultLS800BkgdName)
			.Process(this->DefaultLS800BkgdPal)

			.Process(this->LimboLaunchers)
			.Process(this->UndergroundTracker)
			.Process(this->FallingDownTracker)
			;

	}

public:
	static IStream* g_pStm;
	static bool CellParsed;
	static bool UpdateLightSources;

	static void Allocate(ScenarioClass* pThis);
	static void Remove(ScenarioClass* pThis);

	static void s_LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI);

	COMPILETIMEEVAL FORCEDINLINE static ScenarioExtData* Instance()
	{
		return Data.get();
	}

	FORCEDINLINE static void Clear()
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