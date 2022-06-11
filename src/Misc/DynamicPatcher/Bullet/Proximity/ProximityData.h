#pragma once

//for Bullet
#include <Unsorted.h>

#include <Utilities/TemplateDef.h>

class WarheadTypeClass;
class WeaponTypeClass;
struct ProximityData
{
	Valueable<bool> Force;
	Valueable<int> Arm;
	Valueable<int> ZOffset;
	Valueable<bool> AffectsOwner;
	Valueable<bool> AffectsAllies;
	Valueable<bool> AffectsEnemies;
	Valueable<bool> Penetration;
	Valueable<WarheadTypeClass*> PenetrationWarhead;
	Valueable<WeaponTypeClass*> PenetrationWeapon;
	Valueable<int> PenetrationTimes;
	Valueable<bool> PenetrationBuildingOnce;

	ProximityData() :
		Force { false }
		, Arm { 128 }
		, ZOffset { Unsorted::LevelHeight }
		, AffectsOwner { false }
		, AffectsAllies { false }
		, AffectsEnemies { true }

		, Penetration { false }
		, PenetrationWarhead { nullptr }
		, PenetrationWeapon { nullptr }
		, PenetrationTimes { -1 }
		, PenetrationBuildingOnce { false }
	{ }

	~ProximityData() = default;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ Debug::Log("Loading Element From ProximityData ! \n"); return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm)
	{ return Serialize(Stm); }


	void Read(INI_EX& parser, const char* pSection, bool Allocate = false)
	{
		Force.Read(parser, pSection, "Proximity.Force");
		Arm.Read(parser, pSection, "Proximity.Arm");
		ZOffset.Read(parser, pSection, "Proximity.ZOffset");
		AffectsOwner.Read(parser, pSection, "Proximity.AffectsOwner");
		AffectsAllies.Read(parser, pSection, "Proximity.AffectsAllies");
		AffectsEnemies.Read(parser, pSection, "Proximity.AffectsEnemies");
		Penetration.Read(parser, pSection, "Proximity.Penetration");
		PenetrationWarhead.Read(parser, pSection, "Proximity.PenetrationWarhead", Allocate);
		PenetrationWeapon.Read(parser, pSection, "Proximity.PenetrationWeapon", Allocate);
		PenetrationTimes.Read(parser, pSection, "Proximity.PenetrationTimes");
		PenetrationBuildingOnce.Read(parser, pSection, "Proximity.PenetrationBuildingOnce");
	}

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Force)
			.Process(Arm)
			.Process(ZOffset)
			.Process(AffectsOwner)
			.Process(AffectsAllies)
			.Process(AffectsEnemies)
			.Process(Penetration)
			.Process(PenetrationWarhead)
			.Process(PenetrationWeapon)
			.Process(PenetrationTimes)
			.Process(PenetrationBuildingOnce)
			.Success()
			;
	}
};