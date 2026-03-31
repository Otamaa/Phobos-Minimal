#pragma once

#include <Utilities/VectorHelper.h>
#include <CellStruct.h>

class SWTypeExtData;
class HouseClass;
class BuildingClass;
struct TargetingData final
{
public:

	TargetingData();
	TargetingData(SWTypeExtData* pTypeExt, HouseClass* pOwner) noexcept;

	void reset();

	struct LaunchSite
	{
		BuildingClass* Building;
		CellStruct Center;
		double MinRange;
		double MaxRange;
	};

	struct RangedItem
	{
		int RangeSqr;
		CellStruct Center;
	};

	SWTypeExtData* TypeExt;
	HouseClass* Owner;
	bool NeedsLaunchSite;

	bool NeedsDesignator;
	bool NeedsAttractors;
	bool NeedsSupressors;
	bool NeedsInhibitors;

	HelperedVector<LaunchSite> LaunchSites;
	HelperedVector<RangedItem> Designators;
	HelperedVector<RangedItem> Inhibitors;

	//Enemy Inhibitors
	HelperedVector<RangedItem> Attractors;
	//Enemy Designator
	HelperedVector<RangedItem> Suppressors;

};
