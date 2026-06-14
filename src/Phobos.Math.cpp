#include "Phobos.Math.h"

#include <Utilities/Debug.h>
#include <YRMath.h>

float WWMath_Sin(double radians)
{
	COMPILETIMEEVAL float radToIndex = std::bit_cast<float>(0x4522F983u);

	// Convert angle to fixed-point table index
	const long long idx64 = static_cast<long long>(radians * radToIndex);

	const bool isOdd = (idx64 & 1) != 0;               // v2
	int idx = static_cast<int>((idx64 / 2) & 0x80001FFF); // v3

	// Sign-extend the 13-bit index (because &0x80001FFF keeps only certain bits)
	if (idx < 0)
	{
		int t = (idx - 1) | 0xFFFFE000;
		idx = t + 1 + ((t + 1) < 0 ? 0x2000 : 0);
	}

	// Odd-bit correction (round up if necessary)
	if (isOdd && idx < 8191)
	{
		idx++;
	}


	return Math::FastMath_sin_Table[idx];
}

float WWMath_Cos(double radians)
{
	COMPILETIMEEVAL float radToIndex = std::bit_cast<float>(0x4522F983u);

	int64_t t = (int64_t)(radians * radToIndex);

	int bit = t & 1;
	int idx = (t >> 1) & 0x80001FFF;

	if (idx < 0)
	{
		idx = ((idx - 1) | 0xFFFFE000) + 1;
		idx += (idx < 0) ? 0x2800 : 0x0800; // 10240 or 2048
	}
	else
	{
		idx += 2048;
	}

	if (bit && idx < 10239)
		idx++;

	return Math::FastMath_sin_Table[idx];
}

float WWMath_sqrt(double n)
{
	union FastMathUnion
	{
		float f;
		unsigned int i;
	};

	// Handle zero early
	if (n == 0.0)
		return 0.0f;

	// Use absolute value, but only via their original codepath
	double v = n;
	if (n < 0.0)
		v = -n;

	// Reinterpret bits as IEEE754 float
	FastMathUnion num;
	num.f = static_cast<float>(v);

	// Extract mantissa (lower 23 bits)
	uint32_t mant = num.i & 0x7FFFFF;

	// Extract unbiased exponent
	int exp = (num.i >> 23) - 127;

	// If exponent is odd, normalize the mantissa
	if (exp & 1)
		mant |= 0x800000u;

	// Build the result float from:
	//   - new exponent: (exp >> 1) + 127
	//   - table lookup from mantissa
	uint32_t out_bits =
		(((exp >> 1) + 127) << 23) |
		Math::FastMath_sqrt_Table[mant >> 10];

	FastMathUnion out;
	out.i = out_bits;
	return out.f;
}

