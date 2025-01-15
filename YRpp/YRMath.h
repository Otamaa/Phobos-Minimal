#pragma once

#include <YRPPCore.h>
#include <cmath>
#include <algorithm>
#include <Base/Always.h>
#include <Helpers/CompileTime.h>

#define MATH_FUNC(ret ,arg ,name, address)\
	OPTIONALINLINE NAKED ret __cdecl name(arg value) noexcept\
	{\
		JMP(address);\
	}

#define MATH_FUNC_TWOVAL(name, address)\
	OPTIONALINLINE NAKED float __cdecl name(double valuea , double valueb) noexcept\
	{\
		JMP(address);\
	}

//#define MATH_FUNC_FLOAT_CDCEL(name, address)\
//	OPTIONALINLINE NAKED double __cdecl name(float value)\
//	{\
//		JMP_STD(address);\
//	}

#define MATH_FUNC_FLOAT(ret , arg , name, address)\
	OPTIONALINLINE ret __stdcall name(arg value) noexcept\
	{\
		JMP_STD(address);\
	}

#define MATH_FUNC_TWOVAL_FLOAT(name, address)\
	OPTIONALINLINE double __stdcall name(float valuea , float valueb) noexcept\
	{\
		JMP_STD(address);\
	}

namespace Math
{
	static COMPILETIMEEVAL constant_ptr<float, 0x8610A4> const FastMath_OverflowTable {};
	static COMPILETIMEEVAL reference<int, 0x8650BCu, 16384u> const FastMath_sqrt_Table {};
	static COMPILETIMEEVAL reference<float, 0x8610B4u, 4096u> const FastMath_atan_Table {};
	static COMPILETIMEEVAL reference<float, 0x85D0A4u, 4096u> const FastMath_tan_Table {};
	static COMPILETIMEEVAL reference<float, 0x859094u, 4096u> const FastMath_asin_Table {};
	static COMPILETIMEEVAL reference<float, 0x84F084u, 4096u> const FastMath_sin_Table {};

