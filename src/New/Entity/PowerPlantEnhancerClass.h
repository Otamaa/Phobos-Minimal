#pragma once

#include <Base/Always.h>
#include <Utilities/SavegameDef.h>
#include <utility>
#include <map>

class BuildingClass;
class BuildingTypeClass;
class HouseClass;

class PowerPlantEnhancerClass
{
public:

	BuildingClass* AttachedToObject {};
	bool Registered {};

public:

	PowerPlantEnhancerClass() = default;

	PowerPlantEnhancerClass(BuildingClass* pBuilding)
		: AttachedToObject(pBuilding)
		, Registered(false)
	{}

	~PowerPlantEnhancerClass()
	{
		this->Unregister();
	}

	// ---- Registration lifecycle ----

	bool IsValidEnhancer() const;					//!< Checks if the attached building qualifies as an enhancer (has PowerPlantEnhancer settings).
	void Register();								//!< Registers this enhancer with its owner's HouseExt vector.
	void Unregister();								//!< Unregisters this enhancer from its owner's HouseExt vector.
	void OnOwnerChanged(HouseClass* pOldOwner, HouseClass* pNewOwner);	//!< Handles ownership transfer between houses.

	// ---- Power calculation (static) ----

	//!< Calculates the enhanced power output for a given power plant, considering all active enhancers in range.
	static std::pair<int, int> GetEnhancedPower(
		BuildingTypeClass* pBuilding,
		int output,
		HouseClass* pHouse,
		BuildingClass* pPowerPlant = nullptr
	);

	// ---- Range indicator ----

	int GetRangeInCells() const;					//!< Returns the enhancer range in cells for radial indicator display.
	
	static void DrawIndicators(BuildingClass* pCurrentBuilding);

	// ---- Serialization ----

	DefaultSaveLoadFunc(PowerPlantEnhancerClass)

		// ---- Move semantics ----

	PowerPlantEnhancerClass(PowerPlantEnhancerClass&& other) noexcept
		: AttachedToObject(std::exchange(other.AttachedToObject, nullptr))
		, Registered(std::exchange(other.Registered, false))
	{}

	PowerPlantEnhancerClass& operator=(PowerPlantEnhancerClass&& other) noexcept
	{
		if (this == &other)
			return *this;

		// Clean up current state first
		this->Unregister();

		this->AttachedToObject = std::exchange(other.AttachedToObject, nullptr);
		this->Registered = std::exchange(other.Registered, false);

		return *this;
	}

private:

	PowerPlantEnhancerClass(const PowerPlantEnhancerClass&) = delete;
	PowerPlantEnhancerClass& operator=(const PowerPlantEnhancerClass&) = delete;

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