#pragma once

#include <LocomotionClass.h>
#include <CoordStruct.h>

class ALIGN(4) DECLSPEC_UUID("4A582743-9839-11d1-B709-00A024DDAFD1") NOVTABLE
	TunnelLocomotionClass : public LocomotionClass
{
public:

	enum class State : int
	{
		IDLE = 0x0,
		PRE_DIG_IN = 0x1,
		DIGGING_IN = 0x2,
		DUG_IN = 0x3,
		DIGGING = 0x4,
		PRE_DIG_OUT = 0x5,
		DIGGING_OUT = 0x6,
		DUG_OUT = 0x7,
	};

	//Destructor
	virtual ~TunnelLocomotionClass() RX;

	//TunnelLocomotionClass
	bool ProcessPreDigIn()
	{ JMP_THIS(0x7291F0); }

	bool ProcessDiggingIn()
	{ JMP_THIS(0x729370); }

	bool ProcessDugIn()
	{ JMP_THIS(0x7294E0); }

	bool ProcessDigging()
	{ JMP_THIS(0x729580); }

	bool ProcessPreDigOut()
	{ JMP_THIS(0x7298F0); }

	bool ProcessDiggingOut()
	{ JMP_THIS(0x729AA0); }

	bool ProcessDugOut()
	{ JMP_THIS(0x729480); }

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

	State State;
	CoordStruct _CoordsNow;
	RateTimer Timer;
	bool _bool_38;
};

static_assert(sizeof(TunnelLocomotionClass) == 0x3C, "Invalid size.");