#pragma once

#include <Utilities/SavegameDef.h>

class TechnoClass;
class BuildingClass;
class AresJammer
{
	int LastScan {};							//!< Frame number when the last scan was performed.
	TechnoClass* AttachedToObject {};			//!< Pointer to game object this jammer is on
	bool Registered {};

public:

	static COMPILETIMEEVAL int ScanInterval = 30;

	bool InRangeOf(BuildingClass*);		//!< Calculates if the jammer is in range of this building.
	bool IsEligible(BuildingClass*);		//!< Checks if this building can/should be jammed.

	void Jam(BuildingClass*);				//!< Attempts to jam the given building. (Actually just registers the Jammer with it, the jamming happens in a hook.)
	void Unjam(BuildingClass*) const;			//!< Attempts to unjam the given building. (Actually just unregisters the Jammer with it, the unjamming happens in a hook.)

public:

	~AresJammer()
	{
		this->UnjamAll();
	}

	AresJammer(TechnoClass* GameObject) :
		LastScan(0),
		AttachedToObject(GameObject),
		Registered(false)
	{
	}

	AresJammer() = default;

	void UnjamAll();						//!< Unregisters this Jammer on all structures.
	void Update();							//!< Updates this Jammer's status on all eligible structures.

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AresJammer*>(this)->Serialize(Stm);
	}

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->LastScan)
			.Process(this->AttachedToObject, true)
			.Process(this->Registered)
			.Success()
			;
	}
};

template <>
struct Savegame::ObjectFactory<AresJammer>
{
	std::unique_ptr<AresJammer> operator() (PhobosStreamReader& Stm) const {
		return std::make_unique<AresJammer>();
	}
};
