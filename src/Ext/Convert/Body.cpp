#include "Body.h"

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

ConvertExt::ExtContainer ConvertExt::ExtMap;

ConvertExt::ExtContainer::ExtContainer() : Container("ConvertClass") {}
ConvertExt::ExtContainer::~ExtContainer() = default;

void ConvertExt::ExtData::InitializeConstants() { }

//DEFINE_HOOK(0x48EBD6, ConvertClass_CTOR, 0x5)
//{
//	GET(ConvertClass*, pItem, ESI);
//	ConvertExt::ExtMap.JustAllocate(pItem, pItem, "Trying to Allocate from nullptr ! \n");
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
//
//DEFINE_HOOK(0x48E955, ConvertClass_FillColorPointer, 0x6)
//{
//	GET(ConvertClass*, pThis, ESI);
//	GET_STACK(BytePalette*, pData, STACK_OFFS(0x24, 0x4));
//
//	auto ptr = GameCreate<char>(ColorStruct::Max);
//	pThis->BufferB = ptr;
//	
//	for (int i = 0; i < ColorStruct::Max; i++)
//	{
//		HSVClass nHSVResult;
//		pData->Entries[(i % ColorStruct::Max) + (i % ColorStruct::Max)].ConstructHSV(&nHSVResult);
//		nHSVResult.Val = nHSVResult.Val >> 1;
//		auto nColorArg = nHSVResult.ToColorStruct();
//		pThis->BufferB[i] = (char)pData->Closest_Color(nColorArg);
//	}
//
//	return 0x48E955;
//}