	COMPILETIMEEVAL auto const WWMATH_PI = 3.14159265358979323846f;
	COMPILETIMEEVAL auto const Pif = 3.1415927f;
	COMPILETIMEEVAL auto const SMALL_FLOAT = 0.0000001f;
	COMPILETIMEEVAL auto const HalfPiF = 1.57079632f;
	COMPILETIMEEVAL auto const Pi = 3.1415926535897932384626433832795;
	COMPILETIMEEVAL auto const C_Sharp_Pi = 3.1415926535897931;
	COMPILETIMEEVAL auto const TwoPi = 6.283185307179586476925286766559;
	COMPILETIMEEVAL auto const HalfPi = 1.5707963267948966192313216916398;
	COMPILETIMEEVAL auto const Sqrt2 = 1.4142135623730950488016887242097;
	COMPILETIMEEVAL auto const Sqrt3 = 1.73205080756887719318f;
	COMPILETIMEEVAL auto const Sqrtoo2 = 0.7071067811865475244008442f;
	COMPILETIMEEVAL auto const Sqrtoo3 = 0.5773502691896257645091489f;
	COMPILETIMEEVAL auto const Sqrtoo6 = 0.4082482904638630163662140f;
	COMPILETIMEEVAL auto const epsilon = 0.0001f;
	COMPILETIMEEVAL auto const EPSILON = 1.0E-6;
	COMPILETIMEEVAL auto const short_epsilon = 0.1f;
	COMPILETIMEEVAL auto const float_max = FLT_MAX;
	COMPILETIMEEVAL auto const float_min = FLT_MIN;
	COMPILETIMEEVAL auto const float_tiny = 1.0e-37f;
	COMPILETIMEEVAL auto const E = 2.71828182845904523536f; // Holds the value for "e": Euler's number or Napier's constant, to 15 significant figures. This is a mathematically useful number.
	COMPILETIMEEVAL auto const LOG2E = 1.44269504088896340736f;
	COMPILETIMEEVAL auto const LOG10E = 0.434294481903251827651f;
	COMPILETIMEEVAL auto const LN2 = 0.693147180559945309417f;
	COMPILETIMEEVAL auto const LN10 = 2.30258509299404568402f;
	COMPILETIMEEVAL auto const P4 = 0.785398163397448309616f;// Holds the value for PI / 4 OR 45 degrees. Only up to 16 significant figures.
	COMPILETIMEEVAL auto const P8 = 0.39269908169872413f; // Holds the value for PI / 8 OR 22.5 degrees. Only up to 17 significant figures.
	COMPILETIMEEVAL auto const P16 = 0.19634954084936206f; // Holds the value for PI / 16 OR 11.25 degrees. Only up to 17 significant figures.
	COMPILETIMEEVAL auto const PI = 0.318309886183790671538f;
	COMPILETIMEEVAL auto const _2_PI = 0.636619772367581343076f;
	COMPILETIMEEVAL auto const _1_SQRTPI = 0.564189583547756286948f;
	COMPILETIMEEVAL auto const _2_SQRTPI = 1.12837916709551257390f;
	COMPILETIMEEVAL auto const THREE_PI_2 = 4.7123889803846895f; // Holds the value for 3 * PI_2 OR 270 degrees. Only up to 17 significant figures.
	COMPILETIMEEVAL auto const TIGHT_CORNER_RADIUS = 0.5f;
	COMPILETIMEEVAL auto const RAD_TO_DEG = 57.295779513082325225835265587527f; // Holds the value for 180 / PI which is used to convert radians to degrees.
	COMPILETIMEEVAL auto const DEG_TO_RAD = 0.017453292519943294444444444444444f; // Holds the value for PI / 180 which is used to convert degrees to radians.
	COMPILETIMEEVAL auto const DEG90_AS_RAD = 1.570796326794897;
	COMPILETIMEEVAL auto const BINARY_ANGLE_MAGIC_VALUE = -10430.06004058427;
	COMPILETIMEEVAL const float ZERO = (float)1e-7;
	COMPILETIMEEVAL const float PI_HALVES = 0.50f * Math::PI;
	COMPILETIMEEVAL const float PI_THIRDS = Math::PI * 0.3333333333333f;
	COMPILETIMEEVAL const float PI_FOURTHS = 0.25f * Math::PI;
	COMPILETIMEEVAL const float PI_SIXTHS = Math::PI * 0.6666666666667f;
	COMPILETIMEEVAL const float PI_2 = 2.00f * Math::PI;
	COMPILETIMEEVAL const float PI_DIV180 = Math::PI / 180.0f;
	COMPILETIMEEVAL const float PI_INVx180 = 180.0f / Math::PI;
	COMPILETIMEEVAL const float PI_INV = 1.0f / Math::PI;
	COMPILETIMEEVAL const double DEG_TO_RAD_Double = Math::Pi / 180;
	COMPILETIMEEVAL const double BINARY_ANGLE_MAGIC = -(360.0 / (65535 - 1)) * DEG_TO_RAD;
	COMPILETIMEEVAL const double DEG_TO_RAD_ALTERNATE = C_Sharp_Pi / 180;
	COMPILETIMEEVAL const double BINARY_ANGLE_MAGIC_ALTERNATE = -(360.0 / (65535 - 1)) * DEG_TO_RAD_ALTERNATE;
	COMPILETIMEEVAL const double PI_BYSIXTEEN = Math::Pi / 16;
	COMPILETIMEEVAL const double BinaryAngleMagic = -(360.0 / (65535 - 1)) * Pi / 180.0;
	COMPILETIMEEVAL const double radToIndex = 2607.5945;
	COMPILETIMEEVAL const double gameval_ = 651.8986469044033;
	COMPILETIMEEVAL const double ATANTABLEEND = 0.024413355;

	// Game degrees to radians coefficient, called 'binary angle magic' by some.
	COMPILETIMEEVAL auto const GameDegreesToRadiansCoefficient = -(360.0 / (65535 - 1)) * Pi / 180.0;
	COMPILETIMEEVAL auto const GameDegrees90 = 0X3FFF;

