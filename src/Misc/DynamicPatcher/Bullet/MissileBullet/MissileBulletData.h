#pragma once

#include <Utilities/TemplateDef.h>

struct MissileBulletData
{
	Valueable<bool> ReverseVelocity;
	Valueable<bool> ReverseVelocityZ;
	Valueable<double> ShakeVelocity;

	MissileBulletData() :
		ReverseVelocity { }
		, ReverseVelocityZ { }
		, ShakeVelocity { }
	{ }

	void Read(INI_EX& parser, const char* pSection)
	{
		ReverseVelocity.Read(parser, pSection, "ROT.Reverse");
		ReverseVelocityZ.Read(parser, pSection, "ROT.ReverseZ");
		ShakeVelocity.Read(parser, pSection, "ROT.ShakeMultiplier");
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		Debug::LogInfo("Loading Element From MissileBulletData ! ");
		return Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(ReverseVelocity)
			.Process(ReverseVelocityZ)
			.Process(ShakeVelocity)
			.Success()
			;
	}
};
