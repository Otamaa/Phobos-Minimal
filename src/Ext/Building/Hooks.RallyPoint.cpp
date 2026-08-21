#include "Body.h"
#include <Ext/BuildingType/Body.h>

#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <TacticalClass.h>

bool FakeBuildingClass::__CanDrawRallyPoint()
{
	auto pTypeExt = (BuildingTypeExtData*)this->_GetExtData()->TypeExtData;
	return pTypeExt->HaveRallyPoint.Get(this->IsUnitFactory());
}
DEFINE_FUNCTION_JUMP(CALL6, 0x6DAAA4, FakeBuildingClass::__CanDrawRallyPoint)
DEFINE_FUNCTION_JUMP(CALL6, 0x455D5F, FakeBuildingClass::__CanDrawRallyPoint)
DEFINE_FUNCTION_JUMP(CALL6, 0x44F5D5, FakeBuildingClass::__CanDrawRallyPoint)
DEFINE_FUNCTION_JUMP(CALL6, 0x44774F, FakeBuildingClass::__CanDrawRallyPoint)
DEFINE_FUNCTION_JUMP(CALL6, 0x447635, FakeBuildingClass::__CanDrawRallyPoint)

bool __fastcall _CanRally(TechnoClass* pThis) {
	if (auto pBld = cast_to<FakeBuildingClass*, false>(pThis))
		return pBld->__CanDrawRallyPoint();

	return pThis->IsUnitFactory();
}
DEFINE_FUNCTION_JUMP(CALL6, 0x700B1E, _CanRally)

ASMJIT_PATCH(0x44748E, BuildingClass_MouseOverObject_RallyPointAircraft, 0x6)
{
	enum { AllowRally = 0x44749D , ForbidRally = 0x44750D };

	GET(BuildingClass* const, pThis, ESI);
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->HaveRallyPoint.Get(pThis->Type->Factory != AbstractType::AircraftType) ? AllowRally : ForbidRally;
}

ASMJIT_PATCH(0x447674, BuildingClass_MouseOverCell_RallyPoint1, 0x6)
{
	enum { AllowRally = 0x447683, ForbidRally = 0x4476F3 };

	GET(BuildingClass* const, pThis, ESI);
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->HaveRallyPoint.Get(pThis->Type->Factory != AbstractType::AircraftType) ? AllowRally : ForbidRally;
}

ASMJIT_PATCH(0x443627, BuildingClass_ActiveClickWith_ActionNType_RallyPoint, 0x6)
{
	enum { AllowRally = 0x44368D, ForbidRally = 0x44363C };
	GET(BuildingClass* const, pThis, EBX);
	const auto toBuild = pThis->Type->Factory;
	return BuildingTypeExtContainer::Instance.Find(pThis->Type)->HaveRallyPoint.Get(toBuild == AbstractType ::Aircraft || toBuild ==  AbstractType::UnitType ||toBuild == AbstractType::AircraftType) ? AllowRally : ForbidRally;
}

ASMJIT_PATCH(0x4473FA, BuildingClass_WhatActionNType_RallyPoint, 0x6)
{
	enum { AllowRally = 0x447413, ForbidRally = 0x44750D };
	GET(BuildingClass* const, pThis, ESI);
	const auto toBuild = pThis->Type->Factory;

	const bool _allowRally = BuildingTypeExtContainer::Instance.Find(pThis->Type)->HaveRallyPoint.Get(toBuild == AbstractType::Aircraft || toBuild == AbstractType::UnitType || toBuild == AbstractType::AircraftType);

	return _allowRally ? AllowRally : ForbidRally;
}

ASMJIT_PATCH(0x4438C9, BuildingClass_SetRallyPoint_PathFinding, 0x6)
{
	GET(BuildingClass* const, pThis, EBP);
	GET(MovementZone, movementzone, ESI);
	GET_STACK(SpeedType, speedtype, STACK_OFFSET(0xA4, -0x84));

	auto const pExt = BuildingTypeExtContainer::Instance.Find(pThis->Type);
	R->ESI(pExt->RallyPointMovementZone.Get(movementzone));
	R->Stack(STACK_OFFSET(0xA4, -0x84), pExt->RallyPointSpeedType.Get(speedtype));

	return 0;
}