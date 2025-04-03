#pragma once
#include <Utilities/TemplateDef.h>
#include "AttachEffectType.h"

struct AttachEffectData {
	ValueableVector<AttachEffectType*> Types;
	Valueable<int> CabinLength;

	AttachEffectData() : Types { }
		, CabinLength { 0 }
	{ }

 	void Read(INI_EX& parser, const char* pSection)
	{
		Types.Read(parser, pSection, "AttachEffectTypes");
		CabinLength.Read(parser, pSection, "StandTrainCabinLength");
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		Debug::LogInfo("Processing Element From AttachEffectData ! ");
		return Stm
			.Process(Types)
			.Process(CabinLength)
			.Success()
			;
	}

};
