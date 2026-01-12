#pragma once
#include <cmath>

struct PhobosMath
{
	static double __cdecl acosd(double a1)
	{
		return std::acos(a1);
	}

	static double __cdecl asind(double a1)
	{
		return std::asin(a1);
	}

	static double __cdecl atand(double a1)
	{
		return std::atan(a1);
	}

	static double __cdecl atan2d(double a1, double a2)
	{
		return std::atan2(a1, a2);
	}

	static double __cdecl cosd(double a1)
	{
		return std::cos(a1);
	}

	static double __cdecl sind(double a1)
	{
		return std::sin(a1);
	}

	static double __cdecl sqrtd(double a1)
	{
		return std::sqrt(a1);
	}

	static double __cdecl tand(double a1)
	{
		return std::tan(a1);
	}

	//===================================
	static float __cdecl acosf(float a1)
	{
		return (float)std::acos(a1);
	}

	static float __cdecl asinf(float a1)
	{
		return (float)std::asin(a1);
	}

	static float __cdecl atanf(float a1)
	{
		return (float)std::atan(a1);
	}

	static float __cdecl atan2f(float a1, float a2)
	{
		return (float)std::atan2(a1, a2);
	}

	static float __cdecl cosf(float a1)
	{
		return (float)std::cos(a1);
	}

	static float __cdecl sinf(float a1)
	{
		return (float)std::sin(a1);
	}

	static float __cdecl sqrtf(float a1)
	{
		return (float)std::sqrt(a1);
	}

	static float __cdecl tanf(float a1)
	{
		return (float)std::tan(a1);
	}
};