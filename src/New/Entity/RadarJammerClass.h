#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/MemoryPoolUniquePointer.h>

class TechnoClass;
class BuildingClass;
class RadarJammerClass final : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(RadarJammerClass, "RadarJammerClass")

public:

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

	RadarJammerClass() = default;
	RadarJammerClass(TechnoClass* GameObject) :
		LastScan(0),
		AttachedToObject(GameObject),
		Registered(false)
	{ }

	void UnjamAll();						//!< Unregisters this Jammer on all structures.
	void Update();							//!< Updates this Jammer's status on all eligible structures.

	DefaultSaveLoadFunc(RadarJammerClass)

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->LastScan)
			.Process(this->AttachedToObject)
			.Process(this->Registered)
			.Success()
			;
	}
};

template <>
struct Savegame::ObjectFactory<RadarJammerClass> {
	MemoryPoolUniquePointer<RadarJammerClass> operator() (PhobosStreamReader& Stm) const {
		return RadarJammerClass::createInstance();
	}
};
