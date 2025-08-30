#include "Body.h"

#include <Ext/Anim/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/Macro.h>

// =============================
// load / save

template <typename T>
void BombExtData::Serialize(T& Stm) {

	Stm
		.Process(this->Weapon, true)
		;
}

// =============================
// container
BombExtContainer BombExtContainer::Instance;
std::vector<BombExtData*>  Container<BombExtData>::Array;

// =============================
// container hooks

// not initEd :
// Ownerhouse
// target
// state
// ticksound

ASMJIT_PATCH(0x4385FC, BombClass_CTOR, 0x6)
{
	GET(BombClass*, pItem, ESI);

	BombExtContainer::Instance.Allocate(pItem);

	return 0;
}ASMJIT_PATCH_AGAIN(0x438EE9, BombClass_CTOR, 0x6)

ASMJIT_PATCH(0x4393F2, BombClass_SDDTOR, 0x5)
{
	GET(BombClass *, pItem, ECX);
	BombExtContainer::Instance.Remove(pItem);
	return 0;
}

HRESULT __stdcall FakeBombClass::_Load(IStream* pStm)
{

	BombExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->BombClass::Load(pStm);

	if (SUCCEEDED(res))
		BombExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeBombClass::_Save(IStream* pStm, bool clearDirty)
{

	BombExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->BombClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		BombExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3D24, FakeBombClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E3D28, FakeBombClass::_Save)