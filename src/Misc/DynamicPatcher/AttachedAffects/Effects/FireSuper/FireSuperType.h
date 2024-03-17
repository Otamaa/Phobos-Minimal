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
		std::string tag = IsElite ? "Elite" : Phobos::readDefval;
		std::string _section = pSection;
		std::string _base = "FireSuperWeapon.";

		Supers.Read(parser, pSection, (_base + pSection + tag +"Types").c_str(), true);

		if (!Supers.empty())
		{
			Enabled = true;
			LaunchCount.Read(parser, pSection, (_base + pSection + tag +"LaunchCount").c_str());

			if (LaunchCount <= 0) {
				Enabled = false;
				return;
			}

			Chances.Read(parser, pSection, (_base + pSection + tag +"Chances").c_str());
			InitDelay.Read(parser, pSection, (_base + pSection + tag +"InitDelay").c_str());
			RandomInitDelay.Read(parser, pSection, (_base + pSection + tag +"RandomInitDelay").c_str());

			if (RandomInitDelay.isset() && (abs(RandomInitDelay.Get().X) > 0 || abs(RandomInitDelay.Get().Y) > 0))
			{
				int InitialMinDelay = abs(RandomInitDelay.Get().X);
				int InitialMaxDelay = abs(RandomInitDelay.Get().Y);

				if (InitialMinDelay > InitialMaxDelay)
					std::swap(InitialMinDelay, InitialMaxDelay);

				RandomInitDelay = Point2D(InitialMinDelay, InitialMaxDelay);
			}

			InitDelay.Read(parser, pSection, (_base + pSection + tag +"InitDelay").c_str());
			RandomDelay.Read(parser, pSection, (_base + pSection + tag +"RandomDelay").c_str());

			if (RandomDelay.isset() && (abs(RandomDelay.Get().X) > 0 || abs(RandomDelay.Get().Y) > 0))
			{
				int InitialMinDelay = abs(RandomDelay.Get().X);
				int InitialMaxDelay = abs(RandomDelay.Get().Y);

				if (InitialMinDelay > InitialMaxDelay)
					std::swap(InitialMinDelay, InitialMaxDelay);

				RandomDelay = Point2D(InitialMinDelay, InitialMaxDelay);
			}

			RealLaunch.Read(parser, pSection, (_base + pSection + tag +"RealLaunch").c_str());
			WeaponIndex.Read(parser, pSection, (_base + pSection + tag +"Weapon").c_str());
			ToTarget.Read(parser, pSection,(_base + pSection + tag +"ToTarget").c_str());
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
