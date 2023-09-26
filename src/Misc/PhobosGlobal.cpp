#include "PhobosGlobal.h"

#include <AbstractClass.h>

std::unique_ptr<PhobosGlobal> PhobosGlobal::GlobalObject;

void PhobosGlobal::Clear()
{
	auto pInstance = PhobosGlobal::Instance();
	pInstance->maxColor = ColorStruct::Empty;
	// Just big enough to hold all types
	std::memset(pInstance->BuildTimeDatas, 0, sizeof(pInstance->BuildTimeDatas));
	pInstance->DetonateDamageArea = true;

	pInstance->TempFoundationData1.clear();
	pInstance->TempFoundationData2.clear();
	pInstance->TempCoveredCellsData.clear();
}

void PhobosGlobal::PointerGotInvalid(AbstractClass* ptr, bool removed)
{

}

bool PhobosGlobal::SaveGlobals(PhobosStreamWriter& stm) { return PhobosGlobal::Instance()->Serialize(stm); }
bool PhobosGlobal::LoadGlobals(PhobosStreamReader& stm)
{

	if (PhobosGlobal::Instance()->Serialize(stm))
	{

#ifdef ENABLE_FOUNDATIONHOOK
		MouseClass::Instance->CurrentFoundation_Data = Instance()->TempFoundationData1.data();
		MouseClass::Instance->CurrentFoundationCopy_Data = Instance()->TempFoundationData2.data();
#endif
		return true;
	}

	return false;
}