bool MathTesters::InspectMathDetailed()
{
	struct MathTest
	{
		const char* name;
	};

	struct DoubleMathTest : public MathTest
	{
		std::pair<const char*, double> first;
		std::pair<const char*, double> second;

		DoubleMathTest(const char* testName, const char* firstname, double firstRes, const char* secondname, double secondRes)
			: MathTest { testName }, first { firstname, firstRes }, second { secondname , secondRes } {}
	};

	struct FloatMathTest : public MathTest
	{
		std::pair<const char*, float>  first;
		std::pair<const char*, float>  second;

		FloatMathTest(const char* testName, const char* firstname, float firstRes, const char* secondname, float secondRes)
			: MathTest { testName }, first { firstname ,firstRes }, second { secondname , secondRes } {}
	};

	struct IntMathTest : public MathTest
	{
		std::pair<const char*, int> first;
		std::pair<const char*, int>  second;

		IntMathTest(const char* testName, const char* firstname, int firstRes, const char* secondname, int secondRes)
			: MathTest { testName }, first { firstname ,firstRes }, second { secondname , secondRes } {}
	};

	// Collect all test cases
	std::vector<DoubleMathTest> testsdouble;
	std::vector<FloatMathTest> testsfloat;
	std::vector<IntMathTest> testsint;

	/*testsint.emplace_back(*/
	//	"cast testing , game ftol infamous for true'ing (F2I(-5.00) == -4.00)",
	//	"dll cast",
	//	(int)(-5.00),
	//	"game ftol",
	//	Math::F2I(-5.00)
	//);
	//
	//tests.emplace_back(
	//	"expected : 0.3763770469559380854890894443664",
	//	Unsorted::LevelHeight / gcem::sqrt(float(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LevelHeight / std::sqrt(float(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.9264665771223091335116047861327",
	//	Unsorted::LeptonsPerCell / gcem::sqrt(float(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LeptonsPerCell / std::sqrt(float(Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.3522530794922131411764879370407",
	//	Unsorted::LevelHeight / gcem::sqrt(float(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LevelHeight / std::sqrt(float(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.8670845033654477321267395373309",
	//	Unsorted::LeptonsPerCell / gcem::sqrt(float(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LeptonsPerCell / std::sqrt(float(2 * Unsorted::LevelHeight * Unsorted::LevelHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.5333964609104418418483761938761",
	//	Unsorted::CellHeight / gcem::sqrt(float(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::CellHeight / std::sqrt(float(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	//tests.emplace_back(
	//	"expected : 0.6564879518897745745826168540013",
	//	Unsorted::LeptonsPerCell / gcem::sqrt(float(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell)),
	//	Unsorted::LeptonsPerCell / std::sqrt(float(2 * Unsorted::CellHeight * Unsorted::CellHeight + Unsorted::LeptonsPerCell * Unsorted::LeptonsPerCell))
	//);

	/*testsfloat.emplace_back(
		"cos(1.570748388432313)",
		WWMath_Cos(1.570748388432313),
		(float)std::cos(1.570748388432313)
	);

	testsfloat.emplace_back(
		"sin(1.570748388432313)",
		WWMath_Sin(1.570748388432313),
		(float)std::sin(1.570748388432313)
	);

	testsfloat.emplace_back(
		"cos(0.7853262558535721)",
		WWMath_Cos(0.7853262558535721),
		(float)std::cos(0.7853262558535721)
	);

	testsfloat.emplace_back(
		"sin(0.7853262558535721)",
		WWMath_Sin(0.7853262558535721),
		(float)std::sin(0.7853262558535721)
	);

	testsfloat.emplace_back(
		"1 / sqrt(5)",
		1.0 / WWMath_sqrt(5.0),
		1.0 / (float)std::sqrt(5.0)
	);

	testsdouble.emplace_back(
		"2 / sqrt(5)",
		2.0 / WWMath_sqrt(5.0),
		2.0 / (float)std::sqrt(5.0)
	);*/

	//constexpr double base_ = gcem::pow(256.0, 2.0);
	//testsdouble.emplace_back(
	//	"POW 1",
	//	"gcem" , base_,
	//	"std" , std::pow(256.0, 2.0)
	//);

	////testsdouble.emplace_back(
	////	"POW 2",
	////	base_,
	////	Game_pow2(256.0, 2.0)
	////);

	//constexpr float tile_diag = gcem::sqrt(base_ + base_);
	//static constexpr reference<double, 0xAC1368> TileDiagonal;
	//constexpr double MAP_TILE_DIAGONAL = std::bit_cast<double>(0x4076A09E60000000ull);
	//testsdouble.emplace_back(
	//	"Tile Diagonal1 sqrt",
	//	"gcem" , tile_diag,
	//	"std"  , std::sqrt(base_ + base_)
	//);


	//// = 1.047197551196598 (π/3, 60 degrees)
	//constexpr double PI_BY_THREE = std::bit_cast<double>(0x3FF0C15238732D65ull);
	//constexpr double tan_ = gcem::tan(Math::PI_BY_TWO_APPROX - PI_BY_THREE);
	//constexpr double floor = gcem::tan(Math::PI_BY_TWO_APPROX - PI_BY_THREE) * tile_diag * 0.5;
	//double std_floor = std::tan(Math::PI_BY_TWO_APPROX - PI_BY_THREE) * tile_diag * 0.5;
	//static constexpr reference<int, 0xAC13C8> CellHeight;
	//testsdouble.emplace_back(
	//	"Floor Height tan",
	//	 "gcem" , tan_,
	//	 "std"   , std::tan(Math::PI_BY_TWO_APPROX - PI_BY_THREE)
 //	);

	//testsdouble.emplace_back(
	//	"Floor Height overral",
	//	"gcem", floor,
	//	"std", std_floor
	//);

	// Run all tests
	int passed = 0;
	int failed = 0;

	Debug::Log("\n=== Math Validation Report ===\n\n");

	for (const auto& test : testsdouble)
	{
		if (*(uint32_t*)&test.first.second == *(uint32_t*)&test.second.second)
		{
			Debug::Log("[PASS] %s\n", test.name);
			passed++;
		}
		else
		{
			Debug::Log("[FAIL] %s\n", test.name);
			Debug::Log("       %s:    %.20f (0x%08X)\n",
					  test.first.first, test.first.second, *(uint32_t*)&test.first.second);
			Debug::Log("       %s:   %.20f (0x%08X)\n",
					  test.second.first, test.second.second, *(uint32_t*)&test.second);
			Debug::Log("       Diff:     %.20e\n\n",
					  test.first.second - test.second.second);
			failed++;
		}
	}

	for (const auto& test : testsfloat)
	{
		if (*(uint32_t*)&test.first.second == *(uint32_t*)&test.second.second)
		{
			Debug::Log("[PASS] %s\n", test.name);
			passed++;
		}
		else
		{
			Debug::Log("[FAIL] %s\n", test.name);
			Debug::Log("       %s:    %.20f (0x%08X)\n",
					  test.first.first, test.first.second, *(uint32_t*)&test.first.second);
			Debug::Log("       %s:   %.20f (0x%08X)\n",
					  test.second.first, test.second.second, *(uint32_t*)&test.second);
			Debug::Log("       Diff: %.20e\n\n",
					  test.first.second - test.second.second);
			failed++;
		}
	}

	for (const auto& test : testsint)
	{
		if (*(uint32_t*)&test.first.second == *(uint32_t*)&test.second.second)
		{
			Debug::Log("[PASS] %s\n", test.name);
			passed++;
		}
		else
		{
			Debug::Log("[FAIL] %s\n", test.name);
			Debug::Log("       %s:    %d (0x%08X)\n",
					  test.first.first, test.first.second, *(uint32_t*)&test.first.second);
			Debug::Log("       %s:   %d (0x%08X)\n",
					  test.second.first, test.second.second, *(uint32_t*)&test.second);
			Debug::Log("       Diff: %d\n\n",
					  test.first.second - test.second.second);
			failed++;
		}
	}

	Debug::Log("\n=== Summary ===\n");
	Debug::Log("Passed: %d\n", passed);
	Debug::Log("Failed: %d\n", failed);
	Debug::Log("Total:  %d\n", passed + failed);

	if (failed > 0)
	{
		Debug::FatalError(
			"\n%d math validation(s) failed!\n"
			"gcem constexpr != std runtime values\n"
			"This WILL cause multiplayer desyncs.\n",
			failed
		);
		return false;
	}


	struct Generate
	{
		double value;
		const char* name;
	};

	std::vector<Generate> gens;

	//gens.emplace_back(std::sqrt(3),"sqrt(3)");

	for (const auto& gen : gens)
	{
		uint64_t bits = std::bit_cast<uint64_t>(gen.value);
		Debug::Log("Generating %s :  0x%016llX\n", gen.name, (unsigned long long)bits);
	}
	return true;
}
