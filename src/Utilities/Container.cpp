#include "Container.h"
#include <AbstractClass.h>

AbstractExtended::AbstractExtended(AbstractClass* abs) : AttachedToObject(abs), Initialized(InitState::Blank) { };

//withnoint_t , with less instasiation
AbstractExtended::AbstractExtended(AbstractClass* abs, noinit_t) : AttachedToObject(abs) { };

void AbstractExtended::Internal_LoadFromStream(PhobosStreamReader& Stm)
{
	Stm.Process(Initialized);
}

void AbstractExtended::Internal_SaveToStream(PhobosStreamWriter& Stm) const
{
	Stm.Process(Initialized);
}