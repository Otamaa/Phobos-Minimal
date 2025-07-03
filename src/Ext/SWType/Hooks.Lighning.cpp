#include "Body.h"

#include <ScenarioClass.h>
#include <ConvertClass.h>
#include <BuildingLightClass.h>

#include <Utilities/Macro.h>

ASMJIT_PATCH(0x555E50, LightConvertClass_CTOR_Lighting, 5)
{
	GET(LightConvertClass*, pThis, ESI);

	auto lighting = SWTypeExtData::GetLightingColor();

	if (lighting.HasValue)
	{
		if (pThis->Color1.Red == -1)
		{
			pThis->Color1.Red = 1000;
			pThis->Color1.Green = 1000;
			pThis->Color1.Blue = 1000;
		}
	}
	else
	{
		lighting.Red = pThis->Color1.Red;
		lighting.Green = pThis->Color1.Green;
		lighting.Blue = pThis->Color1.Blue;
	}

	pThis->UpdateColors(lighting.Red, lighting.Green, lighting.Blue, lighting.HasValue);

	return 0x55606C;
}

// replace entire function
ASMJIT_PATCH(0x53C280, ScenarioClass_UpdateLighting, 5)
{
	const auto lighting = SWTypeExtData::GetLightingColor();

	if (lighting.HasValue)
	{
		// something changed the lighting
		ScenarioClass::Instance->AmbientTarget = lighting.Ambient;
		ScenarioClass::Instance->RecalcLighting(lighting.Red, lighting.Green, lighting.Blue, true);
	}
	else
	{
		// default lighting
		ScenarioClass::Instance->AmbientTarget = ScenarioClass::Instance->AmbientOriginal;
		ScenarioClass::Instance->RecalcLighting(-1, -1, -1, false);
	}

	return 0x53C441;
}