#pragma once

#include <Base/Always.h>

//Random number range
struct RandomStruct
{
	int Min, Max;

	COMPILETIMEEVAL bool IsValid() {
		return this->Min != 0 || this->Max != 0;
	}
};