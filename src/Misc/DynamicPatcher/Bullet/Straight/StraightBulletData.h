#pragma once
#include <Utilities/TemplateDef.h>

struct StraightBulletData
{
	Valueable<bool> Enabled;
	Valueable<bool> AbsolutelyStraight;

	StraightBulletData()
		: Enabled { false }, AbsolutelyStraight { false }
	{ }

	bool IsStraight()
	{
		return Enabled;
	}

	~StraightBulletData() = default;

	bool Load(PhobosStreamReader & Stm, bool RegisterForChange)
	{ Debug::LogInfo("Loading Element From StraightBulletData ! "); return Serialize(Stm); }

	bool Save(PhobosStreamWriter & Stm)
	{ return Serialize(Stm); }

	void Read(INI_EX& parser, const char* pSection)
	{
		Enabled.Read(parser, pSection, "Straight");
		AbsolutelyStraight.Read(parser, pSection, "AbsolutelyStraight");
	}

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Enabled)
			.Process(AbsolutelyStraight)
			.Success()
			;
	}
};
