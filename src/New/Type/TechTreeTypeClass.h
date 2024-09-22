#pragma once

#include <functional>
#include <map>
#include <set>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class TechTreeTypeClass final : public Enumerable<TechTreeTypeClass>
{
public:
	enum class BuildType
	{
		BuildPower,
		BuildRefinery,
		BuildBarracks,
		BuildWeapons,
		BuildRadar,
		BuildHelipad,
		BuildNavalYard,
		BuildTech,
		BuildAdvancedPower,
		BuildDefense,
		BuildOther,

		count
	};

	Valueable<int> SideIndex;
	Valueable<BuildingTypeClass*> ConstructionYard;
	ValueableVector<BuildingTypeClass*> BuildPower;
	ValueableVector<BuildingTypeClass*> BuildRefinery;
	ValueableVector<BuildingTypeClass*> BuildBarracks;
	ValueableVector<BuildingTypeClass*> BuildWeapons;
	ValueableVector<BuildingTypeClass*> BuildRadar;
	ValueableVector<BuildingTypeClass*> BuildHelipad;
	ValueableVector<BuildingTypeClass*> BuildNavalYard;
	ValueableVector<BuildingTypeClass*> BuildTech;
	ValueableVector<BuildingTypeClass*> BuildAdvancedPower;
	ValueableVector<BuildingTypeClass*> BuildDefense;
	ValueableVector<BuildingTypeClass*> BuildOther;
	ValueableVector<int> BuildOtherCounts;
	PhobosMap<BuildingTypeClass*, size_t> BuildOtherCountMap;

	TechTreeTypeClass(const char* pTitle) : Enumerable<TechTreeTypeClass>(pTitle) { }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	bool IsCompleted(HouseClass* pHouse, std::function<bool(BuildingTypeClass*)> const& filter) const;
	size_t CountSideOwnedBuildings(HouseClass* pHouse, BuildType buildType) const;
	constexpr const std::vector<BuildingTypeClass*>* GetBuildList(BuildType buildType) const {
		switch (buildType)
		{
		case BuildType::BuildPower:
			return &this->BuildPower;
		case BuildType::BuildRefinery:
			return &this->BuildRefinery;
		case BuildType::BuildBarracks:
			return &this->BuildBarracks;
		case BuildType::BuildWeapons:
			return &this->BuildWeapons;
		case BuildType::BuildRadar:
			return &this->BuildRadar;
		case BuildType::BuildHelipad:
			return &this->BuildHelipad;
		case BuildType::BuildNavalYard:
			return &this->BuildNavalYard;
		case BuildType::BuildTech:
			return &this->BuildTech;
		case BuildType::BuildAdvancedPower:
			return &this->BuildAdvancedPower;
		case BuildType::BuildDefense:
			return &this->BuildDefense;
		case BuildType::BuildOther:
			return &this->BuildOther;
		default:
			return nullptr;
		}
	}

	std::vector<BuildingTypeClass*> GetBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter) const;
	BuildingTypeClass* GetRandomBuildable(BuildType buildType, std::function<bool(BuildingTypeClass*)> const& filter) const;


private:
	template <typename T>
	void Serialize(T& Stm);
};