#pragma once
#include <LocomotionClass.h>

class DECLSPEC_UUID("4A582742-9839-11d1-B709-00A024DDAFD1") NOVTABLE
	HoverLocomotionClass : public LocomotionClass
{
public:

	//Destructor
	virtual ~HoverLocomotionClass() RX;

	//HoverLocomotionClass
	int sub_514F70(bool bArg) const {
		JMP_THIS(0x514F70);
	}

	//Constructor
	HoverLocomotionClass()
		: HoverLocomotionClass(noinit_t())
	{ JMP_THIS(0x513C20); }

protected:
	explicit __forceinline HoverLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//*((unsigned long*)this) = (unsigned long)0x7EADC8;
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long)0x7EACFC;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct MovingDestination;
	CoordStruct CoordHeadTo;
	FacingClass Facing;
	double __Height;
	double __Accel;
	double __Boost;
	double __Gravity_HoverDampen;
	bool _being_shoved68;
	int _shove_rand6C;
	bool _BeignPushed;
	int _unk74;

};

static_assert(sizeof(HoverLocomotionClass) == 0x78, "Invalid size.");