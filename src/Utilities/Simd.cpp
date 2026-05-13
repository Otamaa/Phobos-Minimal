#include "Simd.h"

#include <Utilities/Debug.h>
#include <intrin.h>

namespace
{
	enum CpuidRegister : int
	{
		EAX = 0,
		EBX = 1,
		ECX = 2,
		EDX = 3
	};

	constexpr int CpuFeatureSSE2 = 26;
	constexpr int CpuFeatureSSE = 25;
	constexpr int CpuFeatureMMX = 23;
	constexpr int CpuFeatureSSE3 = 0;
	constexpr int CpuFeatureSSE41 = 19;
	constexpr int CpuFeatureSSE42 = 20;
	constexpr int CpuFeatureOsXSave = 27;
	constexpr int CpuFeatureAVX = 28;
	constexpr int CpuFeatureAVX2 = 5;
	constexpr int CpuFeatureAVX512F = 16;
	constexpr int CpuFeatureAVX512BW = 30;
	constexpr int CpuFeatureAVX512VL = 31;

	constexpr unsigned __int64 Xcr0SSE = 1ull << 1;
	constexpr unsigned __int64 Xcr0AVX = 1ull << 2;
	constexpr unsigned __int64 Xcr0Opmask = 1ull << 5;
	constexpr unsigned __int64 Xcr0ZmmHi256 = 1ull << 6;
	constexpr unsigned __int64 Xcr0Hi16Zmm = 1ull << 7;

	bool IsBitSet(int value, int bit)
	{
		return (value & (1 << bit)) != 0;
	}
}

Simd::Level Simd::CurrentLevel = Simd::Level::SSE2;
bool Simd::Initialized = false;

void Simd::Initialize(Level maxLevel)
{
	if (Simd::Initialized)
		return;

	const Level detectedLevel = Simd::DetectBestLevel();
	Simd::CurrentLevel = Simd::ClampLevel(detectedLevel, maxLevel);
	Simd::Initialized = true;

	Debug::Log("SIMD dispatch path: %s (detected: %s, max: %s)\n", Simd::GetLevelName(Simd::GetCurrentLevel()), Simd::GetLevelName(detectedLevel), Simd::GetLevelName(maxLevel));
}

const char* Simd::GetLevelName(Level level)
{
	switch (level)
	{
	case Level::Vanilla:
		return "Vanilla";
	case Level::Scalar:
		return "Scalar";
	case Level::AVX512:
		return "AVX512"; // (AVX512F + AVX512BW + AVX512VL)
	case Level::AVX2:
		return "AVX2";
	case Level::SSE2:
		return "SSE2";
	default:
		return "SSE2";
	}
}

Simd::Level Simd::ParseLevel(const char* pValue, Level defaultLevel)
{
	if (_stricmp(pValue, "Vanilla") == 0)
		return Level::Vanilla;

	if (_stricmp(pValue, "Scalar") == 0)
		return Level::Scalar;

	if (_stricmp(pValue, "SSE2") == 0)
		return Level::SSE2;

	if (_stricmp(pValue, "AVX2") == 0)
		return Level::AVX2;

	if (_stricmp(pValue, "AVX512") == 0)
		return Level::AVX512;

	return defaultLevel;
}

Simd::Level Simd::DetectBestLevel()
{
	int cpuInfo[4] = {};
	__cpuid(cpuInfo, 0);
	const int maxBasicLeaf = cpuInfo[EAX];

	if (maxBasicLeaf < 1)
	{
		Debug::Log("SIMD CPUID fallback: maxBasicLeaf=%d, forcing SSE2 baseline\n", maxBasicLeaf);
		return Level::Scalar;
	}

	__cpuidex(cpuInfo, 1, 0);

	bool hasAVX512 = false;
	bool hasAVX2 = false;
	const bool hasSSE3Bit = IsBitSet(cpuInfo[ECX], CpuFeatureSSE3);
	const bool hasSSE41Bit = IsBitSet(cpuInfo[ECX], CpuFeatureSSE41);

	const bool hasSSE2 = IsBitSet(cpuInfo[EDX], CpuFeatureSSE2);
	const bool hasSSE41 = hasSSE2 && hasSSE3Bit && hasSSE41Bit;

	const bool hasOsXSave = IsBitSet(cpuInfo[ECX], CpuFeatureOsXSave);
	const bool hasAVX = IsBitSet(cpuInfo[ECX], CpuFeatureAVX);
	const unsigned __int64 xcr0 = (hasOsXSave && hasAVX) ? _xgetbv(0) : 0;

	const bool hasAVXState = (xcr0 & (Xcr0SSE | Xcr0AVX)) == (Xcr0SSE | Xcr0AVX);
	if (maxBasicLeaf >= 7 && hasAVXState)
	{
		__cpuidex(cpuInfo, 7, 0);
		const bool hasAVX2Bit = IsBitSet(cpuInfo[EBX], CpuFeatureAVX2);
		hasAVX2 = hasSSE41 && hasAVX && hasAVX2Bit;

		const bool hasAVX512F = IsBitSet(cpuInfo[EBX], CpuFeatureAVX512F);
		const bool hasAVX512BW = IsBitSet(cpuInfo[EBX], CpuFeatureAVX512BW);
		const bool hasAVX512VL = IsBitSet(cpuInfo[EBX], CpuFeatureAVX512VL);
		const unsigned __int64 avx512Mask = Xcr0SSE | Xcr0AVX | Xcr0Opmask | Xcr0ZmmHi256 | Xcr0Hi16Zmm;
		hasAVX512 = hasAVX2 && hasAVX512F && hasAVX512BW && hasAVX512VL && ((xcr0 & avx512Mask) == avx512Mask);
	}

	if (hasAVX512)
		return Level::AVX512;

	if (hasAVX2)
		return Level::AVX2;

	if (hasSSE2)
		return Level::SSE2;

	return Level::Scalar;
}

Simd::Level Simd::ClampLevel(Level level, Level maxLevel)
{
	if (static_cast<int>(level) > static_cast<int>(maxLevel))
		return maxLevel;

	return level;
}
