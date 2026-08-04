#include "Body.h"
#include <CCINIClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

#include <Ext/TAction/Body.h>
#include <Ext/TeamType/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/ScriptType/Body.h>

template <typename T>
void TaskForceExtData::Serialize(T& Stm)
{

	Stm
		.Process(this->OriginalEntries)
		.Process(this->OriginalCountEntries)
		.Process(this->IsModified)
		;
}

void TaskForceExtData::CaptureOriginal()
{
	auto const pType = this->This();

	if (this->OriginalCountEntries > 0)
	{
		Debug::Log("[Phobos] CaptureOriginal: TaskForce [%s] already captured, skip\n",
			pType ? pType->ID : "null");
		return;
	}

	if (pType->CountEntries <= 0)
	{
		Debug::Log("[Phobos] CaptureOriginal: TaskForce [%s] has no entries, skip\n",
			pType ? pType->ID : "null");
		return;
	}

	this->OriginalCountEntries = pType->CountEntries;

	for (int i = 0; i < this->OriginalCountEntries && i < 6; ++i)
	{
		this->OriginalEntries[i] = pType->Entries[i];
	}

	Debug::Log("[Phobos] CaptureOriginal: TaskForce [%s] captured %d entries\n",
		pType->ID, this->OriginalCountEntries);
}

void TaskForceExtData::RestoreOriginal()
{
	auto const pType = this->This();
	if (!this->IsModified)
		return;

	Debug::Log("[Phobos] RestoreOriginal: TaskForce [%s] restore %d entries\n",
		pType->ID, this->OriginalCountEntries);

	pType->CountEntries = this->OriginalCountEntries;

	for (int i = 0; i < 6; ++i)
	{
		pType->Entries[i] = this->OriginalEntries[i];
	}

	this->IsModified = false;

	// Refresh all teams using this TaskForce to reflect restored state
	TaskForceManipulator::RefreshTeamsUsingTaskForce(pType);
}

// ============================================================================
// Helper: lookup helpers - INI param → "0"+num → Find
// ============================================================================
static std::string MakeID(int param) { return "0" + std::to_string(param); }
static ScriptTypeClass* FindScript(int param) { return ScriptTypeClass::Find(MakeID(param).c_str()); }
static ScriptTypeClass* FindScript(const char* text) { return text && text[0] ? ScriptTypeClass::Find(text) : nullptr; }
static TeamTypeClass* FindTeam(int param) { return TeamTypeClass::Find(MakeID(param).c_str()); }

// ============================================================================
// Helper: read a variable via Interop API with Direct fallback
// ============================================================================
static int ReadVar(bool bGlobal, int index)
{
	int maxIndex = bGlobal ? 50 : 100;

	if (index < 0 || index >= maxIndex)
		return 0;

	auto it = ScenarioExtData::GetVariables(bGlobal)->get_key_iterator(index);

	if (it != ScenarioExtData::GetVariables(bGlobal)->end())
		return it->second.Value;

	return 0;
}

// ============================================================================
// Capture original ScriptType actions and TeamType Script bindings from INI.
// Called eagerly during scenario loading to avoid picking up modifications
// made by other DLLs (which would happen with lazy capture).
// ============================================================================
void ScriptManipulator::CaptureFromINI(CCINIClass* pINI)
{
	if (!pINI)
		return;

	// --- Read [ScriptTypes] to capture original actions for each ScriptType ---
	int scriptCount = pINI->GetKeyCount("ScriptTypes");
	Debug::Log("[Phobos] CaptureFromINI: [ScriptTypes] has %d entries\n", scriptCount);

	for (int i = 0; i < scriptCount; ++i)
	{
		const char* keyName = pINI->GetKeyName("ScriptTypes", i);
		char scriptID[256];
		if (pINI->ReadString("ScriptTypes", keyName, "", scriptID, sizeof(scriptID)) <= 0)
			continue;

		auto const pScript = ScriptTypeClass::Find(scriptID);
		if (!pScript)
			continue;

		auto const pExt = ScriptTypeExtContainer::Instance.Find(pScript);
		int const nActions = pINI->GetKeyCount(scriptID);
		pExt->OriginalActionsCount = (nActions > 50) ? 50 : nActions;

		for (int j = 0; j < pExt->OriginalActionsCount; ++j)
		{
			char actBuf[256];
			if (pINI->ReadString(scriptID, std::to_string(j).c_str(), "", actBuf, sizeof(actBuf)) <= 0)
				continue;

			char* comma = strchr(actBuf, ',');
			if (comma)
			{
				*comma = '\0';
				pExt->OriginalActions[j].Action = (TeamMissionType)std::atoi(actBuf);
				pExt->OriginalActions[j].Argument = std::atoi(comma + 1);
			}
		}

		Debug::Log("[Phobos] CaptureFromINI: Script [%s] captured %d actions\n",
			scriptID, pExt->OriginalActionsCount);
	}

	// --- Read [TeamTypes] to capture original ScriptType index for each TeamType ---
	int teamCount = pINI->GetKeyCount("TeamTypes");
	Debug::Log("[Phobos] CaptureFromINI: [TeamTypes] has %d entries\n", teamCount);

	for (int i = 0; i < teamCount; ++i)
	{
		const char* keyName = pINI->GetKeyName("TeamTypes", i);
		char teamID[256];
		if (pINI->ReadString("TeamTypes", keyName, "", teamID, sizeof(teamID)) <= 0)
			continue;

		auto const pTeamType = TeamTypeClass::Find(teamID);
		if (!pTeamType)
			continue;

		auto const pExt = TeamTypeExtContainer::Instance.Find(pTeamType);

		char scriptID[256];
		if (pINI->ReadString(teamID, "Script", "", scriptID, sizeof(scriptID)) > 0)
		{
			for (int j = 0; j < ScriptTypeClass::Array->Count; ++j)
			{
				if (_stricmp(ScriptTypeClass::Array->Items[j]->ID, scriptID) == 0)
				{
					pExt->OriginalScriptTypeIndex = j;
					Debug::Log("[Phobos] CaptureFromINI: TeamType [%s] -> Script [%s] index=%d\n",
						teamID, scriptID, j);
					break;
				}
			}
		}
	}

	Debug::Log("[Phobos] CaptureFromINI: complete\n");
}

