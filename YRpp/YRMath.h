#pragma once

#include <YRPPCore.h>
#include <cmath>
#include <algorithm>

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

	static COMPILETIMEEVAL float radToIndex = std::bit_cast<float>(0x4522F983u);

	//-0.00009587672516830327
	static COMPILETIMEEVAL double DIRECTION_FIXED_MAGIC = std::bit_cast<double>(uint64_t { 0xBF19222D989F5E57ull });

	//-10430.06004058427
	static COMPILETIMEEVAL double BINARY_ANGLE_MAGIC = std::bit_cast<double>(uint64_t { 0xC0C45F07AF68ECEFull });

	//1.570796326794897
	static COMPILETIMEEVAL double DEG90_AS_RAD = std::bit_cast<double>(uint64_t { 0x3FF921FB54442D18ull });

	//0.017453292519943294444444444444444
	static COMPILETIMEEVAL double DEG_TO_RAD = std::bit_cast<double>(uint64_t { 0x3F91DF46A2529D39ull });

	static COMPILETIMEEVAL double TWO_BY_PI = std::bit_cast<double>(uint64_t { 0x3FC25E5374344960ull });

	//π/16 , 0.1963495408493621
	static COMPILETIMEEVAL double PI_BY_SIXTEEN = std::bit_cast<double>(uint64_t { 0x3FC921FB54442D18ull });

	//0.78532625585357209119199168682098
	static COMPILETIMEEVAL double PI_BY_FOUR_APPROX = std::bit_cast<double>(0x3FE921648732995Cull);

	//1.5707963267948965580592013981729
	static COMPILETIMEEVAL double PI_BY_TWO_APPROX = std::bit_cast<double>(0x3FF921C90FE8FBDAull);

	// = 0.7853981633974483 (π/4, 45 degrees)
	static COMPILETIMEEVAL float PI_BY_FOUR_F = std::bit_cast<float>(0x3F490FDBu);

	//-1.5707963267948966
	static COMPILETIMEEVAL double NEGA_PI_BY_TWO = std::bit_cast<double>(0xBFF921FB54442D18ull);

	// True π/2
	static COMPILETIMEEVAL double PI_BY_TWO_ACCURATE = std::bit_cast<double>(0x3FF921FB54442D18ull);

	// True π/4
	static COMPILETIMEEVAL double PI_BY_FOUR_ACCURATE = std::bit_cast<double>(0x3FE921FB54442D18ull);

	// = 4.7123889802303795... (approx 3π/2, 270 degrees)
	static COMPILETIMEEVAL double THREE_PI_BY_TWO_APPROX = std::bit_cast<double>(0x4012D9891049EE22ull);

	// = 4.71238898038469... (true 3π/2, 270 degrees)
	static COMPILETIMEEVAL double THREE_PI_BY_TWO = std::bit_cast<double>(0x4012D97C7F3321D2ull);

	// = π/8 = 0.39269908169872414 (22.5 degrees)
	static COMPILETIMEEVAL double PI_BY_EIGHT = std::bit_cast<double>(0x3FD921FB54442D18ull);

	static COMPILETIMEEVAL double ONE_FIFTEENTH = std::bit_cast<double>(0x3FB1111111111111ull);

	// = 0.14285714285714285 = 1/7
	static COMPILETIMEEVAL double INV_7 = std::bit_cast<double>(0x3FC2492492492492ull);

	// = 1/e = 0.36787944117144233
	static COMPILETIMEEVAL double INV_E = std::bit_cast<double>(0x3FD78B56362CEF38ull);

	// = 4.656612877414201e-10 (1/INT_MAX)
	constexpr double INV_INT_MAX = std::bit_cast<double>(0x3E00000000400000ull);

	// = π/(2√2) = 1.1780972450961724
	static COMPILETIMEEVAL double PI_SQRT_TWO_BY_FOUR = std::bit_cast<double>(0x3FF2D97C7F3321D2ull);

	// = 3.141592653589793 (180 degrees)
	static COMPILETIMEEVAL double GAME_PI = std::bit_cast<double>(0x400921FB54442D18ULL);

	// = 6.283185307179586 (360 degrees)
	static COMPILETIMEEVAL double GAME_TWOPI = std::bit_cast<double>(0x401921FB54442D18ULL);

	// = 12.566370614359172 (720 degrees)
	static COMPILETIMEEVAL double GAME_FOURPI = std::bit_cast<double>(0x402921FB54442D18ULL);

	static COMPILETIMEEVAL double CloudHeightFactor = std::bit_cast<double>(0x401BDFB59E463B4Eull);

	// = 0.029999999329447746 ≈ 0.03 = 3/100
	static COMPILETIMEEVAL float flt_1 = std::bit_cast<float>(0x3CF5C28Fu);

	// = 0.014999999664723873 ≈ 0.015 = 15/1000 = 3/200
	static COMPILETIMEEVAL float flt_2 = std::bit_cast<float>(0x3C75C28Fu);

	static COMPILETIMEEVAL double BUILDINGLIGHT_SCALLING_FACTOR = std::bit_cast<double>(0x4017E4B17E4B17E4ull);

	static COMPILETIMEEVAL float ONE_TWENTIETH_F = std::bit_cast<float>(0x3D4CCCCDu); // ≈ 0.05 (best float representation)

	static COMPILETIMEEVAL double ONE_HALF = // or HALF
		std::bit_cast<double>(uint64_t { 0x3FE0000000000000ull }); // = 0.5

	//Ares

	// = -0.00009587526218325454
	static COMPILETIMEEVAL double APPROX_CORRECTION_TERM_1 = std::bit_cast<double>(0xBF19221476583A71ull);

	// = -2.44921270764e-16
	static COMPILETIMEEVAL double APPROX_CORRECTION_TERM_2 = std::bit_cast<double>(0xBCB1A5FFFFFFDA55ull);

	//sqrt(2)
	static COMPILETIMEEVAL double SQRT_TWO = std::bit_cast<double>(0x3FF6A09E667F3BCDull);

	//~360° (wraps back to 0)
	static COMPILETIMEEVAL  uint16_t BINARY_ANGLE_MASK = 0x3FFF;

	//Generating std::cos(Math::DIRECTION_FIXED_MAGIC) :
	static COMPILETIMEEVAL double COS_DIRECTION_FIXED_MAGIC = std::bit_cast<double>(0x3FEFFFFFFD884E88ull);
	//Generating std::sin(Math::DIRECTION_FIXED_MAGIC) :
	static COMPILETIMEEVAL double SIN_DIRECTION_FIXED_MAGIC = std::bit_cast<double>(0xBF19222D97F9FC90ull);
	//Generating std::sqrt(10000.0) :
	static COMPILETIMEEVAL double SQRT_TENTOUSAND = std::bit_cast<double>(0x4059000000000000ull);
	//Generating std::sin(Math::PI_BY_FOUR_ACCURATE) :
	static COMPILETIMEEVAL double SIN_PI_BY_FOUR_ACCURATE = std::bit_cast<double>(0x3FE6A09E667F3BCCull);
	//Generating std::cos(Math::PI_BY_FOUR_ACCURATE) :
	static COMPILETIMEEVAL double COS_PI_BY_FOUR_ACCURATE = std::bit_cast<double>(0x3FE6A09E667F3BCDull);
	//Generating std::sin(Math::PI_BY_TWO_ACCURATE) :
	static COMPILETIMEEVAL double SIN_PI_BY_TWO_ACCURATE = std::bit_cast<double>(0x3FF0000000000000ull);
	//Generating std::cos(Math::PI_BY_TWO_ACCURATE) :
	static COMPILETIMEEVAL double COS_PI_BY_TWO_ACCURATE = std::bit_cast<double>(0x3C91A60000000000ull);
	//Generating std::sqrt(3) :
	static COMPILETIMEEVAL double SQRT_THREE = std::bit_cast<double>(0x3FFBB67AE8584CAAull);
	//Generating std::sqrt(5) :
	static COMPILETIMEEVAL double SQRT_FIVE = std::bit_cast<double>(0x4001E3779B97F4A8ull);
	//Generating std::sqrt(8) :
	static COMPILETIMEEVAL double SQRT_EIGHT = std::bit_cast<double>(0x4006A09E667F3BCDull);

	static OPTIONALINLINE COMPILETIMEEVAL double rad2deg(double rad) { return rad * 180.0 / GAME_PI; }
	static OPTIONALINLINE COMPILETIMEEVAL double deg2rad(double deg) { return deg * GAME_PI / 180.0; }

	template<typename T> requires std::is_integral_v<T> || std::is_floating_point_v<T>
	static COMPILETIMEEVAL float DEG_TO_RADF(T val) { return (((float)val) * GAME_PI / 180.0f); }



