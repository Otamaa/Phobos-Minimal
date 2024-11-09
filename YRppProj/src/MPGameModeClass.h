/*
	Base class for all MP gamemodes
*/

/*
NOTE:
	The game modes should NOT become victims of Operation: The Cleansing,
	since we will possibly want to derive classes from them!

	-pd
*/

#pragma once

#include <GeneralDefinitions.h>
#include <MPTeams.h>
#include <Helpers/CompileTime.h>
#include <Wstring.h>
#include <ArrayClasses.h>

//forward declarations
class HouseClass;
class CCINIClass;

class NOVTABLE MPGameModeClass
{
public:
	//global arrays
	static DynamicVectorClass<MPGameModeClass*>* GameModes;
	static constexpr reference<MPGameModeClass*, 0xA8B23C> const Instance{};

	static MPGameModeClass* __fastcall Get(int index)
		;//{ JMP_STD(0x5D5F30); }

	static bool Set(int index);//
	//{
	//	THISCALL_EX(index, 0x5D5F30);
	//	MEM_WRITEIMM32(0xA8B23C, eax);
	//	VAR8_REG(bool, success, al);
	//	return success;
	//}

	virtual ~MPGameModeClass();

	virtual bool vt_entry_04()
		;//{ return 0; }

	virtual bool vt_entry_08()
		;//{ return 0; }

	virtual bool vt_entry_0C(DWORD dwUnk)
		;//{ return 1; }

	virtual bool vt_entry_10()
		;//{ JMP_THIS(0x5D62C0); }

	virtual bool vt_entry_14(DWORD dwUnk)
		;//{ return 1; }

	virtual bool vt_entry_18()
		;//{ return 1; }

	virtual bool vt_entry_1C(DWORD dwUnk)
		;//{ return 1; }

	virtual void vt_entry_20()
		;//{ }

	virtual void vt_entry_24()
		;//{ }

	virtual signed int vt_entry_28()
		;//{ return -2; }

	virtual signed int vt_entry_2C()
		;//{ return this->MustAlly ? 0 : -2; } // (-(this->MustAlly != 0) & 2) - 2;

	virtual signed int vt_entry_30()
		;//{ return this->AlliesAllowed ? 3 : -2 ; } // (-(this->AlliesAllowed != 0) & 5) - 2

	virtual bool CanAllyWith(int idx)
		;//{ JMP_THIS(0x5D5DE0); }

	virtual void vt_entry_38(DWORD dwUnk)
		;//{ }

	virtual bool IsAIAllowed()
		;//{ return AIAllowed; }

	virtual bool vt_entry_40()
		;//{ return 0; }

	virtual signed int FirstValidMapIndex()
		;//{ JMP_THIS(0x5D6370); }

	virtual void PopulateTeamDropdown(HWND hWnd, DynamicVectorClass<MPTeam*> *vecTeams, MPTeam *Team)
		;//{ JMP_THIS(0x5D6450); }

	virtual void DrawTeamDropdown(HWND hWnd, DynamicVectorClass<MPTeam*> *vecTeams, MPTeam *Team)
		;//{ JMP_THIS(0x5D64C0); }

	virtual void PopulateTeamDropdownForPlayer(HWND hWnd, int idx)
		;//{ JMP_THIS(0x5D6540); }

	//argh! source of fail
	virtual bool vt_entry_54(int a1, int a2, void *ptr, int a4, __int16 a5, int a6, int a7)
		;//{ JMP_THIS(0x5C0EB0); } // 0x5C0EB0

	//argh! source of fail
	virtual bool vt_entry_58(int a1, int a2, void *ptr, int a4, __int16 a5, int a6, int a7, int a8, int a9)
		;//{ JMP_THIS(0x5C0E90); } // 0x5C0E90

	virtual bool vt_entry_5C(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3)
		;//{ return 1; }

	virtual bool vt_entry_60()
		;//{ return 1; }

	virtual bool vt_entry_64()
		;//{ return 1; }

	virtual bool vt_entry_68()
		;//{ return 1; }

	virtual int RandomHumanCountryIndex()
		;//{ JMP_THIS(0x5D6430); }

	virtual int RandomAICountryIndex()
		;//{ return this->RandomHumanCountryIndex(); }

	virtual void vt_entry_74(DWORD dwUnk1, DWORD dwUnk2)
		;//{ }

	virtual void vt_entry_78(DWORD dwUnk1)
		;//{ }

