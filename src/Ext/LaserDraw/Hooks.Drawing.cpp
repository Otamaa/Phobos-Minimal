#include <Helpers/Macro.h>

#include <LaserDrawClass.h>
#include <GeneralStructures.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>
#include <Misc/PhobosGlobal.h>

ColorStruct maxColor;

DEFINE_HOOK(0x550D1F, LaserDrawClass_DrawInHouseColor_Context_Set, 0x6)
{
	LEA_STACK(ColorStruct*, pColor, 0x14);
	maxColor = *pColor;
	return 0;
}

//Enables proper laser thickness and falloff of it
DEFINE_HOOK(0x550F47, LaserDrawClass_DrawInHouseColor_BetterDrawing, 0x5) //0
{
	// Restore overridden code that's needed - Kerbiter
	GET_STACK(bool, noQuickDraw, 0x13);
	R->ESI(noQuickDraw ? 8u : 64u);

	GET(LaserDrawClass*, pThis, EBX);
	GET_STACK(int, currentThickness, 0x5C)

	double mult = 1.0;
	if (pThis->Thickness > 1) {
		double falloffStep = 1.0 / pThis->Thickness;
		double falloffMult = GeneralUtils::SecsomeFastPow(1.0 - falloffStep, currentThickness);
		mult = (1.0 - falloffStep * currentThickness) * falloffMult;
	}

	R->EAX((unsigned int)(mult * maxColor.R));
	R->ECX((unsigned int)(mult * maxColor.G));
	R->EDX((unsigned int)(mult * maxColor.B));

	return 0x550F9D;
}
