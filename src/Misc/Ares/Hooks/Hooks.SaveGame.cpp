#include "Header.h"

#include <ScenarioClass.h>
#include <LoadOptionsClass.h>

#include <Helpers/Macro.h>

ASMJIT_PATCH(0x74fdc0, GetModuleVersion, 5)
{
	R->EAX("Ares r21.352.1218");
	return 0x74FEEF;
}

ASMJIT_PATCH(0x74fae0, GetModuleInternalVersion, 5)
{
	R->EAX("1.001/Ares 3.0p1");
	return 0x74FC7B;
}