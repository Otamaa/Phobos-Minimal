#include "Body.h"
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>


void ScriptTypeExtData::CaptureOriginal()
{
	auto const pType = this->This();

	// Only capture if not already captured (lazy: OriginalActionsCount == 0)
	if (this->OriginalActionsCount > 0 || pType->ActionsCount <= 0)
	{
		return;
	}

	this->OriginalActionsCount = pType->ActionsCount;

	for (int i = 0; i < this->OriginalActionsCount && i < 50; ++i)
	{
		this->OriginalActions[i] = pType->ScriptActions[i];
	}
}

void ScriptTypeExtData::RestoreOriginal()
{
	auto const pType = this->This();
	if (!this->IsModified)
	{
		return;
	}

	pType->ActionsCount = this->OriginalActionsCount;

	for (int i = 0; i < 50; ++i)
	{
		pType->ScriptActions[i] = this->OriginalActions[i];
	}

	this->IsModified = false;
}

// =============================
// load / save

template <typename T>
void ScriptTypeExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->OriginalActions)
		.Process(this->OriginalActionsCount)
		.Process(this->IsModified)
		;
}

// =============================
// container
ScriptTypeExtContainer ScriptTypeExtContainer::Instance;

// =============================
// container hooks
//

ASMJIT_PATCH(0x691769, ScriptTypeClass_CTOR, 0x6)
{
	GET(ScriptTypeClass*, pThis, ESI);
	if (!Phobos::Otamaa::DoingLoadGame)
		ScriptTypeExtContainer::Instance.Allocate(pThis);

	return 0;
}ASMJIT_PATCH_AGAIN(0x691D05, ScriptTypeClass_CTOR, 0x6)
ASMJIT_PATCH_AGAIN(0x691ACC, ScriptTypeClass_CTOR, 0x5)

ASMJIT_PATCH(0x691796, ScriptTypeClass_DTOR, 0x6)
{
	GET(ScriptTypeClass*, pThis, ESI);

	ScriptTypeExtContainer::Instance.Remove(pThis);

	return 0x0;
}

ASMJIT_PATCH(0x691C62, ScriptTypeClass_CreateFromName_RemoveInline, 0x5)
{
	GET(char*, pName, EDI);
	R->ESI(GameCreate<ScriptTypeClass>(pName));
	return 0x691D2C;
}

HRESULT __stdcall FakeScriptTypeClass::__Load(IStream* pStm)
{
	HRESULT hr = this->ScriptTypeClass::Load(pStm);

	if (SUCCEEDED(hr)) {
		if (!ScriptTypeExtContainer::Instance.LoadByKey(this, pStm))
			return PHOBOS_E_EXTDATA_LOAD_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F101C, FakeScriptTypeClass::__Load)

HRESULT __stdcall FakeScriptTypeClass::__Save(IStream* pStm, BOOL fClearDirty)
{
	HRESULT hr = this->ScriptTypeClass::Save(pStm, fClearDirty);

	if (SUCCEEDED(hr)) {
		if (!ScriptTypeExtContainer::Instance.SaveByKey(this, pStm))
			return PHOBOS_E_EXTDATA_SAVE_FAILED;
	}

	return hr;
}
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F1020, FakeScriptTypeClass::__Save)

#include <Utilities/Helpers.h>

ASMJIT_PATCH(0x723CA0, ScriptActionNode_Read_Addition, 0x5)
{
	enum { SkipGameCode = 0x723CD3 };

	GET_STACK(char*, pBuffer, 0x4);

	if (!pBuffer) {
		R->EAX(pBuffer);
		return SkipGameCode;
	}

	GET(ScriptActionNode*, pThis, ECX);
	char arg[0x20];
	sscanf(pBuffer, "%d,%s", &pThis->Action, arg);
	R->EAX(pThis->Action);
 
	if (Helpers::Alex::is_any_of(pThis->Action,
		TeamMissionType::Attack_enemy_building,
		TeamMissionType::Moveto_enemy_building, 
		TeamMissionType::Chrono_prep_for_abwp, 
		TeamMissionType::Move_to_own_building)) {

		char* scanMode = nullptr;
		char* targetName = strtok_s(arg, ",", &scanMode);
		const int idx = BuildingTypeClass::FindIndexById(targetName);

		if (idx != -1) {
			int offset = 0x20000;

			if (scanMode) {
				if (!_stricmp(scanMode, "low"))
					offset = 0;
				else if (!_stricmp(scanMode, "hight"))
					offset = 0x10000;
				else if (!_stricmp(scanMode, "far"))
					offset = 0x30000;
			}

			pThis->Argument = idx + offset;
			return SkipGameCode;
		}
	}

	sscanf(arg, "%d", &pThis->Argument);
	return SkipGameCode;
}