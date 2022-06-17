#include "Body.h"


DEFINE_HOOK(0x47F9BB, CellClass_DrawOverlayType_Wall_Palette, 0x6)
{
	GET(OverlayTypeClass*, pOverlayType, EBX);
	GET(ColorScheme*, pColorScheme, EDX);

	auto pOverlayTypeExt = OverlayTypeExt::ExtMap.Find(pOverlayType);

	Debug::Log_WithBool(false, __FUNCTION__" Called Ext[%x] ! \n" , pOverlayTypeExt);
	auto nConvert = reinterpret_cast<DWORD_PTR>(pOverlayTypeExt->Palette.GetOrDefaultConvert(pColorScheme->LightConvert));
	R->EDX(nConvert);

	return 0x47F9C1;
}

DEFINE_HOOK(0x47F800, CellClass_DrawOverlayType_Dummy_Palette, 0x3)
{
	GET(CellClass*, pThis, ESI);

	Debug::Log_WithBool(false, __FUNCTION__" Called ! \n");
	auto pOverlayType = OverlayTypeClass::Array()->GetItem(pThis->OverlayTypeIndex);
	auto pOverlayTypeExt = OverlayTypeExt::ExtMap.Find(pOverlayType);
	R->EDX(pOverlayTypeExt->Palette.GetOrDefaultConvert(pThis->LightConvert));

	return 0x47F803;
}

DEFINE_HOOK(0x47FB77, CellClass_DrawOverlayType_Others_Palette, 0x3)
{
	GET(CellClass*, pThis, ESI);

	Debug::Log_WithBool(false, __FUNCTION__" Called ! \n");
	auto pOverlayType = OverlayTypeClass::Array()->GetItem(pThis->OverlayTypeIndex);
	auto pOverlayTypeExt = OverlayTypeExt::ExtMap.Find(pOverlayType);
	R->EDX(pOverlayTypeExt->Palette.GetOrDefaultConvert(pThis->LightConvert));

	return 0x47FB7A;
}