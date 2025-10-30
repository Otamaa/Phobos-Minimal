#include "TargetingData.h"

#include <HouseClass.h>

#include <Ext/SWType/Body.h>

TargetingData::TargetingData() : TypeExt { nullptr }
, Owner { nullptr }
, NeedsLaunchSite { false }
, NeedsDesignator { false }
, NeedsAttractors { false }
, NeedsSupressors { false }
, NeedsInhibitors { false }
, LaunchSites {}
, Designators {}
, Inhibitors {}
, Attractors {}
, Suppressors {}
{ }

TargetingData::TargetingData(SWTypeExtData* pTypeExt, HouseClass* pOwner) noexcept : 
	TypeExt { pTypeExt }
, Owner { pOwner }
, NeedsLaunchSite { false }
, NeedsDesignator { false }
, NeedsAttractors { false }
, NeedsSupressors { false }
, NeedsInhibitors { false }
, LaunchSites {}
, Designators {}
, Inhibitors {}
, Attractors {}
, Suppressors {}
{ }

void TargetingData::reset()
{
	NeedsLaunchSite = false;
	NeedsDesignator = false;
	NeedsAttractors = false;
	NeedsSupressors = false;
	NeedsInhibitors = false;
	LaunchSites.clear();
	Designators.clear();
	Inhibitors.clear();
	Attractors.clear();
	Suppressors.clear();
}
