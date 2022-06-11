#pragma once

#include <vector>
#include <CoordStruct.h>

namespace CircleDifferentiator
{
	std::vector<CoordStruct> DivideArcByTolerance(CoordStruct center, int radius, int tolerance = 128, Vector3D<float> upVector = Vector3D<float>::Empty);
	std::vector<CoordStruct> DivideArcByCount(CoordStruct center, int radius, int pointCount, Vector3D<float> upVector = Vector3D<float>::Empty);
}