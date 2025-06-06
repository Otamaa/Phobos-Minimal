#include "Body.h"

#include <Utilities/Macro.h>


//ToDo : Overlap , better render code ?
//https://github.com/Phobos-developers/Phobos/issues/448

ASMJIT_PATCH(0x588B23, MapClass_ClearSmudge_Clearable, 0x5)
{
	GET(SmudgeTypeClass*, pSmudge, EBX);
	return !SmudgeTypeExtContainer::Instance.Find(pSmudge)->Clearable.Get() ? 0x588BCA : 0x0;
}

ASMJIT_PATCH(0x43F325, BuildingClass_Mark_ClearSmudge_Clearable, 0x5)
{
	GET(SmudgeTypeClass*, pSmudge, EBP);
	return !SmudgeTypeExtContainer::Instance.Find(pSmudge)->Clearable.Get() ? 0x43F3FB : 0x0;
}

ASMJIT_PATCH(0x43F82C , BuildingClass_Mark_ClearSmudge_Clearable_B ,0x5)
{
	GET(SmudgeTypeClass*, pSmudge, EBP);
	return !SmudgeTypeExtContainer::Instance.Find(pSmudge)->Clearable.Get() ? 0x43F8F6 : 0x0;
}