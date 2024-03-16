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
		std::string pTag = IsElite ? "Elite" : "";
		//char nBuffer[0x100];

		std::string _buffer = std::format("FireSuperWeapon.{}Types", pTag);
		//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sTypes", pTag);
		Supers.Read(parser, pSection,  _buffer.c_str(), true);

		if (!Supers.empty())
		{
			Enabled = true;
			_buffer = std::format("FireSuperWeapon.{}LaunchCount", pTag);
			//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sLaunchCount", pTag);
			LaunchCount.Read(parser, pSection,  _buffer.c_str());

			if (LaunchCount <= 0) {
				Enabled = false;
				return;
			}

			_buffer = std::format("FireSuperWeapon.{}Chances", pTag);
			//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sChances", pTag);
			Chances.Read(parser, pSection,  _buffer.c_str());

			_buffer = std::format("FireSuperWeapon.{}InitDelay", pTag);
			//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sInitDelay", pTag);
			InitDelay.Read(parser, pSection,  _buffer.c_str());

			_buffer = std::format("FireSuperWeapon.{}RandomInitDelay", pTag);
			//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sRandomInitDelay", pTag);
			RandomInitDelay.Read(parser, pSection,  _buffer.c_str());

			if (RandomInitDelay.isset() && (abs(RandomInitDelay.Get().X) > 0 || abs(RandomInitDelay.Get().Y) > 0))
			{
				int InitialMinDelay = abs(RandomInitDelay.Get().X);
				int InitialMaxDelay = abs(RandomInitDelay.Get().Y);

				if (InitialMinDelay > InitialMaxDelay)
					std::swap(InitialMinDelay, InitialMaxDelay);

				RandomInitDelay = Point2D(InitialMinDelay, InitialMaxDelay);
			}

			_buffer = std::format("FireSuperWeapon.{}InitDelay", pTag);
			//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sInitDelay", pTag);
			InitDelay.Read(parser, pSection,  _buffer.c_str());

			_buffer = std::format("FireSuperWeapon.{}RandomDelay", pTag);
			//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sRandomDelay", pTag);
			RandomDelay.Read(parser, pSection,  _buffer.c_str());

			if (RandomDelay.isset() && (abs(RandomDelay.Get().X) > 0 || abs(RandomDelay.Get().Y) > 0))
			{
				int InitialMinDelay = abs(RandomDelay.Get().X);
				int InitialMaxDelay = abs(RandomDelay.Get().Y);

				if (InitialMinDelay > InitialMaxDelay)
					std::swap(InitialMinDelay, InitialMaxDelay);

				RandomDelay = Point2D(InitialMinDelay, InitialMaxDelay);
			}

			_buffer = std::format("FireSuperWeapon.{}RealLaunch", pTag);
			//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sRealLaunch", pTag);
			RealLaunch.Read(parser, pSection,  _buffer.c_str());

			_buffer = std::format("FireSuperWeapon.{}Weapon", pTag);
			//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sWeapon", pTag);
			WeaponIndex.Read(parser, pSection,  _buffer.c_str());

			_buffer = std::format("FireSuperWeapon.{}ToTarget", pTag);
			//IMPL_SNPRNINTF(nBuffer, sizeof(nBuffer), "FireSuperWeapon.%sToTarget", pTag);
			ToTarget.Read(parser, pSection, _buffer.c_str());
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
