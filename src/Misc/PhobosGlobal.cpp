#include "PhobosGlobal.h"

#include <AbstractClass.h>

std::unique_ptr<PhobosGlobal> PhobosGlobal::GlobalObject = std::make_unique<PhobosGlobal>();
PhobosGlobal::ColorsData PhobosGlobal::ColorDatas {};

void PhobosGlobal::Clear()
{
	auto pInstance = PhobosGlobal::Instance();
	pInstance->DetonateDamageArea = true;

	pInstance->TempFoundationData1.clear();
	pInstance->TempFoundationData2.clear();
	pInstance->TempCoveredCellsData.clear();
	PhobosGlobal::ColorDatas.reset();
	pInstance->PathfindTechno.Clear();
}

void PhobosGlobal::PointerGotInvalid(AbstractClass* ptr, bool removed)
{
	auto pInstance = PhobosGlobal::Instance();
	pInstance->PathfindTechno.InvalidatePointer(ptr , removed);
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
