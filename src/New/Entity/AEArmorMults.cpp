#include "AEArmorMults.h"
#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>

double AEArmorMults::Get(double initial, WarheadTypeClass* who, TechnoClass* pOwner, bool playHitAnim) const
{
	for (const auto& entry : mults)
	{
		if (entry.Chance < ScenarioClass::Instance->Random.RandomDouble())
			continue;

		if (!entry.Eligible(who))
			continue;

		if (playHitAnim && entry.HitAnims)
		{
			AnimExtData::CreateRandomAnim(*entry.HitAnims, pOwner->GetCoords(), pOwner, nullptr, true);
		}

		initial *= entry.Mult;
	}

	return initial;
}
