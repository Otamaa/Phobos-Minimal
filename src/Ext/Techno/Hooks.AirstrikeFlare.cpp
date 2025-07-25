#include "Body.h"
#include <Ext/TechnoType/Body.h>
#include <Utilities/Macro.h>

static TechnoTypeExtData* TechnoClass_DrawAirstrikeFlare_pType {};

ASMJIT_PATCH(0x705860, TechnoClass_DrawAirstrikeFlare_SetContext, 0x8)
{
	GET(TechnoClass*, pThis, ECX);

	// This is not used in vanilla function so ECX gets overwritten later.
	TechnoClass_DrawAirstrikeFlare_pType = TechnoTypeExtContainer::Instance.Find(pThis->GetTechnoType());

	return 0;
}

ASMJIT_PATCH(0x7058F6, TechnoClass_DrawAirstrikeFlare, 0x5)
{
	enum { SkipGameCode = 0x705976 };

	GET(int, zSrc, EBP);
	GET(int, zDest, EBX);
	REF_STACK(ColorStruct, color, STACK_OFFSET(0x70, -0x60));

	// Fix depth buffer value.
	const int zValue = MinImpl(zSrc, zDest)+ RulesExtData::Instance()->AirstrikeLineZAdjust;
	R->EBP(zValue);
	R->EBX(zValue);

	// Allow custom colors.
	auto const baseColor = TechnoClass_DrawAirstrikeFlare_pType->AirstrikeLineColor.Get(RulesExtData::Instance()->AirstrikeLineColor);
	double percentage = Random2Class::Global->RandomRanged(745, 1000) / 1000.0;
	color = { (BYTE)(baseColor.R * percentage), (BYTE)(baseColor.G * percentage), (BYTE)(baseColor.B * percentage) };
	R->ESI(Drawing::RGB_To_Int(baseColor));

	return SkipGameCode;
}

// Always draw the dot and skip setting color, it is already done in previous hook.
ASMJIT_PATCH(0x70597A, TechnoClass_DrawAirstrikeFlare_DotColor, 0x6)
{
	GET_STACK(int, xCoord, STACK_OFFSET(0x70, -0x38));

	// Restore overridden instructions.
	R->ECX(xCoord);
	return 0x7059C7;
}