	virtual bool UnfixAlliances()
		;//{ JMP_THIS(0x5D6790); }

	virtual bool StartingPositionsToHouseBaseCells(char unused)
		;//{ JMP_THIS(0x5D6BE0); }

	virtual bool StartingPositionsToHouseBaseCells2(bool arg)
		;//{ JMP_THIS(0x5D6C70); }

	virtual bool AllyTeams()
		;//{ JMP_THIS(0x5D74A0); }

	virtual bool vt_entry_8C()
		;//{ return 1; }

	virtual void vt_entry_90()
		;//{ }

	virtual void vt_entry_94()
		;//{ }

	virtual signed int vt_entry_98()
		;//{ return -1; }

	virtual void vt_entry_9C()
		;//{ }

	virtual void vt_entry_A0(DWORD dwUnk)
		;//{ }

	virtual void vt_entry_A4(DWORD dwUnk)
		;//{ }

	virtual bool vt_entry_A8()
		;//{ return 0; }

	virtual void vt_entry_AC()
		;//{ }

	virtual void vt_entry_B0()
		;//{ }

	virtual bool vt_entry_B4(int a1, void *ptr, int a3, __int16 a4, int a5, int a6, int a7)
		;//{ JMP_THIS(0); }

	virtual int vt_entry_B8()
		;//{ return 0; }

	virtual bool vt_entry_BC()
		;//{ return 1; }

	virtual void CreateMPTeams(DynamicVectorClass<MPTeam> *vecTeams)
		;//{ JMP_THIS(0x5D6690); }

	virtual CellStruct * AssignStartingPositionsToHouse(CellStruct *result, int idxHouse,
		DynamicVectorClass<CellStruct> *vecCoords, byte *housesSatisfied)
		;//{ JMP_THIS(0x5D6890); }

	virtual bool SpawnBaseUnits(HouseClass *House, int* AmountToSpend)
		;//{ JMP_THIS(0x5D7030); }

	virtual bool GenerateStartingUnits(HouseClass *House, int* AmountToSpend)
		;//{ JMP_THIS(0x5D70F0); }

protected:
	MPGameModeClass(wchar_t** CSFTitle, wchar_t** CSFTooltip, char** INIFileName, char** mapfilter, bool AIAllowed, int MPModeIndex);

public:

	bool unknown_4;
	DynamicVectorClass<MPTeam*> MPTeams;
	WideWstring CSFTitle;
	WideWstring CSFTooltip;
	int MPModeIndex;
	Wstring INIFilename;
	Wstring MapFilter;
	bool AIAllowed;
	CCINIClass* INI;
	bool AlliesAllowed;
	bool wolTourney;
	bool wolClanTourney;
	bool MustAlly;
};

static_assert(sizeof(MPGameModeClass) == 0x40, "Invalid Size!");

class NOVTABLE MPBattleClass : public MPGameModeClass
{
	//Destructor
	virtual ~MPBattleClass();

	virtual bool vt_entry_40()
		{ return 1; }

	/*
	static (void __stdcall * Deallocate)() = (void __stdcall *)()0x5D7FF0;
	static (void __stdcall * Allocate)(INIT_ARGLIST) = (void __stdcall * Allocate)(INIT_ARGLIST)0x5D8170;
	*/

	MPBattleClass(wchar_t** CSFTitle, wchar_t** CSFTooltip, char** INIFileName, char** mapfilter, bool AIAllowed, int MPModeIndex);

};

static_assert(sizeof(MPBattleClass) == 0x40, "Invalid Size!");

class NOVTABLE MPManBattleClass : public MPGameModeClass
{
	//Destructor
	virtual ~MPManBattleClass();

	//Constructor
	MPManBattleClass(wchar_t** CSFTitle, wchar_t** CSFTooltip, char** INIFileName, char** mapfilter, bool AIAllowed, int MPModeIndex);

};

static_assert(sizeof(MPManBattleClass) == 0x40, "Invalid Size!");

class NOVTABLE MPFreeForAllClass : public MPGameModeClass
{
	//Destructor
	virtual ~MPFreeForAllClass();

	virtual bool vt_entry_10()
		;//{ JMP_THIS(0x5C5D30); }

	virtual bool vt_entry_14(DWORD dwUnk)
		;//{ JMP_THIS(0x5C5D40); }

	virtual bool vt_entry_18()
		;//{ JMP_THIS(0x5C5D90); }

