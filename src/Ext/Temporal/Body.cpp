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

bool TemporalExtContainer::LoadGlobals(PhobosStreamReader& Stm)
{
	Clear();

	size_t Count = 0;
	if (!Stm.Load(Count))
		return false;

	Array.reserve(Count);

	for (size_t i = 0; i < Count; ++i)
	{

		void* oldPtr = nullptr;

		if (!Stm.Load(oldPtr))
			return false;

		auto newPtr = new TemporalExtData(nullptr, noinit_t());
		PHOBOS_SWIZZLE_REGISTER_POINTER((long)oldPtr, newPtr, "TemporalExtData")
		ExtensionSwizzleManager::RegisterExtensionPointer(oldPtr, newPtr);
		newPtr->LoadFromStream(Stm);
		Array.push_back(newPtr);
	}

	return true;
}

bool TemporalExtContainer::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(Array.size());

	for (auto& item : Array) {
		Stm.Save(item);
		item->SaveToStream(Stm);
	}

	return true;
}

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

