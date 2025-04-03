#include "Body.h"

#include <Helpers\Macro.h>

#include <HouseClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <OverlayTypeClass.h>
#include <VocClass.h>

#include <Utilities/Macro.h>

// we hook on the very first call
// ares doing it before the switch statement call
 ASMJIT_PATCH(0x71E940, TEventClass_Execute, 0x5)
 {
 	GET(TEventClass*, pThis, ECX);
 	REF_STACK(EventArgs const, args, 0x4);
 	enum { return_value = 0x71EA2D , continue_check = 0x0 };

 	bool result = false;
 	if (TEventExtData::Occured(pThis, args, result))
 	{
 		R->AL(result);
 		return return_value;
 	}

 	return continue_check; // will continue ares and vanilla checks
 }

ASMJIT_PATCH(0x7271F9, TEventClass_GetFlags, 0x5)
{
	GET(TriggerAttachType, eAttach, EAX);
	GET(TEventClass*, pThis, ESI);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= (int)PhobosTriggerEvent::LocalVariableGreaterThan && nEvent <= (int)PhobosTriggerEvent::ShieldBroken)
		eAttach |= TriggerAttachType::Logic; // LOGIC

	R->EAX(eAttach);

	return 0;
}

ASMJIT_PATCH(0x71F3FE, TEventClass_BuildINIEntry, 0x5)
{
	GET(LogicNeedType, eNeedType, EAX);
	GET(TEventClass*, pThis, ECX);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= (int)PhobosTriggerEvent::LocalVariableGreaterThan && nEvent <= (int)PhobosTriggerEvent::ShieldBroken)
		eNeedType = LogicNeedType::NumberNTech;

	R->EAX(eNeedType);

	return 0;
}

ASMJIT_PATCH(0x726577, TEventClass_Persistable, 0x7)
{
	GET(TEventClass*, pThis, EDI);

	int nEvent = static_cast<int>(pThis->EventKind);
	if (nEvent >= (int)PhobosTriggerEvent::LocalVariableGreaterThan && nEvent <= (int)PhobosTriggerEvent::ShieldBroken)
		R->AL(true);
	else
		R->AL(pThis->GetStateB());

	return 0x72657E;
}
