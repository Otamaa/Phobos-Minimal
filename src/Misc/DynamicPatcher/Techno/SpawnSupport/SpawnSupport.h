#pragma once
#include <Utilities/TemplateDef.h>

class SpawnSupport
{
public:

	int supportFLHMult;
	CDTimerClass supportFireROF;
	CDTimerClass spawnFireOnceDelay;
	bool spawnFireFlag;

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(supportFLHMult)
			.Process(supportFireROF)
			.Process(spawnFireOnceDelay)
			.Process(spawnFireFlag)
			;

		//Stm.RegisterChange(this);
	}
};
