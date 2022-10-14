#pragma once
#include <Ext/AA/Foot/Body.h>
#include <UnitClass.h>

class UnitClassExt final : public UnitClass
{
	class ExtData final : public FootClassExt::ExtData
	{
	public:

		ExtData(UnitClass* OwnerObject) : FootClassExt::ExtData(OwnerObject)
		{ }

		ExtData(UnitClassExt* OwnerObject) : ExtData(static_cast<UnitClass*>(OwnerObject))
		{ }

		FootClassExt* GetFootExt() const {
			return static_cast<FootClassExt*>(Get());
		}
	};
};

static_assert(sizeof(UnitClassExt) == sizeof(UnitClass), "Missmatch Size !");