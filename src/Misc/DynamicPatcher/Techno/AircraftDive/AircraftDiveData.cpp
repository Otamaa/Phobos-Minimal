#include "AircraftDiveData.h"
void AircraftDiveData::Read(INI_EX& parser, const char* pSection, bool Allocate)
{
	Enable.Read(parser, pSection, "Dive");
	Distance.Read(parser, pSection, "Dive.Distance");
	Speed.Read(parser, pSection, "Dive.Speed");
	Delay.Read(parser, pSection, "Dive.Delay");
	FlightLevel.Read(parser, pSection, "Dive.FlightLevel");
	PullUpAfterFire.Read(parser, pSection, "Dive.PullUpAfterFire");
}
