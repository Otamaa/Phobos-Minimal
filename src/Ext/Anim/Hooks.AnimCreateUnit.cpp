// Anim-to--Unit
// Author: Otamaa

#include "Body.h"

#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/House/Body.h>

ASMJIT_PATCH(0x423BC8, AnimClass_Update_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	AnimTypeExtData::CreateUnit_MarkCell(pThis);
	return 0;
}

ASMJIT_PATCH(0x424932, AnimClass_Update_CreateUnit_ActualAffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);
	AnimTypeExtData::CreateUnit_Spawn(pThis);
	return 0;
}
