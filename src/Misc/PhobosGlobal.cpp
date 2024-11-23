#include "PhobosGlobal.h"

#include <AbstractClass.h>

PhobosGlobal PhobosGlobal::GlobalObject;

void PhobosGlobal::Clear()
{
	auto pInstance = PhobosGlobal::Instance();
	pInstance->DetonateDamageArea = true;

	pInstance->TempFoundationData1.clear();
	pInstance->TempFoundationData2.clear();
	pInstance->TempCoveredCellsData.clear();
	pInstance->ColorDatas.reset();
	pInstance->PathfindTechno.Clear();
	pInstance->CurCopyArray.clear();

}

void PhobosGlobal::PointerGotInvalid(AbstractClass* ptr, bool removed)
{
	auto pInstance = PhobosGlobal::Instance();
	pInstance->PathfindTechno.InvalidatePointer(ptr , removed);
	for (auto& copyArr : pInstance->CurCopyArray) {
		copyArr.second.Invalidate(ptr, removed);
	}
}

bool PhobosGlobal::SaveGlobals(PhobosStreamWriter& stm) { return PhobosGlobal::Instance()->Serialize(stm); }
bool PhobosGlobal::LoadGlobals(PhobosStreamReader& stm)
{

	if (PhobosGlobal::Instance()->Serialize(stm))
	{
#ifndef ENABLE_FOUNDATIONHOOK
		if (Unsorted::CursorSize())
		{
			Unsorted::CursorSize = PhobosGlobal::Instance()->TempFoundationData1.data();
		}

		if (Unsorted::CursorSizeSecond())
		{
			Unsorted::CursorSizeSecond = PhobosGlobal::Instance()->TempFoundationData2.data();
		}
#endif
		return true;
	}

	return false;
}
