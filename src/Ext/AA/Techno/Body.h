#pragma once
#include <Ext/AA/Object/Body.h>
#include <TechnoClass.h>

class TechnoClassExt : public TechnoClass
{
public:
	class ExtData : public ObjectClassExt::ExtData
	{
	public:

		ExtData(TechnoClass* OwnerObject) : ObjectClassExt::ExtData(OwnerObject)
		{ }

		ExtData(TechnoClassExt* OwnerObject) : ExtData(static_cast<TechnoClass*>(OwnerObject))
		{ }

		~ExtData() = default;

		ObjectClassExt* GetObjectExt() const {
			return static_cast<ObjectClassExt*>(Get());
		}
	};
};

static_assert(sizeof(TechnoClassExt) == sizeof(TechnoClass), "Missmatch Size !");