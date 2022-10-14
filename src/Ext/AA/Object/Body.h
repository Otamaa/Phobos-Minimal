#pragma once
#include <Ext/Abstract/Body.h>
#include <ObjectClass.h>

class ObjectClassExt : public ObjectClass
{
public:

	class ExtData : public TExtension<ObjectClass>
	{
	public:
		bool Test;

		ExtData(ObjectClass* OwnerObject) : TExtension<ObjectClass>(OwnerObject)
		{ }

		ExtData(ObjectClassExt* OwnerObject) : ExtData(static_cast<ObjectClass*>(OwnerObject))
		{ }

		~ExtData() = default;

	};
};

static_assert(sizeof(ObjectClassExt) == sizeof(ObjectClass), "Missmatch Size !");