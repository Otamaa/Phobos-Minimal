#include "AttachFireData.h"
void AttachFireData::Read(INI_EX& parser, const char* pSection)
{
	UseROF.Read(parser, pSection, "AttachFire.UseROF");
	CheckRange.Read(parser, pSection, "AttachFire.CheckRange");
	RadialFire.Read(parser, pSection, "AttachFire.RadialFire");
	RadialAngle.Read(parser, pSection, "AttachFire.RadialAngle");
	SimulateBurst.Read(parser, pSection, "AttachFire.SimulateBurst");
	SimulateBurstDelay.Read(parser, pSection, "AttachFire.SimulateBurstDelay");
	SimulateBurstMode.Read(parser, pSection, "AttachFire.SimulateBurstMode");
	OnlyFireInTransport.Read(parser, pSection, "AttachFire.OnlyFireInTransport");
	UseAlternateFLH.Read(parser, pSection, "AttachFire.UseAlternateFLH");
}
