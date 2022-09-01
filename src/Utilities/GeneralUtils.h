#pragma once
#include <StringTable.h>
#include <CCINIClass.h>
#include <CellSpread.h>
#include <StopWatch.h>
#include <Theater.h>

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
	static std::vector<CellStruct> AdjacentCellsInRange(unsigned int range);
	static const bool ProduceBuilding(HouseClass* pOwner, int idxBuilding);

	static inline const int GetRangedRandomOrSingleValue(Point2D const& range)
	{
		return range.X >= range.Y ?
			range.X : ScenarioClass::Instance->Random.RandomRanged(range.X, range.Y);
	}

	static const double GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor const ArmorType);

	// Weighted random element choice (weight) - roll for one.
	// Takes a vector of integer type weights, which are then summed to calculate the chances.
	// Returns chosen index or -1 if nothing is chosen.
	static inline int ChooseOneWeighted(const double& dice, const std::vector<int>* weights)
	{
		float sum = 0.0;
		float sum2 = 0.0;

		std::for_each(weights->begin(), weights->end(), [&sum](auto const weights) { sum += weights; });

		//for (size_t i = 0; i < weights->size(); i++)
		//	sum += (*weights)[i];

		for (size_t i = 0; i < weights->size(); i++)
		{
			sum2 += (*weights)[i];
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

	// Checks if health ratio has changed threshold (Healthy/ConditionYellow/Red).
	static inline bool HasHealthRatioThresholdChanged(double const& oldRatio, double const& newRatio)
	{
		if (oldRatio == newRatio)
			return false;

		if (oldRatio > RulesGlobal->ConditionYellow && newRatio <= RulesGlobal->ConditionYellow)
		{
			return true;
		}
		else if (oldRatio <= RulesGlobal->ConditionYellow && oldRatio > RulesGlobal->ConditionRed &&
			(newRatio <= RulesGlobal->ConditionRed || newRatio > RulesGlobal->ConditionYellow))
		{
			return true;
		}
		else if (oldRatio <= RulesGlobal->ConditionRed && newRatio > RulesGlobal->ConditionRed)
		{
			return true;
		}

		return false;
	}

	static bool inline ApplyTheaterSuffixToString(char* str )
	{
		  str = _strlwr(str);

		if (auto pSuffix = CRT::strstr(str, "~~~"))
		{
			auto pExtension = Theater::GetTheater(ScenarioClass::Instance->Theater).Extension;
			pSuffix[0] = pExtension[0];
			pSuffix[1] = pExtension[1];
			pSuffix[2] = pExtension[2];
			return true;
		}

		return false;
	}

	static CellClass* GetCell(CellClass* pIn, CoordStruct& InOut, size_t nSpread, bool EmptyCell)
	{
		CellStruct cell = CellClass::Coord2Cell(InOut);
		auto const nDummy = GeneralUtils::AdjacentCellsInRange(nSpread);

		int const max = (int)nDummy.size();
		for (int i = 0; i < max; i++)
		{
			int index = ScenarioGlobal->Random.RandomFromMax(max - 1);
			CellStruct const offset = nDummy[index];

			if (offset == CellStruct::Empty)
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

#pragma region Otamaa
	static const int GetAnimIndexFromFacing(FootClass* pFoot, int nVectorSize);

	static const int ScaleF2I(float value, int scale)
	{
		value = std::clamp(value, 0.0f, 1.0f);
		return static_cast<int>(value * scale);
	}


	//Point2Dir
	static const DirStruct& Desired_Facing(int x1, int y1, int x2, int y2)
	{
		DirStruct dir;
		unsigned short value = static_cast<short>(Game::F2I((Math::atan2(static_cast<double>(y2 - y1), static_cast<double>(x2 - x1)) - Math::deg2rad(-(360.0 / (USHRT_MAX - 1))))));
		dir.value(value);
		return dir;
	}

	static const DirStruct& Desired_Facing(const Point2D &point1, const Point2D &point2)
	{
		return Desired_Facing(point1.X, point1.Y, point2.X, point2.Y);
	}

	static const DirStruct& Coord2DirSTruct(CoordStruct Loc1, CoordStruct Loc2)
	{
		auto angle = Math::atan2((double)(Loc2.X - Loc1.X), (double)(Loc2.Y - Loc1.Y));
		auto theta = angle * (180 / Math::Pi);

		return DirStruct(theta);
	}

	static const Leptons& PixelToLeptons(int pixel)
	{ return Leptons((((pixel * 256) + (48 / 2) - ((pixel < 0) ? (48 - 1) : 0)) / 48)); }

	static const Leptons& DistanceToLeptons(int distance)
	{ return Leptons(distance * 256); }

	//https://noobtuts.com/cpp/compare-float-values
	static __forceinline bool cmpf(float A, float B, float epsilon = 0.005f) {
		return (fabs(A - B) < epsilon);
	}

	template<typename First , typename ... T>
	static inline decltype(auto) variadic_min(const First& f, const T& ... t) {
		const First* retval = std::addressof(f);
		( (retval = std::addressof(std::min(*retval,t))),...);
		return *retval;
	}

	template<typename First, typename ... T>
	static inline decltype(auto) variadic_max(const First& f, const T& ... t)
	{
		const First* retval = std::addressof(f);
		((retval = std::addressof(std::max(*retval, t))), ...);
		return *retval;
	}

	template<typename First, typename ... T>
	static inline decltype(auto) variadic_fmin(const First& f, const T& ... t)
	{
		First retval = f;
		((retval = std::fmin(retval, t)), ...);
		return retval;
	}

	template<typename First, typename ... T>
	static inline decltype(auto) variadic_fmax(const First& f, const T& ... t)
	{
		First retval = f;
		((retval = std::fmax(retval, t)), ...);
		return retval;
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
