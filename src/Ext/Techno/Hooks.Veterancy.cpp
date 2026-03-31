#include "Body.h"

#include <FootClass.h>
#include <UnitClass.h>
#include <Utilities/Macro.h>
#include <Helpers/Macro.h>
#include <Base/Always.h>

#include <HouseClass.h>
#include <Utilities/Debug.h>

#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/VoxelAnim/Body.h>

#include <New/Type/ArmorTypeClass.h>

#include <Notifications.h>

#include <Helpers/Enumerators.h>

// #346, #464, #970, #1014
// handle all veterancy gains ourselves
ASMJIT_PATCH(0x702E9D, TechnoClass_RegisterDestruction_Veterancy, 0x6)
{

	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pThis, ESI);
	GET(const int, Score, EBP);

	// get the unit that receives veterancy
	TechnoClass* pExpReceiver = nullptr;
	double ExpFactor = 1.0;
	bool promoteImmediately = false;

	// this replace Killer with Airstrike Designator
	TechnoExtData::AddAirstrikeFactor(pKiller, ExpFactor);

	// this evalueate ExpReceiver with various Killer properties also update the Expfactor
	TechnoExtData::EvaluateExtReceiverData(pExpReceiver, pKiller, ExpFactor, promoteImmediately);

	// update the veterancy
	TechnoExtData::UpdateVeterancy(pExpReceiver, pKiller, pThis, Score, ExpFactor, promoteImmediately);

	// skip the entire veterancy handling
	return 0x702FF5;
}
