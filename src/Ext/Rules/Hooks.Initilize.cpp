// InitRules.cpp
// Backport of Init_Rules() from IDA pseudocode.
// Original address: VERIFY (fill in from IDB)
//
// Loading order recap:
//   1. Glob "RULEMD*.INI" via FindFirstFile.
//      - RULESMD.INI  -> prepended (index 0), read=true
//      - everything else -> appended in MFT order
//   2. If glob missed or RULESMD.INI was absent -> fallback direct load of RULESMD.INI.
//   3. If still empty -> bail false.
//   4. Load ARTMD.INI (hard fail).
//   5. count==1 -> use rules[0]; else -> show selection dialog.
//   6. RulesClass callbacks, GameOptions sync.
//   7. LANGRULE.INI optional overlay (Process only; >1 digest sections = hard fail).
//   8. Destroy non-selected INIs.
//   9. Load AIMD.INI.

#include "Body.h"
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#ifdef ORIGINAL

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace
{
	// Mirrors the original front-insert: shift all elements right by one, place
	// new_ini at index 0.  Matches memmove_ALSO_memcpy(Vector+1, Vector, 4*count).
	void PrependToRulesVector(DynamicVectorClass<CCINIClass*>& vec, CCINIClass* new_ini)
	{
		// Grow if needed (grow-increment = 10, matching original v18.vtable = 10)
		if (vec.Count >= vec.VectorMax)
		{
			constexpr int GROW_INCREMENT = 10; // ORIG: v18.vtable initialised to 10
			if (!vec.IsAllocated && vec.VectorMax)
				return; // fixed external buffer, cannot grow
			if (!vec.Resize(vec.VectorMax + GROW_INCREMENT))
				return;
		}

		if (vec.Count > 0)
			memmove(&vec.Vector[1], &vec.Vector[0], sizeof(CCINIClass*) * vec.Count);

		vec.Vector[0] = new_ini;
		++vec.Count;
	}

	// Mirrors the original append path.
	void AppendToRulesVector(DynamicVectorClass<CCINIClass*>& vec, CCINIClass* new_ini)
	{
		constexpr int GROW_INCREMENT = 10;
		if (vec.Count >= vec.VectorMax)
		{
			if (!vec.IsAllocated && vec.VectorMax)
				return;
			if (!vec.Resize(vec.VectorMax + GROW_INCREMENT))
				return;
		}
		vec.Vector[vec.Count++] = new_ini;
	}

	// Allocate + construct a CCINIClass and load it from file.
	// Returns nullptr on allocation failure.
	CCINIClass* AllocAndLoadINI(CCFileClass& file, bool allowDigest = false)
	{
		CCINIClass* ini = new (std::nothrow) CCINIClass();
		if (!ini)
			return nullptr;
		CCINIClass::Load(ini, &file, allowDigest ? 1 : 0, 0);
		return ini;
	}

	// Destroy a CCINIClass.  Original: (*v14->ini.vtable)(v14, 1) = virtual dtor.
	void DestroyINI(CCINIClass* ini)
	{
		if (ini)
			ini->~CCINIClass(); // ORIG: (*vtable)(ptr, 1)
		// NOTE: original does NOT call operator delete after the vdtor in the
		//       cleanup loop — only the vdtor is invoked.  If YRpp CCINIClass
		//       uses a deleting destructor pattern verify this. VERIFY.
	}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Init_Rules
// ---------------------------------------------------------------------------
bool Init_Rules()
{
	// --- Phase 1: glob scan for "RULEMD*.INI" ----------------------------

	DynamicVectorClass<CCINIClass*> rules; // ORIG: sub_538B30 init + vftable set
	int  count = 0;
	bool read = false; // true once RULESMD.INI is seen in the glob

	WIN32_FIND_DATAA findData {};
	HANDLE hFind = FindFirstFileA("RULEMD*.INI", &findData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// Skip hidden / system / directory / temporary entries
			constexpr DWORD SKIP_ATTRS =
				FILE_ATTRIBUTE_TEMPORARY |
				FILE_ATTRIBUTE_DIRECTORY |
				FILE_ATTRIBUTE_SYSTEM |
				FILE_ATTRIBUTE_HIDDEN;

			if (findData.dwFileAttributes & SKIP_ATTRS)
				continue;

			// Prefer the 8.3 alternate name when available (matches original v1 logic)
			const char* filename = findData.cAlternateFileName[0]
				? findData.cAlternateFileName
				: findData.cFileName;

			CCFileClass  file(filename);
			CCINIClass* ini = AllocAndLoadINI(file);
			if (!ini)
				continue;

			if (_strcmpi(filename, "RULESMD.INI") == 0)
			{
				// RULESMD.INI always goes to front
				PrependToRulesVector(rules, ini);
				++count;
				read = true;
			}
			else
			{
				// All other RULEMD*.INI variants appended in MFT order
				AppendToRulesVector(rules, ini);
				++count;
			}

		}
		while (FindNextFileA(hFind, &findData));

		FindClose(hFind);
	}

	// --- Phase 2: fallback if RULESMD.INI was never found ----------------
	//
	// Covers two cases:
	//   a) FindFirstFile returned INVALID_HANDLE_VALUE (no matches at all)
	//   b) Glob ran fine but RULESMD.INI was not among the matches
	//
	if (!read)
	{
		CCFileClass  file("RULESMD.INI");
		CCINIClass* ini = AllocAndLoadINI(file);
		if (ini)
		{
			PrependToRulesVector(rules, ini);
			++count;
		}
	}

	// --- Phase 3: bail if still empty ------------------------------------

	auto ClearAndFail = [&]() -> bool
		{
			rules.Clear(); // ORIG: downcast to VectorClass vftable then Clear
			return false;
		};

	if (count <= 0)
		return ClearAndFail();

	// --- Phase 4: load ARTMD.INI (mandatory) -----------------------------

	{
		CCFileClass artFile("ARTMD.INI");
		if (!CCINIClass::Load(&ArtINI, &artFile, 0, 0))
		{
			WWDebugString("Failed to load ARTMD.INI!\n");
			return ClearAndFail();
		}
	}

	// --- Phase 5: pick which rules INI to actually use -------------------

	CCINIClass* selectedRules = nullptr;
	if (count == 1)
	{
		selectedRules = rules.Vector[0];
	}
	else
	{
		// Show selection dialog; returns index (-1 treated as 0)
		int idx = static_cast<int>(
			DialogBoxParamA(ProgramInstance, 147, MainWindow,
				Select_Rules_Dialog, &rules));
		if (idx < 0)
			idx = 0;
		selectedRules = rules.Vector[idx];
	}

	RuleINI = selectedRules;

	// --- Phase 6: RulesClass callbacks + GameOptions sync ----------------

	RulesClass::Colors(Rule, selectedRules);
	RulesClass::ColorAdd(Rule, RuleINI);
	RulesClass::Movies(Rule, &ArtINI);
	RulesClass::AudioVisual(Rule, RuleINI);
	RulesClass::MPlayer(Rule, RuleINI);

	GameModeOptionsClass::Instance->Credits = pRules->MPDefaultMoney;
	GameModeOptionsClass::Instance->UnitCount = pRules->MPUnitCount;
	BuildLevel = pRules->MPTechLevel;
	GameModeOptionsClass::Instance->GameSpeed = pRules->GameSpeed;
	GameModeOptionsClass::Instance->AIDifficulty = pRules->AIDifficulty;
	GameModeOptionsClass::Instance->AIPlayers = pRules->AIPlayers;
	GameModeOptionsClass::Instance->BridgeDestruction = pRules->BridgeDestruction;
	GameModeOptionsClass::Instance->Bases = pRules->IsMPBasesOn;
	GameModeOptionsClass::Instance->Goodies = pRules->IsMPCrates;
	GameModeOptionsClass::Instance->CaptureTheFlag = pRules->IsMPCaptureTheFlag;
	GameModeOptionsClass::Instance->HarvesterTruce = pRules->HarvesterTruce;
	GameModeOptionsClass::Instance->CrapEngineers = pRules->MultiEngineer;
	GameModeOptionsClass::Instance->AlliesAllowed = pRules->AlliesAllowed;
	GameModeOptionsClass::Instance->ShortGame = pRules->ShortGame;
	GameModeOptionsClass::Instance->SWAllowed = pRules->SuperWeaponsAllowed;
	GameModeOptionsClass::Instance->BuildOffAlly = pRules->BuildOffAlly;
	GameModeOptionsClass::Instance->FogOfWar = pRules->FogOfWar;
	GameModeOptionsClass::Instance->MCVRedeploys = pRules->MCVRedeploys;

	// --- Phase 7: LANGRULE.INI optional overlay --------------------------

	{
		CCFileClass langFile("LANGRULE.INI");
		if (CCFileClass::Is_Available(&langFile, 0))
		{
			CCINIClass langINI;
			// Original: Load returns digest-count; >1 means the file is invalid
			// (has extra sections) and we abort entirely.
			int digestSections = CCINIClass::Load(&langINI, &langFile, 1, 0);
			if (digestSections > 1)
			{
				// ORIG: cleans up artFile, frees rules vector, returns 0
				return ClearAndFail();
			}
			RulesClass::Process(Rule, &langINI);
			// langINI destructs here via RAII
		}
	}

	// --- Phase 8: destroy non-selected INIs ------------------------------

	for (int i = 0; i < count; ++i)
	{
		CCINIClass* ini = rules.Vector[i];
		if (ini != RuleINI && ini)
			DestroyINI(ini);
	}

	// --- Phase 9: load AIMD.INI ------------------------------------------

	{
		CCFileClass aiFile("AIMD.INI");
		CCINIClass::Load(&AIINI, &aiFile, 1, 0);
		// aiFile destructs here via RAII
	}

	// rules vector destructs here; Vector memory freed by DynamicVectorClass dtor
	return true;
}

