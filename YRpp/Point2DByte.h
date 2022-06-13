#pragma once

#include <utility>

class Point2DBYTE
{
public:
	static const Point2DBYTE Empty;

	auto operator()()
	{
		// returns a tuple to make it work with std::tie
		return std::make_pair(X, Y);
	}

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	BYTE X, Y;
};