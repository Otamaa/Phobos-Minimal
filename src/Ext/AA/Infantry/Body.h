#pragma once
#include <Ext/AA/Foot/Body.h>
#include <InfantryClass.h>

class InfantryClassExt : public InfantryClass
{
	class ExtData final : public FootClassExt::ExtData
	{
	public:
		ExtData(InfantryClass* OwnerObject) : FootClassExt::ExtData(OwnerObject)
		{ }

		ExtData(InfantryClassExt* OwnerObject) : ExtData(static_cast<InfantryClass*>(OwnerObject))
		{ }

		~ExtData() = default;

		FootClassExt* GetFootExt() const {
			return static_cast<FootClassExt*>(Get());
		}

	};
};

static_assert(sizeof(InfantryClassExt) == sizeof(InfantryClass), "Missmatch Size !");