#pragma once

#include <AircraftTypeClass.h>

class FakeAircraftTypeClass : public AircraftTypeClass {
public:
	bool _CanAttackMove() { return true; };
};

static_assert(sizeof(FakeAircraftTypeClass) == sizeof(FakeAircraftTypeClass), "Invalid Size !");
