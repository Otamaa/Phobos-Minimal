#pragma once

#include <functional>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/PhobosMap.h>

#include <BuildingTypeClass.h>
#include <HouseClass.h>

class TechTreeTypeClass final : public Enumerable<TechTreeTypeClass>
{
public:
	static COMPILETIMEEVAL const char* MainSection = "TechTreeTypes";
	static COMPILETIMEEVAL const char* ClassName = "TechTreeTypeClass";

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

	TechTreeTypeClass(const char* pTitle) : Enumerable<TechTreeTypeClass>(pTitle)
		, SideIndex {}
		, ConstructionYard {}
		, BuildPower {}
		, BuildBarracks {}
		, BuildWeapons {}
		, BuildRadar {}
		, BuildHelipad {}
		, BuildNavalYard {}
		, BuildTech {}
		, BuildAdvancedPower {}
		, BuildDefense {}
		, BuildOtherCounts {}
		, BuildOtherCountMap {}
	{ }

	void LoadFromINI(CCINIClass* pINI);
	void LoadFromStream(PhobosStreamReader& Stm);
	void SaveToStream(PhobosStreamWriter& Stm);

	template <typename Func>
	bool IsCompleted(HouseClass* pHouse, Func&& filter) const
	{
		for (BuildType i = BuildType::BuildPower; i < BuildType::BuildOther; i = BuildType((int)i + 1)) {
			if (!GetBuildable(i, std::forward<Func>(filter)).empty() && CountSideOwnedBuildings(pHouse, i) < 1) {
				return false;
			}
		}

		for (const auto& [type, count] : BuildOtherCountMap) {
			if (filter(type) && CountSideOwnedBuildings(pHouse, BuildType::BuildOther) < count) {
				return false;
			}
		}

		return true;
	}

	size_t CountSideOwnedBuildings(HouseClass* pHouse, BuildType buildType) const
	{
		size_t count = 0;
		if (auto pBuild = this->GetBuildList(buildType)) {
			for (const auto pBuilding : *pBuild) {
				count += pHouse->ActiveBuildingTypes.get_count(pBuilding->ArrayIndex);
			}
		}

		return count;
	}

	COMPILETIMEEVAL FORCEDINLINE const std::vector<BuildingTypeClass*>* GetBuildList(BuildType buildType) const {
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

	template <typename Func>
	COMPILETIMEEVAL FORCEDINLINE std::vector<BuildingTypeClass*> GetBuildable(BuildType buildType, Func&& filter) const
	{
		std::vector<BuildingTypeClass*> filtered;
		if(auto pBuild = this->GetBuildList(buildType)){
			std::ranges::copy_if(*pBuild, std::back_inserter(filtered), std::forward<Func>(filter));
		}

		return filtered;
	}

	template <typename Func>
	COMPILETIMEEVAL BuildingTypeClass* GetRandomBuildable(BuildType buildType, Func&& filter) const
	{
		const std::vector<BuildingTypeClass*> buildable = GetBuildable(buildType, std::forward<Func>(filter));
		if (!buildable.empty()) {
			return buildable[ScenarioClass::Instance->Random.RandomRanged(0, buildable.size() - 1)];

		}

		return nullptr;
	}

private:
	template <typename T>
	void Serialize(T& Stm);
};