// ============================================================================
// Helper: backup script content before modification.
// If already captured from INI at load time, this is a no-op.
// Returns ExtData for IsModified flag.
// ============================================================================
static ScriptTypeExtData* CaptureOriginalScriptContent(ScriptTypeClass* pScript)
{
	if (!pScript)
		return nullptr;

	auto const pExt = ScriptTypeExtContainer::Instance.Find(pScript);

	// Already captured from INI at load time (OriginalActionsCount > 0) or capture now
	if (pExt->OriginalActionsCount <= 0)
		pExt->CaptureOriginal();

	return pExt;
}

// ============================================================================
// Reset all TeamClass instances that use a given ScriptType
// ============================================================================
void ScriptManipulator::ResetTeamsUsingScript(ScriptTypeClass* pScript)
{
	if (!pScript)
		return;

	int nReset = 0;

	for (int i = 0; i < TeamClass::Array->Count; ++i)
	{
		auto const pTeam = TeamClass::Array->Items[i];
		if (!pTeam || !pTeam->CurrentScript)
			continue;

		if (pTeam->CurrentScript->Type != pScript)
			continue;

		++nReset;
		Debug::Log("[Phobos] ResetTeamsUsingScript: Team #%d [%s] Script.CurrentMission=%d->0\n",
			i, pTeam->Type->ID, pTeam->CurrentScript->CurrentMission);

		// Set to -1 so NextMission() increments to 0 on next tick (action 0 will execute)
		pTeam->CurrentScript->CurrentMission = -1;
		pTeam->StepCompleted = true;
	}

	Debug::Log("[Phobos] ResetTeamsUsingScript: Script [%s] reset %d teams\n",
		pScript->ID, nReset);
}

// ============================================================================
// 650: Clear script content
// ============================================================================
void ScriptManipulator::ClearScript(TActionClass* pThis)
{
	ScriptTypeClass* const pScript = FindScript(pThis->Param3);
	if (!pScript)
	{
		Debug::Log("[Phobos] ClearScript: Param3=%d -> ScriptType not found!\n", pThis->Param3);
		return;
	}

	auto const pExt = CaptureOriginalScriptContent(pScript);

	Debug::Log("[Phobos] ClearScript: Script [%s] Param3=%d ActionsCount=%d IsModified=%d\n",
		pScript->ID, pThis->Param3, pScript->ActionsCount, pExt->IsModified);
	pScript->ActionsCount = 0;
	for (int i = 0; i < 50; ++i)
		pScript->ScriptActions[i] = { 0, 0 };
	pExt->IsModified = true;

	Debug::Log("[Phobos] ClearScript: Script [%s] cleared, ActionsCount=0 IsModified=1\n",
		pScript->ID);

	ResetTeamsUsingScript(pScript);
}

// ============================================================================
// 651: Copy script from source to destination
// ============================================================================
void ScriptManipulator::CopyScript(TActionClass* pThis)
{
	auto const pSrc = FindScript(pThis->Param3);
	auto const pDst = FindScript(pThis->Param4);

	if (!pSrc || !pDst)
		return;

	auto const pDstExt = CaptureOriginalScriptContent(pDst);

	int count = pSrc->ActionsCount;
	if (count > 50)
		count = 50;

	Debug::Log("[Phobos] CopyScript: Src=[%s](%d actions) Dst=[%s] Param3=%d Param4=%d\n",
		pSrc->ID, pSrc->ActionsCount, pDst->ID, pThis->Param3, pThis->Param4);

	pDst->ActionsCount = count;
	for (int i = 0; i < count; ++i)
	{
		pDst->ScriptActions[i] = pSrc->ScriptActions[i];
	}
	// Clear remaining slots to prevent stale data leaks
	for (int i = count; i < 50; ++i)
	{
		pDst->ScriptActions[i] = { 0, 0 };
	}

	pDstExt->IsModified = true;
	ResetTeamsUsingScript(pDst);
}

