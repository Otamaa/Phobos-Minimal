#pragma once

#include <Utilities/TemplateDefB.h>

class BuildingTypeClass;
class PrismForwardingData
{
public:
	enum class EnabledState : int
	{
		No, Yes, Forward, Attack
	};

	//properties
	EnabledState Enabled;	//is this tower a prism tower? Forward means can support, but not attack. Attack means can attack but not support.
	ValueableVector<BuildingTypeClass*> Targets;	//the types of buiding that this tower can forward to
	Nullable<int> MaxFeeds;					//max number of towers that can feed this tower
	Valueable<signed int> MaxChainLength;				//max length of any given (preceding) branch of the network
	Nullable<int> MaxNetworkSize;				//max number of towers that can be in the network
	Nullable<int> SupportModifier; 				//Per-building PrismSupportModifier
	Valueable<signed int> DamageAdd; 					//amount of flat damage to add to the firing beam (before multiplier)
	Valueable<signed int> Intensity;						//amount to adjust beam thickness by when supported
	Valueable<int> ChargeDelay;					//the amount to delay start of charging per backward chain
	Valueable<bool> ToAllies;						//can this tower support allies' towers or not
	Valueable<bool> BreakSupport;					//can the slave tower become a master tower at the last second
	Valueable<signed int> SupportWeaponIndex;
	Valueable<signed int> EliteSupportWeaponIndex;

	//methods
	COMPILETIMEEVAL signed int GetUnusedWeaponSlot(BuildingTypeClass* pThis, bool elite)
	{
		for (auto idxWeapon = 2u; idxWeapon < 13u; ++idxWeapon)
		{ //13-18 is AlternateFLH0-4
			auto Weapon = (elite ? pThis->GetEliteWeapon(idxWeapon) : pThis->GetWeapon(idxWeapon))->WeaponType;

			if (!Weapon)
			{
				return static_cast<int>(idxWeapon);
			}
		}
		return -1;
	}

	COMPILETIMEEVAL void Initialize(BuildingTypeClass* pThis)
	{
		this->Enabled = EnabledState::No;
		if (pThis == RulesClass::Instance->PrismType)
		{
			this->Enabled = EnabledState::Yes;
		}
		this->Targets.push_back(pThis);
	}

	void LoadFromINIFile(BuildingTypeClass* pThis, CCINIClass* pINI);

	COMPILETIMEEVAL int GetMaxFeeds() const
	{
		return this->MaxFeeds.Get(RulesClass::Instance->PrismSupportMax);
	}

	COMPILETIMEEVAL int GetMaxNetworkSize() const
	{
		return this->MaxNetworkSize.Get(RulesClass::Instance->PrismSupportMax);
	}

	COMPILETIMEEVAL int GetSupportModifier() const
	{
		return this->SupportModifier.Get(RulesClass::Instance->PrismSupportModifier);
	}

	COMPILETIMEEVAL bool CanAttack() const
	{
		return this->Enabled == EnabledState::Yes || this->Enabled == EnabledState::Attack;
	}

	COMPILETIMEEVAL bool CanForward() const
	{
		return this->Enabled == EnabledState::Yes || this->Enabled == EnabledState::Forward;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	COMPILETIMEEVAL PrismForwardingData* AsPointer() const { return const_cast<PrismForwardingData*>(this); }

	// constructor
	PrismForwardingData() : Enabled(EnabledState::No),
		Targets(),
		MaxFeeds(),
		MaxChainLength(1),
		MaxNetworkSize(),
		SupportModifier(),
		DamageAdd(0),
		Intensity(-2),
		ChargeDelay(1),
		ToAllies(false),
		BreakSupport(false),
		SupportWeaponIndex(-1),
		EliteSupportWeaponIndex(-1)
	{
	}

	~PrismForwardingData() = default;

public:

	PrismForwardingData(const PrismForwardingData&) = delete;
	PrismForwardingData& operator = (const PrismForwardingData&) = delete;
	PrismForwardingData& operator = (PrismForwardingData&&) = delete;
};