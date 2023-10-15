#include "Body.h"

// =============================
// load / save

template <typename T>
void InfantryExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->IsUsingDeathSequence)
		.Process(this->CurrentDoType)
		.Process(this->ForceFullRearmDelay)
		;
}

// =============================
// container

InfantryExtContainer InfantryExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x517ACC, InfantryClass_CTOR, 0x6)
{
	GET(InfantryClass*, pItem, ESI);

	if(pItem->Type)
		InfantryExtContainer::Instance.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x517D90, InfantryClass_DTOR, 0x5)
{
	GET(InfantryClass* const, pItem, ECX);
	InfantryExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x521B00, InfantryClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x521960, InfantryClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(InfantryClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	InfantryExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

// Before : 0x521AEC , 0x6
// After : 521AEA , 0x5
DEFINE_HOOK(0x521AEA, InfantryClass_Load_Suffix, 0x5)
{
	InfantryExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x521B14, InfantryClass_Save_Suffix, 0x3)
{
	GET(const HRESULT, nRes, EAX);

	if (SUCCEEDED(nRes))
		InfantryExtContainer::Instance.SaveStatic();

	return 0;
}

// DEFINE_HOOK(0x51AA23, InfantryClass_Detach, 0x6)
// {
// 	GET(InfantryClass* const, pThis, ESI);
// 	GET(void*, target, EDI);
// 	GET_STACK(bool, all, STACK_OFFS(0x8, -0x8));
//
// 	InfantryExt::ExtMap.InvalidatePointerFor(pThis, target, all);
//
// 	return pThis->Type == target ? 0x51AA2B : 0x51AA35;
// }