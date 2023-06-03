#include "Passengers.h"

void PassengersData::Read(INI_EX& parser, const char* pSection, bool Allocate)
{
	PassiveAcquire.Read(parser, pSection, "Passengers.PassiveAcquire");
	ForceFire.Read(parser, pSection, "Passengers.ForceFire");
	MobileFire.Read(parser, pSection, "Passengers.MobileFire");
	SameFire.Read(parser, pSection, "Passengers.SameFire");
}
