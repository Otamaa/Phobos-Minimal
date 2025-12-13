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
//  ASMJIT_PATCH(0x71E940, TEventClass_Execute, 0x5)
//  {
//  	GET(TEventClass*, pThis, ECX);
//  	REF_STACK(EventArgs const, args, 0x4);
//  	enum { return_value = 0x71EA2D , continue_check = 0x0 };
//
//  	bool result = false;
//  	if (TEventExtData::Occured(pThis, args, result))
//  	{
//  		R->AL(result);
//  		return return_value;
//  	}
//
//  	return continue_check; // will continue ares and vanilla checks
//  }
