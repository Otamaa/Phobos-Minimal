#pragma once

class Point3D
{
public:
	static const Point3D Empty;

	//no constructor, so this class stays aggregate and can be initialized using the curly braces {}
	int X, Y, Z;
};