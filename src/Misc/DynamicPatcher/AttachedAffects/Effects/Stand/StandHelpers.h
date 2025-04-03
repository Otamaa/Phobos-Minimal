#pragma once

#include <Misc/Otamaa/Misc/DynamicPatcher/Helpers/Helpers.h>
#include <GeneralStructures.h>
#include "../../LocationMark.h"

class Stand;
struct AttachEffectManager;
struct StandType;
struct StandHelper
{
	static void UpdateStandLocation(AttachEffectManager* manager, ObjectClass* pObject, Stand* stand, int& markIndex);
	static LocationMark GetLocation(ObjectClass* pObject, StandType* standType , bool createdInthespot = false);
	static DirStruct GetDirection(TechnoClass* pMaster, int dir, bool isOnTurret);
};
