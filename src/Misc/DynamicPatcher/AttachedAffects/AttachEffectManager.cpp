#include"AttachEffectManager.h"

#include "Effects/Stand/StandHelpers.h"

CoordStruct AttachEffectManager::MarkLocation(ObjectClass* pOwner)
{
	CoordStruct location = pOwner->GetCoords();
	if (lastLocation == CoordStruct::Empty)
	{
		lastLocation = location;
	}

	double mileage = location.DistanceFrom(lastLocation);

	if (mileage > locationMarkDistance)
	{
		lastLocation = location;
		double tempMileage = totleMileage + mileage;
		LocationMark locationMark = StandHelper::GetLocation(pOwner, GameCreate<StandType>(),true);

		LocationMarks.insert(LocationMarks.begin(), locationMark);

		if (tempMileage > (Count() + 1) * LocationSpace) {
			LocationMarks.erase(LocationMarks.begin() + (LocationMarks.size() - 1));
		}
		else {
			totleMileage = tempMileage;
		}
	}
	return location;
}

void AttachEffectManager::Render2(ObjectClass* pOwner, bool isDead)
{
	renderFlag = true;
	CoordStruct location = MarkLocation(pOwner);
	int markIndex = 0;
	for (int i = Count() - 1; i >= 0; i--)
	{
		auto ae = AttachEffects[i];
		if (!ae || !ae->Type)
			continue;

		if (ae->IsActive())
		{
			if (ae->GetStand() && ae->GetStand()->IsAlive())
			{
				StandHelper::UpdateStandLocation(this, pOwner, ae->GetStand(), markIndex);
			}
		}
	}
}

void AttachEffectManager::Update(ObjectClass* pOwner, bool isDead)
{
	CoordStruct location = pOwner->GetCoords();

	if (!renderFlag) {
		location = MarkLocation(pOwner);
	}

	int markIndex = 0;

	for (int i = Count() - 1; i >= 0; i--)
	{
		auto ae = AttachEffects[i];
		if (!ae || !ae->Type)
			continue;

		//if (ae->IsActive())
		{
			ae->OnUpdate(pOwner, isDead);
			if (!renderFlag && ae->GetStand() && ae->GetStand()->IsAlive())
			{
				StandHelper::UpdateStandLocation(this, pOwner, ae->GetStand(), markIndex);
			}
		}
		/*
		else
		{
			int delay = ae->Type->Delay;
			if (ae->Type->RandomDelay)
			{
				delay = ScenarioGlobal->Random(ae->Type->MinDelay, ae->Type->MaxDelay);
			}
			if (delay > 0)
			{
				DisableDelayTimers[ae->Type->Name.data()].Start(delay);
			}
			ae->Disable(location);
			AttachEffects.erase(AttachEffects.begin() + i);
			auto nextAE = ae->Type->Next.data();
			if (GeneralUtils::IsValidString(nextAE)) {
				Attach(nextAE, pOwner, ae->House, ae->Attacker, false);
			}
			GameDelete(ae);
		}*/
	}
	renderFlag = false;
}
