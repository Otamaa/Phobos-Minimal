 #include "Body.h"


#include <Utilities/Macro.h>

// =============================
// load / save

template <typename T>
void TemporalExtData::Serialize(T& Stm) {
	Stm
		.Process(this->Initialized)
		;

}

// =============================
// container
TemporalExtContainer TemporalExtContainer::Instance;

// =============================
// container hooks

DEFINE_HOOK(0x71A594, TemporalClass_CTOR, 0x7)
{
	GET(TemporalClass*, pItem, ESI);
	TemporalExtContainer::Instance.Allocate(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x71B1DF, TemporalClass_SDDTOR, 0x7)
DEFINE_HOOK(0x71A5FF, TemporalClass_SDDTOR, 0x7)
{
	GET(TemporalClass*, pItem, ESI);
	TemporalExtContainer::Instance.Remove(pItem);
	return 0;
}

//DEFINE_HOOK(0x71AB68, TemporalClass_Detach, 0x5)
//{
//	GET(TemporalClass*, pThis, ESI);
//	GET(AbstractClass*, target, EAX);
//
//	if (const auto pExt = TemporalExtContainer::Instance.TryFind(pThis))
//		pExt->InvalidatePointer(target, true);
//
//	return 0x0;
//}

HRESULT __stdcall FakeTemporalClass::_Load(IStream* pStm)
{

	TemporalExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->TemporalClass::Load(pStm);

	if (SUCCEEDED(res))
		TemporalExtContainer::Instance.LoadStatic();

	return res;
}

HRESULT __stdcall FakeTemporalClass::_Save(IStream* pStm, bool clearDirty)
{

	TemporalExtContainer::Instance.PrepareStream(this, pStm);
	HRESULT res = this->TemporalClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
		TemporalExtContainer::Instance.SaveStatic();

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5194, FakeTemporalClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5198, FakeTemporalClass::_Save)
