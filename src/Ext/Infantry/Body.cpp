#include "Body.h"

InfantryExt::ExtContainer InfantryExt::ExtMap;

void InfantryExt::ExtData::InitializeConstants()
{
}

void InfantryExt::ExtData::InvalidatePointer(void* const ptr, bool bRemoved)
{
}

// =============================
// load / save

template <typename T>
void InfantryExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(IsUsingDeathSequence)
		.Process(CurrentDoType)
		.Process(ForceFullRearmDelay)
		;
}

bool InfantryExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool InfantryExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

void InfantryExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<InfantryClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void InfantryExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<InfantryClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}


InfantryExt::ExtContainer::ExtContainer() : Container("InfantryClass") { }
InfantryExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//DEFINE_HOOK(0x517B4A, InfantryClass_CTOR, 0xA)
//{
//	GET(InfantryClass*, pItem, ESI);
//	GET(InfantryTypeClass*, pType, EAX);
//
//	if(pType) {
//		InfantryExt::ExtMap.JustAllocate(pItem, pItem, "Failed !");
//		return 0x517B4E;
//	}
//
//	return 0x517BBD;
//}

DEFINE_HOOK(0x517ACC, InfantryClass_CTOR, 0x6)
{
	GET(InfantryClass*, pItem, ESI);

	InfantryExt::ExtMap.Allocate(pItem);
	return 0;
}

DEFINE_HOOK(0x517D90, InfantryClass_DTOR, 0x5)
{
	GET(InfantryClass* const, pItem, ECX);
	InfantryExt::ExtMap.Remove(pItem);
	return 0;
}


DEFINE_HOOK_AGAIN(0x521B00, InfantryClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x521960, InfantryClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(InfantryClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	InfantryExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

// Before : 0x521AEC , 0x6
// After : 521AEA , 0x5
DEFINE_HOOK(0x521AEA, InfantryClass_Load_Suffix, 0x5)
{
	InfantryExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x521B14, InfantryClass_Save_Suffix, 0x3)
{
	GET(const HRESULT, nRes, EAX);

	if (SUCCEEDED(nRes))
		InfantryExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK(0x51AA23, InfantryClass_Detach, 0x6)
{
	GET(InfantryClass* const, pThis, ESI);
	GET(void*, target, EDI);
	GET_STACK(bool, all, STACK_OFFS(0x8, -0x8));

	if (auto pExt = InfantryExt::ExtMap.Find(pThis)) {
		pExt->InvalidatePointer(target, all);
	}

	return pThis->Type == target ? 0x51AA2B : 0x51AA35;
}