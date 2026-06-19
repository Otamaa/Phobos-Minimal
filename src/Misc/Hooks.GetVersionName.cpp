
#include <ScenarioClass.h>
#include <LoadOptionsClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

#include <VersionClass.h>

const char* __fastcall Version_Name()
{
	return "Ares r21.352.1218";
}
DEFINE_FUNCTION_JUMP(LJMP, 0x74fdc0, Version_Name)

const char* __fastcall GetModule_InternalVersion_Name(VersionClass*)
{
	return "1.001/Ares 3.0p1";
}
DEFINE_FUNCTION_JUMP(LJMP, 0x74fae0, GetModule_InternalVersion_Name)