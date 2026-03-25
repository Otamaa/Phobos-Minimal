#pragma once

#include <GeneralDefinitions.h>

class AbstractClass;
struct PhobosExt
{
	static void InvalidatePointers(AbstractClass* const pInvalid, bool const removed, AbstractType  type);
	static void EnsureSeeded(unsigned long seed);
};