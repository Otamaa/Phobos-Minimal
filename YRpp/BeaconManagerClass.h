#include <ASMMacros.h>

#include <Helpers/CompileTime.h>

#include <RectangleStruct.h>
#include <CoordStruct.h>
#include <Point2D.h>

struct SHPStruct;
class Surface;
class BeaconClass;
class __declspec(align(4)) BeaconManagerClass {
public:

	static COMPILETIMEEVAL reference<BeaconManagerClass, 0x89C3B0> const Instance {};
	static COMPILETIMEEVAL reference<SHPStruct*, 0x89C474> const BeaconArt {};
	static COMPILETIMEEVAL reference<SHPStruct*, 0x89C478> const RadarBeaconArt {};

	BeaconManagerClass() JMP_THIS(0x430910);
	~BeaconManagerClass() JMP_THIS(0x430930); // just an inlined Reset

	void Reset() JMP_THIS(0x430980);
	void LoadArt() JMP_THIS(0x4309D0);
	void Draw(Surface* pSurface, RectangleStruct bounds) JMP_THIS(0x430AC0);
	void PlaceBeacon(int houseId, CoordStruct coord, int houseBeaconId = -1) JMP_THIS(0x430BA0);
	bool CanPlaceBeacon(int houseId) JMP_THIS(0x430F30);
	// TODO rest of the functions
	void DrawRadar(Surface* surface, RectangleStruct bounds) JMP_THIS(0x431700);
	// TODO rest of the functions

public:

	BeaconClass* Beacons[8][3];
	int AllocatedCount;
	Point2D BeaconSize;
	int BeaconFrameCount;
	Point2D RadarBeaconSize;
	int RadarBeaconFrameCount;
	int RadarBeaconAnimPeriod;
};

static_assert(sizeof(BeaconManagerClass) == 0x80, "BeaconManagerClass is the wrong size.");