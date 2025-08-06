#pragma once

#include <Misc/Defines.h>

#include <SuperWeaponTypeClass.h>
#include <Utilities/TemplateDefB.h>
#include <Utilities/SavegameDef.h>

#include <bitset>

class BuildingClass;
class HouseClass;
class CCINIClass;
struct SpyAffectTypes
{
	AbstractTypeClass* Owner { nullptr };

	Valueable<bool> SpyEffect_Custom { false };

	NullableIdx<SuperWeaponTypeClass> SpyEffect_VictimSuperWeapon {};
	NullableIdx<SuperWeaponTypeClass> SpyEffect_InfiltratorSuperWeapon {};

	//#814
	Valueable<bool> SpyEffect_InfiltratorSW_JustGrant { false };
	Valueable<bool> SpyEffect_VictimSW_RealLaunch { false };

	Valueable<bool> SpyEffect_RevealProduction { false };
	Valueable<bool> SpyEffect_ResetSW { false };
	Valueable<bool> SpyEffect_ResetRadar { false };
	Valueable<bool> SpyEffect_RevealRadar { false };
	Valueable<bool> SpyEffect_RevealRadarPersist { false };
	Valueable<bool> SpyEffect_GainVeterancy { false };
	Valueable<bool> SpyEffect_UnReverseEngineer { false };
	std::bitset<MaxHouseCount> SpyEffect_StolenTechIndex_result {};
	Valueable<int> SpyEffect_StolenMoneyAmount { 0 };
	Valueable<float> SpyEffect_StolenMoneyPercentage { 0 };
	Valueable<int> SpyEffect_PowerOutageDuration { 0 };
	Valueable<int> SpyEffect_SabotageDelay { 0 };
	Valueable<SuperWeaponTypeClass*> SpyEffect_SuperWeapon { nullptr };
	Valueable<bool> SpyEffect_SuperWeaponPermanent { false };

	Valueable<bool> SpyEffect_InfantryVeterancy { false };
	Valueable<bool> SpyEffect_VehicleVeterancy { false };
	Valueable<bool> SpyEffect_NavalVeterancy { false };
	Valueable<bool> SpyEffect_AircraftVeterancy { false };
	Valueable<bool> SpyEffect_BuildingVeterancy { false };

	Nullable<int> SpyEffect_SellDelay {};

	Valueable<AnimTypeClass*> SpyEffect_Anim {};
	Valueable<int> SpyEffect_Anim_Duration { -1 };
	Valueable<AffectedHouse> SpyEffect_Anim_DisplayHouses { AffectedHouse::All };

	Valueable<bool> SpyEffect_SWTargetCenter { false };

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange) {
		return Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const {
		return const_cast<SpyAffectTypes*>(this)->Serialize(Stm);
	}

	void Read(CCINIClass* pINI);

	void Apply(BuildingClass* entered, HouseClass* enterer);
private:
	template <typename T>
	bool Serialize(T& Stm);

};