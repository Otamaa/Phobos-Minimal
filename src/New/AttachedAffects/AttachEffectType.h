#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

#include "Affects/Stand/StandType.h"

class AttachEffectType
{
public:

	int Duration;
	int HoldDuration;

	int InitialDelay;
	Point2D InitialRandomDelay;

	std::unique_ptr<StandType> Stand {};

};