#pragma once

#include <GeneralDefinitions.h>

class AbstractClass;
class FootClass;
struct PhobosExt
{
	static void InvalidatePointers(AbstractClass* const pInvalid, bool const removed, AbstractType  type);
	static void EnsureSeeded(unsigned long seed);

	struct Global {
		static std::unordered_map<FootClass*, std::pair<int, int>> PathfindFail;
	};
};