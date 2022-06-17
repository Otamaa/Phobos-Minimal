#include "Body.h"

static std::string TempName;

DEFINE_HOOK(0x72ADE7, LoadPaletteFromName_GetFileName, 0x5)
{
	GET(const char*, pName, ECX);
	TempName = pName;
	return 0x0;
}

DEFINE_HOOK(0x72AED4, LoadPaletteFromName_InjectName, 0x9)
{
	GET(ConvertClass* const, pConvert, EAX);
	//LEA_STACK(CCFileClass* , nFile, STACK_OFFS(0x80, 0x6C));

	ConvertExt::GetOrSetName(pConvert, TempName);
	return 0x0;
}

DEFINE_HOOK(0x72AEDD, LoadPaletteFromName_GetFileName_Remove, 0xC)
{
	TempName.clear();
	return 0x0;
}

/*
DEFINE_HOOK(0x52BE3B, GameInit_VoxelDrawer, 0x7)
{
	GET(ConvertClass* const, pConvert, EAX);
	ConvertExt::GetOrSetName(pConvert, "VoxelDrawer");
	return 0x0;
}

DEFINE_HOOK(0x52BE3B, GameInit_AnimPal, 0x7)
{
	GET(ConvertClass* const, pConvert, EAX);
	ConvertExt::GetOrSetName(pConvert, reinterpret_cast<const char*>(0x8260A0));

	return 0x0;
}

DEFINE_HOOK(0x52BFC1, GameInit_PalettePal, 0x7)
{
	GET(ConvertClass* const, pConvert, EAX);
	ConvertExt::GetOrSetName(pConvert, reinterpret_cast<const char*>(0x826094));

	return 0x0;
}
*/
//52C075 , UnitSno 8260D8
// 52C129, Cameo, 8204E0
// 52C1DD,Mouse , 826084
// 52C2BB, GraphicEffectText , 826078