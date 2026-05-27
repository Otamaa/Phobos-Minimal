#include "PhobosSaveVersionInfo.h"
#include <Utilities/Debug.h>

PhobosSaveVersionInfo::PhobosSaveVersionInfo() :
	InternalVersion(0),
	Version(0),
	ScenarioDescription { L"" },
	PlayerHouse { "" },
	CampaignNumber(-1),
	ScenarioNumber(0),
	UnknownString { "" },
	PlayerName { "" },
	ExecutableName { "" },
	GameType(0),
	PhobosVersion(0),
	SessionID(0),
	Difficulty(0)
{
	StartTime.dwLowDateTime = 0;
	StartTime.dwHighDateTime = 0;
	PlayTime.dwLowDateTime = 0;
	PlayTime.dwHighDateTime = 0;
	LastSaveTime.dwLowDateTime = 0;
	LastSaveTime.dwHighDateTime = 0;
}

PhobosSaveVersionInfo::~PhobosSaveVersionInfo()
{ }

void PhobosSaveVersionInfo::SetVersion(int num)
{
	Version = num;
}

int PhobosSaveVersionInfo::GetVersion() const
{
	return Version;
}

void PhobosSaveVersionInfo::SetInternalVersion(int num)
{
	InternalVersion = num;
}

int PhobosSaveVersionInfo::GetInternalVersion() const
{
	return InternalVersion;
}

void PhobosSaveVersionInfo::SetScenarioDescription(const wchar_t* desc)
{
	ScenarioDescription[sizeof(ScenarioDescription) / sizeof(wchar_t) - 1] = 0;
	wcsncpy(ScenarioDescription, desc, sizeof(ScenarioDescription) / sizeof(wchar_t) - 1);
}

const wchar_t* PhobosSaveVersionInfo::GetScenarioDescription() const
{
	return ScenarioDescription;
}

void PhobosSaveVersionInfo::SetPlayerHouse(const char* name)
{
	PlayerHouse[sizeof(PlayerHouse) - 1] = 0;
	strncpy(PlayerHouse, name, sizeof(PlayerHouse) - 1);
}

const char* PhobosSaveVersionInfo::GetPlayerHouse() const
{
	return PlayerHouse;
}

void PhobosSaveVersionInfo::SetCampaignNumber(int num)
{
	CampaignNumber = num;
}

int PhobosSaveVersionInfo::GetCampaignNumber() const
{
	return CampaignNumber;
}

void PhobosSaveVersionInfo::SetScenarioNumber(int num)
{
	ScenarioNumber = num;
}

int PhobosSaveVersionInfo::GetScenarioNumber() const
{
	return ScenarioNumber;
}

void PhobosSaveVersionInfo::SetUnknownString(const char* str)
{
	UnknownString[sizeof(UnknownString) - 1] = 0;
	strncpy(UnknownString, str, sizeof(UnknownString) - 1);
}

const char* PhobosSaveVersionInfo::GetUnknownString() const
{
	return UnknownString;
}

void PhobosSaveVersionInfo::SetPlayerName(const char* name)
{
	PlayerName[sizeof(PlayerName) - 1] = 0;
	strncpy(PlayerName, name, sizeof(PlayerName) - 1);
}

const char* PhobosSaveVersionInfo::GetPlayerName() const
{
	return PlayerName;
}

void PhobosSaveVersionInfo::SetExecutableName(const char* name)
{
	ExecutableName[sizeof(ExecutableName) - 1] = 0;
	strncpy(ExecutableName, name, sizeof(ExecutableName) - 1);
}

const char* PhobosSaveVersionInfo::GetExecutableName() const
{
	return ExecutableName;
}

void PhobosSaveVersionInfo::SetStartTime(FILETIME* time)
{
	StartTime = *time;
}

FILETIME PhobosSaveVersionInfo::GetStartTime() const
{
	return StartTime;
}

void PhobosSaveVersionInfo::SetPlayTime(FILETIME* time)
{
	PlayTime = *time;
}

