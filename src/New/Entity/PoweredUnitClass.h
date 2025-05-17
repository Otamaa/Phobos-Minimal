#pragma once

#include <Utilities/SavegameDef.h>
#include <Utilities/MemoryPoolUniquePointer.h>

class TechnoClass;
class PoweredUnitClass final : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(PoweredUnitClass, "PoweredUnitClass")

public:

	TechnoClass* Techno {};
	int LastScan {};
	bool Powered {};

public:

	static COMPILETIMEEVAL int ScanInterval = 15;

	bool IsPoweredBy(HouseClass* const pOwner) const;
	void PowerUp();
	bool PowerDown();
	bool Update();

	COMPILETIMEEVAL OPTIONALINLINE bool IsPowered() const {
		return this->Powered;
	}

	PoweredUnitClass() = default;
	PoweredUnitClass(TechnoClass* Techno) : Techno(Techno), LastScan(0), Powered(true)
	{ }

	DefaultSaveLoadFunc(PoweredUnitClass)

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->Techno)
			.Process(this->LastScan)
			.Process(this->Powered)
			.Success()
			;
	}
};


template <>
struct Savegame::ObjectFactory<PoweredUnitClass> {
	MemoryPoolUniquePointer<PoweredUnitClass> operator() (PhobosStreamReader& Stm) const {
		return PoweredUnitClass::createInstance();
	}
};
