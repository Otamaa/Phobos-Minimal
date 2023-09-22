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
	Nullable<int> MyHeight;						//Per-building PrismSupportHeight
	Valueable<signed int> Intensity;						//amount to adjust beam thickness by when supported
	Valueable<int> ChargeDelay;					//the amount to delay start of charging per backward chain
	Valueable<bool> ToAllies;						//can this tower support allies' towers or not
	Valueable<bool> BreakSupport;					//can the slave tower become a master tower at the last second
	Valueable<signed int> SupportWeaponIndex;
	Valueable<signed int> EliteSupportWeaponIndex;

	//methods
	signed int GetUnusedWeaponSlot(BuildingTypeClass* pThis, bool elite);
	void Initialize(BuildingTypeClass* pThis);
	void LoadFromINIFile(BuildingTypeClass* pThis, CCINIClass* pINI);

	int GetMaxFeeds() const
	{
		return this->MaxFeeds.Get(RulesClass::Instance->PrismSupportMax);
	}

	int GetMaxNetworkSize() const
	{
		return this->MaxNetworkSize.Get(RulesClass::Instance->PrismSupportMax);
	}

	int GetSupportModifier() const
	{
		return this->SupportModifier.Get(RulesClass::Instance->PrismSupportModifier);
	}

	int GetMyHeight() const
	{
		return this->MyHeight.Get(RulesClass::Instance->PrismSupportHeight);
	}

	bool CanAttack() const
	{
		return this->Enabled == EnabledState::Yes || this->Enabled == EnabledState::Attack;
	}

	bool CanForward() const
	{
		return this->Enabled == EnabledState::Yes || this->Enabled == EnabledState::Forward;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	PrismForwardingData* AsPointer() const { return const_cast<PrismForwardingData*>(this); }

	// constructor
	PrismForwardingData() : Enabled(EnabledState::No),
		Targets(),
		MaxFeeds(),
		MaxChainLength(1),
		MaxNetworkSize(),
		SupportModifier(),
		DamageAdd(0),
		MyHeight(),
		Intensity(-2),
		ChargeDelay(1),
		ToAllies(false),
		BreakSupport(false),
		SupportWeaponIndex(-1),
		EliteSupportWeaponIndex(-1)
	{
	}
};