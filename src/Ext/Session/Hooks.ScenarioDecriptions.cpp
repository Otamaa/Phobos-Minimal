// SessionClass_ReadScenarioDescriptions.cpp
// Backport of SessionClass::Read_Scenario_Descriptions
// Original address: VERIFY (fill from IDB)
//
// Load order:
//   Pass 1 — MISSIONSMD.PKT  (hardcoded priority)
//   Pass 2 — *.PKT           (all loose PKTs, skips MISSIONSMD.PKT)
//   Pass 3 — *.YRO           (MIX archives; derives sidecar .PKT name;
//                              appends player-count suffix to description)
//   Pass 4 — *.YRM           (raw map files; reads [Basic]/[Digest] inline)
//
// ZIP INTEGRATION NOTE:
//   The natural hook point is at the CCFileClass::Set_Name / Is_Available
//   boundary used in passes 2 and 3.  A mounted zip can expose virtual
//   CCFileClass entries through the same path so the INIClass::Load calls
//   below require zero changes.

#include "Body.h"

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <CCINIClass.h>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace
{
	// Attribute bits the original masks with 0x116:
	//   FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM |
	//   FILE_ATTRIBUTE_HIDDEN    | FILE_ATTRIBUTE_TEMPORARY
	constexpr DWORD SKIP_ATTRS = 0x116;

	// Returns the usable filename from a WIN32_FIND_DATAA entry.
	// Prefers 8.3 alternate name when present (matches original logic).
	const char* GetFindName(const WIN32_FIND_DATAA& fd)
	{
		return (fd.cAlternateFileName[0] != '\0')
			? fd.cAlternateFileName
			: fd.cFileName;
	}

	bool ShouldSkipEntry(const WIN32_FIND_DATAA& fd)
	{
		return (fd.dwFileAttributes & SKIP_ATTRS) != 0;
	}

	void LoadMultiMapsFromINI(INIClass& ini,
							  DynamicVectorClass<MultiMission*>& scenarios)
	{
		for (int i = 0; i < ini.GetKeyCount("MultiMaps"); ++i) {

			const char* key = ini.GetKeyName("MultiMaps", i);

			char mapName[64] {};
			if (!ini.ReadString("MultiMaps", key,"", mapName))
				continue;

			scenarios.emplace_back(GameCreate<MultiMission>(&ini, mapName));
		}
	}

	// Build the player-count suffix string used in pass 3.
	// e.g. "(2)" or "(2-4)"
	void BuildPlayerCountSuffix(wchar_t* out, int bufChars,
								int minP, int maxP)
	{
		if (minP == maxP)
			swprintf(out, bufChars, L"%c%d%c", 0x28, minP, 0x29);
		else
			swprintf(out, bufChars, L"%c%d-%d%c", 0x28, minP, maxP, 0x29);
	}

	// Append suffix to mission description, clamped to 44 wchars.
	// Mirrors wcscpy/wcsncat sequence in pass 3.
	void AppendSuffixToDescription(MultiMission* mm,
								   const wchar_t* suffix,
								   const wchar_t* separator) // asc_82083C
	{
		if (!mm)
			return;

		const size_t nameLen = wcslen(mm->Description); // VERIFY: field name in MultiMission
		wchar_t buf[44 + 1] {};
		wcsncpy(buf, mm->Description, 44);
		wcsncat(buf, separator, 44 - nameLen);
		wcsncat(buf, suffix, 44 - (nameLen + 1));
		wcsncpy(mm->Description, buf, 44);
		mm->Description[43] = L'\0'; // VERIFY: field name + size match MultiMission layout
	}

} // anonymous namespace

// ---------------------------------------------------------------------------
// SessionClass::Read_Scenario_Descriptions
// ---------------------------------------------------------------------------

