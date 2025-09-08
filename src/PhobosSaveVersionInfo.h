#pragma once

#include "Phobos.h"

#pragma once

#include "Phobos.h"

class PhobosSaveVersionInfo
{
public:
	enum
	{
		ID_SCENARIO_DESCRIPTION = 2,
		ID_PLAYER_HOUSE = 3,
		ID_VERSION = 9,
		ID_INTERNAL_VERSION = 16,
		ID_START_TIME = 12,
		ID_LAST_SAVE_TIME = 13,
		ID_PLAY_TIME = 10,
		ID_EXECUTABLE_NAME = 18,
		ID_PLAYER_NAME = 4,
		ID_PLAYER_NAME2 = 8,
		ID_SCENARIO_NUMBER = 100,
		ID_CAMPAIGN = 101,
		ID_GAMETYPE = 102,

		ID_PHOBOS_VERSION = 105,
		ID_SESSION_ID = 107,
		ID_DIFFICULTY = 108,
	};

public:
	PhobosSaveVersionInfo();
	~PhobosSaveVersionInfo();

	void SetVersion(int num);
	int GetVersion() const;

	void SetInternalVersion(int num);
	int GetInternalVersion() const;

	void SetScenarioDescription(const wchar_t* desc);
	const wchar_t* GetScenarioDescription() const;

	void SetPlayerHouse(const char* name);
	const char* GetPlayerHouse() const;

	void SetCampaignNumber(int num);
	int GetCampaignNumber() const;

	void SetScenarioNumber(int num);
	int GetScenarioNumber() const;

	void SetUnknownString(const char* name);
	const char* GetUnknownString() const;

	void SetPlayerName(const char* name);
	const char* GetPlayerName() const;

	void SetExecutableName(const char* name);
	const char* GetExecutableName() const;

	void SetStartTime(FILETIME* time);
	FILETIME GetStartTime() const;

	void SetPlayTime(FILETIME* time);
	FILETIME GetPlayTime() const;

	void SetLastTime(FILETIME* time);
	FILETIME GetLastTime() const;

	void SetGameType(int id);
	int GetGameType() const;

	void SetPhobosVersion(int num);
	int GetPhobosVersion() const;

	void SetSessionID(int num);
	int GetSessionID() const;

	void SetDifficulty(int num);
	int GetDifficulty() const;

	HRESULT __fastcall Save(IStorage* storage);
	HRESULT __fastcall Load(IStorage* storage);

private:
	HRESULT __fastcall LoadString(IStorage* storage, int id, char* string, size_t buffer_size);
	HRESULT __fastcall LoadString(IStorage* storage, int id, wchar_t* string, size_t buffer_size);
	HRESULT __fastcall LoadStringSet(IPropertySetStorage* storageset, int id, char* string, size_t buffer_size);
	HRESULT __fastcall LoadStringSet(IPropertySetStorage* storageset, int id, wchar_t* string, size_t buffer_size);

	HRESULT __fastcall LoadInt(IStorage* storage, int id, int* integer);
	HRESULT __fastcall LoadIntSet(IPropertySetStorage* storageset, int id, int* integer);

	HRESULT __fastcall SaveString(IStorage* storage, int id, char* string);
	HRESULT __fastcall SaveString(IStorage* storage, int id, wchar_t* string);
	HRESULT __fastcall SaveStringSet(IPropertySetStorage* storageset, int id, const char* string);
	HRESULT __fastcall SaveStringSet(IPropertySetStorage* storageset, int id, const wchar_t* string);

	HRESULT __fastcall SaveInt(IStorage* storage, int id, int integer);
	HRESULT __fastcall SaveIntSet(IPropertySetStorage* storageset, int id, int integer);

	HRESULT __fastcall LoadTime(IStorage* storage, int id, FILETIME* time);
	HRESULT __fastcall LoadTimeSet(IPropertySetStorage* storageset, int id, FILETIME* time);

	HRESULT __fastcall SaveTime(IStorage* storage, int id, FILETIME* time);
	HRESULT __fastcall SaveTimeSet(IPropertySetStorage* storageset, int id, FILETIME* time);

private:
	int InternalVersion;
	int Version;
	wchar_t ScenarioDescription[128];
	char PlayerHouse[64];
	int CampaignNumber;
	int ScenarioNumber;
	char UnknownString[260];
	char PlayerName[64];
	char ExecutableName[260];
	FILETIME StartTime;
	FILETIME PlayTime;
	FILETIME LastSaveTime;
	int GameType;
	int PhobosVersion;
	int SessionID;
	int Difficulty;
};

const WCHAR* __fastcall PhobosStreamNameFromID(int id);
bool __fastcall PhobosGetSavefileInfo(const char* name, PhobosSaveVersionInfo* info);