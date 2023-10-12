#include "AIBasePlan.h"

#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <MessageListClass.h>

#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>

const char* AIBasePlanCommandClass::GetName() const
{
	return "Dump AI Base Plan";
}

const wchar_t* AIBasePlanCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_AI_BASE_PLAN", L"AI Base Plan Logger");
}

const wchar_t* AIBasePlanCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* AIBasePlanCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DUMP_AI_BASE_PLAN_DESC", L"Dumps the AI Base Plans to the log");
}

void AIBasePlanCommandClass::Execute(WWKey dwUnk) const
{
	if (this->CheckDebugDeactivated()) {
		return;
	}

	Debug::Log("AI Base Plans:\n");
	for (int i = 0; i < HouseClass::Array->Count; ++i)
	{
		auto H = HouseClass::Array->GetItem(i);
		if (!H->IsControlledByHuman())
		{
			Debug::Log("#%02d: country %25s:\n", i, H->Type->ID);
			const auto& b = H->Base.BaseNodes;
			for (int j = 0; j < b.Count; ++j)
			{
				const auto& n = b[j];
				auto idx = n.BuildingTypeIndex;
				if (idx >= 0)
				{
					auto lbl = BuildingTypeClass::Array->GetItem(idx)->ID;
					Debug::Log("\tNode #%03d: %s @ (%05d, %05d), Attempts so far: %d, Placed: %d\n"
						, j, lbl, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
				}
				else
				{
					Debug::Log("\tNode #%03d: Special %d @ (%05d, %05d), Attempts so far: %d, Placed: %d\n"
						, j, idx, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
				}
			}
			Debug::Log("\n");
		}
	}

	MessageListClass::Instance->PrintMessage(L"Dumped AI Base Plan");
}

