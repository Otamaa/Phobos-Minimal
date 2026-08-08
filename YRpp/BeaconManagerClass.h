#include <ASMMacros.h>

#include <Helpers/CompileTime.h>

#include <RectangleStruct.h>
#include <CoordStruct.h>
#include <Point2D.h>

struct SHPCaches;
class Surface;
class BeaconClass;
class ALIGN(4) NOVTABLE BeaconManagerClass {
public:

	static COMPILETIMEEVAL reference<BeaconManagerClass, 0x89C3B0> const Instance {};
	static COMPILETIMEEVAL reference<SHPCaches*, 0x89C474> const BeaconArt {};
	static COMPILETIMEEVAL reference<SHPCaches*, 0x89C478> const RadarBeaconArt {};

	BeaconManagerClass() JMP_THIS(0x430910);
	~BeaconManagerClass() JMP_THIS(0x430930); // just an inlined Reset

	void Reset() JMP_THIS(0x430980);
	void LoadArt() JMP_THIS(0x4309D0);
	void Draw(Surface* pSurface, RectangleStruct bounds) JMP_THIS(0x430AC0);
	void PlaceBeacon(int houseId, int coordX , int coordY , int coordZ, int houseBeaconId = -1) JMP_THIS(0x430BA0);
	void PlaceBeacon(int houseId, CoordStruct coord, int houseBeaconId = -1) { this->PlaceBeacon(houseId, coord.X , coord.Y , coord.Z , houseBeaconId); };
	bool CanPlaceBeacon(int houseId) JMP_THIS(0x430F30);
	void DeleteBeacon(int houseId, int houseBeaconId) JMP_THIS(0x4311C0);
	void DeleteAllBeacons(int houseId) JMP_THIS(0x431410);
	void DrawRadar(Surface* surface, RectangleStruct bounds) JMP_THIS(0x431700);

public:

	std::array<std::array<BeaconClass*,8>, 3> Beacons;
	int AllocatedCount;
	Point2D BeaconSize;
	int BeaconFrameCount;
	Point2D RadarBeaconSize;
	int RadarBeaconFrameCount;
	int RadarBeaconAnimPeriod;
};

static_assert(sizeof(BeaconManagerClass) == 0x80, "BeaconManagerClass is the wrong size.");