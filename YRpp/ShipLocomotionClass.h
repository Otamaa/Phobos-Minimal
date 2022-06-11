#pragma once

#include <LocomotionClass.h>


class DECLSPEC_UUID("2BEA74E1-7CCA-11d3-BE14-00104B62A16C")
	NOVTABLE ShipLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	// TODO stub virtuals implementations

	//Destructor
	virtual ~ShipLocomotionClass() RX;

	//Constructor
	ShipLocomotionClass()
		: ShipLocomotionClass(noinit_t())
	{ JMP_THIS(0x69EC50);}

protected:
	explicit __forceinline ShipLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
		//*((unsigned long*)this) = (unsigned long);
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long);
		// IPiggy
		//*((unsigned long*)this + 18) = (unsigned long);
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DWORD Ramp1;
	DWORD Ramp2;
	RepeatableTimerStruct SlopeTimer;
	CoordStruct Destination;
	CoordStruct HeadToCoord;
	DWORD movementspeed_4C;
	DWORD movementspeed_50;
	DWORD movementspeed_54;
	DWORD TrackNumber;
	int TrackIndex;
	bool IsOnShortTrack;
	BYTE field_61;
	BYTE IsRotating;
	BYTE headtopmatches63;
	BYTE byte64;
	BYTE byte65;
	int dword68;
};
static_assert(sizeof(ShipLocomotionClass) == 0x6C, "Invalid size.");