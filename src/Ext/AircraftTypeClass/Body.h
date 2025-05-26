#pragma once

#include <AircraftTypeClass.h>

class NOVTABLE FakeAircraftTypeClass : public AircraftTypeClass {
public:
	bool _CanAttackMove() { return RulesExtData::Instance()->ExpandAircraftMission; };
};

static_assert(sizeof(FakeAircraftTypeClass) == sizeof(FakeAircraftTypeClass), "Invalid Size !");