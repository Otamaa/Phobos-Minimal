#include "AircraftPutData.h"
void AircraftPutData::Read(INI_EX& parser, const char* pSection)
{
	PosOffset.Read(parser, pSection, "NoHelipadPutOffset");
	ForceOffset.Read(parser, pSection, "ForcePutOffset");
	RemoveIfNoDocks.Read(parser, pSection, "RemoveIfNoDocks");
}
