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
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DUMP_AI_BASE_PLAN", L"AI Base Plan Logger");
}

const wchar_t* AIBasePlanCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* AIBasePlanCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissingNoChecks("TXT_DUMP_AI_BASE_PLAN_DESC", L"Dumps the AI Base Plans to the log");
}

void AIBasePlanCommandClass::Execute(WWKey dwUnk) const
{
	Debug::LogInfo("AI Base Plans:");
	for (int i = 0; i < HouseClass::Array->Count; ++i)
	{
		auto H = HouseClass::Array->Items[i];
		if (!H->IsControlledByHuman())
		{
			Debug::LogInfo("#{}: country {}:", i, H->Type->ID);
			const auto& b = H->Base.BaseNodes;
			for (int j = 0; j < b.Count; ++j)
			{
				const auto& n = b[j];
				auto idx = n.BuildingTypeIndex;
				if (idx >= 0)
				{
					auto lbl = BuildingTypeClass::Array->Items[idx]->ID;
					Debug::LogInfo("\tNode #{}: {} @ ({}, {}), Attempts so far: {}, Placed: {}"
						, j, lbl, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
				}
				else
				{
					Debug::LogInfo("\tNode #{}: Special {} @ ({}, {}), Attempts so far: {}, Placed: {}"
						, j, idx, n.MapCoords.X, n.MapCoords.Y, n.Attempts, n.Placed);
				}
			}
			Debug::LogInfo("");
		}
	}

	MessageListClass::Instance->PrintMessage(L"Dumped AI Base Plan");
}

