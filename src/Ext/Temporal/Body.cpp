 #include "Body.h"


#include <Utilities/Macro.h>

// =============================
// load / save

template <typename T>
void TemporalExtData::Serialize(T& Stm) {


}

// =============================
// container
TemporalExtContainer TemporalExtContainer::Instance;
std::vector<TemporalExtData*> Container<TemporalExtData>::Array;
// =============================
// container hooks

ASMJIT_PATCH(0x71A594, TemporalClass_CTOR, 0x7)
{
	GET(TemporalClass*, pItem, ESI);
	TemporalExtContainer::Instance.Allocate(pItem);
	return 0;
}

ASMJIT_PATCH(0x71A5FF, TemporalClass_SDDTOR, 0x7)
{
	GET(TemporalClass*, pItem, ESI);
	TemporalExtContainer::Instance.Remove(pItem);
	return 0;
}ASMJIT_PATCH_AGAIN(0x71B1DF, TemporalClass_SDDTOR, 0x7)

//ASMJIT_PATCH(0x71AB68, TemporalClass_Detach, 0x5)
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
	auto hr = this->TemporalClass::Load(pStm);

	if (SUCCEEDED(hr))
	{
		hr = TemporalExtContainer::Instance.ReadDataFromTheByteStream(this,
			TemporalExtContainer::Instance.AllocateNoInit(this), pStm);
	}

	return hr;
}

HRESULT __stdcall FakeTemporalClass::_Save(IStream* pStm, BOOL clearDirty)
{
	auto hr = this->TemporalClass::Save(pStm, clearDirty);

	if (SUCCEEDED(hr))
	{
		hr = TemporalExtContainer::Instance.WriteDataToTheByteStream(this, pStm);
	}

	return hr;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5194, FakeTemporalClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F5198, FakeTemporalClass::_Save)
