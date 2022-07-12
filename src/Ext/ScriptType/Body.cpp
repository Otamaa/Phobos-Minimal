#include "Body.h"

template<> const DWORD Extension<ScriptTypeClass>::Canary = 0x414B4B41;
ScriptTypeExt::ExtContainer ScriptTypeExt::ExtMap;

void ScriptTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->OwnerObject()->ID;

	if (!pINI->GetSection(pID))
		return;

	INI_EX exINI(pINI);
}

// =============================
// load / save

template <typename T>
void ScriptTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(PhobosNode)
		;
}

void ScriptTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<ScriptTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void ScriptTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<ScriptTypeClass>::Serialize(Stm);
	this->Serialize(Stm);
}

void ScriptTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {}

bool ScriptTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool ScriptTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

ScriptTypeExt::ExtContainer::ExtContainer() : Container("ScriptTypeClass") { }
ScriptTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
DEFINE_HOOK_AGAIN(0x691D05, ScriptTypeClass_CTOR, 0x6)
DEFINE_HOOK_AGAIN(0x691ACC, ScriptTypeClass_CTOR, 0x5)
DEFINE_HOOK(0x691769, ScriptTypeClass_CTOR, 0x6)
{
	GET(ScriptTypeClass*, pThis, ESI);
	ScriptTypeExt::ExtMap.FindOrAllocate(pThis);
	return 0;
}

DEFINE_HOOK(0x691796, ScriptTypeClass_DTOR, 0xA)
{
	GET(ScriptTypeClass*, pThis, ESI);

	ScriptTypeExt::ExtMap.Remove(pThis);

	return 0x0;
}

DEFINE_HOOK_AGAIN(0x691D90, ScriptTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x691DE0, ScriptTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(ScriptTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	ScriptTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x691DD1, ScriptTypeClass_Load_Suffix, 0xA)
{
	ScriptTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x691DFA, ScriptTypeClass_Save_Suffix, 0x5)
{
	ScriptTypeExt::ExtMap.SaveStatic();
	return 0;
}