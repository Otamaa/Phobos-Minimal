#include "ImageSwapModules.h"

#include <Helpers/Macro.h>
#include <Phobos.h>

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