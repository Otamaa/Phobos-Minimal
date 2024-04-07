#include "Body.h"

#include <Notifications.h>

DEFINE_JUMP(LJMP, 0x71BC31 , 0x71BC86);

DEFINE_HOOK(0x71BE74, TerrainClass_CTOR, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	TerrainExtContainer::Instance.Allocate(pItem);
	//PointerExpiredNotification::NotifyInvalidObject->Add(pItem);
	return 0;
}

DEFINE_HOOK(0x71BCA5, TerrainClass_CTOR_MoveAndAllocate, 0x5)
{
	GET(TerrainClass*, pItem, ESI);
	GET_STACK(CellStruct*, pCoord, 0x24);

	TerrainExtContainer::Instance.FindOrAllocate(pItem);

	if (pCoord->IsValid()) {
		//vtable may not instantiated
		if (!pItem->TerrainClass::Unlimbo(CellClass::Cell2Coord(*pCoord), static_cast<DirType>(0))) {
			pItem->ObjectClass::UnInit();
		}
	}

	return 0x0;
}

//Remove Ext later , dont do it to early otherwise some stuffs broke
DEFINE_HOOK(0x71B824, TerrainClass_DTOR, 0x5)
{
	GET(TerrainClass*, pItem, ESI);

	if(Unsorted::WTFMode() || pItem->Type)
	{
		pItem->IsAlive = true;
		if (!pItem->Limbo())
			pItem->AnnounceExpiredPointer();
	}

	if(auto pExt = TerrainExtContainer::Instance.TryFind(pItem)) {
		delete pExt;
		TerrainExtContainer::Instance.ClearExtAttribute(pItem);
		//PointerExpiredNotification::NotifyInvalidObject->Remove(pItem);
	}

	return 0x71B845;
}

DEFINE_HOOK_AGAIN(0x71CDA0, TerrainClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x71CF30, TerrainClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(TerrainClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TerrainExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x71CEAC, TerrainClass_Load_Suffix, 0x9)
{
	TerrainExtContainer::Instance.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x71CF44, TerrainClass_Save_Suffix, 0x5)
{
	TerrainExtContainer::Instance.SaveStatic();
	return 0;
}

void __fastcall TerrainClass_Detach_Wrapper(TerrainClass* pThis, DWORD, AbstractClass* target, bool all)
{
	TerrainExtContainer::Instance.InvalidatePointerFor(pThis, target, all);
	pThis->TerrainClass::PointerExpired(target, all);
}
DEFINE_JUMP(VTABLE, 0x7F5254, GET_OFFSET(TerrainClass_Detach_Wrapper));