// ============================================================================
// 652: Modify script by direct parameters
// ============================================================================
void ScriptManipulator::ModifyScriptByParam(TActionClass* pThis)
{
	auto const pScript = FindScript(pThis->Text);
	Debug::Log("[Phobos] ModifyScriptByParam: Text=[%s] Param3=%d Param4=%d Param5=%d Param6=%d\n",
		pThis->Text, pThis->Param3, pThis->Param4, pThis->Param5, pThis->Param6);
	if (!pScript)
		return;

	int lineNum = pThis->Param3;
	int actionType = pThis->Param4;
	int param1 = pThis->Param5;
	int param2 = pThis->Param6;

	if (lineNum < 0 || lineNum >= 50)
		return;

	auto const pExt = CaptureOriginalScriptContent(pScript);

	int encodedArg = (param2 << 16) | (param1 & 0xFFFF);

	pScript->ScriptActions[lineNum] = { actionType, encodedArg };

	if (lineNum >= pScript->ActionsCount)
		pScript->ActionsCount = lineNum + 1;

	pExt->IsModified = true;
	ResetTeamsUsingScript(pScript);
}

// ============================================================================
// 653: Modify script using local variables from ScenarioClass
//   Text=ScriptID  Param3=lineNum  Param4=varAction  Param5=varParam1  Param6=varParam2
// ============================================================================
void ScriptManipulator::ModifyScriptByLocalVar(TActionClass* pThis)
{
	auto const pScript = FindScript(pThis->Text);
	Debug::Log("[Phobos] ModifyScriptByLocalVar: Text=[%s] Param3=%d Param4=%d Param5=%d Param6=%d\n",
		pThis->Text, pThis->Param3, pThis->Param4, pThis->Param5, pThis->Param6);
	if (!pScript)
		return;

	int lineNum = pThis->Param3;
	if (lineNum < 0 || lineNum >= 50)
		return;

	auto const pExt = CaptureOriginalScriptContent(pScript);

	int actionType = ReadVar(false, pThis->Param4);
	int param1 = ReadVar(false, pThis->Param5);
	int param2Val = ReadVar(false, pThis->Param6);

	int encodedArg = (param2Val << 16) | (param1 & 0xFFFF);

	pScript->ScriptActions[lineNum] = { actionType, encodedArg };

	if (lineNum >= pScript->ActionsCount)
		pScript->ActionsCount = lineNum + 1;

	pExt->IsModified = true;
	ResetTeamsUsingScript(pScript);
}

// ============================================================================
// 654: Modify script using global variables from ScenarioClass
//   Text=ScriptID  Param3=lineNum  Param4=varAction  Param5=varParam1  Param6=varParam2
// ============================================================================
void ScriptManipulator::ModifyScriptByGlobalVar(TActionClass* pThis)
{
	auto const pScript = FindScript(pThis->Text);
	Debug::Log("[Phobos] ModifyScriptByGlobalVar: Text=[%s] Param3=%d Param4=%d Param5=%d Param6=%d\n",
		pThis->Text, pThis->Param3, pThis->Param4, pThis->Param5, pThis->Param6);
	if (!pScript)
		return;

	int lineNum = pThis->Param3;
	if (lineNum < 0 || lineNum >= 50)
		return;

	auto const pExt = CaptureOriginalScriptContent(pScript);

	int actionType = ReadVar(true, pThis->Param4);
	int param1 = ReadVar(true, pThis->Param5);
	int param2Val = ReadVar(true, pThis->Param6);

	int encodedArg = (param2Val << 16) | (param1 & 0xFFFF);

	pScript->ScriptActions[lineNum] = { actionType, encodedArg };

	if (lineNum >= pScript->ActionsCount)
		pScript->ActionsCount = lineNum + 1;

	pExt->IsModified = true;
	ResetTeamsUsingScript(pScript);
}

// ============================================================================
// Helper: lazily capture the original ScriptType index for a TeamType.
// If already captured from INI at load time (OriginalScriptTypeIndex >= 0),
// this is a no-op. Otherwise captures from current in-memory state.
// ============================================================================
void ScriptManipulator::CaptureOriginalScriptIndex(TeamTypeExtData* pExt, TeamTypeClass* pTeamType)
{
	// Already captured from INI at load time
	if (pExt->OriginalScriptTypeIndex >= 0)
		return;

	if (!pTeamType->ScriptType)
		return;

	for (int j = 0; j < ScriptTypeClass::Array->Count; ++j)
	{
		if (ScriptTypeClass::Array->Items[j] == pTeamType->ScriptType)
		{
			pExt->OriginalScriptTypeIndex = j;
			break;
		}
	}
}

// ============================================================================
// Helper: reset all TeamClass instances of a given TeamType to re-run
// their script from action 0, optionally rebinding to a different ScriptType.
// ============================================================================
static void ResetTeamsOfType(TeamTypeClass* pTeamType, ScriptTypeClass* pBindTo = nullptr)
{
	for (int i = 0; i < TeamClass::Array->Count; ++i)
	{
		auto const pTeam = TeamClass::Array->Items[i];
		if (!pTeam || pTeam->Type != pTeamType)
			continue;

		if (pTeam->CurrentScript)
		{
			if (pBindTo)
				pTeam->CurrentScript->Type = pBindTo;
			pTeam->CurrentScript->CurrentMission = -1;
		}

		pTeam->StepCompleted = true;
	}
}

// ============================================================================
// 655: Rebind TeamType to a different ScriptType
// ============================================================================
void ScriptManipulator::RebindTeamTypeScript(TActionClass* pThis)
{
	auto const pTeamType = FindTeam(pThis->Param3);
	auto const pNewScript = FindScript(pThis->Param4);

	if (!pTeamType || !pNewScript)
		return;

	auto const pExt = TeamTypeExtContainer::Instance.Find(pTeamType);
	CaptureOriginalScriptIndex(pExt, pTeamType);

	Debug::Log("[Phobos] RebindTeamTypeScript: TeamType Param3=%d NewScript Param4=%d\n",
		pThis->Param3, pThis->Param4);

	pTeamType->ScriptType = pNewScript;
	ResetTeamsOfType(pTeamType, pNewScript);
}

