#pragma once

#include <Base/Always.h>
#include <Utilities/SavegameDef.h>
#include <utility>

class BuildingClass;
class BuildingTypeClass;
class TechnoTypeClass;
class HouseClass;

class FactoryPlantClass
{
public:

	BuildingClass* AttachedToObject {};
	bool Registered {};

public:

	FactoryPlantClass() = default;

	FactoryPlantClass(BuildingClass* pBuilding)
		: AttachedToObject(pBuilding)
		, Registered(false)
	{}

	~FactoryPlantClass()
	{
		this->Unregister();
	}

	// ---- Core queries ----

	bool HasRestrictions() const;				//!< Returns true if this plant has AllowTypes or DisallowTypes set.
	bool IsAffecting(TechnoTypeClass* pType) const;	//!< Checks if this plant's bonus should apply to the given TechnoType.

	// ---- Registration lifecycle ----

	bool Register();							//!< Registers with owner's HouseExt. Returns true if this is a restricted plant (caller should skip vanilla).
	void Unregister();							//!< Unregisters from owner's HouseExt.
	void OnOwnerChanged(HouseClass* pOldOwner, HouseClass* pNewOwner);	//!< Handles ownership transfer.

	// ---- Cost calculation (static) ----

	//!< Calculates the combined cost multiplier from all restricted factory plants owned by the house.
	static float GetCostMultiplier(HouseClass* pHouse, TechnoTypeClass* pTechnoType);

	// ---- Serialization ----

	DefaultSaveLoadFunc(FactoryPlantClass)

		// ---- Move semantics ----

		FactoryPlantClass(FactoryPlantClass&& other) noexcept
		: AttachedToObject(std::exchange(other.AttachedToObject, nullptr))
		, Registered(std::exchange(other.Registered, false))
	{}

	FactoryPlantClass& operator=(FactoryPlantClass&& other) noexcept
	{
		if (this == &other)
			return *this;

		this->Unregister();

		this->AttachedToObject = std::exchange(other.AttachedToObject, nullptr);
		this->Registered = std::exchange(other.Registered, false);

		return *this;
	}

private:

	FactoryPlantClass(const FactoryPlantClass&) = delete;
	FactoryPlantClass& operator=(const FactoryPlantClass&) = delete;

	//!< Gets the raw cost bonus value from the building type for this techno category.
	float GetRawCostBonus(TechnoTypeClass* pTechnoType) const;

private:

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(this->AttachedToObject)
			.Process(this->Registered)
			.Success()
			;
	}
};