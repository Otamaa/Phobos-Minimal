#pragma once
#include "LocomotionClass.h"

class  DECLSPEC_UUID("55D141B8-DB94-11d1-AC98-006008055BB5") NOVTABLE
	MechLocomotionClass : public LocomotionClass
{
public:
	static constexpr inline uintptr_t vtable = 0x7EDC38;
	static constexpr inline uintptr_t ILoco_vtable = 0x7EDB6C;
	static constexpr inline CLSID ClassGUID = __uuidof(MechLocomotionClass);

	//Destructor
	virtual ~MechLocomotionClass() RX;

	//MechLocomotionClass

	//Constructor
	MechLocomotionClass()
		: MechLocomotionClass(noinit_t())
	{ JMP_THIS(0x5AFEF0);}

protected:
	explicit __forceinline MechLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
		//*((unsigned long*)this) = (unsigned long)0x7EDC38;
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long)0x7EDB6C;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct MovingDestination;
	CoordStruct CoordHeadTo;
	bool IsMoving;
};

static_assert(sizeof(MechLocomotionClass) == 0x34, "Invalid size.");