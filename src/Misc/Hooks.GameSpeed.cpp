#include <Phobos.h>
#include <Utilities/Macro.h>
#include <SessionClass.h>
#include <GameOptionsClass.h>
#include <Unsorted.h>

namespace GameSpeedTemp
{
	static int counter = 0;
}

ASMJIT_PATCH(0x69BAE7, SessionClass_Resume_CampaignGameSpeed, 0xA)
{
	GameOptionsClass::Instance->GameSpeed = Phobos::Config::CampaignDefaultGameSpeed;
	return 0x69BAF1;
}

ASMJIT_PATCH(0x55E160, SyncDelay_Start, 0x6)
{
	if (Phobos::Misc::CustomGS && !SessionClass::IsMultiplayer()) {
		auto& FrameTimer = Game::FrameTimer();
		auto const& ChangeIntervals = Phobos::Misc::CustomGS_ChangeInterval;
		auto const& ChangeDelays = Phobos::Misc::CustomGS_ChangeDelay;
		auto const& DefaultDelays = Phobos::Misc::CustomGS_DefaultDelay;

		if ((ChangeIntervals[FrameTimer.TimeLeft] > 0)
			&& (GameSpeedTemp::counter % ChangeIntervals[FrameTimer.TimeLeft] == 0))
		{
			FrameTimer.TimeLeft = ChangeDelays[FrameTimer.TimeLeft];
			GameSpeedTemp::counter = 1;
		}
		else
		{
			FrameTimer.TimeLeft = DefaultDelays[FrameTimer.TimeLeft];
			GameSpeedTemp::counter++;
		}
	}

	return 0;
}

ASMJIT_PATCH(0x55E33B, SyncDelay_End, 0x5)
{
	if (Phobos::Misc::CustomGS && SessionClass::IsSingleplayer()) {
		Game::FrameTimer->TimeLeft = GameOptionsClass::Instance->GameSpeed;
	}

	return 0;
}

