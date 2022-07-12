#include "Body.h"

#include <Utilities/Debug.h>

template<> const DWORD Extension<ConvertClass>::Canary = 0xAAAAAACC;
ConvertExt::ExtContainer ConvertExt::ExtMap;

ConvertExt::ExtContainer::ExtContainer() : Container("ConvertClass") {}
ConvertExt::ExtContainer::~ExtContainer() = default;

void ConvertExt::ExtData::InitializeConstants() { }
void ConvertExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) {}

void ConvertExt::GetOrSetName(ConvertClass* const pConvert, const std::string_view nName) {
	if (pConvert) {
		if (auto pConvertExt = ConvertExt::ExtMap.Find(pConvert)) {
			if (pConvertExt->Name)
				pConvertExt->Name = nName.data();
			else
				if(strcmp(pConvertExt->Name.data(),nName.data()))
					Debug::Log("Found Duplicate Convert[%x][%s] with same Palette ! \n", pConvert,pConvertExt->Name.data());
		}
	}
}

bool ConvertExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool ConvertExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

DEFINE_HOOK(0x48EBD6, ConvertClass_CTOR, 0x5)
{
	GET(ConvertClass*, pItem, ESI);

	//Debug::Log("Constructing %p.\n", pItem);
	ConvertExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x490400, ConvertClass_DTOR, 0xA)
DEFINE_HOOK(0x491210, ConvertClass_DTOR, 0xA)
{
	GET(ConvertClass*, pItem, ECX);

	//Debug::Log("%p Has been removed.\n", pItem);
	ConvertExt::ExtMap.Remove(pItem);

	return 0;
}