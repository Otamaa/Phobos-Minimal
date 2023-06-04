#pragma once
#include <StringTable.h>
#include <CCINIClass.h>
#include <CellSpread.h>
#include <StopWatch.h>
#include <Theater.h>
#include <ScenarioClass.h>

#include <Helpers/Iterators.h>
#include <Helpers/Enumerators.h>

#include <Utilities/Debug.h>
#include <Utilities/Constructs.h>

#include <string.h>
#include <iterator>
#include <vector>
#include <YRMath.h>

#define MIN_VAL(x) std::numeric_limits<x>::min()
#define MAX_VAL(x) std::numeric_limits<x>::max()

class GeneralUtils final
{
	NO_CONSTRUCT_CLASS(GeneralUtils)
public:

	static bool IsValidString(const char* str);
	static void IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min = MIN_VAL(int), int max = MAX_VAL(int));
	static void DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min = MIN_VAL(double), double max = MAX_VAL(double));
	static const wchar_t* LoadStringOrDefault(const char* key, const wchar_t* defaultValue);
	static const wchar_t* LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue);
	static void AdjacentCellsInRange(std::vector<CellStruct>& nCells , size_t range);
	static const bool ProduceBuilding(HouseClass* pOwner, int idxBuilding);

	static bool is_number(const std::string& s)
	{
		return !s.empty() && std::find_if(s.begin(),
			s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
	}

	// Gets integer representation of color from ColorAdd corresponding to given index, or 0 if there's no color found.
	// Code is pulled straight from game's draw functions that deal with the tint colors.
	static inline int GetColorFromColorAdd(int colorIndex)
	{
		auto const& colorAdd = RulesClass::Instance->ColorAdd;
		int colorValue = 0;

		if (colorIndex < 0 || colorIndex >= (sizeof(colorAdd) / sizeof(ColorStruct)))
			return colorValue;

		return GetColorFromColorAdd(colorAdd[colorIndex]);
	}

	static inline void GetRandomAnimVal(int& Idx, int count, int facing, bool bRandom)
	{
		if (bRandom)
			Idx = ScenarioClass::Instance->Random.RandomFromMax(count - 1);
		else
		{
			if (count >= 8)
			{
				Idx = count % 2 == 0 ? Idx *= static_cast<int>(facing / 256.0) : count;
			}
		}
	}

	static inline int GetColorFromColorAdd(ColorStruct const& colors)
	{
		int colorValue = 0;
		int red = colors.R;
		int green = colors.G;
		int blue = colors.B;

		if (Drawing::ColorMode() == RGBMode::RGB565)
			colorValue |= blue | (32 * (green | (red << 6)));

		if (Drawing::ColorMode() != RGBMode::RGB655)
			colorValue |= blue | (((32 * red) | (green >> 1)) << 6);

		colorValue |= blue | (32 * ((32 * red) | (green >> 1)));

		return colorValue;
	}

	static inline bool IsOperator(char c)
	{
		return c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')';
	}

	static inline  bool OperatorPriorityGreaterThan(char opa, char opb)
	{
		if (opb == '(' || opb == ')')
			return false;

		if (opa == '(' || opa == ')')
			return true;

		if (opb == '*' || opb == '/' || opb == '%')
			return false;

		if (opa == '*' || opa == '/' || opa == '%')
			return true;

		return false;
	}

	static int GetValue(int a1)
	{
		int result = 0;

		if (a1)
		{
			while (1)
			{
				a1 >>= 1;
				if (!a1)
					break;
				++result;
			}
		}

		return result;
	}

	static void CalculateShakeVal(int& pShakeVal, int nInput)
	{
		int v4 = nInput;
		if (pShakeVal < 0)
		{
			v4 = -nInput;
			if (-nInput >= pShakeVal)
				return;
		}
		else if (nInput <= pShakeVal)
		{
			return;
		}

		int v7 = v4;
		int v6 = v7;

		if (Random2Class::NonCriticalRandomNumber->RandomBool())
			v6 = -v7;

		pShakeVal = v6;
	}

	static void IntToDigits(std::string& sDigits, int num)
	{
		if (num == 0)
		{
			sDigits.push_back('0');
			return;
		}

		while (num)
		{
			sDigits.push_back(static_cast<char>(num % 10) + '0');
			num /= 10;
		}

		std::reverse(sDigits.begin(), sDigits.end());
	}

	static inline const int GetRangedRandomOrSingleValue(const Point2D& range)
	{
		return range.X >= range.Y ?
			range.X : ScenarioClass::Instance->Random.RandomRanged(range.X, range.Y);
	}

	static inline double GetRangedRandomOrSingleValue(const PartialVector2D<double>& range)
	{
		int min = static_cast<int>(range.X * 100);
		int max = static_cast<int>(range.Y * 100);

		return range.X >= range.Y || range.ValueCount < 2 ? range.X : (ScenarioClass::Instance->Random.RandomRanged(min, max) / 100.0);
	}

	static inline int GetRangedRandomOrSingleValue(const PartialVector2D<int>& range)
	{
		return range.X >= range.Y || range.ValueCount < 2 ? range.X : ScenarioClass::Instance->Random.RandomRanged(range.X, range.Y);
	}

	static const double GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor const ArmorType);

	// Weighted random element choice (weight) - roll for one.
	// Takes a vector of integer type weights, which are then summed to calculate the chances.
	// Returns chosen index or -1 if nothing is chosen.
	static inline int ChooseOneWeighted(const double& dice, const std::vector<int>& weights)
	{
		float sum = 0.0;
		float sum2 = 0.0;

		std::for_each(weights.begin(), weights.end(), [&sum](auto const weights) { sum += weights; });

		//for (size_t i = 0; i < weights->size(); i++)
		//	sum += (*weights)[i];

		for (size_t i = 0; i < weights.size(); i++)
		{
			sum2 += weights[i];
			if (dice < (sum2 / sum))
				return i;
		}

		return -1;
	}

	// Direct multiplication pow
	static inline double FastPow(double x, double n)
	{
		double r = 1.0;

		while (n > 0)
		{
			r *= x;
			--n;
		}

		return r;
	}

	template<typename T>
	static inline T SecsomeFastPow(T x, size_t n)
	{
		// Real fast pow calc x^n in O(log(n))
		T result = 1;
		T base = x;
		while (n)
		{
			if (n & 1) result *= base;
			base *= base;
			n >>= 1;
		}
		return result;
	}

	// Checks if health ratio has changed threshold (Healthy/ConditionYellow/Red).
	static inline bool HasHealthRatioThresholdChanged(double const& oldRatio, double const& newRatio)
	{
		if (oldRatio == newRatio)
			return false;

		if (oldRatio > RulesClass::Instance->ConditionYellow && newRatio <= RulesClass::Instance->ConditionYellow)
		{
			return true;
		}
		else if (oldRatio <= RulesClass::Instance->ConditionYellow && oldRatio > RulesClass::Instance->ConditionRed &&
			(newRatio <= RulesClass::Instance->ConditionRed || newRatio > RulesClass::Instance->ConditionYellow))
		{
			return true;
		}
		else if (oldRatio <= RulesClass::Instance->ConditionRed && newRatio > RulesClass::Instance->ConditionRed)
		{
			return true;
		}

		return false;
	}

	static bool ApplyTheaterSuffixToString(char* str);
	static bool ApplyTheaterExtToString(std::string& flag);
	static std::string ApplyTheaterSuffixToString(const std::string& str);

	template <size_t size>
	static inline void lowercase(char (&nBuff)[size] , char const (&nData)[size])
	{
		for (size_t i = 0; i < size; ++i) {
			nBuff[i] = (char)std::tolower(nData[i]);
		}
	}

	template <size_t size>
	static inline void uppercase(char(&nBuff)[size], char(&nData)[size])
	{
		for (size_t i = 0; i < size; ++i) {
			nBuff[i] = (char)std::toupper(nData[i]);
		}
	}

	//
	//  Lowercases string
	//
	template <typename T>
	static std::basic_string<T> lowercase(const std::basic_string<T>& s)
	{
		std::basic_string<T> s2 = s;
		std::transform(s2.begin(), s2.end(), s2.begin(),
			[](const T v) { return static_cast<T>(std::tolower(v)); });
		return s2;
	}

	//
	// Uppercases string
	//
	template <typename T>
	static std::basic_string<T> uppercase(const std::basic_string<T>& s)
	{
		std::basic_string<T> s2 = s;
		std::transform(s2.begin(), s2.end(), s2.begin(),
			[](const T v) { return static_cast<T>(std::toupper(v)); });
		return s2;
	}

	static CellClass* GetCell(CellClass* pIn, CoordStruct& InOut, size_t nSpread, bool EmptyCell)
	{
		if(!pIn)
			return nullptr;

		CellStruct const cell = CellClass::Coord2Cell(InOut);
		std::vector<CellStruct> nDummy {};
		GeneralUtils::AdjacentCellsInRange(nDummy ,nSpread);
		int const max = (int)nDummy.size();

		for (int i = 0; i < max; i++)
		{
			CellStruct const offset = nDummy[ScenarioClass::Instance->Random.RandomFromMax(max - 1)];

			if (!offset.IsValid())
				continue;

			if (const auto pNewCell = MapClass::Instance->TryGetCellAt(cell + offset))
			{
				if (pNewCell->LandType != pIn->LandType || (EmptyCell && pNewCell->GetContent()))
					continue;

				InOut = pNewCell->GetCoordsWithBridge();
				return pNewCell;
				break;
			}
		}

		return nullptr;
	}

	static inline ColorStruct HSV2RGB(int h, int s, int v)
	{
		float R = 0.0f, G = 0.0f, B = 0.0f;
		float C = 0, X = 0, Y = 0, Z = 0;
		float H = (float)(h);
		float S = (float)(s) / 100.0f;
		float V = (float)(v) / 100.0f;
		if (S == 0)
			R = G = B = V;
		else
		{
			H = H / 60;
			int i = (int)H;
			C = H - i;

			X = V * (1 - S);
			Y = V * (1 - S * C);
			Z = V * (1 - S * (1 - C));

			switch (i)
			{
			case 0: R = V; G = Z; B = X; break;
			case 1: R = Y; G = V; B = X; break;
			case 2: R = X; G = V; B = Z; break;
			case 3: R = X; G = Y; B = V; break;
			case 4: R = Z; G = X; B = V; break;
			case 5: R = V; G = X; B = Y; break;
			}
		}

		return { (BYTE)(R * 255), (BYTE)(G * 255), (BYTE)(B * 255) };
	}


	static const char* GetLocomotionName(const CLSID& clsid);

