#include <RectangleStruct.h>
#include <CoordStruct.h>

#include <ASMMacros.h>

class Surface;
class BeaconClass {
public:

	BeaconClass() JMP_THIS(0x430210);

	void Draw(Surface* pSurface, RectangleStruct bounds) JMP_THIS(0x430250);
	void SetCoordAndHouse(CoordStruct coord, int houseId) JMP_THIS(0x430590);
	// TODO bitfield functions
	void SetText(const wchar_t* pText) JMP_THIS(0x430620);
	void DrawRadar(Surface* pSurface, RectangleStruct bounds, bool toClear = false) JMP_THIS(0x430650);
	bool VisibleToPlayer() const JMP_THIS(0x4308B0);

public:

	CoordStruct Coord;
	byte Bitfield;
	byte gapD[1];
	wchar_t Text[128];
	byte field_10E;
	byte field_10F;
	int HouseID;
};

static_assert(sizeof(BeaconClass) == 0x114, "BeaconClass size is incorrect");
