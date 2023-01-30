#include "Body.h"

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

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

//DEFINE_JUMP(LJMP, 0x48E955 ,0x48E9E1)
//
//DEFINE_HOOK(0x48EBD6, ConvertClass_CTOR, 0x5)
//{
//	GET(ConvertClass*, pItem, ESI);
//	ConvertExt::ExtMap.JustAllocate(pItem , pItem , "Trying to Allocate from nullptr ! \n");
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x490400, ConvertClass_DTOR, 0xA)
//DEFINE_HOOK(0x491210, ConvertClass_DTOR, 0xA)
//{
//	GET(ConvertClass*, pItem, ECX);
//	ConvertExt::ExtMap.Remove(pItem);
//	return 0;
//}