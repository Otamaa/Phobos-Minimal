#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/SavegameDef.h>

class RocketTypeClass final : public Enumerable<RocketTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "RocketTypes";
	static COMPILETIMEEVAL const char* ClassName = "RocketTypeClass";

public:

	RocketStruct RocketData {};
	Valueable<WarheadTypeClass*> Warhead { nullptr };
	Valueable<WarheadTypeClass*> EliteWarhead { nullptr };
	Valueable<AnimTypeClass*> TakeoffAnim { nullptr };
	Valueable<AnimTypeClass*> PreLauchAnim { nullptr };
	Valueable<AnimTypeClass*> TrailerAnim { nullptr };
	Valueable<int> TrailerSeparation { 3 };
	Valueable<WeaponTypeClass*> Weapon { nullptr };
	Valueable<WeaponTypeClass*> EliteWeapon { nullptr };
	Promotable<bool> Raise { true };
	Valueable<Point2D> Offset {};

	RocketTypeClass(const char* const pTitle) : Enumerable<RocketTypeClass>(pTitle) {}
	virtual ~RocketTypeClass() = default;

	static void COMPILETIMEEVAL OPTIONALINLINE AddDefaults() {
		Array.reserve(DefaultRockets.size());

		for (auto& rocket:  DefaultRockets) {
			FindOrAllocate(rocket);
		}
	}

	static COMPILETIMEEVAL std::array<const char*, 3u> DefaultRockets {
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