// ============================================================================
// 656: Reset TeamType script binding to original
// ============================================================================
void ScriptManipulator::ResetTeamTypeScript(TActionClass* pThis)
{
	auto const pTeamType = FindTeam(pThis->Param3);
	if (!pTeamType)
		return;

	Debug::Log("[Phobos] ResetTeamTypeScript: Param3=%d\n", pThis->Param3);

	auto const pExt = TeamTypeExtContainer::Instance.Find(pTeamType);
	CaptureOriginalScriptIndex(pExt, pTeamType);

	if (pExt->OriginalScriptTypeIndex < 0)
		return;

	auto const pOriginalScript = ScriptTypeClass::Array->Items[pExt->OriginalScriptTypeIndex];
	pTeamType->ScriptType = pOriginalScript;
	ResetTeamsOfType(pTeamType, pOriginalScript);
}

// ============================================================================
// 657: Reset ALL TeamType script bindings to original
// ============================================================================
void ScriptManipulator::ResetAllTeamTypeScripts()
{
	Debug::Log("[Phobos] ResetAllTeamTypeScripts\n");

	for (int i = 0; i < TeamTypeClass::Array->Count; ++i)
	{
		auto const pTeamType = TeamTypeClass::Array->Items[i];
		if (!pTeamType)
			continue;

		auto const pExt = TeamTypeExtContainer::Instance.Find(pTeamType);
		CaptureOriginalScriptIndex(pExt, pTeamType);

		if (pExt->OriginalScriptTypeIndex < 0)
			continue;

		pTeamType->ScriptType = ScriptTypeClass::Array->Items[pExt->OriginalScriptTypeIndex];
	}

	// Reset all teams: rebind to their (now restored) TeamType's ScriptType
	for (int i = 0; i < TeamClass::Array->Count; ++i)
	{
		auto const pTeam = TeamClass::Array->Items[i];
		if (!pTeam)
			continue;

		if (pTeam->CurrentScript)
		{
			pTeam->CurrentScript->Type = pTeam->Type->ScriptType;
			pTeam->CurrentScript->CurrentMission = -1;
		}

		pTeam->StepCompleted = true;
	}
}

// ============================================================================
// Helper: restore script content from backup, if it was modified.
// Returns true if restoration actually happened.
// ============================================================================
static bool RestoreOriginalScriptContent(ScriptTypeClass* pScript)
{
	if (!pScript)
		return false;

	auto const pExt = ScriptTypeExtContainer::Instance.Find(pScript);
	if (!pExt)
		return false;

	pExt->RestoreOriginal();
	return true;
}

// ============================================================================
// 658: Restore a single script content to its original (INI-defined) state
// ============================================================================
void ScriptManipulator::RestoreScriptContent(TActionClass* pThis)
{
	auto const pScript = FindScript(pThis->Param3);
	Debug::Log("[Phobos] RestoreScriptContent: Param3=%d\n", pThis->Param3);

	if (RestoreOriginalScriptContent(pScript))
		ResetTeamsUsingScript(pScript);
}

// ============================================================================
// 659: Restore ALL modified script contents to original state
// ============================================================================
void ScriptManipulator::RestoreAllScriptContents()
{
	Debug::Log("[Phobos] RestoreAllScriptContents\n");

	for (int i = 0; i < ScriptTypeClass::Array->Count; ++i)
	{
		auto const pScript = ScriptTypeClass::Array->Items[i];
		if (!pScript)
			continue;

		if (RestoreOriginalScriptContent(pScript))
			ResetTeamsUsingScript(pScript);
	}
}

// ============================================================================
// 660: Seek/jump script execution line for all instances of a TeamType
//   Param3=TeamType index  Param4=target line number (0-based, 0=first action)
// ============================================================================
void ScriptManipulator::SeekTeamTypeScript(TActionClass* pThis)
{
	auto const pTeamType = FindTeam(pThis->Param3);
	if (!pTeamType)
		return;

	int const targetLine = pThis->Param4;
	int const seekTo = (targetLine <= 0) ? -1 : (targetLine - 1);

	Debug::Log("[Phobos] SeekTeamTypeScript: TeamType [%s] targetLine=%d seekTo=%d\n",
		pTeamType->ID, targetLine, seekTo);

	for (int i = 0; i < TeamClass::Array->Count; ++i)
	{
		auto const pTeam = TeamClass::Array->Items[i];
		if (!pTeam || pTeam->Type != pTeamType)
			continue;

		if (pTeam->CurrentScript)
			pTeam->CurrentScript->CurrentMission = seekTo;

		pTeam->StepCompleted = true;
	}
}

// =============================
// container
TaskForceExtContainer TaskForceExtContainer::Instance;
// =============================

ASMJIT_PATCH(0x6E7F3A, TaskForceClass_CTOR, 0x5)
{
	GET(TaskForceClass*, pItem, ESI);

	if (!Phobos::Otamaa::DoingLoadGame)
		TaskForceExtContainer::Instance.Allocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x6E7F80, TaskForceClass_SDDTOR, 0x6)
{
	GET(TaskForceClass*, pItem, ECX);
	TaskForceExtContainer::Instance.Remove(pItem);
	return 0;
}ASMJIT_PATCH_AGAIN(0x6E87F0, TaskForceClass_SDDTOR, 0x6)

