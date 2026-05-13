#pragma once

class Simd
{
public:
	enum class Level : int
	{
		Vanilla = -1,
		Scalar = 0,
		SSE2,
		AVX2,
		AVX512
	};

private:
	static Level CurrentLevel;
	static bool Initialized;

public:
	static void Initialize(Level maxLevel = Level::AVX512);
	static Level GetCurrentLevel()
	{
		return Simd::CurrentLevel;
	}

	static const char* GetLevelName(Level level);
	static Level ParseLevel(const char* pValue, Level defaultLevel);

	template <typename T>
	static T Dispatch(
		T scalarPath,
		T sse2Path,
		T avx2Path = nullptr,
		T avx512Path = nullptr)
	{
		switch (GetCurrentLevel())
		{
		case Level::AVX512:
			if (avx512Path)
				return avx512Path;
			[[fallthrough]];
		case Level::AVX2:
			if (avx2Path)
				return avx2Path;
			[[fallthrough]];
		case Level::SSE2:
			if (sse2Path)
				return sse2Path;
			[[fallthrough]];
		default:
		case Level::Scalar:
			return scalarPath;
		}
	}

private:
	static Level DetectBestLevel();
	static Level ClampLevel(Level level, Level maxLevel);
};
