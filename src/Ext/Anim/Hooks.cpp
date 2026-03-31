 #include "Body.h"

#include <Utilities/Macro.h>

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/Infantry/Body.h>
#include <Ext/AnimType/Body.h>

#include <Misc/Hooks.Otamaa.h>

 ASMJIT_PATCH(0x4519A2, BuildingClass_UpdateAnim_SetParentBuilding, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	GET(FakeAnimClass*, pAnim, EBP);
	pAnim->_GetExtData()->ParentBuilding = pThis;
	return 0;
}

ASMJIT_PATCH(0x4232E2, AnimClass_DrawIt_AltPalette, 0x6)
{
	enum { SkipGameCode = 0x4232EA  , SetAltPaletteLightConvert = 0x4232F0 };

	GET(FakeAnimClass*, pThis, ESI);

	const auto pTypeExt = pThis->_GetTypeExtData();
	int schemeIndex = RulesExtData::Instance()->AnimRemapDefaultColorScheme;

	if (((pTypeExt->CreateUnitType && pTypeExt->CreateUnitType->RemapAnim.Get(pTypeExt->RemapAnim)) || pTypeExt->RemapAnim) && pThis->Owner) {
		schemeIndex = pThis->Owner->ColorSchemeIndex - 1;
	}

	schemeIndex += pTypeExt->AltPalette_ApplyLighting ? 1 : 0;
	R->ECX(ColorScheme::Array->Items[schemeIndex]);

	return SkipGameCode;
}

ASMJIT_PATCH(0x424C3D, AnimClass_AttachTo_AttachedAnimPosition, 0x6)
{
	enum { SkipGameCode = 0x424C76 };

	GET(FakeAnimClass*, pThis, ESI);

	auto pExt = pThis->_GetTypeExtData();

	if (pExt->AttachedAnimPosition != AttachedAnimPosition::Default)
	{
		pThis->SetLocation(CoordStruct::Empty);
		return SkipGameCode;
	}

	return 0;
}

ASMJIT_PATCH(0x424CB0, AnimClass_InWhichLayer_AttachedObjectLayer, 0x6)
{
	enum { ReturnValue = 0x424CBF };

	GET(FakeAnimClass*, pThis, ECX);

	if(!pThis->Type)
		return 0x0;

	auto pExt = pThis->_GetTypeExtData();

	if (pThis->OwnerObject && pExt->Layer_UseObjectLayer.isset())
	{
		Layer layer = pThis->Type->Layer;

		if (pExt->Layer_UseObjectLayer.Get())
			layer = pThis->OwnerObject->InWhichLayer();

		R->EAX(layer);

		return ReturnValue;
	}

	return 0;
}

ASMJIT_PATCH(0x423365, AnimClass_DrawIt_ExtraShadow, 0x8)
{
	enum { DrawShadow = 0x42336D, SkipDrawShadow = 0x4233EE };

	GET(FakeAnimClass*, pThis, ESI);

	if (!pThis->Type->Shadow)
		return SkipDrawShadow;

	const bool hasExtra = R->AL();

	return hasExtra && pThis->_GetTypeExtData()->ExtraShadow ?
		DrawShadow : SkipDrawShadow;
}