bool __fastcall FakTaskForceClass::_LoadEntryINI(CCINIClass* pINI, TaskForceType type)
{
	auto String_To_ID = [](const char* str)
		{
			int result = 0;

			if (!str)
			{
				return result;
			}

			while (*str)
			{
				const char ch = *str;

				if (!std::isxdigit(static_cast<unsigned char>(ch)))
				{
					break;
				}

				result *= 16;

				if (ch >= '0' && ch <= '9')
				{
					result += ch - '0';
				}
				else
				{
					result += std::toupper(static_cast<unsigned char>(ch)) - 'A' + 10;
				}

				++str;
			}

			return result;
		};

	const char* const section = "TaskForces";

	const int count = pINI->GetKeyCount(section);

	if (count <= 0)
		return false;

	for (int index = 0; index < count; ++index) {

		const char* entryKey = pINI->GetKeyName(section, index);
		std::string value(24, '\0');
		const int len = pINI->ReadString(section, entryKey, "", value.data(), 24);

		if (len <= 0)
			continue;

		value.resize(static_cast<size_t>(len));

		TaskForceClass* pTag = TaskForceClass::FindOrAllocate(value.c_str());
		size_t intID = String_To_ID(value.c_str());
		Debug::Log("TaskForce[%s - %x] want to remap as [%x] \n", value.c_str(), pTag, intID);
		//PhobosSwizzle::Instance.Here_I_Am((void*)ID, pType);

		if (pTag) {
			pTag->LoadFromINI(pINI);
			pTag->Type = type;
		}
	}

	return true;
}

DEFINE_FUNCTION_JUMP(LJMP, 0x6E8220, FakTaskForceClass::_LoadEntryINI);
DEFINE_FUNCTION_JUMP(CALL, 0x6879B4, FakTaskForceClass::_LoadEntryINI);
DEFINE_FUNCTION_JUMP(CALL, 0x6879BD, FakTaskForceClass::_LoadEntryINI);

bool FakTaskForceClass::_LoadFromINI(CCINIClass* pINI)
{
	pINI->Reset();

	if (!this->AbstractTypeClass::LoadFromINI(pINI))
		return false;

	this->CountEntries = 0;
	for (int i = 0; i < 6; ++i) {
		char buffer[128];

		if (pINI->ReadString(this->ID, std::to_string(i).c_str(), "", buffer) > 0) {
			this->Entries[this->CountEntries].Read(buffer);
			int v4 = this->CountEntries;
			if (this->Entries[v4].Type) {
				this->CountEntries = v4 + 1;
			}
		}
	}

	this->Group = pINI->ReadInteger(this->ID, "Group", this->Group);
	return true;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F46E4, FakTaskForceClass::_LoadFromINI);
DEFINE_FUNCTION_JUMP(LJMP , 0x6E8420, FakTaskForceClass::_LoadFromINI);

bool FakTaskForceClass::_WriteToINI(CCINIClass* pINI)
{
	if (!this->AbstractTypeClass::SaveToINI(pINI)) {
		return 0;
	}

	for (int i = 0; i < this->CountEntries; ++i) {
		pINI->WriteString(this->ID, std::to_string(i).c_str(), this->Entries[i].Write());
	}

	pINI->Reset();
	pINI->WriteInteger(this->ID, "Group", this->Group, false);

	return true;
}
DEFINE_FUNCTION_JUMP(LJMP, 0x6E8510, FakTaskForceClass::_WriteToINI);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F46E8, FakTaskForceClass::_WriteToINI);

#pragma region TaskForceManipulator

TaskForceClass* TaskForceManipulator::FindTaskForce(int param)
{
	// Try 1: "0"+param as ID string (e.g. param=1000094 → "01000094")
	std::string withZero = "0" + std::to_string(param);
	auto pTF = TaskForceClass::Find(withZero.c_str());
	if (pTF) return pTF;

	// Try 2: raw number as ID string (e.g. param=135 → "135")
	pTF = TaskForceClass::Find(std::to_string(param).c_str());
	if (pTF) return pTF;

	// Try 3: array index fallback
	if (param >= 0 && param < TaskForceClass::Array->Count)
		return TaskForceClass::Array->Items[param];

	return nullptr;
}

TeamTypeClass* TaskForceManipulator::FindTeamType(int param)
{
	return TeamTypeClass::Find(("0" + std::to_string(param)).c_str());
}

void TaskForceManipulator::CaptureOriginalContent(TaskForceClass* pTF)
{
	if (!pTF)
		return;

	TaskForceExtContainer::Instance.Find(pTF)->CaptureOriginal();
}

