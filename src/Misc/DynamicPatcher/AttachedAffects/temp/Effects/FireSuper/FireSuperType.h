#pragma once

#include <Utilities/TemplateDef.h>
#include "../CommonProperties.h"

struct AEFireSuperData
{
	Valueable<bool> Enabled;
	ValueableVector<SuperWeaponTypeClass*> Supers;
	ValueableVector<double> Chances;
	Valueable<int> InitDelay;
	Nullable<Point2D> RandomInitDelay;
	Valueable<int> Delay;
	Nullable<Point2D> RandomDelay;
	Valueable<int> LaunchCount;
	Valueable<bool> RealLaunch;

	Valueable<int> WeaponIndex;
	Valueable<bool> ToTarget;


	AEFireSuperData() :
		Enabled { false }
		, Supers {}
		, Chances {}
		, InitDelay { 0 }
		, RandomInitDelay {}
		, Delay { 0 }
		, RandomDelay {}
		, LaunchCount { 1 }
		, RealLaunch { false }
		, WeaponIndex { -1 }
		, ToTarget { true }
	{ }


	void Read(INI_EX& parser, const char* pSection, bool IsElite = false)
	{
		const char* pTag = IsElite ? "Elite" : "";
		char nBuffer[0x100];

		IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "Types");
		Supers.Read(parser, pSection, nBuffer, true);

		if (!Supers.empty())
		{
			Enabled = true;

			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "LaunchCount");
			LaunchCount.Read(parser, pSection, nBuffer);

			if (LaunchCount <= 0) {
				Enabled = false;
				return;
			}

			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "Chances");
			Chances.Read(parser, pSection, nBuffer);

			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "InitDelay");
			InitDelay.Read(parser, pSection, nBuffer);

			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "RandomInitDelay");
			RandomInitDelay.Read(parser, pSection, nBuffer);

			if (RandomInitDelay.isset() && (abs(RandomInitDelay.Get().X) > 0 || abs(RandomInitDelay.Get().Y) > 0))
			{
				int InitialMinDelay = abs(RandomInitDelay.Get().X);
				int InitialMaxDelay = abs(RandomInitDelay.Get().Y);

				if (InitialMinDelay > InitialMaxDelay)
					std::swap(InitialMinDelay, InitialMaxDelay);

				RandomInitDelay = Point2D(InitialMinDelay, InitialMaxDelay);
			}

			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "InitDelay");
			InitDelay.Read(parser, pSection, nBuffer);

			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "RandomDelay");
			RandomDelay.Read(parser, pSection, nBuffer);

			if (RandomDelay.isset() && (abs(RandomDelay.Get().X) > 0 || abs(RandomDelay.Get().Y) > 0))
			{
				int InitialMinDelay = abs(RandomDelay.Get().X);
				int InitialMaxDelay = abs(RandomDelay.Get().Y);

				if (InitialMinDelay > InitialMaxDelay)
					std::swap(InitialMinDelay, InitialMaxDelay);

				RandomDelay = Point2D(InitialMinDelay, InitialMaxDelay);
			}

			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "RealLaunch");
			RealLaunch.Read(parser, pSection, nBuffer);

			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "Weapon");
			WeaponIndex.Read(parser, pSection, nBuffer);

			IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%s%s", pTag, "ToTarget");
			ToTarget.Read(parser, pSection, nBuffer);
		}
	}

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(Enabled)
			.Process(Supers)
			.Process(Chances)
			.Process(InitDelay)
			.Process(RandomInitDelay)
			.Process(Delay)
			.Process(RandomDelay)
			.Process(LaunchCount)
			.Process(RealLaunch)
			.Process(WeaponIndex)
			.Process(ToTarget)
			;
	}
};

struct AEFireSuperType
{
	AEFireSuperData Data;
	AEFireSuperData EliteData;
	CommonProperties CommonData;

	AEFireSuperType() :
		Data { }
		, EliteData { }
		, CommonData { }
	{}

	template <typename T>
	void Serialize(T& Stm)
	{
		Data.Serialize(Stm);
		EliteData.Serialize(Stm);
		CommonData.Serialize(Stm);
	}
};
