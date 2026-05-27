#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/AircraftType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Bullet/Body.h>

#include <Locomotor/FlyLocomotionClass.h>

DEFINE_FUNCTION_JUMP(CALL, 0x4CD809, FakeAircraftClass::_Destroyed);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2504, FakeAircraftClass::_Mission_ParadropApproach)
DEFINE_FUNCTION_JUMP(LJMP, 0x4158E0, FakeAircraftClass::_Mission_ParadropApproach)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2508, FakeAircraftClass::_Mission_ParadropOverfly)
DEFINE_FUNCTION_JUMP(LJMP, 0x415960, FakeAircraftClass::_Mission_ParadropOverfly)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E24A8, FakeAircraftClass::_Mission_Sleep)
DEFINE_FUNCTION_JUMP(LJMP, 0x5B2E10, FakeAircraftClass::_Mission_Sleep)
DEFINE_FUNCTION_JUMP(LJMP, 0x416D50, FakeAircraftClass::_Mission_Move_ForCarryAll);
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2510, FakeAircraftClass::_Mission_SpyPlaneApproach)
DEFINE_FUNCTION_JUMP(LJMP, 0x4155F0, FakeAircraftClass::_Mission_SpyPlaneApproach)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E2514, FakeAircraftClass::_Mission_SpyPlaneOverfly)
DEFINE_FUNCTION_JUMP(LJMP, 0x4157C0, FakeAircraftClass::_Mission_SpyPlaneOverfly)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7E24D0, FakeAircraftClass::_Mission_Move)
DEFINE_FUNCTION_JUMP(LJMP, 0x4166C0, FakeAircraftClass::_Mission_Move)

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
