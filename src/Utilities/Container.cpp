#include "Container.h"
#include <AbstractClass.h>

AbstractExtended::AbstractExtended(AbstractClass* abs) : AttachedToObject(abs)
 , AbsType()
 , Initialized()
{ };

AbstractExtended::AbstractExtended(AbstractClass* abs, noinit_t) : AttachedToObject(abs)
	, AbsType()
	, Initialized()
{ };

void AbstractExtended::Internal_LoadFromStream(PhobosStreamReader& Stm)
{
	Stm
		.Process(AbsType)
		.Process(Initialized)
		;
}

void AbstractExtended::Internal_SaveToStream(PhobosStreamWriter& Stm) const
{
	Stm
		.Process(AbsType)
		.Process(Initialized)
		;
}
