#include "BuildSpeedBonus.h"

void BuildSpeedBonus::Read(INI_EX& parser, const char* pSection)
{
	Nullable<double> nBuff {};
	nBuff.Read(parser, pSection, "BuildSpeedBonus.Aircraft");

	if (nBuff.isset() && nBuff.Get() != 0.000)
	{
		Enabled = true;
		SpeedBonus_Aircraft = nBuff.Get();
	}

	nBuff.Reset();
	nBuff.Read(parser, pSection, "BuildSpeedBonus.Building");

	if (nBuff.isset() && nBuff.Get() != 0.000)
	{
		Enabled = true;
		SpeedBonus_Building = nBuff.Get();
	}

	nBuff.Reset();
	nBuff.Read(parser, pSection, "BuildSpeedBonus.Infantry");

	if (nBuff.isset() && nBuff.Get() != 0.000)
	{
		Enabled = true;
		SpeedBonus_Infantry = nBuff.Get();
	}

	nBuff.Reset();
	nBuff.Read(parser, pSection, "BuildSpeedBonus.Unit");

	if (nBuff.isset() && nBuff.Get() != 0.000)
	{
		Enabled = true;
		SpeedBonus_Unit = nBuff.Get();
	}

	if (Enabled)
		AffectedType.Read(parser, pSection, "BuildSpeedBonus.AffectedTypes");
}