#endif
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <CCFileClass.h>

/*
*	Otamaa : 
*	- Added log for AIMD loading error
*   - Remove most part that community never use or not use
*/
#include <Phobos.INI.h>
#include <MixFileClass.h>

bool __fastcall Init_Rules()
{
	//Rules
	{
		const char* _realName = GameStrings::RULESMD_INI();
		const auto crc = SafeChecksummer()(_realName, strlen(_realName));
		CCFileClass rulesFile(GameStrings::RULESMD_INI());
		CCINIClass::INI_Rules = GameCreate<CCINIClass>();

		PhobosINIContainer::Rules_INI =  std::make_unique<PhobosINIClass>();

		if (!CCINIClass::INI_Rules->ReadCCFile(&rulesFile) || !PhobosINIContainer::Rules_INI->LoadFile(&rulesFile)) {
			Debug::Log("Failed to load CRC %x [%s - %s]!\n", crc, _realName , rulesFile.Filename);
			return false;
		} else {
			Debug::Log("Succes to load CRC %x [%s - %s]!\n", crc, _realName,  rulesFile.Filename);
		}
	}

	//Art
	{
		const char* _realName = GameStrings::ARTMD_INI();
		const auto crc = SafeChecksummer()(_realName, strlen(_realName));
		CCFileClass artFile(GameStrings::ARTMD_INI());
		PhobosINIContainer::Art_INI = std::make_unique<PhobosINIClass>();

		if (!CCINIClass::INI_Art->ReadCCFile(&artFile, true) || !PhobosINIContainer::Art_INI->LoadFile(&artFile)) {
			Debug::Log("Failed to load CRC %x [%s - %s]!\n", crc, _realName, artFile.Filename);
			return false;
		} else {
			Debug::Log("Succes to load CRC %x [%s - %s]!\n", crc, _realName, artFile.Filename);
		}
	}

	//Pre-Init GameMode
	{
		FakeRulesClass* pRules = (FakeRulesClass*)RulesClass::Instance();

		pRules->_ReadColors(CCINIClass::INI_Rules());
		pRules->_ReadColorAdd(CCINIClass::INI_Rules());
		pRules->_ReadMovies(CCINIClass::INI_Rules());
		pRules->_ReadAudioVisual(CCINIClass::INI_Rules());
		pRules->_ReadMPlayer(CCINIClass::INI_Rules());

		GameModeOptionsClass::Instance->Money = pRules->Money;
		GameModeOptionsClass::Instance->UnitCount = pRules->UnitCount;
		Game::TechLevel = pRules->TechLevel;
		GameModeOptionsClass::Instance->GameSpeed = pRules->GameSpeed;
		GameModeOptionsClass::Instance->AIDifficulty = pRules->AIDifficulty;
		GameModeOptionsClass::Instance->AIPlayers = pRules->AIPlayers;
		GameModeOptionsClass::Instance->BridgeDestruction = pRules->BridgeDestruction;
		GameModeOptionsClass::Instance->Bases = pRules->Bases;
		GameModeOptionsClass::Instance->Crates = pRules->Crates;
		GameModeOptionsClass::Instance->CTF = pRules->CaptureTheFlag;
		GameModeOptionsClass::Instance->HarvesterTruce = pRules->HarvesterTruce;
		GameModeOptionsClass::Instance->MultiEngineer = pRules->MultiEngineer;
		GameModeOptionsClass::Instance->AlliesAllowed = pRules->AlliesAllowed;
		GameModeOptionsClass::Instance->ShortGame = pRules->ShortGame;
		GameModeOptionsClass::Instance->SWAllowed = pRules->SuperWeaponsAllowed;
		GameModeOptionsClass::Instance->BuildOffAlly = pRules->BuildOffAlly;
		GameModeOptionsClass::Instance->FogOfWar = pRules->FogOfWar;
		GameModeOptionsClass::Instance->MCVRedeploy = pRules->MCVRedeploys;

		//ASMJIT_PATCH(0x52D21F, Game_InitRules, 0x6)
		Phobos::Config::Read_RULESMD();
	}

	//AI
	{

		const char* _realName = GameStrings::AIMD_INI();
		const auto crc = SafeChecksummer()(_realName, strlen(_realName));
		CCFileClass aiFile(GameStrings::AIMD_INI());
		PhobosINIContainer::Ai_INI = std::make_unique<PhobosINIClass>();

		if(!CCINIClass::INI_AI->ReadCCFile(&aiFile, false) || !PhobosINIContainer::Ai_INI->LoadFile(&aiFile)) {
			Debug::Log("Failed to load CRC %x [%s - %s]!\n", crc, _realName , aiFile.Filename);
		} else {
			Debug::Log("Succes to load CRC %x [%s - %s]!\n", crc, _realName ,aiFile.Filename);
		}
	}

	return true;
}

 DEFINE_FUNCTION_JUMP(LJMP , 0x52CD70, Init_Rules)
 DEFINE_FUNCTION_JUMP(CALL, 0x52C95C, Init_Rules)

DEFINE_JUMP(LJMP, 0x687683 , 0x687694)
DEFINE_JUMP(LJMP, 0x535331 , 0x535342)

//stupid crash
ASMJIT_PATCH(0x68796B, _ReadScenario_ConfigureAdvancedCommandBar, 0x6)
{
	const bool isSinglePlayer = SessionClass::Instance->IsSingleplayer();
	RulesClass::Instance->Read_AdvancedCommandBar(CCINIClass::INI_UIMD.operator->(), isSinglePlayer ? 0 : 1);
	RulesClass::Instance->Read_AdvancedCommandBar(CCINIClass::INI_Rules.operator->(), isSinglePlayer ? 0 : 1);
	return 0x687975;
}