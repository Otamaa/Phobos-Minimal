#include "AEArmorMults.h"
#include <ScenarioClass.h>

#include <Ext/Anim/Body.h>

double AEArmorMults::Get(double initial, WarheadTypeClass* who, TechnoClass* pOwner, HouseClass* pInvoker, bool playHitAnim, bool isReallyHit)
{
	for (auto& entry : mults)
	{
		if (entry.Chance < ScenarioClass::Instance->Random.RandomDouble())
			continue;

		if (entry.ArmorMultiplierTimer.InProgress())
			continue;

		if (!entry.Eligible(who))
			continue;
		
		if (pInvoker && !EnumFunctions::CanTargetHouse(entry.allowhouse, pOwner->Owner, pInvoker))
			continue;

		if (isReallyHit)
		{
			if(entry.delay > 0)
				entry.ArmorMultiplierTimer.Start(entry.delay);

			if(playHitAnim && entry.HitAnims)
				AnimExtData::CreateRandomAnim(*entry.HitAnims, pOwner->GetCoords(), pOwner, nullptr, true);
		}

		initial *= entry.Mult;
	}

	return initial;
}
