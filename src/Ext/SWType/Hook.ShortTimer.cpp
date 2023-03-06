#include "Body.h"

DEFINE_HOOK(0x6CB5EB, SuperClass_Grant_ShowTimer, 0x5)
{
	GET(SuperClass*, pThis, ESI);

	if (SuperClass::ShowTimers->AddItem(pThis)) { 
		std::sort(SuperClass::ShowTimers->begin(), SuperClass::ShowTimers->end(),
			[](SuperClass* a, SuperClass* b) {
			const auto aExt = SWTypeExt::ExtMap.Find(a->Type);
			const auto bExt = SWTypeExt::ExtMap.Find(b->Type);
			return aExt->SW_Priority.Get() > bExt->SW_Priority.Get();
		});
	}

	return 0x6CB63E;
}