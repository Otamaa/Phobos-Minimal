#pragma once
#include <Ext/AA/Techno/Body.h>
#include <FootClass.h>

class FootClassExt : public FootClass
{
public:
	class ExtData : public TechnoClassExt::ExtData
	{
	public:

		ExtData(FootClass* OwnerObject) : TechnoClassExt::ExtData(OwnerObject)
		{ }

		ExtData(FootClassExt* OwnerObject) : ExtData(static_cast<FootClass*>(OwnerObject))
		{ }

		~ExtData() = default;

		TechnoClassExt* GetTechnoExt() const {
			return static_cast<TechnoClassExt*>(Get());
		}
	};
};

static_assert(sizeof(FootClassExt) == sizeof(FootClass), "Missmatch Size !");