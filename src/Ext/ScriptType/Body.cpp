#include "Body.h"

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
//ScriptTypeExt::ExtContainer ScriptTypeExt::ExtMap;

// =============================
// container hooks
//
//ASMJIT_PATCH_AGAIN(0x691D05, ScriptTypeClass_CTOR, 0x6)
//ASMJIT_PATCH_AGAIN(0x691ACC, ScriptTypeClass_CTOR, 0x5)
//ASMJIT_PATCH(0x691769, ScriptTypeClass_CTOR, 0x6)
//{
//	GET(ScriptTypeClass*, pThis, ESI);
//	ScriptTypeExt::ExtMap.Allocate(pThis);
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x691796, ScriptTypeClass_DTOR, 0x6)
//{
//	GET(ScriptTypeClass*, pThis, ESI);
//
//	ScriptTypeExt::ExtMap.Remove(pThis);
//
//	return 0x0;
//}
//
//ASMJIT_PATCH_AGAIN(0x691D90, ScriptTypeClass_SaveLoad_Prefix, 0x5)
//ASMJIT_PATCH(0x691DE0, ScriptTypeClass_SaveLoad_Prefix, 0x8)
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
//ASMJIT_PATCH(0x691DCF, ScriptTypeClass_Load_Suffix, 0x6)
//{
//	ScriptTypeExt::ExtMap.LoadStatic();
//	return 0;
//}
//
//// Before : 691DFA , 5
//// After : 0x691DF4
//ASMJIT_PATCH(0x691DF4, ScriptTypeClass_Save_Suffix, 0x6)
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