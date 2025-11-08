#include "Container.h"
#include <AbstractClass.h>

AbstractExtended::AbstractExtended(AbstractClass* abs) : AttachedToObject(abs)
 , Name()
 , AbsType()
 , Initialized()
{ };

AbstractExtended::AbstractExtended(AbstractClass* abs, noinit_t) : AttachedToObject(abs)
	, Name()
	, AbsType()
	, Initialized()
{ };

void AbstractExtended::Internal_LoadFromStream(PhobosStreamReader& Stm)
{
	Stm
		.Process(Name)
		.Process(AbsType)
		.Process(Initialized)
		;
}

void AbstractExtended::Internal_SaveToStream(PhobosStreamWriter& Stm) const
{
	Stm
		.Process(Name)
		.Process(AbsType)
		.Process(Initialized)
		;
}