#ifndef Original

	static OPTIONALINLINE uint64_t F2I64(double val)
	{
		double something = val;
		__asm { fld something };
		ASM_CALL(0x7C5F00);
	}

	static OPTIONALINLINE int F2I(double val)
	{
		return int(F2I64(val));
	}

	static OPTIONALINLINE double __cdecl pow(double a, double b)
	{
		__asm {
			push    dword ptr[b + 4]     // High DWORD of b
			push    dword ptr[b]       // Low DWORD of b
			push    dword ptr[a + 4]     // High DWORD of a
			push    dword ptr[a]       // Low DWORD of a
			mov     eax, 0x7C8FC9         // Load address
			call    eax                   // Call function
			add     esp, 16             // Clean up stack
			// Result already in ST(0), will be returned
		}
	}

	static OPTIONALINLINE double powb(double base, double exponent)
	{
		double result;
		__asm {
			finit                         // Clear FPU
			fld     qword ptr[base]       // ST(0) = base
			fld     qword ptr[exponent]   // ST(0) = exp, ST(1) = base
			mov     eax, 0x7C8FB0
			call    eax
			fstp    qword ptr[result]
			finit                         // Clear FPU again
		}
		return result;
	}

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

	float FORCEDINLINE sqrt(int value) noexcept {
		return Math::sqrt(static_cast<double>(value));
	}
