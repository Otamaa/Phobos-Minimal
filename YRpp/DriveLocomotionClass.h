//Locomotor = {4A582741-9839-11d1-B709-00A024DDAFD1}

#pragma once

#include <LocomotionClass.h>

class DECLSPEC_UUID("4A582741-9839-11d1-B709-00A024DDAFD1")
	NOVTABLE DriveLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	// TODO stub virtuals implementations

	//Destructor
	virtual ~DriveLocomotionClass() RX;

	//Constructor
	DriveLocomotionClass()
		: DriveLocomotionClass(noinit_t())
	{ JMP_THIS(0x4AF540); }

protected:
	explicit __forceinline DriveLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
		//*((unsigned long*)this) = (unsigned long)0x7E7F7C;
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long)0x7E7EB0;
		// IPiggy
		//*((unsigned long*)this + 18) = (unsigned long)0x7E7E8C;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DWORD PreviousRamp;
	DWORD CurrentRamp;
	RateTimer SlopeTimer;
	CoordStruct Destination;
	CoordStruct HeadToCoord;
	int SpeedAccum;
	double movementspeed_50;
	DWORD TrackNumber;
	int TrackIndex;
	bool IsOnShortTrack;
	BYTE IsTurretLockedDown;
	bool IsRotating;
	bool IsDriving; 
	bool IsLocked;
	AbstractClass* Raider;
	int field_6C;
};

static_assert(sizeof(DriveLocomotionClass) == 0x70, "Invalid size.");
