#include "BuildSpeedBonus.h"

#include <TechnoTypeClass.h>

void BuildSpeedBonus::Read(INI_EX& parser, const char* pSection)
{
	Nullable<double> nBuff {};
	nBuff.Read(parser, pSection, "BuildSpeedBonus.Aircraft");

	if (nBuff.isset() && nBuff.Fetch() != 0.000)
	{
		Enabled = true;
		SpeedBonus_Aircraft = nBuff.Fetch();
	}

	nBuff.Reset();
	nBuff.Read(parser, pSection, "BuildSpeedBonus.Building");

	if (nBuff.isset() && nBuff.Fetch() != 0.000)
	{
		Enabled = true;
		SpeedBonus_Building = nBuff.Fetch();
	}

	nBuff.Reset();
	nBuff.Read(parser, pSection, "BuildSpeedBonus.Infantry");

	if (nBuff.isset() && nBuff.Fetch() != 0.000)
	{
		Enabled = true;
		SpeedBonus_Infantry = nBuff.Fetch();
	}

	nBuff.Reset();
	nBuff.Read(parser, pSection, "BuildSpeedBonus.Unit");

	if (nBuff.isset() && nBuff.Fetch() != 0.000)
	{
		Enabled = true;
		SpeedBonus_Unit = nBuff.Fetch();
	}

	if (Enabled)
		AffectedType.Read(parser, pSection, "BuildSpeedBonus.AffectedTypes");
}

bool BuildSpeedBonus::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return Serialize(stm);
}

bool BuildSpeedBonus::Save(PhobosStreamWriter& stm) const
{
	return const_cast<BuildSpeedBonus*>(this)->Serialize(stm);
}

template <typename T>
bool BuildSpeedBonus::Serialize(T& stm)
{
	return stm
		.Process(Enabled)
		.Process(SpeedBonus_Aircraft)
		.Process(SpeedBonus_Building)
		.Process(SpeedBonus_Infantry)
		.Process(SpeedBonus_Unit)
		.Process(AffectedType)
		.Success()
		//&& stm.RegisterChange(this)
		; // announce this type
}