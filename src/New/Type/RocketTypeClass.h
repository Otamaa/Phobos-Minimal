#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/SavegameDef.h>

class RocketTypeClass final : public Enumerable<RocketTypeClass>
{
public:

	RocketStruct RocketData;
	Valueable<WarheadTypeClass*> Warhead;
	Valueable<WarheadTypeClass*> EliteWarhead;
	Valueable<AnimTypeClass*> TakeoffAnim;
	Valueable<AnimTypeClass*> PreLauchAnim;
	Valueable<AnimTypeClass*> TrailerAnim;
	Valueable<int> TrailerSeparation;
	Valueable<WeaponTypeClass*> Weapon;
	Valueable<WeaponTypeClass*> EliteWeapon;
	Promotable<bool> Raise;
	Valueable<Point2D> Offset;

	RocketTypeClass(const char* const pTitle) : Enumerable<RocketTypeClass>(pTitle)
		, RocketData {}
		, Warhead { nullptr }
		, EliteWarhead { nullptr }
		, TakeoffAnim { nullptr }
		, PreLauchAnim { nullptr }
		, TrailerAnim { nullptr }
		, TrailerSeparation { 3 }
		, Weapon { nullptr }
		, EliteWeapon { nullptr }
		, Raise { true }
		, Offset { }
	{ }

	static void constexpr inline AddDefaults() {
		for (auto& rocket:  DefaultRockets) {
			FindOrAllocate(rocket.c_str());
		}
	}

	static constexpr std::array<std::string, 3u> DefaultRockets {
		{ "CMisl" , "DMisl" , "V3Rocket" }
	};

	static void ReadListFromINI(CCINIClass* pINI, bool bDebug = false);

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm) { Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { Serialize(Stm); }

private:

	void LoadFromINI_B(CCINIClass* pINI , size_t idx);

	template <typename T>
	void Serialize(T& Stm)
	{
		Stm
		.Process(RocketData)
		.Process(Warhead)
		.Process(EliteWarhead)
		.Process(TakeoffAnim)
		.Process(PreLauchAnim)
		.Process(TrailerAnim)
		.Process(TrailerSeparation)
		.Process(Weapon)
		.Process(EliteWeapon)
		.Process(Raise)
		.Process(Offset)
		;
	}
};