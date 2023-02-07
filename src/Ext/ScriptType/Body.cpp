#include "Body.h"

ScriptTypeExt::ExtContainer ScriptTypeExt::ExtMap;

void ScriptTypeExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{
	const char* pID = this->Get()->ID;

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
	TExtension<ScriptTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void ScriptTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TExtension<ScriptTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

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

ScriptTypeExt::ExtContainer::ExtContainer() : TExtensionContainer("ScriptTypeClass") { }
ScriptTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks
//
//DEFINE_HOOK_AGAIN(0x691D05, ScriptTypeClass_CTOR, 0x6)
//DEFINE_HOOK_AGAIN(0x691ACC, ScriptTypeClass_CTOR, 0x5)
//DEFINE_HOOK(0x691769, ScriptTypeClass_CTOR, 0x6)
//{
//	GET(ScriptTypeClass*, pThis, ESI);
//	ScriptTypeExt::ExtMap.FindOrAllocate(pThis);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x691796, ScriptTypeClass_DTOR, 0x6)
//{
//	GET(ScriptTypeClass*, pThis, ESI);
//
//	ScriptTypeExt::ExtMap.Remove(pThis);
//
//	return 0x0;
//}
//
//DEFINE_HOOK_AGAIN(0x691D90, ScriptTypeClass_SaveLoad_Prefix, 0x5)
//DEFINE_HOOK(0x691DE0, ScriptTypeClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(ScriptTypeClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	ScriptTypeExt::ExtMap.PrepareStream(pItem, pStm);
//
//	return 0;
//}
//
//// Before : 691DD1 , 0xA
//// After : 691DCF, 0x6
//DEFINE_HOOK(0x691DCF, ScriptTypeClass_Load_Suffix, 0x6)
//{
//	ScriptTypeExt::ExtMap.LoadStatic();
//	return 0;
//}
//
//// Before : 691DFA , 5
//// After : 0x691DF4
//DEFINE_HOOK(0x691DF4, ScriptTypeClass_Save_Suffix, 0x6)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if (SUCCEEDED(nRes)) {
//		nRes = 0;
//		ScriptTypeExt::ExtMap.SaveStatic();
//	}
//
//	return 0x691DFA;
//}