FILETIME PhobosSaveVersionInfo::GetPlayTime() const
{
	return PlayTime;
}

void PhobosSaveVersionInfo::SetLastTime(FILETIME* time)
{
	LastSaveTime = *time;
}

FILETIME PhobosSaveVersionInfo::GetLastTime() const
{
	return LastSaveTime;
}

void PhobosSaveVersionInfo::SetGameType(int type)
{
	GameType = type;
}

int PhobosSaveVersionInfo::GetGameType() const
{
	return GameType;
}

void PhobosSaveVersionInfo::SetPhobosVersion(int num)
{
	PhobosVersion = num;
}

int PhobosSaveVersionInfo::GetPhobosVersion() const
{
	return PhobosVersion;
}

void PhobosSaveVersionInfo::SetSessionID(int num)
{
	SessionID = num;
}

int PhobosSaveVersionInfo::GetSessionID() const
{
	return SessionID;
}

void PhobosSaveVersionInfo::SetDifficulty(int num)
{
	Difficulty = num;
}

int PhobosSaveVersionInfo::GetDifficulty() const
{
	return Difficulty;
}

HRESULT __fastcall PhobosSaveVersionInfo::Save(IStorage* storage)
{
	if (storage == nullptr)
	{
		return E_POINTER;
	}

	Debug::Log("Attempting to obtain PropertySetStorage interface\n");

	IPropertySetStoragePtr storageset;
	HRESULT res = storage->QueryInterface(IID_IPropertySetStorage, (void**)&storageset);

	if (SUCCEEDED(res))
	{
		Debug::Log("Saving version information the new way.\n");

		res = SaveStringSet(storageset, ID_SCENARIO_DESCRIPTION, ScenarioDescription);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveStringSet(storageset, ID_PLAYER_HOUSE, PlayerHouse);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveIntSet(storageset, ID_VERSION, Version);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveIntSet(storageset, ID_INTERNAL_VERSION, InternalVersion);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveTimeSet(storageset, ID_START_TIME, &StartTime);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveTimeSet(storageset, ID_LAST_SAVE_TIME, &LastSaveTime);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveTimeSet(storageset, ID_PLAY_TIME, &PlayTime);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveStringSet(storageset, ID_EXECUTABLE_NAME, ExecutableName);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveStringSet(storageset, ID_PLAYER_NAME, PlayerName);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveStringSet(storageset, ID_PLAYER_NAME2, PlayerName);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveIntSet(storageset, ID_SCENARIO_NUMBER, ScenarioNumber);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveIntSet(storageset, ID_CAMPAIGN, CampaignNumber);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveIntSet(storageset, ID_GAMETYPE, GameType);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveIntSet(storageset, ID_PHOBOS_VERSION, PhobosVersion);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveIntSet(storageset, ID_SESSION_ID, SessionID);
		if (FAILED(res))
		{
			return res;
		}

		res = SaveIntSet(storageset, ID_DIFFICULTY, Difficulty);
		if (FAILED(res))
		{
			return res;
		}
	}
	else
	{
		Debug::Log("Failed to save the new way!\n");
	}

	Debug::Log("Saving version information the old way.\n");

	res = SaveString(storage, ID_SCENARIO_DESCRIPTION, ScenarioDescription);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveString(storage, ID_PLAYER_HOUSE, PlayerHouse);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveInt(storage, ID_VERSION, Version);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveInt(storage, ID_INTERNAL_VERSION, InternalVersion);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveTime(storage, ID_START_TIME, &StartTime);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveTime(storage, ID_LAST_SAVE_TIME, &LastSaveTime);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveTime(storage, ID_PLAY_TIME, &PlayTime);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveString(storage, ID_EXECUTABLE_NAME, ExecutableName);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveString(storage, ID_PLAYER_NAME, PlayerName);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveString(storage, ID_PLAYER_NAME2, PlayerName);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveInt(storage, ID_SCENARIO_NUMBER, ScenarioNumber);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveInt(storage, ID_CAMPAIGN, CampaignNumber);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveInt(storage, ID_GAMETYPE, GameType);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveInt(storage, ID_PHOBOS_VERSION, PhobosVersion);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveInt(storage, ID_SESSION_ID, SessionID);
	if (FAILED(res))
	{
		return res;
	}

	res = SaveInt(storage, ID_DIFFICULTY, Difficulty);
	if (FAILED(res))
	{
		return res;
	}

	return S_OK;
}

