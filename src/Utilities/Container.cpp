#include "Container.h"
#include <AbstractClass.h>

AbstractExtended::AbstractExtended(AbstractClass* abs) : AttachedToObject(abs)
, MyEntity()
{
	this->MyEntity = Phobos::gEntt->create();
	Phobos::gEntt->emplace<ExtensionIdentifierComponent>(this->MyEntity);
};

//withnoint_t , with less instasiation
AbstractExtended::AbstractExtended(AbstractClass* abs, noinit_t) : AttachedToObject(abs)
, MyEntity()
{
	this->MyEntity = Phobos::gEntt->create();
};

void AbstractExtended::Internal_LoadFromStream(PhobosStreamReader& Stm)
{
	EntitySerializer<ExtensionIdentifierComponent>::Load(Stm, this->MyEntity);
}

void AbstractExtended::Internal_SaveToStream(PhobosStreamWriter& Stm) const
{
	EntitySerializer<ExtensionIdentifierComponent>::Save(Stm, this->MyEntity);
}

AbstractExtended::~AbstractExtended()
{
	if (this->MyEntity != entt::null) {
		Phobos::gEntt->destroy(this->MyEntity);
	}
}