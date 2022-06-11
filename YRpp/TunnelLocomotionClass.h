#pragma once

#include <LocomotionClass.h>
#include <CoordStruct.h>

class DECLSPEC_UUID("4A582743-9839-11d1-B709-00A024DDAFD1") NOVTABLE
	TunnelLocomotionClass : public LocomotionClass
{
public:

	//Destructor
	virtual ~TunnelLocomotionClass() RX;

	//TunnelLocomotionClass

	//Constructor
	TunnelLocomotionClass()
		: TunnelLocomotionClass(noinit_t())
	{ JMP_THIS(0x728A00); }

protected:
	explicit __forceinline TunnelLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
		//*((unsigned long*)this) = (unsigned long)0x7F5AF0;
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long)0x7F5A24;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int __ZAdjust;
	CoordStruct _CoordsNow;
	RepeatableTimerStruct Timer;
	BYTE _bool_38;
};

static_assert(sizeof(TunnelLocomotionClass) == 0x3C, "Invalid size.");