void TaskForceManipulator::CaptureFromINI(CCINIClass* pINI)
{
	if (!pINI)
		return;

	int tfCount = pINI->GetKeyCount("TaskForces");
	Debug::Log("[Phobos] TaskForce CaptureFromINI: [TaskForces] has %d entries\n", tfCount);

	for (int i = 0; i < tfCount; ++i)
	{
		const char* keyName = pINI->GetKeyName("TaskForces", i);
		char tfID[256];
		if (pINI->ReadString("TaskForces", keyName, "", tfID, sizeof(tfID)) <= 0)
			continue;

		auto const pTF = TaskForceClass::Find(tfID);
		if (!pTF)
			continue;

		auto const pExt = TaskForceExtContainer::Instance.Find(pTF);

		int const nEntries = pINI->GetKeyCount(tfID);
		pExt->OriginalCountEntries = (nEntries > 6) ? 6 : nEntries;

		for (int j = 0; j < pExt->OriginalCountEntries; ++j)
		{
			char entryBuf[256];
			if (pINI->ReadString(tfID, std::to_string(j).c_str(), "", entryBuf, sizeof(entryBuf)) <= 0)
				continue;

			// Format: "Amount,TechnoTypeID" e.g. "3,MTNK"
			char* comma = strchr(entryBuf, ',');
			if (comma)
			{
				*comma = '\0';
				pExt->OriginalEntries[j].Amount = std::atoi(entryBuf);
				pExt->OriginalEntries[j].Type = TechnoTypeClass::Find(comma + 1);
			}
		}

		Debug::Log("[Phobos] TaskForce CaptureFromINI: [%s] captured %d entries\n",
			tfID, pExt->OriginalCountEntries);
	}

	// --- Read [TeamTypes] to capture original TaskForce binding for each TeamType ---
	int teamCount = pINI->GetKeyCount("TeamTypes");
	Debug::Log("[Phobos] TaskForce CaptureFromINI: [TeamTypes] has %d entries\n", teamCount);

	for (int i = 0; i < teamCount; ++i)
	{
		const char* keyName = pINI->GetKeyName("TeamTypes", i);
		char teamID[256];
		if (pINI->ReadString("TeamTypes", keyName, "", teamID, sizeof(teamID)) <= 0)
			continue;

		auto const pTeamType = TeamTypeClass::Find(teamID);
		if (!pTeamType)
			continue;

		auto const pExt = TeamTypeExtContainer::Instance.Find(pTeamType);

		char tfID[256];
		if (pINI->ReadString(teamID, "TaskForce", "", tfID, sizeof(tfID)) > 0)
		{
			for (int j = 0; j < TaskForceClass::Array->Count; ++j)
			{
				if (_stricmp(TaskForceClass::Array->Items[j]->ID, tfID) == 0)
				{
					pExt->OriginalTaskForceIndex = j;
					Debug::Log("[Phobos] TaskForce CaptureFromINI: TeamType [%s] -> TaskForce [%s] index=%d\n",
						teamID, tfID, j);
					break;
				}
			}
		}
	}

	Debug::Log("[Phobos] TaskForce CaptureFromINI: complete\n");
}

static void SyncTeamState(TeamClass* pTeam, TaskForceClass* pTF)
{
	if (!pTeam || !pTF)
		return;

	int newCountObjects[6] = { 0 };
	int totalRequired = 0;
	int totalHave = 0;

	for (int k = 0; k < pTF->CountEntries && k < 6; ++k)
	{
		auto const pEntry = &pTF->Entries[k];
		if (!pEntry->Type || pEntry->Amount <= 0)
			continue;

		totalRequired += pEntry->Amount;

		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit && pUnit->GetTechnoType() == pEntry->Type)
				++newCountObjects[k];
		}

		totalHave += newCountObjects[k];
	}

	for (int k = 0; k < 6; ++k)
		pTeam->CountObjects[k] = newCountObjects[k];

	pTeam->IsFullStrength = (totalHave >= totalRequired);
	pTeam->IsUnderStrength = (totalHave < totalRequired);
}

void TaskForceManipulator::RefreshTeamsUsingTaskForce(TaskForceClass* pTF)
{
	if (!pTF)
		return;

	// Build lookup: TechnoType* → allowed amount
	std::map<TechnoTypeClass*, int> allowedCounts;
	for (int i = 0; i < pTF->CountEntries && i < 6; ++i)
	{
		if (pTF->Entries[i].Type && pTF->Entries[i].Amount > 0)
			allowedCounts[pTF->Entries[i].Type] += pTF->Entries[i].Amount;
	}

	int nTeamTypesUpdated = 0;
	int nTeamsPruned = 0;

	for (int i = 0; i < TeamTypeClass::Array->Count; ++i)
	{
		auto const pTeamType = TeamTypeClass::Array->Items[i];
		if (!pTeamType || pTeamType->TaskForce != pTF)
			continue;

		++nTeamTypesUpdated;
		pTeamType->ProcessTaskForce();

		for (int j = 0; j < TeamClass::Array->Count; ++j)
		{
			auto const pTeam = TeamClass::Array->Items[j];
			if (!pTeam || pTeam->Type != pTeamType)
				continue;

			// Count current members by type
			std::map<TechnoTypeClass*, int> currentCounts;
			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				if (pUnit)
					++currentCounts[pUnit->GetTechnoType()];
			}

			// Liberate excess members
			for (auto pUnit = pTeam->FirstUnit; pUnit; )
			{
				auto pNext = pUnit->NextTeamMember;
				auto const pType = pUnit->GetTechnoType();
				int allowed = allowedCounts[pType];
				int& current = currentCounts[pType];

				if (current > allowed)
				{
					--current;
					Debug::Log("[Phobos] RefreshTeamsUsingTF: Liberate [%s] from Team [%s] (exceed %d>%d)\n",
						pType->ID, pTeamType->ID, current + 1, allowed);
					pTeam->LiberateMember(pUnit);
					++nTeamsPruned;
				}

				pUnit = pNext;
			}

			SyncTeamState(pTeam, pTF);
		}
	}

	Debug::Log("[Phobos] RefreshTeamsUsingTF: [%s] updated %d TeamTypes, pruned %d units\n",
		pTF->ID, nTeamTypesUpdated, nTeamsPruned);
}

