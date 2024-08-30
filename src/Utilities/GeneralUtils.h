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
#include <Utilities/Iterator.h>

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
	static bool IsValidString(const wchar_t* str);
	static void IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min = MIN_VAL(int), int max = MAX_VAL(int));
	static void DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min = MIN_VAL(double), double max = MAX_VAL(double));
	static const wchar_t* LoadStringOrDefault(const char* key, const wchar_t* defaultValue);
	static const wchar_t* LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue);
	static void AdjacentCellsInRange(std::vector<CellStruct>& nCells, size_t range);
	static const bool ProduceBuilding(HouseClass* pOwner, int idxBuilding);

	static constexpr bool is_number(const std::string& s)
	{
		return !s.empty() && std::find_if(s.begin(),
			s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
	}

	// Gets integer representation of color from ColorAdd corresponding to given index, or 0 if there's no color found.
	// Code is pulled straight from game's draw functions that deal with the tint colors.
	static constexpr inline int GetColorFromColorAdd(int colorIndex)
	{
		auto const& colorAdd = RulesClass::Instance->ColorAdd;
		int colorValue = 0;

		if (colorIndex < 0 || colorIndex >= (sizeof(colorAdd) / sizeof(ColorStruct)))
			return colorValue;

		return GetColorFromColorAdd(colorAdd[colorIndex]);
	}

	static constexpr inline void GetRandomAnimVal(int& Idx, int count, int facing, bool bRandom)
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

	static constexpr inline int GetColorFromColorAdd(ColorStruct const& colors)
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

	static constexpr inline bool IsOperator(char c)
	{
		return c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')';
	}

	static constexpr inline  bool OperatorPriorityGreaterThan(char opa, char opb)
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

	static constexpr int GetValue(int a1)
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

	static constexpr void CalculateShakeVal(int& pShakeVal, int nInput , bool Alternate = true)
	{
		if (!Alternate) {
			pShakeVal = nInput;
			return;
		}

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

	static constexpr std::string IntToDigits(int num)
	{
		std::string sDigits;

		if (num == 0)
		{
			sDigits.push_back('0');
			return sDigits;
		}

		while (num)
		{
			sDigits.push_back(static_cast<char>(num % 10) + '0');
			num /= 10;
		}

		std::reverse(sDigits.begin(), sDigits.end());
		return sDigits;
	}

	static constexpr inline const int GetRangedRandomOrSingleValue(const Point2D& range)
	{
		return range.X >= range.Y ?
			range.X : ScenarioClass::Instance->Random.RandomRanged(range.X, range.Y);
	}

	template<typename T>
	static constexpr inline int GetRandomValue(Vector2D<T> range, int defVal)
	{
		int min = static_cast<int>(range.X);
		int max = static_cast<int>(range.Y);
		if (min > max)
		{
			int tmp = min;
			min = max;
			max = tmp;
		}
		if (max > 0)
		{
			return ScenarioClass::Instance->Random.RandomRanged(min, max);
		}
		return defVal;
	}

	static constexpr inline CoordStruct GetRandomOffset(int min, int max)
	{
		double r = ScenarioClass::Instance->Random.RandomRanged(min, max);
		if (r > 0)
		{
			double theta = ScenarioClass::Instance->Random.RandomDouble() * Math::TwoPi;
			CoordStruct offset { static_cast<int>(r * Math::cos(theta)), static_cast<int>(r * Math::sin(theta)), 0 };
			return offset;
		}
		return CoordStruct::Empty;
	}

	static constexpr inline CoordStruct GetRandomOffset(double maxSpread, double minSpread)
	{
		int min = static_cast<int>((minSpread <= 0 ? 0 : minSpread) * 256);
		int max = static_cast<int>((maxSpread > 0 ? maxSpread : 1) * 256);
		return GetRandomOffset(min, max);
	}

	static constexpr inline double GetRangedRandomOrSingleValue(const PartialVector2D<double>& range)
	{
		int min = static_cast<int>(range.X * 100);
		int max = static_cast<int>(range.Y * 100);

		return range.X >= range.Y || range.ValueCount < 2 ? range.X : (ScenarioClass::Instance->Random.RandomRanged(min, max) / 100.0);
	}

	static constexpr inline int GetRangedRandomOrSingleValue(const PartialVector2D<int>& range)
	{
		return range.X >= range.Y || range.ValueCount < 2 ? range.X : ScenarioClass::Instance->Random.RandomRanged(range.X, range.Y);
	}

	static const double GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor const ArmorType);

	// Weighted random element choice (weight) - roll for one.
	// Takes a vector of integer type weights, which are then summed to calculate the chances.
	// Returns chosen index or -1 if nothing is chosen.
	static inline constexpr int ChooseOneWeighted(const double& dice, const std::vector<int>& weights)
	{
		float sum = 0.0;
		float sum2 = 0.0;

		std::for_each(weights.begin(), weights.end(), [&sum](auto const weights) { sum += weights; });;

		for (size_t i = 0; i < weights.size(); i++)
		{
			sum2 += weights[i];
			if (dice < (sum2 / sum))
				return i;
		}

		return -1;
	}

	static constexpr inline PhobosMap<Point2D, int> MakeTargetPad(std::vector<int>& weights, int count, int& maxValue)
	{
		const int weightCount = weights.size();
		PhobosMap<Point2D, int> targetPad {};
		maxValue = 0;

		for (int index = 0; index < count; index++)
		{
			Point2D target {};
			target.X = maxValue;
			int weight = 1;
			if (weightCount > 0 && index < weightCount)
			{
				int w = weights[index];
				if (w > 0)
				{
					weight = w;
				}
			}
			maxValue += weight;
			target.Y = maxValue;
			targetPad[target] = index;
		}
		return targetPad;
	}

	static constexpr inline int Hit(PhobosMap<Point2D, int>& targetPad, int maxValue)
	{
		int index = 0;
		int p = ScenarioClass::Instance->Random.RandomFromMax(maxValue);
		for (auto& it : targetPad)
		{
			Point2D tKey = it.first;
			if (p >= tKey.X && p < tKey.Y)
			{
				index = it.second;
				break;
			}
		}
		return index;
	}

	static constexpr inline bool Bingo(double chance)
	{
		if (chance > 0)
		{
			return chance >= 1 || chance >= ScenarioClass::Instance->Random.RandomDouble();
		}
		return false;
	}

	static constexpr inline bool Bingo(std::vector<double>& chances, int index)
	{
		int size = chances.size();
		if (size < index + 1)
		{
			return true;
		}
		double chance = chances[index];
		return Bingo(chance);
	}

	// Direct multiplication pow
	static constexpr inline double FastPow(double x, double n)
	{
		double r = 1.0;

		while (n > 0)
		{
			r *= x;
			--n;
		}

		return r;
	}

	// 2nd order Pade approximant just in case someone complains about performance
	static constexpr inline double Pade2_2(double in)
	{
		const double s = in - static_cast<int>(in);
		return GeneralUtils::FastPow(0.36787944117144233, static_cast<int>(in))
			* (12. - 6 * s + s * s) / (12. + 6 * s + s * s);
	}

	template<typename T>
	static constexpr inline T SecsomeFastPow(T x, size_t n)
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
	static constexpr inline bool HasHealthRatioThresholdChanged(double const& oldRatio, double const& newRatio)
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

	static CellClass* GetCell(CellClass* pIn, CoordStruct& InOut, size_t nSpread, bool EmptyCell)
	{
		if (!pIn)
			return nullptr;

		CellStruct const cell = CellClass::Coord2Cell(InOut);
		std::vector<CellStruct> nDummy {};
		GeneralUtils::AdjacentCellsInRange(nDummy, nSpread);
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

	static inline constexpr ColorStruct HSV2RGB(int h, int s, int v)
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

	static int constexpr CountDigitsInNumber(int number)
	{
		int digits = 0;

		while (number)
		{
			number /= 10;
			digits++;
		}

		return digits;
	}

	// Calculates a new coordinates based on current & target coordinates within specified distance (can be negative to switch the direction) in leptons.
	static CoordStruct CalculateCoordsFromDistance(CoordStruct currentCoords, CoordStruct targetCoords, int distance)
	{
		int deltaX = currentCoords.X - targetCoords.X;
		int deltaY = targetCoords.Y - currentCoords.Y;

		double atan = Math::atan2((double)deltaY, (double)deltaX);
		double radians = (((atan - Math::HalfPi) * (1.0 / Math::GameDegreesToRadiansCoefficient)) - Math::GameDegrees90) * Math::GameDegreesToRadiansCoefficient;
		int x = static_cast<int>(targetCoords.X + Math::cos(radians) * distance);
		int y = static_cast<int>(targetCoords.Y - Math::sin(radians) * distance);

		return { x, y, targetCoords.Z };
	}

	template <typename T>
	static constexpr void shuffleVector(std::vector<T>& items)
	{
		std::shuffle(items.begin(), items.end(), ScenarioClass::Instance->Random.Random());
	}

#pragma region Otamaa
	static const int GetAnimIndexFromFacing(FootClass* pFoot, int nVectorSize);
	static const int GetAnimIndexFromFacing(TechnoClass* pFirer, int nVectorSize);
	static AnimTypeClass* GetAnimFacingFromVector(TechnoClass* pFirer, const Iterator<AnimTypeClass*> iter);

	static inline constexpr int ScaleF2I(float value, int scale)
	{
		value = std::clamp(value, 0.0f, 1.0f);
		return static_cast<int>(value * scale);
	}


	//Point2Dir
	static const DirStruct Desired_Facing(int x1, int y1, int x2, int y2)
	{
		DirStruct dir {};
		unsigned short value = static_cast<short>(int((Math::atan2(static_cast<double>(y2 - y1), static_cast<double>(x2 - x1)) - Math::deg2rad(-(360.0 / (USHRT_MAX - 1))))));
		dir.SetValue<16>(value);
		return dir;
	}

	static const DirStruct Desired_Facing(const Point2D& point1, const Point2D& point2)
	{
		return Desired_Facing(point1.X, point1.Y, point2.X, point2.Y);
	}

	static const DirStruct Coord2DirSTruct(CoordStruct Loc1, CoordStruct Loc2)
	{
		auto angle = Math::atan2((double)(Loc2.X - Loc1.X), (double)(Loc2.Y - Loc1.Y));
		auto theta = angle * (180 / Math::Pi);
		return DirStruct(theta);
	}

	static inline constexpr Leptons PixelToLeptons(int pixel)
	{ return Leptons((((pixel * 256) + (60 / 2) - ((pixel < 0) ? (60 - 1) : 0)) / 60)); }

	static inline constexpr Leptons DistanceToLeptons(int distance)
	{ return Leptons(distance * 256); }

	//https://noobtuts.com/cpp/compare-float-values
	static FORCEINLINE bool cmpf(float A, float B, float epsilon = 0.005f)
	{
		return (fabs(A - B) < epsilon);
	}

	template <typename T>
	static constexpr void Shuffle(std::vector<T>& items)
	{
		if (items.size() <= 1)
			return;

		size_t size = items.size();

		for (size_t i = 0; i < size; i++)
		{
			size_t idx = ScenarioClass::Instance->Random.RandomRanged(i, size - 1);
			std::swap(items[i], items[idx]);
		}
	}

	template<bool UseCriticalRandomNumber = true>
	static constexpr int GetRandomValue(const Point2D point, int defVal)
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

	template<typename T>
	static inline constexpr T GetItemByHealthRatio(double ratio, T green , T yellow , T red) {

		if (ratio <= RulesClass::Instance->ConditionRed)
			return red;
		else if (ratio <= RulesClass::Instance->ConditionYellow)
			return yellow;

		return green;
	}

	static constexpr CoordStruct CoordinatesFromCell(const CellStruct& cell, bool snap = false, int zValue = 0) {
		CoordStruct tmp { cell.X * 256, cell.Y * 256, zValue };
		if (snap) {
			tmp.X += 256 / 2;
			tmp.Y += 256 / 2;
		}
		return tmp;
	}

	static constexpr CellStruct CellFromCoordinates(const CoordStruct& coord) {
		return { short(coord.X / 256) , short(coord.Y / 256) };
	}

	// Zero out a non-array pointer.
	template <typename T>
	static inline void
		MemsetZero(T* t)
	{
		std::memset(t, 0, sizeof(*t));
	}

	template <typename T>
	struct cast_to_pointer {
		static inline constexpr void* cast(const T& t) {
			return reinterpret_cast<void*>((uintptr_t)t);
		}
	};

	template <typename T>
	struct cast_to_pointer<T*> {
		static inline constexpr const void* cast(const T* ptr) {
			return ptr;
		}

		static inline constexpr void* cast(T* ptr) {
			return ptr;
		}
	};

	static int GetLSAnimHeightFactor(AnimTypeClass* pType, CellClass* pCell , bool checklevel = false);
#pragma endregion
};
