#include "Body.h"

//DEFINE_HOOK(0x41E471, AITriggerTypeClass_CTOR, 0x7)
//{
//	GET(AITriggerTypeClass*, pThis, ESI);
//	AITriggerTypeExt::ExtMap.Allocate(pThis);
//	return 0x0;
//}
//
//DEFINE_HOOK(0x41E4AF, AITriggerTypeClass_DTOR, 0x6)
//{
//	GET(AITriggerTypeClass*, pThis, ESI);
//	AITriggerTypeExt::ExtMap.Remove(pThis);
//	return 0x0;
//}
//
//DEFINE_HOOK_AGAIN(0x41E540 , AITriggerTypeClass_SaveLoad_Prefix, 0x5)
//DEFINE_HOOK(0x41E5C0, AITriggerTypeClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(AITriggerTypeClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	AITriggerTypeExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//// BEfore -> 41E5A1
//// After -> 41E5B2
//// Better -> 41E5A1
//DEFINE_HOOK(0x41E5B2, AITriggerTypeClass_Load_Suffix, 0x6)
//{
//	AITriggerTypeExt::ExtMap.LoadStatic();
//	return 0;
//}
//// BEfore -> 41E5DA
//// After -> 41E5D8
//// Better -> 41E5D4
//DEFINE_HOOK(0x41E5D8, AITriggerTypeClass_Save_Suffix, 0x5)
//{
//	AITriggerTypeExt::ExtMap.SaveStatic();
//	return 0;
//}