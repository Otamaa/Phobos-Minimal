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
	static constexpr inline DWORD vtable = 0x7E8344;
	static constexpr inline DWORD ILoco_vtable = 0x7E8278;
	static constexpr inline DWORD IPiggy_vtable = 0x7E8254;

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
		VTable::Set(this, vtable,0);
		VTable::Set(this, ILoco_vtable, 0x4);
		VTable::Set(this, IPiggy_vtable , 0x18);
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