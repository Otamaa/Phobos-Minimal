#include "Body.h"

//DEFINE_HOOK_AGAIN(0x471998, CaptureManagerClass_CTOR, 0x6) // factory
//DEFINE_HOOK(0x471887, CaptureManagerClass_CTOR, 0x6)
//{
//	GET(CaptureManagerClass* const, pItem, ESI);
//
//	CaptureExt::ExtMap.Allocate(pItem);
//
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x4718ED , CaptureManagerClass_DTOR , 0x6) // Factory
//DEFINE_HOOK(0x4729EF, CaptureManagerClass_DTOR, 0x7)
//{
//	GET(CaptureManagerClass* const, pItem, ESI);
//	CaptureExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x4728E0, CaptureManagerClass_SaveLoad_Prefix, 0x5)
//DEFINE_HOOK(0x472720, CaptureManagerClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(CaptureManagerClass*, pThis, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	CaptureExt::ExtMap.PrepareStream(pThis, pStm);
//	return 0;
//}
//
//DEFINE_HOOK(0x4728CA, CaptureManagerClass_Load_Suffix, 0x7)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if(SUCCEEDED(nRes))
//		CaptureExt::ExtMap.LoadStatic();
//
//	return 0;
//}
//
//DEFINE_HOOK(0x472958, CaptureManagerClass_Save_Suffix, 0x7)
//{
//	GET(HRESULT, nRes, EAX);
//
//	if (SUCCEEDED(nRes))
//		CaptureExt::ExtMap.SaveStatic();
//
//	return 0;
//}