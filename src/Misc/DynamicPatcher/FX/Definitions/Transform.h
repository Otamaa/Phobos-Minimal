#pragma once
#include "FloatVectors.h"
#include "Rotator.h"

struct Transform
{
	Transform(FloatVector3 location, Rotator rotation, FloatVector3 scale)
		: Location(location), Rotation(rotation), Scale(scale)
	{ }

	FloatVector3 Location;
	Rotator Rotation;
	FloatVector3 Scale;

};