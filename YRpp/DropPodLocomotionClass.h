#pragma once
#include <LocomotionClass.h>

enum DropPodDirType : BYTE
{
	DPOD_DIR_NE,
	DPOD_DIR_NW,
	DPOD_DIR_SE,
	DPOD_DIR_SW,
};

class DECLSPEC_UUID("4A582745-9839-11d1-B709-00A024DDAFD1") NOVTABLE
	DropPodLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	// TODO stub virtuals implementations

	//Destructor
	virtual ~DropPodLocomotionClass() RX;

	//Constructor
	DropPodLocomotionClass()
		: DropPodLocomotionClass(noinit_t())
	{ JMP_THIS(0x4B5AB0); }

protected:
	explicit __forceinline DropPodLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
		//*((unsigned long*)this) = (unsigned long)0x7E8344;
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long)0x7E8278;
		// IPiggy
		//*((unsigned long*)this + 18) = (unsigned long)0x7E8254;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DropPodDirType DroppodDir;
	CoordStruct CoordDest;
	void* Piggybackee;
};

static_assert(sizeof(DropPodLocomotionClass) == 0x30, "Invalid size.");