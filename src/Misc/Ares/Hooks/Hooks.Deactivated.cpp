#include <Ext/Techno/Body.h>

DEFINE_OVERRIDE_HOOK(0x417F83, AircraftClass_GetActionOnCell_Deactivated, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	if (pThis->Deactivated) {
		R->EAX(Action::None);
		return 0x417F94;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D7D58, FootClass_ActionOnCell_Deactivated, 0x6)
{
	GET(FootClass*, pThis, ESI);
	return (pThis->Deactivated)
		? 0x4D7D62
		: 0
		;
}

DEFINE_OVERRIDE_HOOK(0x4436F7, BuildingClass_ActionOnCell_Deactivated, 0x5)
{
	GET(BuildingClass*, pThis, ECX);
	return (pThis->Deactivated)
		? 0x443729 : 0 ;
}

DEFINE_OVERRIDE_HOOK(0x447548, BuildingClass_GetActionOnCell_Deactivated, 0x6)
{
	GET(BuildingClass*, pThis, ESI);

	if (pThis->Deactivated) {
		R->EBX(Action::None);
		return 0x44776D;
	}

	return 0;
}

DEFINE_OVERRIDE_HOOK(0x4D74EC, FootClass_ActionOnObject_Deactivated, 0x6)
{
	GET(FootClass*, pThis, ESI);
	GET_STACK(Action, nAction, 0x10C);

	return (nAction != Action::Detonate && (pThis->Deactivated || pThis->Berzerk))
		? 0x4D77EC
		: 0x4D74FA
		;
}

DEFINE_OVERRIDE_HOOK(0x51F808, InfantryClass_GetActionOnCell_Deactivated, 0x6)
{
	GET(InfantryClass*, pThis, EDI);
	if (pThis->Deactivated)
	{
		R->EBX(Action::None);
		return 0x51FAE2;
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x51D0DD, InfantryClass_Scatter_Deactivated, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	return (pThis->Deactivated)
		? 0x51D6E6
		: 0
		;
}

DEFINE_OVERRIDE_HOOK(0x7404B9, UnitClass_GetCursorOverCell_Deactivated, 6)
{
	GET(UnitClass*, pThis, ESI);
	if (pThis->Deactivated) {
		R->EAX(Action::None);
		return 0x740805;
	}
	return 0;
}

DEFINE_OVERRIDE_HOOK(0x5200B3, InfantryClass_UpdatePanic_Deactivated, 6)
{
	GET(InfantryClass*, pThis, ESI);
	if (pThis->Deactivated) {
		if (pThis->PanicDurationLeft > 0) {
			--pThis->PanicDurationLeft;
		}
		return 0x52025A;
	}
	return 0;
}