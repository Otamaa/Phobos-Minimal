 #include "Body.h"


// =============================
// load / save

//template <typename T>
//void TemporalExt::ExtData::Serialize(T& Stm) {
//	//Debug::Log("Processing Element From TemporalExt ! \n");
//
//	Stm
//		.Process(this->Initialized)
//		.Process(this->Weapon)
//
//		;
//
//}

// =============================
// container
//TemporalExt::ExtContainer TemporalExt::ExtMap;

// =============================
// container hooks

//DEFINE_HOOK_AGAIN(0x71A4CD, TemporalClass_CTOR, 0x6) //factory ?
//DEFINE_HOOK(0x71A594, TemporalClass_CTOR, 0x7)
//{
//	GET(TemporalClass*, pItem, ESI);
//	TemporalExt::ExtMap.Allocate(pItem);
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x71B1DF, TemporalClass_SDDTOR, 0x7)
//DEFINE_HOOK(0x71A5FF, TemporalClass_SDDTOR, 0x7)
//{
//	GET(TemporalClass*, pItem, ESI);
//	TemporalExt::ExtMap.Remove(pItem);
//	return 0;
//}
//
//DEFINE_HOOK(0x71A700, TemporalClass_Save_Replace, 0x8)
//{
//	GET_STACK(TemporalClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	GET_STACK(bool, bClearDirty, 0xC);
//
//	TemporalExt::ExtMap.PrepareStream(pItem, pStm);
//	auto const nRes = AbstractClass::_Save(pItem, pStm, bClearDirty);
//
//	if (SUCCEEDED(nRes))
//		TemporalExt::ExtMap.SaveStatic();
//
//	R->EAX(nRes);
//	return 0x71A714;
//}
//
//DEFINE_HOOK(0x71A660, TemporalClass_Load_Prefix, 0x5)
//{
//	GET_STACK(TemporalClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//	TemporalExt::ExtMap.PrepareStream(pItem, pStm);
//	return 0;
//}
//
//DEFINE_HOOK(0x71A6E8, TemporalClass_Load_Suffix, 0x9)
//{
//	GET(TemporalClass*, pItem, ESI);
//
//	SwizzleManagerClass::Instance->Swizzle((void**)&pItem->SourceSW);
//	TemporalExt::ExtMap.LoadStatic();
//
//	return 0x71A6F6;
//}

//DEFINE_HOOK(0x71AB68, TemporalClass_Detach, 0x5)
//{
//	GET(TemporalClass*, pThis, ESI);
//	GET(void*, target, EAX);
//
//	if (const auto pExt = TemporalExt::ExtMap.Find(pThis))
//		pExt->InvalidatePointer(target, true);
//
//	return 0x0;
//}