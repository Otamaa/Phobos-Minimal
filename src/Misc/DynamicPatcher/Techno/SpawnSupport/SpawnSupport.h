#pragma once
#include <Utilities/TemplateDef.h>

class SpawnSupport
{
public:

	int supportFLHMult;
	CDTimerClass supportFireROF;
	CDTimerClass spawnFireOnceDelay;
	bool spawnFireFlag;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<SpawnSupport*>(this)->Serialize(Stm); }

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(supportFLHMult)
			.Process(supportFireROF)
			.Process(spawnFireOnceDelay)
			.Process(spawnFireFlag)
			.Success() && Stm.RegisterChange(this);

	}
};
