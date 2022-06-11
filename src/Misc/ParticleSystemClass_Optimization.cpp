#include <ParticleSystemClass.h>
#include <ScenarioClass.h>

bool Optimize_Gas(ParticleSystemClass* pPart)
{
	if (pPart->SparkSpawnFrames-- > 0)
	{
		if (ScenarioClass::Instance->Random.RandomDouble() < 0.3)
		{
			auto nSpotRadius = pPart->SpotlightRadius - 3;
			if (nSpotRadius < 17)
				nSpotRadius = 17;

			pPart->SpotlightRadius = nSpotRadius;

			auto pType = pPart->Type;
			if (!pPart->SparkSpawnFrames ||
				pType->SpawnSparkPercentage > ScenarioClass::Instance->Random.RandomDouble()) {
				int ParticleCap = pType->ParticleCap >= 0 ?  pType->ParticleCap:0;

			}
		}
	}
	else
	{
		pPart->TimeToDie = true;
	}
}