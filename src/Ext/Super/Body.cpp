#include "Body.h"

// =============================
// load / save

template <typename T>
void SuperExt::ExtData::Serialize(T& Stm) { 

	Stm
		.Process(this->Initialized)
		.Process(this->Temp_CellStruct)
		.Process(this->Temp_IsPlayer)

		;
}

// =============================
// container
SuperExt::ExtContainer SuperExt::ExtMap;

SuperExt::ExtContainer::ExtContainer() : Container("SuperClass") { }
SuperExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks


DEFINE_HOOK_AGAIN(0x6CAF32 , SuperClass_CTOR, 0x6)
DEFINE_HOOK(0x6CB10E, SuperClass_CTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);
	SuperExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6CB1BD, SuperClass_SDDTOR, 0x7)
{
	GET(SuperClass*, pItem, ESI);
	SuperExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6CDEF0, SuperClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6CDFD0, SuperClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(SuperClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	SuperExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6CDFC4, SuperClass_Load_Suffix, 0x7)
{
	SuperExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6CDFE8, SuperClass_Save_Suffix, 0x5)
{
	SuperExt::ExtMap.SaveStatic();
	return 0;
}

//DEFINE_HOOK(0x6CE001 , SuperClass_Detach , 0x5)
//{
//	GET(SuperClass*, pThis, ESI);
//	GET(void*, target, EAX);
//	GET_STACK(bool, all, STACK_OFFS(0x4, -0x8));
//
//	if (auto pExt = SuperExt::ExtMap.Find(pThis))
//		pExt->InvalidatePointer(target, all);
//
//	return target == pThis->Type ? 0x6CE006 : 0x6CE009;
//}