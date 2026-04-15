#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>

#include <Locomotor/FlyLocomotionClass.h>

#pragma region Otamaa

DEFINE_FUNCTION_JUMP(CALL, 0x4CD809, FakeAircraftClass::_Destroyed);

ASMJIT_PATCH(0x415991, AircraftClass_Mission_Paradrop_Overfly_Radius, 0x6)
{
	enum { ConditionMeet = 0x41599F, ConditionFailed = 0x4159C8 };

	GET(AircraftClass* const, pThis, ESI);
	GET(int, comparator, EAX);

	const int nRadius = TechnoTypeExtContainer::Instance.Find(pThis->Type)->ParadropOverflRadius.Get(RulesClass::Instance->ParadropRadius);
	return comparator > nRadius ? ConditionMeet : ConditionFailed;
}

ASMJIT_PATCH(0x415934, AircraftClass_Mission_Paradrop_Approach_Radius, 0x6)
{
	enum { ConditionMeet = 0x415942, ConditionFailed = 0x415956 };

	GET(AircraftClass* const, pThis, ESI);
	GET(int, comparator, EAX);

	const int nRadius = TechnoTypeExtContainer::Instance.Find(pThis->Type)->ParadropRadius.Get(RulesClass::Instance->ParadropRadius);
	return  comparator <= nRadius ? ConditionMeet : ConditionFailed;
}

ASMJIT_PATCH(0x417A2E, AircraftClass_EnterIdleMode_Opentopped, 0x5)
{
	GET(AircraftClass*, pThis, ESI);

	R->EDI(2);

	//this plane stuck on mission::Move ! so letst redirect it to other address that deal with this
	return !pThis->Spawned &&
		pThis->Type->OpenTopped &&
		(pThis->QueuedMission != Mission::Attack) && !pThis->Target
		? 0x417944 : 0x417AD4;
}

ASMJIT_PATCH(0x416EC9, AircraftClass_MI_Move_Carryall_AllowWater, 0x6) //was 8
{
	GET(AircraftClass*, pCarryall, ESI);

	AbstractClass* const pDest = AircraftExtData::IsValidLandingZone(pCarryall) ?
		pCarryall->Destination : pCarryall->NewLandingZone_(pCarryall->Destination);

	pCarryall->SetDestination(pDest, true);
	return 0x416EE4;
}

ASMJIT_PATCH(0x416FFD, AircraftClass_MI_Move_Carryall_AllowWater_LZClear, 0x6) //5
{
	GET(AircraftClass*, pThis, ESI);

	if (AircraftExtData::IsValidLandingZone(pThis)) {
		R->AL(true);
	} else {
		R->AL(pThis->IsLandingZoneClear(pThis->Destination));
	}

	return 0x41700E;
}

#pragma endregion