#pragma once
#include "LocomotionClass.h"

class //DECLSPEC_UUID("2BEA74E1-7CCA-11d3-BE14-00104B62A16C")
	NOVTABLE ShipLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	static COMPILETIMEEVAL OPTIONALINLINE uintptr_t vtable = 0x7F2E58;
	static COMPILETIMEEVAL OPTIONALINLINE uintptr_t ILoco_vtable = 0x7F2D8C;
	static COMPILETIMEEVAL reference<CLSID const, 0x7E9AB0u> const ClassGUID {};

	// TODO stub virtuals implementations
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override R0;
	virtual ULONG __stdcall AddRef() override R0;
	virtual ULONG __stdcall Release() override R0;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~ShipLocomotionClass() override RX;

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
	bool IsRocking;
	bool IsLocked;
	ILocomotion* Piggybackee;
};

static_assert(sizeof(ShipLocomotionClass) == 0x70, "Invalid size.");
