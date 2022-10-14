#pragma once
#include <Ext/AA/Techno/Body.h>
#include <BuildingClass.h>

class BuildingClassExt : public BuildingClass
{
public:
	class ExtData : public TechnoClassExt::ExtData
	{
		ExtData(BuildingClass* OwnerObject) : TechnoClassExt::ExtData(OwnerObject)
		{ }

		ExtData(BuildingClassExt* OwnerObject) : ExtData(static_cast<BuildingClass*>(OwnerObject))
		{ }

		~ExtData() = default;

		TechnoClassExt* GetTechnoExt() const
		{
			return static_cast<TechnoClassExt*>(Get());
		}
	};
};

static_assert(sizeof(BuildingClassExt) == sizeof(BuildingClass), "Missmatch Size !");