HRESULT __fastcall PhobosSaveVersionInfo::Load(IStorage* storage)
{
	char buffer[256];

	if (storage == nullptr)
	{
		return E_POINTER;
	}

	Debug::Log("Attempting to obtain PropertySetStorage interface\n");

	IPropertySetStoragePtr storageset;
	HRESULT res = storage->QueryInterface(IID_IPropertySetStorage, (void**)&storageset);

	if (SUCCEEDED(res))
	{
		Debug::Log("Loading version information.\n");

		res = LoadStringSet(storageset, ID_SCENARIO_DESCRIPTION, ScenarioDescription, sizeof(ScenarioDescription) / sizeof(wchar_t));
		if (FAILED(res))
		{
			return res;
		}

		res = LoadStringSet(storageset, ID_PLAYER_HOUSE, buffer, sizeof(buffer));
		if (FAILED(res))
		{
			return res;
		}
		strcpy(PlayerHouse, buffer);

		res = LoadIntSet(storageset, ID_VERSION, &Version);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadIntSet(storageset, ID_INTERNAL_VERSION, &InternalVersion);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadTimeSet(storageset, ID_START_TIME, &StartTime);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadTimeSet(storageset, ID_LAST_SAVE_TIME, &LastSaveTime);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadTimeSet(storageset, ID_PLAY_TIME, &PlayTime);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadStringSet(storageset, ID_EXECUTABLE_NAME, buffer, sizeof(buffer));
		if (FAILED(res))
		{
			return res;
		}
		strcpy(ExecutableName, buffer);

		res = LoadStringSet(storageset, ID_PLAYER_NAME, buffer, sizeof(buffer));
		if (FAILED(res))
		{
			return res;
		}
		strcpy(PlayerName, buffer);

		res = LoadIntSet(storageset, ID_SCENARIO_NUMBER, &ScenarioNumber);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadIntSet(storageset, ID_CAMPAIGN, &CampaignNumber);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadIntSet(storageset, ID_GAMETYPE, &GameType);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadIntSet(storageset, ID_PHOBOS_VERSION, &PhobosVersion);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadIntSet(storageset, ID_SESSION_ID, &SessionID);
		if (FAILED(res))
		{
			return res;
		}

		res = LoadIntSet(storageset, ID_DIFFICULTY, &Difficulty);
		if (FAILED(res))
		{
			return res;
		}
	}
	else
	{
		Debug::Log("Failed to load the new way!\n");
	}

	Debug::Log("Loading version information the old way.\n");

	res = LoadString(storage, ID_SCENARIO_DESCRIPTION, ScenarioDescription, sizeof(ScenarioDescription) / sizeof(wchar_t));
	if (FAILED(res))
	{
		return res;
	}

	res = LoadString(storage, ID_PLAYER_HOUSE, buffer, sizeof(buffer));
	if (FAILED(res))
	{
		return res;
	}
	strcpy(PlayerHouse, buffer);

	res = LoadInt(storage, ID_VERSION, &Version);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadInt(storage, ID_INTERNAL_VERSION, &InternalVersion);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadTime(storage, ID_START_TIME, &StartTime);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadTime(storage, ID_LAST_SAVE_TIME, &LastSaveTime);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadTime(storage, ID_PLAY_TIME, &PlayTime);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadString(storage, ID_EXECUTABLE_NAME, buffer, sizeof(buffer));
	if (FAILED(res))
	{
		return res;
	}
	strcpy(ExecutableName, buffer);

	res = LoadString(storage, ID_PLAYER_NAME, buffer, sizeof(buffer));
	if (FAILED(res))
	{
		return res;
	}
	strcpy(PlayerName, buffer);

	res = LoadInt(storage, ID_SCENARIO_NUMBER, &ScenarioNumber);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadInt(storage, ID_CAMPAIGN, &CampaignNumber);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadInt(storage, ID_GAMETYPE, &GameType);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadInt(storage, ID_PHOBOS_VERSION, &PhobosVersion);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadInt(storage, ID_SESSION_ID, &SessionID);
	if (FAILED(res))
	{
		return res;
	}

	res = LoadInt(storage, ID_DIFFICULTY, &Difficulty);
	if (FAILED(res))
	{
		return res;
	}

	return S_OK;
}