#else

	float FORCEDINLINE sqrt(int value) noexcept {
		return (float)std::sqrt((double)value);
	}

	static FORCEDINLINE double __cdecl acos(double a1) {
		return std::acos(a1);
	}

	static FORCEDINLINE double __cdecl asin(double a1) {
		return std::asin(a1);
	}

	static FORCEDINLINE double __cdecl atan(double a1) {
		return std::atan(a1);
	}

	static FORCEDINLINE double __cdecl atan2(double a1, double a2) {
		return std::atan2(a1, a2);
	}

	static FORCEDINLINE float __cdecl cos(double a1) {
		return (float)std::cos(a1);
	}

	static FORCEDINLINE float __cdecl sin(double a1) {
		return (float)std::sin(a1);
	}

	static FORCEDINLINE float __cdecl sqrt(double a1) {
		return (float)std::sqrt(a1);
	}

	static FORCEDINLINE double __cdecl tan(double a1) {
		return std::tan(a1);
	}

	//===================================

	static FORCEDINLINE double __cdecl acos(float a1) {
		return std::acos((double)a1);
	}

	static FORCEDINLINE double __stdcall asin(float a1) {
		return std::asin((double)a1);
	}

	static FORCEDINLINE double __stdcall atan(float a1) {
		return std::atan((double)a1);
	}

	static FORCEDINLINE double __stdcall atan2(float a1, float a2) {
		return std::atan2((double)a1, (double)a2);
	}

	static FORCEDINLINE double __cdecl cos(float a1) {
		return std::cos((double)a1);
	}

	static FORCEDINLINE double __cdecl sin(float a1) {
		return std::sin((double)a1);
	}

	static FORCEDINLINE double __cdecl sqr(float a1) {
		return std::sqrt((double)a1);
	}

	static FORCEDINLINE double __cdecl tan(float a1) {
		return std::tan((double)a1);
	}

	template<typename T>
	T FORCEDINLINE pow(T a , T b) noexcept {
		return std::pow(a, b);
	}

#endif

	//famous Quake 3 Fast Inverse Square Root
	OPTIONALINLINE COMPILETIMEEVAL float Q_invsqrt(float number) noexcept {
		static_assert(std::numeric_limits<float>::is_iec559, "Float Must be IEC559 !");
		COMPILETIMEEVAL float threehalfs = 1.5F;

		float x2 = number * 0.5F;
		float y = number;
		int32_t i = std::bit_cast<int32_t>(y);   // type-safe bit level hacking
		i = 0x5f3759df - (i >> 1);               // what the fuck?
		y = std::bit_cast<float>(i);
		y = y * (threehalfs - (x2 * y * y));   // 1st iteration
	 // y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration,this can be removed

		return y;
	}

	template <typename T>
	[[nodiscard]] OPTIONALINLINE COMPILETIMEEVAL auto signum(T x)noexcept
			requires std::is_arithmetic_v<T> {
		if COMPILETIMEEVAL (std::is_signed<T>()){
			return (T{ 0 } < x) - (x < T{ 0 });
		} else {
			return T{ 0 } < x;
		}
	}

	OPTIONALINLINE COMPILETIMEEVAL int StepSnapClamped(int value, int min, int max, int step)
	{
		if (value < min) return min;

		int snapped = min + ((value - min) / step) * step;
		return (snapped > max) ? max : snapped;
	}

	//https://github.com/rhalbersma/xstd/blob/master/include/xstd/cstdlib.hpp

	template<class T>
	OPTIONALINLINE T abs(T const& x) noexcept
		requires std::is_arithmetic_v<T> {
		if constexpr (std::is_same<T, double>::value || std::is_same<T, int>::value || std::is_same<T, short>::value)
			return static_cast<T>(std::abs(x));
		else if constexpr (std::is_same<T, float>::value)
			return static_cast<T>(std::fabs(x));
		else
			static_assert("Not supported");
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

	template <typename T> requires std::is_arithmetic_v<T> && std::is_floating_point_v<T>
	OPTIONALINLINE COMPILETIMEEVAL T PercentAtMax(T value, int range, int distance, T percent_at_max)
	{
		if (range != 0 && percent_at_max != 1.0f)
		{

			/**
			 *  Calculate the damage at the furthest point.
			 */
			T at_max = static_cast<T>(value) * percent_at_max;

			/**
			 *  Reduce the damage based on the distance and the damage % at max distance.
			 */
			value = static_cast<T>((static_cast<T>(value) - at_max) * static_cast<T>(range - distance) / static_cast<T>(range) + at_max);

			/**
			 *  Our damage was originally positive, don't allow it to go negative.
			 */
			value = MaxImpl(static_cast<T>(0), value);
		}

		return value;
	}
};

// float cmp
#define CLOSE_ENOUGH(x, y) \
	(Math::abs(x - y) < 0.001)

#define LESS_EQUAL(x, y) \
	((x - y) <= 0.001)