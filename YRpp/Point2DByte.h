#pragma once

class Point2DBYTE
{
public:
	static const Point2DBYTE Empty;

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	BYTE X, Y;
};