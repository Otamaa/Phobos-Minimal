#include "Body.h"
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

// =============================
// load / save

//template <typename T>
//void ScriptTypeExt::ExtData::Serialize(T& Stm)
//{
//	Stm
//		.Process(this->Initialized)
//		.Process(this->PhobosNode)
//		;
//}

// =============================
// container
ScriptTypeExtContainer ScriptTypeExtContainer::Instance;

// =============================
// container hooks
//
ASMJIT_PATCH_AGAIN(0x691D05, ScriptTypeClass_CTOR, 0x6)
ASMJIT_PATCH_AGAIN(0x691ACC, ScriptTypeClass_CTOR, 0x5)
ASMJIT_PATCH(0x691769, ScriptTypeClass_CTOR, 0x6)
{
	GET(ScriptTypeClass*, pThis, ESI);
	if (!Phobos::Otamaa::DoingLoadGame)
		ScriptTypeExtContainer::Instance.Allocate(pThis);

	return 0;
}

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