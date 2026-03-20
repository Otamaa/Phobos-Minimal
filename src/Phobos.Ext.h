#pragma once

#include <Utilities/SavegameDef.h>

struct PhobosExt
{
	static void InvalidatePointers(AbstractClass* const pInvalid , bool const removed,AbstractType  type);
	static void EnsureSeeded(unsigned long seed);
}