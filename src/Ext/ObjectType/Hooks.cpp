#include "Body.h"
#include <Misc/ImageSwapModules.h>

#include <Ext/House/Body.h>

#include <Helpers/Macro.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <Phobos.h>

ASMJIT_PATCH(0x5F9652, ObjectTypeClass_GetAplha, 0x6)
{
	GET(ObjectTypeClass*, pThis, EBX);
	R->CL(pThis->AlphaImageFile[0] && strlen(pThis->AlphaImageFile));
	return 0x5F9658;
}

ASMJIT_PATCH(0x524734, InfantryTypeClass_ReadINI, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(InfantryTypeClass*, pType, ESI);
		TechnoImageReplacer::Replace(pType);
	}

	return 0;
}

ASMJIT_PATCH(0x747B49, UnitTypeClass_ReadINI, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(UnitTypeClass*, pType, EDI);
		TechnoImageReplacer::Replace(pType);
	}

	return 0;
}

ASMJIT_PATCH(0x74809E, UnitTypeClass_Load, 0x9)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(UnitTypeClass*, pType, ESI);
		TechnoImageReplacer::Replace(pType);
	}

	return 0;
}

ASMJIT_PATCH(0x41CD54, AircraftTypeClass_ReadINI, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(AircraftTypeClass*, pType, ESI);
		TechnoImageReplacer::Replace(pType);
	}

	return 0;
}

ASMJIT_PATCH(0x41CE7E, AircraftTypeClass_Load, 0x6)
{
	if (Phobos::Config::ArtImageSwap)
	{
		GET(AircraftTypeClass*, pType, ESI);
		TechnoImageReplacer::Replace(pType);
	}

	return 0;
}

// Superseded by DEFINE_FUNCTION_JUMP(LJMP, 0x5F7900, FakeObjectTypeClass::WhoCanBuildMe) below.
// Keeping this here as a reference for the original hook logic.
#if 0
ASMJIT_PATCH(0x5F7900, ObjectTypeClass_FindFactory, 5)
{
	GET(TechnoTypeClass*, pThis, ECX);
	GET_STACK(HouseClass*, pHouse, 0x10);
	GET_STACK(bool, bSkipAircraft, 0x4);
	GET_STACK(bool, bRequirePower, 0x8);
	GET_STACK(bool, bCheckCanBuild, 0xC);

	const auto nBuffer = HouseExtData::HasFactory(
	pHouse,
	pThis,
	bSkipAircraft,
	bRequirePower,
	bCheckCanBuild,
	false);

	R->EAX(nBuffer.first >= NewFactoryState::Available_Alternative ?
		nBuffer.second : nullptr);

	return 0x5F7A89;
}
#endif

DEFINE_FUNCTION_JUMP(VTABLE, 0x7E28FC, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E369C, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E4604, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E49DC, FakeObjectTypeClass::WhoCanBuildMe);


DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB6A4, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7ECCDC, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF36C, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF694, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F013C, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F021C, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F35BC, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F4F6C, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F54EC, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F62AC, FakeObjectTypeClass::WhoCanBuildMe);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7F65DC, FakeObjectTypeClass::WhoCanBuildMe);

DEFINE_FUNCTION_JUMP(LJMP, 0x5F7900, FakeObjectTypeClass::WhoCanBuildMe);

/*
DEFINE_FUNCTION_JUMP(CALL, 0x41CE72, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x42884E, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x46C417, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x46C6E1, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x5249AD, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x5FEB3E, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x6B5891, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x71609C, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x71E211, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x74805A, FakeObjectTypeClass::_LoadVoxel);
DEFINE_FUNCTION_JUMP(CALL, 0x74B8BB, FakeObjectTypeClass::_LoadVoxel);

// Replaces the full ObjectTypeClass::LoadVoxel function (0x5F8110–0x5F8CDB).
// The 4 conflicting hooks inside that range in Hooks.VXLArtData.cpp are disabled.
DEFINE_FUNCTION_JUMP(LJMP, 0x5F8110, FakeObjectTypeClass::_LoadVoxel);*/