#pragma once

#include <Utilities/SavegameDef.h>

class TechnoClass;
class PoweredUnitClass
{
	TechnoClass* Techno {};
	int LastScan {};
	bool Powered {};
	bool IsActive {};

public:

	static COMPILETIMEEVAL int ScanInterval = 15;

	bool IsPoweredBy(HouseClass* const pOwner) const;
	void PowerUp();
	bool PowerDown();
	bool Update();

	void reset() {
		this->IsActive = false;
	}

	void Activate(TechnoClass* pTechno){
		this->Techno = pTechno;
		this->Powered = true;
		this->IsActive = true;
	}

	COMPILETIMEEVAL OPTIONALINLINE bool IsPowered() const {
		return this->Powered;
	}

	COMPILETIMEEVAL PoweredUnitClass() = default;
	COMPILETIMEEVAL ~PoweredUnitClass() = default;

	explicit operator bool() const {
        return IsActive && this->Techno;
    }

	DefaultSaveLoadFunc(PoweredUnitClass)

private:
	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->Techno, true)
			.Process(this->LastScan)
			.Process(this->Powered)
			.Process(this->IsActive)
			.Success()
			;
	}
};