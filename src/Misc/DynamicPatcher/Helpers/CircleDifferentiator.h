#pragma once

#include <vector>
#include <CoordStruct.h>

namespace CircleDifferentiator
{
	std::vector<CoordStruct> DivideArcByTolerance(const CoordStruct& center, int radius, int tolerance = 128, const Vector3D<float>& upVector = Vector3D<float>::Empty);
	std::vector<CoordStruct> DivideArcByCount(const CoordStruct& center, int radius, int pointCount, const Vector3D<float>& upVector = Vector3D<float>::Empty);
}