#pragma once
#include <Helpers/CompileTime.h>
#include <ASMMacros.h>

class FPSCounter
{
public:
	//!< The number of frames processed in the last second.
	static COMPILETIMEEVAL reference<unsigned int, 0xABCD44u> const CurrentFrameRate{};

	//!< The total number of frames elapsed.
	static COMPILETIMEEVAL reference<unsigned int, 0xABCD48u> const TotalFramesElapsed{};

	//!< The time it took to process TotalFramesElapsed frames.
	static COMPILETIMEEVAL reference<unsigned int, 0xABCD4Cu> const TotalTimeElapsed{};

	//!< Whether the current fps is considered too low.
	static COMPILETIMEEVAL reference<bool, 0xABCD50u> const ReducedEffects{};

	//!< The average frame rate for all frames processed.
	static OPTIONALINLINE double GetAverageFrameRate()
	{
		if(TotalTimeElapsed) {
			return static_cast<double>(TotalFramesElapsed)
				/ static_cast<double>(TotalTimeElapsed);
		}

		return 0.0;
	}
};

class Detail {
public:
	//!< What is considered the minimum acceptable FPS.
	static COMPILETIMEEVAL reference<unsigned int, 0x829FF4u> const MinFrameRate{};

	//!< The zone that needs to be left to change
	static COMPILETIMEEVAL reference<unsigned int, 0x829FF8u> const BufferZoneWidth{};

	//!< The minimum frame rate considering the buffer zone.
	static unsigned int __cdecl GetMinFrameRate()
		{ JMP_STD(0x55AF60); }

	//!< Whether effects should be reduced.
	static OPTIONALINLINE bool ReduceEffects()
	{
		return FPSCounter::CurrentFrameRate < GetMinFrameRate();
	}
};
