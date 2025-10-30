
#include "Body.h"
#include <Ext/TechnoType/Body.h>

TechnoClass* Sub_705860_Aitrstrike_pThis;
int Sub_705860_Aitrstrike_Color;

ASMJIT_PATCH(0x705860, Sub_705860_AitrstrikeTargetLaser_FetchECX, 0x8)
{
	Sub_705860_Aitrstrike_pThis = R->ECX<TechnoClass*>();
	return 0;
}

ASMJIT_PATCH(0x7058F6, Sub_705860_AitrstrikeTargetLaser, 0x5)
{
	GET(int, zSrc, EBP);
	GET(int, zDest, EBX);
	REF_STACK(ColorStruct, color, STACK_OFFSET(0x70, -0x60));

	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(Sub_705860_Aitrstrike_pThis->Airstrike->Owner->GetTechnoType());
	const int colorIndex = pTypeExt->LaserTargetColor.Get(RulesClass::Instance->LaserTargetColor);
	Sub_705860_Aitrstrike_Color = GeneralUtils::GetColorFromColorAdd(colorIndex);
	color = Drawing::Int_To_RGB(Sub_705860_Aitrstrike_Color);

	return 0x705976;
}

ASMJIT_PATCH(0x705986, Sub_705860_AitrstrikeTargetPoint, 0x6)
{
	enum { SkipGameCode = 0x7059C7 };

	R->ECX(R->Stack<DWORD>(STACK_OFFSET(0x70, -0x38)));
	R->ESI(Sub_705860_Aitrstrike_Color);

	return SkipGameCode;
}

// ASMJIT_PATCH(0x43D39C, BuildingClass_Draw_LaserTargetColor, 0x6)
// {
// 	enum { SkipGameCode = 0x43D3A2 };

// 	GET(BuildingClass*, pThis, ESI);
// 	GET(RulesClass*, pRules, ECX);
// 	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Airstrike->Owner->GetTechnoType());
// 	R->EAX(pTypeExt->LaserTargetColor.Get(pRules->LaserTargetColor));

// 	return SkipGameCode;
// }

ASMJIT_PATCH(0x43DC36, BuildingClass_DrawFogged_LaserTargetColor, 0x6)
{
	enum { SkipGameCode = 0x43DC3C };

	GET(BuildingClass*, pThis, EBP);
	GET(RulesClass*, pRules, ECX);
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Airstrike->Owner->GetTechnoType());
	R->EAX(pTypeExt->LaserTargetColor.Get(pRules->LaserTargetColor));

	return SkipGameCode;
}

ASMJIT_PATCH(0x42343C, AnimClass_Draw_LaserTargetColor, 0x6)
{
	enum { SkipGameCode = 0x423448 };

	GET(BuildingClass*, pThis, ECX);
	const auto pRules = RulesClass::Instance();
	const auto pTypeExt = TechnoTypeExtContainer::Instance.Find(pThis->Airstrike->Owner->GetTechnoType());
	R->ECX(pRules);
	R->EAX(pTypeExt->LaserTargetColor.Get(pRules->LaserTargetColor));

	return SkipGameCode;
}