void TaskForceManipulator::CaptureOriginalTaskForceIndex(TeamTypeExtData* pExt, TeamTypeClass* pTeamType)
{
	if (pExt->OriginalTaskForceIndex >= 0)
		return;

	if (!pTeamType->TaskForce)
		return;

	for (int j = 0; j < TaskForceClass::Array->Count; ++j) {
		if (TaskForceClass::Array->Items[j] == pTeamType->TaskForce) {
			pExt->OriginalTaskForceIndex = j;
			break;
		}
	}
}

void TaskForceManipulator::RefreshTeamsOfType(TeamTypeClass* pTeamType)
{
	if (!pTeamType || !pTeamType->TaskForce)
		return;

	auto const pTF = pTeamType->TaskForce;

	// Build allowed counts from TaskForce entries
	std::map<TechnoTypeClass*, int> allowedCounts;
	for (int i = 0; i < pTF->CountEntries && i < 6; ++i)
	{
		if (pTF->Entries[i].Type && pTF->Entries[i].Amount > 0)
			allowedCounts[pTF->Entries[i].Type] += pTF->Entries[i].Amount;
	}

	for (int j = 0; j < TeamClass::Array->Count; ++j)
	{
		auto const pTeam = TeamClass::Array->Items[j];
		if (!pTeam || pTeam->Type != pTeamType)
			continue;

		// Count current members by type
		std::map<TechnoTypeClass*, int> currentCounts;
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit)
				++currentCounts[pUnit->GetTechnoType()];
		}

		// Liberate excess members
		for (auto pUnit = pTeam->FirstUnit; pUnit; )
		{
			auto pNext = pUnit->NextTeamMember;
			auto const pType = pUnit->GetTechnoType();
			int allowed = allowedCounts[pType];
			int& current = currentCounts[pType];

			if (current > allowed)
			{
				--current;
				Debug::Log("[Phobos] RefreshTeamsOfType: Liberate [%s] from Team [%s]\n",
					pType->ID, pTeamType->ID);
				pTeam->LiberateMember(pUnit);
			}

			pUnit = pNext;
		}

		SyncTeamState(pTeam, pTF);
	}
}

void TaskForceManipulator::ClearTaskForce(TActionClass* pThis)
{
	auto const pTF = FindTaskForce(pThis->Param3);
	if (!pTF)
	{
		Debug::Log("[Phobos] ClearTaskForce: Param3=%d -> TaskForce not found!\n", pThis->Param3);
		return;
	}

	CaptureOriginalContent(pTF);

	pTF->CountEntries = 0;
	for (int i = 0; i < 6; ++i)
		pTF->Entries[i] = { 0, nullptr };

	TaskForceExtContainer::Instance.Find(pTF)->IsModified = true;

	Debug::Log("[Phobos] ClearTaskForce: TaskForce [%s] cleared\n", pTF->ID);

	RefreshTeamsUsingTaskForce(pTF);
}

void TaskForceManipulator::CopyTaskForce(TActionClass* pThis)
{
	auto const pSrc = FindTaskForce(pThis->Param3);
	auto const pDst = FindTaskForce(pThis->Param4);

	if (!pSrc || !pDst)
	{
		Debug::Log("[Phobos] CopyTaskForce: Src=%d Dst=%d -> not found!\n",
			pThis->Param3, pThis->Param4);
		return;
	}

	CaptureOriginalContent(pDst);

	int count = pSrc->CountEntries;
	if (count > 6) count = 6;

	pDst->CountEntries = count;
	for (int i = 0; i < count; ++i)
		pDst->Entries[i] = pSrc->Entries[i];
	for (int i = count; i < 6; ++i)
		pDst->Entries[i] = { 0, nullptr };

	TaskForceExtContainer::Instance.Find(pDst)->IsModified = true;

	Debug::Log("[Phobos] CopyTaskForce: [%s](%d entries) -> [%s]\n",
		pSrc->ID, pSrc->CountEntries, pDst->ID);

	RefreshTeamsUsingTaskForce(pDst);
}

void TaskForceManipulator::ModifyTaskForceEntry(TActionClass* pThis)
{
	auto const pTF = FindTaskForce(pThis->Param3);
	if (!pTF)
		return;

	int const entryIdx = pThis->Param4;
	if (entryIdx < 0 || entryIdx >= 6)
		return;

	CaptureOriginalContent(pTF);

	int const amount = pThis->Param5;
	const char* technoID = pThis->Text;

	if (amount <= 0)
	{
		// Remove entry: shift remaining entries left
		for (int i = entryIdx; i < 5; ++i)
			pTF->Entries[i] = pTF->Entries[i + 1];
		pTF->Entries[5] = { 0, nullptr };

		// Recalculate CountEntries (last non-empty slot)
		int newCount = 0;
		for (int i = 0; i < 6; ++i)
		{
			if (pTF->Entries[i].Amount > 0 && pTF->Entries[i].Type)
				newCount = i + 1;
		}
		pTF->CountEntries = newCount;
	}
	else
	{
		pTF->Entries[entryIdx].Amount = amount;

		if (technoID && technoID[0])
		{
			auto const pType = TechnoTypeClass::Find(technoID);
			if (pType)
				pTF->Entries[entryIdx].Type = pType;
		}

		if (entryIdx >= pTF->CountEntries)
			pTF->CountEntries = entryIdx + 1;
	}

	TaskForceExtContainer::Instance.Find(pTF)->IsModified = true;

	Debug::Log("[Phobos] ModifyTaskForceEntry: [%s] entry[%d] amount=%d type=%s\n",
		pTF->ID, entryIdx, amount, technoID ? technoID : "(keep)");

	RefreshTeamsUsingTaskForce(pTF);
}