HRESULT __fastcall PhobosSaveVersionInfo::LoadString(IStorage* storage, int id, char* string, size_t buffer_size)
{
	HRESULT res;
	IStreamPtr stm;

	*string = '\0';

	res = storage->OpenStream(PhobosStreamNameFromID(id), nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
	if (FAILED(res))
	{
		return res;
	}

	int i = 0;
	WCHAR buffer[128];

	do
	{
		res = stm->Read(&buffer[i], sizeof(buffer[i]), nullptr);
		if (FAILED(res))
		{
			return res;
		}
	}
	while (buffer[i++]);

	WideCharToMultiByte(CP_ACP, 0, buffer, -1, string, buffer_size - 1, nullptr, nullptr);
	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::LoadString(IStorage* storage, int id, wchar_t* string, size_t buffer_size)
{
	HRESULT res;
	IStreamPtr stm;

	*string = '\0';

	res = storage->OpenStream(PhobosStreamNameFromID(id), nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
	if (FAILED(res))
	{
		return res;
	}

	int i = 0;
	WCHAR buffer[128];

	do
	{
		res = stm->Read(&buffer[i], sizeof(buffer[i]), nullptr);
		if (FAILED(res))
		{
			return res;
		}
	}
	while (buffer[i++]);

	wcscpy_s(string, buffer_size, buffer);
	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::LoadStringSet(IPropertySetStorage* storageset, int id, char* string, size_t buffer_size)
{
	HRESULT res;
	IPropertyStoragePtr storage;

	*string = '\x0';

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &storage);
	if (FAILED(res))
	{
		return res;
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	res = storage->ReadMultiple(1, &propsec, &propvar);
	if (FAILED(res))
	{
		return res;
	}

	if (propvar.vt == VT_LPWSTR)
	{
		WideCharToMultiByte(CP_ACP, 0, propvar.pwszVal, -1, string, buffer_size, nullptr, nullptr);
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::LoadStringSet(IPropertySetStorage* storageset, int id, wchar_t* string, size_t buffer_size)
{
	HRESULT res;
	IPropertyStoragePtr storage;

	*string = L'\x0';

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &storage);
	if (FAILED(res))
	{
		return res;
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	res = storage->ReadMultiple(1, &propsec, &propvar);
	if (FAILED(res))
	{
		return res;
	}

	if (propvar.vt == VT_LPWSTR)
	{
		wcscpy_s(string, buffer_size, propvar.pwszVal);
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::LoadInt(IStorage* storage, int id, int* integer)
{
	HRESULT res;
	IStreamPtr stm;

	*integer = 0;

	res = storage->OpenStream(PhobosStreamNameFromID(id), nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
	if (FAILED(res))
	{
		return res;
	}

	res = stm->Read(integer, sizeof(*integer), nullptr);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::LoadIntSet(IPropertySetStorage* storageset, int id, int* integer)
{
	HRESULT res;
	IPropertyStoragePtr storage;

	*integer = 0;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &storage);
	if (FAILED(res))
	{
		return res;
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	res = storage->ReadMultiple(1, &propsec, &propvar);
	if (FAILED(res))
	{
		return res;
	}

	if (propvar.vt == VT_I4)
	{
		*integer = propvar.lVal;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::SaveString(IStorage* storage, int id, char* string)
{
	WCHAR buffer[128];

	MultiByteToWideChar(CP_ACP, 0, string, -1, buffer, sizeof(buffer) / sizeof(WCHAR) - 1);

	IStreamPtr stm(nullptr);

	HRESULT res = storage->CreateStream(PhobosStreamNameFromID(id), STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
	if (FAILED(res))
	{
		return res;
	}

	res = stm->Write(buffer, sizeof(WCHAR) * wcslen(buffer) + 2, nullptr);
	if (FAILED(res))
	{
		return res;
	}
	res = stm->Commit(STGC_DEFAULT);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::SaveString(IStorage* storage, int id, wchar_t* string)
{
	WCHAR buffer[128];
	wcscpy_s(buffer, sizeof(buffer) / sizeof(WCHAR) - 1, string);

	IStreamPtr stm(nullptr);

	HRESULT res = storage->CreateStream(PhobosStreamNameFromID(id), STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
	if (FAILED(res))
	{
		return res;
	}

	res = stm->Write(buffer, sizeof(WCHAR) * wcslen(buffer) + 2, nullptr);
	if (FAILED(res))
	{
		return res;
	}
	res = stm->Commit(STGC_DEFAULT);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::SaveStringSet(IPropertySetStorage* storageset, int id, const char* string)
{
	WCHAR buffer[128];

	MultiByteToWideChar(CP_ACP, 0, string, -1, buffer, sizeof(buffer) / sizeof(WCHAR) - 1);

	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &storage);
	if (FAILED(res))
	{
		res = storageset->Create(FMTID_SummaryInformation, nullptr, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE, &storage);
		if (FAILED(res))
		{
			return res;
		}
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	propvar.vt = VT_LPWSTR;
	propvar.pwszVal = buffer;

	res = storage->WriteMultiple(1, &propsec, &propvar, 2);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::SaveStringSet(IPropertySetStorage* storageset, int id, const wchar_t* string)
{
	WCHAR buffer[128];
	wcscpy_s(buffer, sizeof(buffer) / sizeof(WCHAR) - 1, string);

	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &storage);
	if (FAILED(res))
	{
		res = storageset->Create(FMTID_SummaryInformation, nullptr, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE, &storage);
		if (FAILED(res))
		{
			return res;
		}
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	propvar.vt = VT_LPWSTR;
	propvar.pwszVal = buffer;

	res = storage->WriteMultiple(1, &propsec, &propvar, 2);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::SaveInt(IStorage* storage, int id, int integer)
{
	IStreamPtr stm(nullptr);

	HRESULT res = storage->CreateStream(PhobosStreamNameFromID(id), STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
	if (FAILED(res))
	{
		return res;
	}

	res = stm->Write(&integer, sizeof(integer), nullptr);
	if (FAILED(res))
	{
		return res;
	}
	res = stm->Commit(STGC_DEFAULT);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::SaveIntSet(IPropertySetStorage* storageset, int id, int integer)
{
	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &storage);
	if (FAILED(res))
	{
		res = storageset->Create(FMTID_SummaryInformation, nullptr, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE, &storage);
		if (FAILED(res))
		{
			return res;
		}
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	propvar.vt = VT_I4;
	propvar.lVal = integer;

	res = storage->WriteMultiple(1, &propsec, &propvar, 2);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::LoadTime(IStorage* storage, int id, FILETIME* time)
{
	HRESULT res;
	IStreamPtr stm;

	time->dwLowDateTime = 0;
	time->dwHighDateTime = 0;

	res = storage->OpenStream(PhobosStreamNameFromID(id), nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
	if (FAILED(res))
	{
		return res;
	}

	res = stm->Read(time, sizeof(*time), nullptr);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::LoadTimeSet(IPropertySetStorage* storageset, int id, FILETIME* time)
{
	HRESULT res;
	IPropertyStoragePtr storage;

	time->dwLowDateTime = 0;
	time->dwHighDateTime = 0;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &storage);
	if (FAILED(res))
	{
		return res;
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	res = storage->ReadMultiple(1, &propsec, &propvar);
	if (FAILED(res))
	{
		return res;
	}

	if (propvar.vt == VT_FILETIME)
	{
		*time = propvar.filetime;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::SaveTime(IStorage* storage, int id, FILETIME* time)
{
	IStreamPtr stm(nullptr);

	HRESULT res = storage->CreateStream(PhobosStreamNameFromID(id), STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
	if (FAILED(res))
	{
		return res;
	}

	res = stm->Write(time, sizeof(*time), nullptr);
	if (FAILED(res))
	{
		return res;
	}
	res = stm->Commit(STGC_DEFAULT);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

HRESULT __fastcall PhobosSaveVersionInfo::SaveTimeSet(IPropertySetStorage* storageset, int id, FILETIME* time)
{
	HRESULT res;
	IPropertyStoragePtr storage;

	res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, &storage);
	if (FAILED(res))
	{
		res = storageset->Create(FMTID_SummaryInformation, nullptr, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE | STGM_READWRITE | STGM_CREATE, &storage);
		if (FAILED(res))
		{
			return res;
		}
	}

	PROPSPEC propsec;
	propsec.ulKind = PRSPEC_PROPID;
	propsec.propid = id;
	PROPVARIANT propvar;

	propvar.vt = VT_FILETIME;
	propvar.filetime = *time;

	res = storage->WriteMultiple(1, &propsec, &propvar, 2);
	if (FAILED(res))
	{
		return res;
	}

	return res;
}

const WCHAR* __fastcall PhobosStreamNameFromID(int id)
{
	struct StreamID
	{
		int ID;
		WCHAR const* Name;
	};

	static constexpr StreamID _ids[] = {
		{ PhobosSaveVersionInfo::ID_SCENARIO_DESCRIPTION,  L"Scenario Description" },
		{ PhobosSaveVersionInfo::ID_PLAYER_HOUSE,          L"Player House" },
		{ PhobosSaveVersionInfo::ID_VERSION,               L"Version" },
		{ PhobosSaveVersionInfo::ID_INTERNAL_VERSION,      L"Internal Version" },
		{ PhobosSaveVersionInfo::ID_START_TIME,            L"Start Time" },
		{ PhobosSaveVersionInfo::ID_LAST_SAVE_TIME,        L"Last Save Time" },
		{ PhobosSaveVersionInfo::ID_PLAY_TIME,             L"Play Time" },
		{ PhobosSaveVersionInfo::ID_EXECUTABLE_NAME,       L"Executable Name" },
		{ PhobosSaveVersionInfo::ID_PLAYER_NAME,           L"Player Name" },
		{ PhobosSaveVersionInfo::ID_PLAYER_NAME2,          L"Player Name2" },
		{ PhobosSaveVersionInfo::ID_SCENARIO_NUMBER,       L"Scenario Number" },
		{ PhobosSaveVersionInfo::ID_CAMPAIGN,              L"Campaign" },
		{ PhobosSaveVersionInfo::ID_GAMETYPE,              L"GameType" },
		{ PhobosSaveVersionInfo::ID_PHOBOS_VERSION,        L"Phobos Version" },
		{ PhobosSaveVersionInfo::ID_SESSION_ID,            L"Session ID" },
		{ PhobosSaveVersionInfo::ID_DIFFICULTY,            L"Difficulty" },
	};

	for (int i = 0; i < sizeof(_ids) / sizeof(StreamID); i++)
	{
		if (_ids[i].ID == id)
		{
			return _ids[i].Name;
		}
	}

	return nullptr;
}

bool __fastcall PhobosGetSavefileInfo(const char* name, PhobosSaveVersionInfo* info)
{
	IStoragePtr storage;
	WCHAR wname[PATH_MAX];

	MultiByteToWideChar(0, 0, name, -1, wname, sizeof(wname) / sizeof(WCHAR));

	HRESULT result = StgOpenStorage(wname, nullptr, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, nullptr, 0, &storage);
	if (FAILED(result))
	{
		return false;
	}

	result = info->Load(storage);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}