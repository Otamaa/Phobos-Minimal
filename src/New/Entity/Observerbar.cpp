#include <HouseClass.h>
#include <FactoryClass.h>

class ObserverBar
{
	const static bool ShowObserverBar { false };

	void DrawAll()
	{
		if (!ShowObserverBar)
			return;
		auto const pCurPlayer = HouseClass::CurrentPlayer();

		if (pCurPlayer->IsPlayerObserver())
		{
			std::for_each(HouseClass::Array->begin(), HouseClass::Array->end(), [](HouseClass* const pObserve) {
				if (pObserve->Type->MultiplayPassive)
					return;

				FactoryClass* pFactory_Combat_Building = pObserve->GetPrimaryFactory(AbstractType::BuildingType, false, BuildCat::Combat);
				FactoryClass* pFactory_Unit = pObserve->GetPrimaryFactory(AbstractType::UnitType, false, BuildCat::DontCare);
				FactoryClass* pFactory_Unit_Naval = pObserve->GetPrimaryFactory(AbstractType::UnitType, true, BuildCat::DontCare);
				FactoryClass* pFactory_Aircraft = pObserve->GetPrimaryFactory(AbstractType::AircraftType, false, BuildCat::DontCare);
				FactoryClass* pFactory_Inf = pObserve->GetPrimaryFactory(AbstractType::InfantryType, false, BuildCat::DontCare);
				double Power = pObserve->GetPowerPercentage();

				//draw
				//pFactory_Combat_Building->Object
			});
		}
	}
};

// passangers speed modifiers ?
// Production Bars
