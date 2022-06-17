#pragma once

#include <tuple>

class Point3D
{
public:
	static const Point3D Empty;

	//auto operator()()
	//{
		// returns a tuple to make it work with std::tie
	//	return std::make_tuple(X, Y, Z);
	//}

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	int X, Y, Z;
};