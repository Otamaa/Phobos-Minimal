#include "Body.h"

DEFINE_HOOK_AGAIN(0x75ED27, WaveClass_CTOR, 0x5)
DEFINE_HOOK(0x75EA59, WaveClass_CTOR, 0x5)
{
	GET(WaveClass*, pItem, ESI);
	WaveExtContainer::Instance.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x75F7D0, WaveClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x75F650, WaveClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(WaveClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	WaveExtContainer::Instance.PrepareStream(pItem, pStm);
	return 0;
}

//we load it before DVC<CellStruct> get loaded
DEFINE_HOOK(0x75F704, WaveClass_Load_Suffix, 0x7)
{
	WaveExtContainer::Instance.LoadStatic();
	return 0;
}

//write it before DVC<CellStruct>
DEFINE_HOOK(0x75F7E7, WaveClass_Save_Suffix, 0x6)
{
	GET(HRESULT, nRes, EAX);

	WaveExtContainer::Instance.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x75ED57 , WaveClass_DTOR, 0x6)
DEFINE_HOOK(0x763226, WaveClass_DTOR, 0x6)
{
	GET(WaveClass*, pItem, EDI);
	WaveExtContainer::Instance.Remove(pItem);
	return 0;
}

//void __fastcall WaveClass_Detach_Wrapper(WaveClass* pThis ,DWORD , AbstractClass* target , bool all)\
//{
//	//WaveExtContainer::Instance.InvalidatePointerFor(pThis , target , all);
//	pThis->WaveClass::PointerExpired(target , all);
//}
//DEFINE_JUMP(VTABLE, 0x7F6C1C, GET_OFFSET(WaveClass_Detach_Wrapper))