void FakeSessionClass::_Read_Scenario_Descriptions()
{
 // Spawner : Skip load *.PKT, *.YRO and *.YRM map files
 auto& scenarios = this->MultiMission;
 scenarios.clear(); 
#ifdef IDK_WHY_DISABLED

	auto& scenarios = this->MultiMission;
	scenarios.clear(); // ORIG: v3->Clear(v4)

	INIClass     ini {};   // ORIG: a8, manually zero-inited in IDA
	CCFileClass  file {};  // ORIG: a1, reused across all passes via Set_Name

	// -----------------------------------------------------------------------
	// Pass 1: MISSIONSMD.PKT  (priority hardcoded PKT)
	// -----------------------------------------------------------------------
	{
		file.Open2("MISSIONSMD.PKT", FileAccessMode::Read);
		if (file.IsAvaible(0)) {
			ini.Load(&file, false);
			LoadMultiMapsFromINI(ini, scenarios);
		}
		ini.Clear(nullptr, nullptr);
		file.Close();
	}

	// -----------------------------------------------------------------------
	// Pass 2: *.PKT  (all loose PKTs, skipping MISSIONSMD.PKT)
	// -----------------------------------------------------------------------
	{
		WIN32_FIND_DATAA fd {};
		HANDLE hFind = FindFirstFileA("*.PKT", &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do
			{
				if (ShouldSkipEntry(fd))
					continue;

				const char* name = GetFindName(fd);
				if (_strcmpi(name, "MISSIONSMD.PKT") == 0)
					continue;  // already loaded in pass 1

				file.SetFileName(name);
				ini.Load(&file, false);
				LoadMultiMapsFromINI(ini, scenarios);
			}
			while (FindNextFileA(hFind, &fd));

			FindClose(hFind);
		}
		ini.Clear(nullptr, nullptr);
		file.Close();
	}

	// -----------------------------------------------------------------------
	// Pass 3: *.YRO  (MIX archives with sidecar .PKT)
	//
	// For each .YRO (skipping MISSIONS.YRO):
	//   - Mount as MixFileClass
	//   - Derive sidecar PKT name by replacing last 3 chars with "PKT"
	//   - Load [MultiMaps] and append player-count suffix to each description
	// -----------------------------------------------------------------------
	{
		const int savedCD = CD::Disk();
		CD::SetReqCD(-2); // disable CD check while scanning

		WIN32_FIND_DATAA fd {};
		HANDLE hFind = FindFirstFileA("*.YRO", &fd);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (ShouldSkipEntry(fd))
					continue;

				const char* yroName = GetFindName(fd);
				if (_strcmpi(yroName, "MISSIONS.YRO") == 0)
					continue;

				// Mount MIX
				MixFileClass* mix = GameCreate<MixFileClass>(yroName, MixFileClass::Key());
				if (!mix)
					continue;

				MixFileClass::Array_Alt->emplace_back(mix);

				// Derive sidecar PKT name: strip last 3 chars, append "PKT"
				// ORIG: strncpy(v70, v25, strlen(v25)+1 - 4) then strcat "PKT"
				char pktName[MAX_PATH] {};
				const size_t yroLen = strlen(yroName);

				if (yroLen >= 4) {
					CRT::strncpy(pktName, yroName, yroLen - 3);
					pktName[yroLen - 3] = '\0';
					CRT::strcat(pktName, "PKT");
				}

				file.SetFileName(pktName);
				
				if (!file.IsAvaible(0)) {
					Debug::Log("Can't see .pkt inside .yro!\n");
					continue;
				}

				ini.Load(&file, false);

				for (int i = 0; i < ini.GetKeyCount("MultiMaps"); ++i) {
					const char* key = ini.GetKeyName("MultiMaps", i);

					char mapName[64] {};
					if (!ini.ReadString("MultiMaps", key, "", mapName))
						continue;

					MultiMission* mm = GameCreate<MultiMission>(&ini, mapName);

					if (!mm)
						continue;

					// Build and append "(min)" or "(min-max)" suffix
					wchar_t suffix[16] {};
					BuildPlayerCountSuffix(suffix, 16, mm->MinPlayers, mm->MaxPlayers);

					// ORIG: asc_82083C is a separator wchar_t* between name and suffix
					// VERIFY: resolve asc_82083C from IDB (likely L" " or L" - ")
					AppendSuffixToDescription(mm, suffix, L" ");

					scenarios.emplace_back(mm);
				}

				ini.Clear(nullptr, nullptr);
			}
			while (FindNextFileA(hFind, &fd));

			FindClose(hFind);
		}

		CD::SetReqCD(savedCD);

		ini.Clear(nullptr, nullptr);
		file.Close();
	}

	// -----------------------------------------------------------------------
	// Pass 4: *.YRM  (raw map files; reads [Basic] + [Digest] inline)
	//
	// NOTE: Original reads from `a8` (ini) without calling INIClass::Load
	//       first for each file — it relies on a8 still holding data from
	//       the previous pass iteration or being empty.  This looks like a
	//       vanilla bug (reading stale INI state).  Preserved as-is.
	//       SUSPECT: may need INIClass::Load(&ini, &file, 0) before
	//                Get_String calls if maps show wrong names in practice.
	// -----------------------------------------------------------------------
	{
		WIN32_FIND_DATAA fd {};
		HANDLE hFindYRM = FindFirstFileA("*.YRM", &fd);
		bool official = false;

		if (hFindYRM != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (ShouldSkipEntry(fd))
					continue;

				const char* filename = GetFindName(fd);

				char   mapNameBuf[64] {};
				char   digestBuf[32] {};
				wchar_t description[64] {};

				ini.ReadString("Basic", "Name","No Name", mapNameBuf);
				ini.ReadString("Digest", "1","No Digest", digestBuf);

				mbstowcs(description, mapNameBuf, -1);
				official = ini.ReadBool("Basic", "Official", 0) != 0;

				int minPlayers = 2;
				int maxPlayers = 4;

				// ORIG: SessionClass_Read_Basic — fills minPlayers/maxPlayers
				// and possibly official/digest from the actual file.
				// VERIFY: signature — takes (filename, desc, digest, a4,
				//          &official, &min, &max)
				CCINIClass::ReadBasic(filename, description, digestBuf,
										0, official,
										minPlayers, maxPlayers);

				scenarios.emplace_back(GameCreate<MultiMission>(
					filename, description, digestBuf,
					official, 0, minPlayers, maxPlayers));

			}
			while (FindNextFileA(hFindYRM, &fd));

			FindClose(hFindYRM);
		}
	}
#endif
}

DEFINE_FUNCTION_JUMP(LJMP, 0x699980, FakeSessionClass::_Read_Scenario_Descriptions)
DEFINE_FUNCTION_JUMP(CALL, 0x697AD7, FakeSessionClass::_Read_Scenario_Descriptions)