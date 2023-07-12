#include "CircleDifferentiator.h"
#include "VectorHelpers.h"

std::vector<CoordStruct> CircleDifferentiator::DivideArcByTolerance(const CoordStruct& center, int radius, int tolerance, const Vector3D<float>& upVector)
{
	tolerance = MinImpl(tolerance, (int)(std::sqrt(2) * radius));

	// start from nearest count n that satisfy: d = sqrt(2) * r * sin(a) <= tolerance, a = 2 * pi / n
	double maxRad = std::asin(tolerance / (std::sqrt(2) * radius));
	int n = (int)(2 * Math::C_Sharp_Pi / maxRad);

	// find n that satisfy d <= length, d = sqrt(2) * r * sin(a), a = 2 * pi / n
	while (true)
	{
		double dRad = 2 * Math::C_Sharp_Pi / n;
		int dLength = (int)(std::sqrt(2) * radius * Math::sin(dRad));
		if (dLength <= tolerance)
			break;
		n++;
	}

	return DivideArcByCount(center, radius, n, upVector);
}

std::vector<CoordStruct> CircleDifferentiator::DivideArcByCount(const CoordStruct& center, int radius, int pointCount, const Vector3D<float>& upVector)
{
	Vector3D<float> UpVec = upVector;

	if (upVector == Vector3D<float>::Empty)
		UpVec = {0,0,1};

	if (pointCount <= 0)
		pointCount = 1;

	std::vector<CoordStruct> list;
	list.reserve(pointCount);

	double dRad = 2 * Math::C_Sharp_Pi / pointCount;
	Quaternion q = Helpers_DP_Vec::FromToRotation(Vector3D<float>{0.0f,0.0f,1.0f}, UpVec);

	Vector3D<float> nDummy = Vector3D<float>::Empty;
	auto const UnitZ = Vector3D<float> { 0.0f , 0.0f , 1.0f };
	CoordStruct nDummy_Coord = CoordStruct::Empty;

	for (double rad = 0; rad < Math::C_Sharp_Pi * 2; rad += dRad)
	{
		nDummy.X = (float)(radius * Math::cos(rad));
		nDummy.Y = (float)(radius * Math::sin(rad));

		if (UpVec != UnitZ) {
			nDummy = Helpers_DP_Vec::Transform(nDummy, q);
		}

		nDummy_Coord.X = (int)nDummy.X;
		nDummy_Coord.Y = (int)nDummy.Y;
		nDummy_Coord.Z = (int)nDummy.Z;
		list.push_back((center + nDummy_Coord));
	}

	return list;
}