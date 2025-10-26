#include "Container.h"
#include <AbstractClass.h>

AbstractExtended::AbstractExtended(AbstractClass* abs) : AttachedToObject(abs)
 , AOName()
 , AbsType()
 , Initialized()
{ };

AbstractExtended::AbstractExtended(AbstractClass* abs, noinit_t) : AttachedToObject(abs)
	, AOName()
	, AbsType()
	, Initialized()
{ };

void AbstractExtended::Internal_LoadFromStream(PhobosStreamReader& Stm)
{
	Stm
		.Process(AOName)
		.Process(AbsType)
		.Process(Initialized)
		;
}

void AbstractExtended::Internal_SaveToStream(PhobosStreamWriter& Stm) const
{
	Stm
		.Process(AOName)
		.Process(AbsType)
		.Process(Initialized)
		;
}

AbstractExtended::~AbstractExtended()
{ }