#pragma region Otamaa
	static const int GetAnimIndexFromFacing(FootClass* pFoot, int nVectorSize);

	static const int ScaleF2I(float value, int scale)
	{
		value = std::clamp(value, 0.0f, 1.0f);
		return static_cast<int>(value * scale);
	}


	//Point2Dir
	static const DirStruct Desired_Facing(int x1, int y1, int x2, int y2)
	{
		DirStruct dir {};
		unsigned short value = static_cast<short>(int((std::atan2(static_cast<double>(y2 - y1), static_cast<double>(x2 - x1)) - Math::deg2rad(-(360.0 / (USHRT_MAX - 1))))));
		dir.SetValue<16>(value);
		return dir;
	}

	static const DirStruct Desired_Facing(const Point2D &point1, const Point2D &point2)
	{
		return Desired_Facing(point1.X, point1.Y, point2.X, point2.Y);
	}

	static const DirStruct Coord2DirSTruct(CoordStruct Loc1, CoordStruct Loc2)
	{
		auto angle = std::atan2((double)(Loc2.X - Loc1.X), (double)(Loc2.Y - Loc1.Y));
		auto theta = angle * (180 / Math::Pi);
		return DirStruct(theta);
	}

	static const Leptons PixelToLeptons(int pixel)
	{ return Leptons((((pixel * 256) + (60 / 2) - ((pixel < 0) ? (60 - 1) : 0)) / 60)); }

	static const Leptons DistanceToLeptons(int distance)
	{ return Leptons(distance * 256); }

	//https://noobtuts.com/cpp/compare-float-values
	static __forceinline bool cmpf(float A, float B, float epsilon = 0.005f) {
		return (fabs(A - B) < epsilon);
	}

	template<bool UseCriticalRandomNumber = true>
	static int GetRandomValue(const Point2D point, int defVal)
	{
		int min = point.X;
		int max = point.Y;

		if (min > max)
		{
			min = max;
			max = point.X;
		}

		if (max > 0)
		{
			if constexpr (UseCriticalRandomNumber)
				return ScenarioClass::Instance->Random.RandomRanged(min, max);
			else
				return Random2Class::NonCriticalRandomNumber->RandomRanged(min, max);
		}

		return defVal;
	}

#pragma endregion

};

struct StopwatchLogger
{
	Stopwatch watch;
	const char* func;
	const char* name;

	StopwatchLogger(const char* pFunc = nullptr, const char* pName = nullptr) : func(pFunc), name(pName) {}

	~StopwatchLogger() {
		Debug::Log("STOPWATCH %s (%s): %lld\n", func, name, watch.get_nano().QuadPart);
	}
};
