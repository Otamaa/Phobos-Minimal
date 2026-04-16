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

	static float __cdecl cosd(double a1)
	{
		return (float)std::cos(a1);
	}

	static float __cdecl sind(double a1)
	{
		return (float)std::sin(a1);
	}

	static float __cdecl sqrtd(double a1)
	{
		return (float)std::sqrt(a1);
	}

	static double __cdecl tand(double a1)
	{
		return std::tan(a1);
	}

	//===================================
	static double __cdecl acosf(float a1)
	{
		return std::acos(a1);
	}

	static double __stdcall asinf(float a1)
	{
		return std::asin(a1);
	}

	static double __stdcall atanf(float a1)
	{
		return std::atan(a1);
	}

	static double __stdcall atan2f(float a1, float a2)
	{
		return std::atan2(a1, a2);
	}

	static double __cdecl cosf(float a1)
	{
		return std::cos(a1);
	}

	static double __cdecl sinf(float a1)
	{
		return std::sin(a1);
	}

	static double __cdecl sqrtf(float a1)
	{
		return std::sqrt(a1);
	}

	static double __cdecl tanf(float a1)
	{
		return std::tan(a1);
	}

};

namespace PhobosMathCache
{
	//TODO : float
	//TODO : int
	//using Computer = double(*)();

	//struct Entry
	//{
	//	const char* name;     // for debugging
	//	Computer    compute;
	//	double      cached;
	//};

	//// Define your constants as a static table of (name, function)
	//inline Entry Entries[] = {
	//	{ "SlopeShallow_InvLen", []() { return 1.0 / sqrt(/*...*/); }, 0.0 },
	//	{ "SlopeMedium_InvLen",  []() { return 1.0 / sqrt(/*...*/); }, 0.0 },
	//	{ "SlopeSteep_InvLen",   []() { return 1.0 / sqrt(/*...*/); }, 0.0 },
	//	{ "Deg45_Sin",           []() { return sin(45.0 * Math::GAME_PI / 180.0); }, 0.0 },
	//};

	//enum Index : size_t
	//{
	//	SlopeShallow_InvLen,
	//	SlopeMedium_InvLen,
	//	SlopeSteep_InvLen,
	//	Deg45_Sin,
	//};

	//inline void Recalculate()
	//{
	//	for (auto& e : Entries) e.cached = e.compute();
	//}

	//[[nodiscard]] inline double Get(Index i) noexcept
	//{
	//	return Entries[i].cached;
	//}
}