#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>

HouseClass*  __fastcall BombExtData::GetOwningHouse(BombClass* pThis, void*)
{
	return pThis->OwnerHouse;
}

void __fastcall BombExtData::InvalidatePointer(BombClass* pThis, void*, void* const ptr, bool removed){ }

// =============================
// load / save

template <typename T>
void BombExtData::Serialize(T& Stm) {

	Stm
		.Process(this->Initialized)
		.Process(this->Weapon)
		;
}

// =============================
// container
BombExtContainer BombExtContainer::Instance;


// =============================
// container hooks

// not initEd :
// Ownerhouse
// target
// state
// ticksound
DEFINE_HOOK_AGAIN(0x438EE9, BombClass_CTOR , 0x6)
DEFINE_HOOK(0x4385FC, BombClass_CTOR, 0x6)
{
	GET(BombClass*, pItem, ESI);

	BombExtContainer::Instance.Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4393F2, BombClass_SDDTOR, 0x5)
{
	GET(BombClass *, pItem, ECX);
	BombExtContainer::Instance.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x438B40, BombClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x438BD0, BombClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(BombClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	BombExtContainer::Instance.PrepareStream(pItem, pStm);
	return 0;
}

DEFINE_HOOK(0x438BAD, BombClass_Load_Suffix, 0x9)
{
	GET(BombClass*, pThis, ESI);
	SwizzleManagerClass::Instance->Swizzle((void**)&pThis->Target);
	BombExtContainer::Instance.LoadStatic();
	return 0x438BBB;
}

DEFINE_HOOK(0x438BE4, BombClass_Save_Suffix, 0x5)
{
	GET(HRESULT, nRes, EAX);

	if(SUCCEEDED(nRes))
		BombExtContainer::Instance.SaveStatic();

	return 0;
}