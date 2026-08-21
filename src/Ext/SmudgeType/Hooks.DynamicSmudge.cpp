#include "Body.h"

#include <Ext/Cell/Body.h>
#include <Ext/Scenario/Body.h>

ASMJIT_PATCH(0x6B60DE, SmudgeTypeClass_Mark_SetContext, 0x6)
{
	GET(CellClass* const, pCell, EAX);

	if(auto pSmudge = SmudgeTypeClass::Array->get_or_default(pCell->SmudgeTypeIndex)){
		if (!SmudgeTypeExtContainer::Instance.Find(pSmudge)->Clearable)
			return 0x0;

		ScenarioExtData::Instance()->Smudges.insert(MapClass::GetCellIndex(pCell->MapCoords));
		const auto pCellExt = CellExtContainer::Instance.Find(pCell);
		pCellExt->SmudgeGenerate = Unsorted::CurrentFrame();
		pCellExt->SmudgeState = BlitterFlags::None;
	}
	return 0;
}

ASMJIT_PATCH(0x6B56AC, SmudgeTypeClass_DrawIt_DrawTrans, 0x5)
{
	GET(CellClass* const, pCell, ESI);
	REF_STACK(BlitterFlags, flags, STACK_OFFSET(0x3C, -0x3C));

	flags |= CellExtContainer::Instance.Find(pCell)->SmudgeState;

	return 0;
}


