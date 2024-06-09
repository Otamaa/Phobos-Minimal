#pragma once

#include <Utilities/SavegameDef.h>

class TechnoClass;
class AresPoweredUnit
{
	TechnoClass* Techno;
	int LastScan;
	bool Powered;
public:

	static constexpr int ScanInterval = 15;

	bool IsPoweredBy(HouseClass* const pOwner) const;
	void PowerUp();
	bool PowerDown();
	bool Update();

	constexpr inline bool IsPowered() const {
		return this->Powered;
	}

	constexpr AresPoweredUnit(TechnoClass* Techno) : Techno(Techno), LastScan(0), Powered(true)
	{ }

	constexpr ~AresPoweredUnit() = default;

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return this->Serialize(Stm);
	}

	bool Save(PhobosStreamWriter& Stm) const
	{
		return const_cast<AresPoweredUnit*>(this)->Serialize(Stm);
	}

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
struct Savegame::ObjectFactory<AresPoweredUnit> {
	std::unique_ptr<AresPoweredUnit> operator() (PhobosStreamReader& Stm) const {
		return std::make_unique<AresPoweredUnit>(nullptr);
	}
};