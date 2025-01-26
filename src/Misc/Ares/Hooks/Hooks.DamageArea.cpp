#include <Ext/WarheadType/Body.h>

#include <Utilities/Macro.h>

#include <Constructable.h>

#include <Misc/PhobosGlobal.h>

#include <AircraftClass.h>
#include <Utilities/Helpers.h>

// #895990: limit the number of times a warhead with
// CellSpread will hit the same object for each hit
#include <AircraftTrackerClass.h>
#include <VeinholeMonsterClass.h>
#include <ParticleSystemClass.h>
#include <OverlayTypeClass.h>
#include <VoxelAnimClass.h>
#include <TacticalClass.h>
#include <AnimClass.h>