	virtual bool vt_entry_40()
		{ return 1; }

	virtual void PopulateTeamDropdownForPlayer(HWND hWnd, int idx)
		;//{ JMP_THIS(0x5C5DD0); }

	MPFreeForAllClass(wchar_t** CSFTitle, wchar_t** CSFTooltip, char** INIFileName, char** mapfilter, bool AIAllowed, int MPModeIndex);
};

static_assert(sizeof(MPFreeForAllClass) == 0x40, "Invalid Size!");

class NOVTABLE MPMegawealthClass : public MPGameModeClass
{
	//Destructor
	virtual ~MPMegawealthClass();

	virtual bool vt_entry_8C()
		;//{ JMP_THIS(0x5C9430); }

	//Constructor
	MPMegawealthClass(wchar_t** CSFTitle, wchar_t** CSFTooltip, char** INIFileName, char** mapfilter, bool AIAllowed, int MPModeIndex);
};

static_assert(sizeof(MPMegawealthClass) == 0x40, "Invalid Size!");

class NOVTABLE MPUnholyAllianceClass : public MPGameModeClass
{
	//Destructor
	virtual ~MPUnholyAllianceClass();

	virtual bool vt_entry_10()
		;//{ JMP_THIS(0x5CB3F0); }

	virtual bool vt_entry_14(DWORD dwUnk)
		;//{ JMP_THIS(0x5CB400); }

	virtual bool vt_entry_18()
		;//{ JMP_THIS(0x5CB430); }

	virtual bool SpawnBaseUnits(HouseClass *House, int* AmountToSpend)
		;//{ JMP_THIS(0x5CB440); }

	MPUnholyAllianceClass(wchar_t** CSFTitle, wchar_t** CSFTooltip, char** INIFileName, char** mapfilter, bool AIAllowed, int MPModeIndex);

};

static_assert(sizeof(MPUnholyAllianceClass) == 0x40, "Invalid Size!");

class NOVTABLE MPSiegeClass : public MPGameModeClass
{
	//Destructor
	virtual ~MPSiegeClass();

	virtual bool vt_entry_10()
		;//{ JMP_THIS(0x5CA680); }

	virtual bool vt_entry_14(DWORD dwUnk)
		;//{ JMP_THIS(0x5CA6D0); }

	virtual bool vt_entry_18()
		;//{ JMP_THIS(0x5CA7D0); }

	virtual bool AIAllowed()
		{ return 0; }

	virtual bool UnfixAlliances()
		{ return MPGameModeClass::UnfixAlliances(); } // yes they did code this explicitly -_-

	virtual bool StartingPositionsToHouseBaseCells2(bool arg)
		;//{ JMP_THIS(0x5CA800); }

	virtual void CreateMPTeams(DynamicVectorClass<MPTeam> *vecTeams)
		;//{ JMP_THIS(0x5CA9B0); }

	virtual bool SpawnBaseUnits(HouseClass *House, int* AmountToSpend)
		;//{ JMP_THIS(0x5CAA50); }

	MPSiegeClass(wchar_t** CSFTitle, wchar_t** CSFTooltip, char** INIFileName, char** mapfilter, bool AIAllowed, int MPModeIndex);
};

static_assert(sizeof(MPSiegeClass) == 0x40, "Invalid Size!");

struct MPlayerScoreType
{
	static constexpr reference<MPlayerScoreType, 0xA8D1FCu, 8u> MPScores { };

	char Name[0x40];
	int Scheme;
	int NonGameOvers;
	int Lost[4];
	int Kill[4];
	int Builts[4];
	int Score[4];
};

class MPCooperative : public MPGameModeClass
{
	class CoopCampaignClass
	{
		CoopCampaignClass();
		~CoopCampaignClass() = default;

		char field__0;
		char gap_1[19];
		int House1;
		int Color1;
		char field_bool_1C;
		char gap_1D[19];
		int House2;
		int Color2;
		int CurrentMap;
		int field_3C;
		int field_40;
		int CampaignType;
		int Kills1;
		int Kills2;
		int Built1;
		int Built2;
		int Lost1;
		int Lost2;
		int Score1;
		int Score2;
		int Time;
		char field_bool_6C;
		char gap_6D[1];
		int field_6E;
	};


	MPCooperative(wchar_t** CSFTitle, wchar_t** CSFTooltip, char** INIFileName, char** mapfilter, bool AIAllowed, int MPModeIndex);

};