void TaskForceManipulator::RebindTeamTypeTaskForce(TActionClass* pThis)
{
	auto const pTeamType = FindTeamType(pThis->Param3);
	auto const pNewTF = FindTaskForce(pThis->Param4);

	if (!pTeamType || !pNewTF)
	{
		Debug::Log("[Phobos] RebindTeamTypeTaskForce: TeamType=%d TF=%d -> not found!\n",
			pThis->Param3, pThis->Param4);
		return;
	}

	CaptureOriginalTaskForceIndex(TeamTypeExtContainer::Instance.Find(pTeamType), pTeamType);

	Debug::Log("[Phobos] RebindTeamTypeTaskForce: [%s] -> TaskForce [%s]\n",
		pTeamType->ID, pNewTF->ID);

	pTeamType->TaskForce = pNewTF;
	pTeamType->ProcessTaskForce();
	RefreshTeamsOfType(pTeamType);
}

void TaskForceManipulator::ResetTeamTypeTaskForce(TActionClass* pThis)
{
	auto const pTeamType = FindTeamType(pThis->Param3);
	if (!pTeamType)
		return;

	Debug::Log("[Phobos] ResetTeamTypeTaskForce: Param3=%d\n", pThis->Param3);

	auto const pExt = TeamTypeExtContainer::Instance.Find(pTeamType);
	CaptureOriginalTaskForceIndex(pExt, pTeamType);

	if (pExt->OriginalTaskForceIndex < 0)
	{
		Debug::Log("[Phobos] ResetTeamTypeTaskForce: No original TaskForce index!\n");
		return;
	}

	auto const pOriginalTF = TaskForceClass::Array->Items[pExt->OriginalTaskForceIndex];
	pTeamType->TaskForce = pOriginalTF;
	pTeamType->ProcessTaskForce();
	RefreshTeamsOfType(pTeamType);

	Debug::Log("[Phobos] ResetTeamTypeTaskForce: [%s] -> TaskForce [%s]\n",
		pTeamType->ID, pOriginalTF->ID);
}

void TaskForceManipulator::ResetAllTeamTypeTaskForces()
{
	Debug::Log("[Phobos] ResetAllTeamTypeTaskForces\n");

	for (int i = 0; i < TeamTypeClass::Array->Count; ++i)
	{
		auto const pTeamType = TeamTypeClass::Array->Items[i];
		if (!pTeamType)
			continue;

		auto const pExt = TeamTypeExtContainer::Instance.Find(pTeamType);
		CaptureOriginalTaskForceIndex(pExt, pTeamType);

		if (pExt->OriginalTaskForceIndex < 0)
			continue;

		auto const pOriginalTF = TaskForceClass::Array->Items[pExt->OriginalTaskForceIndex];
		pTeamType->TaskForce = pOriginalTF;
		pTeamType->ProcessTaskForce();
	}

	// Refresh all teams in a second pass
	for (int i = 0; i < TeamTypeClass::Array->Count; ++i)
	{
		auto const pTeamType = TeamTypeClass::Array->Items[i];
		if (!pTeamType)
			continue;

		RefreshTeamsOfType(pTeamType);
	}

	Debug::Log("[Phobos] ResetAllTeamTypeTaskForces: complete\n");
}

void TaskForceManipulator::RestoreTaskForce(TActionClass* pThis)
{
	auto const pTF = FindTaskForce(pThis->Param3);
	if (!pTF)
		return;

	auto const pExt = TaskForceExtContainer::Instance.Find(pTF);
	if (pExt)
	{
		pExt->RestoreOriginal();
		Debug::Log("[Phobos] RestoreTaskForce: [%s] restored\n", pTF->ID);
	}
}

void TaskForceManipulator::RestoreAllTaskForces()
{
	Debug::Log("[Phobos] RestoreAllTaskForces\n");

	for (int i = 0; i < TaskForceClass::Array->Count; ++i)
	{
		auto const pTF = TaskForceClass::Array->Items[i];
		if (!pTF)
			continue;

		auto const pExt = TaskForceExtContainer::Instance.Find(pTF);
		if (pExt)
		{
			pExt->RestoreOriginal();
		}
	}
}

#pragma endregion TaskForceManipulator

HRESULT __stdcall FakTaskForceClass::__Load(IStream* pStm)
{
	HRESULT hr = this->TaskForceClass::Load(pStm);

	if (SUCCEEDED(hr))
	{
		if (!TaskForceExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4694, FakTaskForceClass::__Load)

HRESULT __stdcall FakTaskForceClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->TaskForceClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr))
	{
		if (!TaskForceExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4698, FakTaskForceClass::__Save)