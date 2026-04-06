#pragma once

#include <Utilities/SavegameDef.h>

class TechnoClass;
class PoweredUnitClass
{
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

	PoweredUnitClass(PoweredUnitClass&& other) noexcept
	: Techno(std::exchange(other.Techno, nullptr))
	, LastScan(std::exchange(other.LastScan, 0u))
	, Powered(std::exchange(other.Powered, true))
	{ }

	PoweredUnitClass& operator=(PoweredUnitClass&& other) noexcept
	{
		if (this == &other)
			return *this;

		// Transfer all fields
		this->Techno = std::exchange(other.Techno, nullptr);
		this->LastScan = std::exchange(other.LastScan, 0u);
		this->Powered = std::exchange(other.Powered, true);

		return *this;
	}

	DefaultSaveLoadFunc(PoweredUnitClass)
private:
	PoweredUnitClass(const PoweredUnitClass& other) = delete;
	PoweredUnitClass& operator=(const PoweredUnitClass& other) = delete;
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
