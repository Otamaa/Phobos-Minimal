#include "Body.h"

//DEFINE_HOOK(0x6DD176, TActionClass_CTOR, 0x5)
//{
//	GET(TActionClass*, pItem, ESI);
//	TActionExt::ExtMap.Allocate(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x6DD1E6, TActionClass_SDDTOR, 0x7)
//DEFINE_HOOK(0x6E4696, TActionClass_SDDTOR, 0x7)
//{
//	GET(TActionClass*, pItem, ESI);
//	TActionExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//DEFINE_HOOK(0x6E3E29, TActionClass_Load_Suffix, 0x4)
//{
//	TActionExt::ExtMap.LoadStatic();
//	return 0x0;
//}
//
//DEFINE_HOOK(0x6E3E4A, TActionClass_Save_Suffix, 0x3)
//{
//	TActionExt::ExtMap.SaveStatic();
//	return 0x0;
//}

//DEFINE_HOOK_AGAIN(0x6E3E30, TActionClass_SaveLoad_Prefix, 0x8)
//DEFINE_HOOK(0x6E3DB0, TActionClass_SaveLoad_Prefix, 0x5)
//{
//	GET_STACK(TActionClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	TActionExt::ExtMap.PrepareStream(pItem, pStm);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x6E3E19, TActionClass_Load_Suffix, 0x9)
//{
//	GET(TActionClass*, pItem, ESI);
//
//	SwizzleManagerClass::Instance->Swizzle((void**)&pItem->TriggerType);
//	TActionExt::ExtMap.LoadStatic();
//
//	return 0x6E3E27;
//}
//
//DEFINE_HOOK(0x6E3E44, TActionClass_Save_Suffix, 0x6)
//{
//	GET(HRESULT const, nRes, EAX);
//
//	if(SUCCEEDED(nRes)){
//		TActionExt::ExtMap.SaveStatic();
//		return 0x6E3E48;
//	}
//
//	return 0x6E3E4A;
//}

//DEFINE_HOOK(0x6DD2DE, TActionClass_Detach, 0x5)
//{
//	GET(TActionClass*, pThis, ECX);
//	GET(void*, target, EDX);
//	GET_STACK(bool, all, STACK_OFFS(0xC, -0x8));
//
//	if (auto pExt = TActionExt::ExtMap.Find(pThis))
//		pExt->InvalidatePointer(target, all);
//
//	return pThis->TriggerType == target ? 0x6DD2E3 : 0x6DD2E6;
//}
