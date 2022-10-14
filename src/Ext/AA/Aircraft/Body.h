#pragma once
#include <Ext/AA/Foot/Body.h>
#include <AircraftClass.h>

class AircraftClassExt : public AircraftClass
{
	class ExtData final : public FootClassExt::ExtData
	{
		ExtData(AircraftClass* OwnerObject) : FootClassExt::ExtData(OwnerObject)
		{ }

		ExtData(AircraftClassExt* OwnerObject) : ExtData(static_cast<AircraftClass*>(OwnerObject))
		{ }

		~ExtData() = default;

		FootClassExt* GetFootExt() const
		{
			return static_cast<FootClassExt*>(Get());
		}
	};
};

static_assert(sizeof(AircraftClassExt) == sizeof(AircraftClass), "Missmatch Size !");