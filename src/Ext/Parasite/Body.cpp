#include "Body.h"

//DEFINE_HOOK_AGAIN(0x62924C , ParasiteClass_CTOR,0x5 )
//DEFINE_HOOK(0x62932E, ParasiteClass_CTOR, 0x6)
//{
//	GET(ParasiteClass*, pItem, ESI);
//	ParasiteExt::ExtMap.Allocate(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x62AFFE , ParasiteClass_DTOR, 0x6)
//DEFINE_HOOK(0x62946E, ParasiteClass_DTOR, 0x6)
//{
//	GET(ParasiteClass*, pItem, ESI);
//	ParasiteExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x6296B0, ParasiteClass_SaveLoad_Prefix, 0x8)
//DEFINE_HOOK(0x6295B0, ParasiteClass_SaveLoad_Prefix, 0x5)
//{
//
//	GET_STACK(ParasiteClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	ParasiteExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//DEFINE_HOOK(0x62969D, ParasiteClass_Load_Suffix, 0x5)
//{
//	ParasiteExt::ExtMap.LoadStatic();
//	return 0;
//}
//
//DEFINE_HOOK(0x6296BC, ParasiteClass_Save_Suffix, 0x8)
//{
//	GET(ParasiteClass*, pThis, ECX);
//	GET(IStream*, pStream, EAX);
//	GET(BOOL, bClearDirty, EAX);
//
//	auto const nRes = AbstractClass::_Save(pThis, pStream, bClearDirty);
//
//	if (SUCCEEDED(nRes))
//		ParasiteExt::ExtMap.SaveStatic();
//
//	R->EAX(nRes);
//	return 0x6296C4;
//}
//detach