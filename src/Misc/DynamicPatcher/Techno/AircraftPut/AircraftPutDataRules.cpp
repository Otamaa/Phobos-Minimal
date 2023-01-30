#include "AircraftPutDataRules.h"
#ifdef COMPILE_PORTED_DP_FEATURES
void AircraftPutDataRules::Read(INI_EX& parser, const char* pSection)
{
	PosOffset.Read(parser, pSection, "AircraftNoHelipadPutOffset");
	ForceOffset.Read(parser, pSection, "AircraftForcePutOffset");
	RemoveIfNoDocks.Read(parser, pSection, "RemoveIfNoDocks");
}
#endif