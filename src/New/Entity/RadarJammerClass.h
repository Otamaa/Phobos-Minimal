#pragma once

#include <Utilities/SavegameDef.h>

class TechnoClass;
class BuildingClass;
class RadarJammerClass
{
public:

	TechnoClass* AttachedToObject {};			//!< Pointer to game object this jammer is on
	int LastScan {};							//!< Frame number when the last scan was performed.
	bool Registered {};

public:

	static COMPILETIMEEVAL int ScanInterval = 30;

	bool InRangeOf(BuildingClass*);		//!< Calculates if the jammer is in range of this building.
	bool IsEligible(BuildingClass*);		//!< Checks if this building can/should be jammed.

	void Jam(BuildingClass*);				//!< Attempts to jam the given building. (Actually just registers the Jammer with it, the jamming happens in a hook.)
	void Unjam(BuildingClass*) const;			//!< Attempts to unjam the given building. (Actually just unregisters the Jammer with it, the unjamming happens in a hook.)


public:

	RadarJammerClass() = default;
	RadarJammerClass(TechnoClass* GameObject) :
		AttachedToObject(GameObject),
		LastScan(0),
		Registered(false)
	{ }

	~RadarJammerClass()
	{
		this->UnjamAll();
	}

	void UnjamAll();						//!< Unregisters this Jammer on all structures.
	void Update();							//!< Updates this Jammer's status on all eligible structures.

	DefaultSaveLoadFunc(RadarJammerClass)

	RadarJammerClass(RadarJammerClass&& other) noexcept
		: AttachedToObject(std::exchange(other.AttachedToObject, nullptr))
		, LastScan(std::exchange(other.LastScan, 0u))
		, Registered(std::exchange(other.Registered, true))
	{}

	RadarJammerClass& operator=(RadarJammerClass&& other) noexcept
	{
		if (this == &other)
			return *this;

		// Transfer all fields
		this->AttachedToObject = std::exchange(other.AttachedToObject, nullptr);
		this->LastScan = std::exchange(other.LastScan, 0u);
		this->Registered = std::exchange(other.Registered, true);

		return *this;
	}

private:
	RadarJammerClass(const RadarJammerClass& other) = delete;
	RadarJammerClass& operator=(const RadarJammerClass& other) = delete;

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->AttachedToObject)
			.Process(this->LastScan)
			.Process(this->Registered)
			.Success()
			;
	}
};