	template<typename T> requires std::is_integral_v<T> || std::is_floating_point_v<T>
	static COMPILETIMEEVAL float DEG_TO_RADF(T val) { return (((float)val) * WWMATH_PI / 180.0f); }

	MATH_FUNC(float, double, sqrt, 0x4CAC40);
	MATH_FUNC(float, double, sin, 0x4CACB0);
	MATH_FUNC(float, double, cos, 0x4CAD00);

	MATH_FUNC(double, double, tan, 0x4CAD50);
	MATH_FUNC(double, double, asin, 0x4CAD80);
	MATH_FUNC(double, double, acos, 0x4CADB0);
	MATH_FUNC(double, double, atan, 0x4CADE0);

	MATH_FUNC_TWOVAL(atan2, 0x4CAE30);
	MATH_FUNC_TWOVAL(arctanfoo, 0x4CAE30);

	MATH_FUNC_FLOAT(float, float, sqrt, 0x4CB060);
	MATH_FUNC_FLOAT(double, float, sin, 0x4CB150);
	MATH_FUNC_FLOAT(double, float, cos, 0x4CB1A0);
	MATH_FUNC_FLOAT(double, float, asin, 0x4CB260);
	MATH_FUNC_FLOAT(double, float, tan, 0x4CB320);

	MATH_FUNC_FLOAT(double, float, acos, 0x4CB290);
	MATH_FUNC_FLOAT(double, float, atan, 0x4CB480);
	MATH_FUNC_TWOVAL_FLOAT(atan2, 0x4CB3D0);

	//famous Quaqe 3 Fast Inverse Square Root
	OPTIONALINLINE COMPILETIMEEVAL float Q_invsqrt(float number) noexcept {
		static_assert(std::numeric_limits<float>::is_iec559, "Float Must be IEC559 !");
		COMPILETIMEEVAL float threehalfs = 1.5F;

		float x2 = number * 0.5F;
		float y = number;
		long i = *(long *)&y;						  // evil floating point bit level hacking
		i = 0x5f3759df - (i >> 1);               // what the fuck?
		y = *(float *)&i;
		y = y * (threehalfs - (x2 * y * y));   // 1st iteration
	 // y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration,this can be removed

		return y;
	}

	OPTIONALINLINE COMPILETIMEEVAL double rad2deg(double rad) { return rad * 180.0 / Pi; }
	OPTIONALINLINE COMPILETIMEEVAL double rad2deg_Alternate(double rad) { return rad * 180.0 / C_Sharp_Pi; }
	OPTIONALINLINE double sqrt(int val) { return sqrt(static_cast<double>(val)); }
	OPTIONALINLINE COMPILETIMEEVAL double deg2rad(double deg) { return deg * Pi / 180.0; }
	OPTIONALINLINE COMPILETIMEEVAL double deg2rad_Alternate(double deg) { return deg * C_Sharp_Pi / 180.0; }

	template <typename T>
	[[nodiscard]] OPTIONALINLINE COMPILETIMEEVAL int signum(T x)noexcept
			requires std::is_arithmetic_v<T> {
		if COMPILETIMEEVAL (std::is_signed<T>()){
			return (T{ 0 } < x) - (x < T{ 0 });
		} else {
			return T{ 0 } < x;
		}
	}

	//https://github.com/rhalbersma/xstd/blob/master/include/xstd/cstdlib.hpp

	template<class T>
	[[nodiscard]] OPTIONALINLINE COMPILETIMEEVAL auto abs(T const& x) noexcept
		requires std::is_arithmetic_v<T> {
		return( // deal with signed-zeros
		x == T(0) ? \
			T(0) :
		// else
		x < T(0) ? \
			- x : x);
	}

	template <typename T>
	using value_return_t = std::remove_cv_t<std::remove_reference_t<T>>;

	// use the sign to select min or max.
	// 0 means no change (maximum of 0 and a positive value)
	[[nodiscard]] OPTIONALINLINE COMPILETIMEEVAL auto limit(int value, int limit) noexcept {
		if (limit <= 0) {
			return MaxImpl(value, -limit);
		} else {
			return MinImpl(value, limit);
		}
	}
};
