#pragma once

#include <ScenarioClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PhobosMap.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/VectorHelper.h>

#include <Utilities/PhobosFixedString.h>
#include <Utilities/VectorSet.h>
#include <Utilities/PhobosPCXFile.h>

class VoxClass;

class TechnoExtData;
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
	static COMPILETIMEEVAL DWORD Canary = 0x642EC48E;
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
	VectorSet<int> SWSidebar_Indices {};
	std::vector<std::wstring> RecordMessages {};

	VectorSet<TechnoClass*> LimboLaunchers {};
	VectorSet<TechnoClass*> UndergroundTracker {};
	VectorSet<TechnoClass*> FallingDownTracker {};
	HelperedVector<TechnoClass*> OwnedUniqueTechnos {};

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
	bool IsHouseTypeVoiceNeedCheck { true };
	// 3 bools = 3 bytes, pads to 4 for alignment

	int PrismRelayClaimFrame { -1 };
	TechnoClass* PrismRelayClaimMaster { nullptr };
	int PrismRelayClaimWeaponIndex { -1 };


	int DropshipLoadout_Theme { -1 };
	long DropshipLoadout_Money { -1 };
	NullableIdx<VoxClass*> DropshipLoadout_StartEVA {};
	int DropshipLoadout_StartingDropships {};
	std::vector<TechnoTypeClass*> DropshipLoadout_Carriers {};
	std::vector<int> DropshipLoadout_Carriers_SizeLimit {};
	bool DropshipLoadout_AddUnusedMoneyToPlayer {};
	bool DropshipLoadout_RememberPurchasedCargo { true };
	ConvertClass* DropshipLoadout_Palette {};
	SHPCaches* DropshipLoadout_Background {};
	SHPCaches* DropshipLoadout_UpArrow {};
	SHPCaches* DropshipLoadout_DownArrow {};
	SHPCaches* DropshipLoadout_Loadout {};
	SHPCaches* DropshipLoadout_PilotLit {};
	std::vector<SHPCaches*> DropshipLoadout_DGreenList {};
	PhobosPCXFile DropshipLoadout_BackgroundPCX {};
	PhobosPCXFile DropshipLoadout_UpArrowPCX {};
	PhobosPCXFile DropshipLoadout_DownArrowPCX {};
	std::vector<PhobosPCXFile> DropshipLoadout_LoadoutPCX {};
	Point2D DropshipLoadout_LoadoutLocation {};
	std::vector<PhobosPCXFile> DropshipLoadout_PilotLitPCX {};
	Point2D DropshipLoadout_PilotLitLocation {};
	std::vector<std::vector<PhobosPCXFile>> DropshipLoadout_DGreenListPCX {};
	int DropshipLoadout_DGreenAnimationsCount {};
	std::vector<Point2D> DropshipLoadout_DGreenLocations {};
	Point2D DropshipLoadout_UpArrowLocation {};
	Point2D DropshipLoadout_DownArrowLocation {};
	int DropshipLoadout_SidebarCameosCount {};
	std::vector<Point2D> DropshipLoadout_SidebarCameoLocations {};
	int DropshipLoadout_DropshipCameosCount {};
	std::vector<std::vector<Point2D>> DropshipLoadout_DropshipCameoLocations {};
	std::vector<std::vector<TechnoTypeClass*>> DropshipLoadout_FixedUnits {};
	std::vector<std::vector<TechnoTypeClass*>> DropshipLoadout_InitialUnits {};
	std::map<int, std::vector<TechnoTypeClass*>> DropshipLoadout_AllowableUnitsLists {};
	std::map<int, std::vector<int>> DropshipLoadout_AllowableUnitMaximumsLists {};
	//VocClass DropshipLoadout_SellClickSound;
	NullableIdx<VocClass*> DropshipLoadout_BuyClickSound {};
	NullableIdx<VocClass*> DropshipLoadout_SellClickSound {};
	NullableIdx<VocClass*> DropshipLoadout_ArrowsClickSound {};
	NullableIdx<VocClass*> DropshipLoadout_StartingDragDropSound {};
	NullableIdx<VocClass*> DropshipLoadout_EndingDragDropSound {};
	std::vector<int> DropshipLoadout_ActiveTeamSuffixes {};

	std::set<int> Smudges {};

	DWORD OwnerBitfield_BuildingType { 0 };
	DWORD OwnerBitfield_InfantryType { 0 };
	DWORD OwnerBitfield_VehicleType { 0 };
	DWORD OwnerBitfield_NavyType { 0 };
	DWORD OwnerBitfield_AircraftType { 0 };

	int MissionTimer_Type {};
	int MissionTimer_Variable {};
	bool MissionTimer_Reverse {};
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
	void Serialize(T& Stm);

public:
	static IStream* g_pStm;
	static bool CellParsed;
	static bool UpdateLightSources;

	static void Allocate(ScenarioClass* pThis);
	static void Remove(ScenarioClass* pThis);

	static void s_LoadFromINIFile(ScenarioClass* pThis, CCINIClass* pINI);

	void OPTIONALINLINE InvalidatePointer(AbstractClass* ptr, bool bRemove)
	{
		AnnounceInvalidPointer(PrismRelayClaimMaster, ptr, bRemove);
		LimboLaunchers.InvalidatePointer(ptr, bRemove);
		UndergroundTracker.InvalidatePointer(ptr, bRemove);
		FallingDownTracker.InvalidatePointer(ptr, bRemove);
		AnnounceInvalidPointer<TechnoClass*>(OwnedUniqueTechnos, ptr, bRemove);
	}

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