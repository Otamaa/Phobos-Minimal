#include "Body.h"


// =============================
// load / save

//template <typename T>
//void DiskLaserExt::ExtData::Serialize(T& Stm)
//{
//	Stm
//		.Process(this->Initialized)
//		;
//}

// =============================
// container

//DiskLaserExt::ExtContainer DiskLaserExt::ExtMap;

// =============================
// container hooks

//ASMJIT_PATCH(0x4A7A6A, DiskLaserClass_CTOR, 0x6)
//{
//	GET(DiskLaserClass*, pItem, ESI);
//#
//	DiskLaserExt::ExtMap.Allocate(pItem);
//
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x4A7B00 , DiskLaserClass_SDDTOR, 0x8)
//ASMJIT_PATCH(0x4A7C90, DiskLaserClass_SDDTOR, 0x8)
//{
//	GET(DiskLaserClass *, pItem, ECX);
//	DiskLaserExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x4A7B90, DiskLaserClass_SaveLoad_Prefix, 0x5)
//ASMJIT_PATCH(0x4A7C10, DiskLaserClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(DiskLaserClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	DiskLaserExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//ASMJIT_PATCH(0x4A7BEE, DiskLaserClass_Load_Suffix, 0x9)
//{
//	GET(DiskLaserClass*, pThis, ESI);
//	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->Weapon);
//	DiskLaserExt::ExtMap.LoadStatic();
//	return 0x438BBB;
//}
//
//ASMJIT_PATCH(0x4A7C1C, DiskLaserClass_Save_Suffix, 0x8)
//{
//	GET(ParasiteClass*, pThis, ECX);
//	GET(IStream*, pStream, EAX);
//	GET(BOOL, bClearDirty, EAX);
//
//	const auto nRes = AbstractClass::_Save(pThis, pStream, bClearDirty);
//
//	if(SUCCEEDED(nRes))
//		DiskLaserExt::ExtMap.SaveStatic();
//
//	R->EAX(nRes);
//	return 0x4A7C24;
//}

//detach