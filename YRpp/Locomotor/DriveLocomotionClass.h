#pragma once
#include "LocomotionClass.h"

class //DECLSPEC_UUID("4A582741-9839-11d1-B709-00A024DDAFD1")
	NOVTABLE DriveLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	static COMPILETIMEEVAL OPTIONALINLINE uintptr_t vtable = 0x7E7F7C;
	static COMPILETIMEEVAL OPTIONALINLINE uintptr_t ILoco_vtable = 0x7E7EB0;
	static COMPILETIMEEVAL reference<CLSID const, 0x7E9A30u> const ClassGUID {};

	// TODO stub virtuals implementations
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override JMP_STD(0x4AF720);
	virtual ULONG __stdcall AddRef() override JMP_STD(0x4B4CB0);
	virtual ULONG __stdcall Release() override JMP_STD(0x4B4CC0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x4B4830);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x4AF780);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x4AF800);

	//Destructor
	virtual ~DriveLocomotionClass() override JMP_THIS(0x4B4D00);

	//LocomotionClass
	virtual int Size() override { return 0x70; }

	void Stop_Drive() const JMP_THIS(0x4B0DA0);
	void Start_Drive(CoordStruct* pWhere) const JMP_THIS(0x4B0DF0);

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

	int CurrentRamp;
	int PreviousRamp;
	RateTimer SlopeTimer;
	CoordStruct Destination;
	CoordStruct HeadToCoord;
	int SpeedAccum;
	double movementspeed_50;
	int TrackNumber;
	int TrackIndex;
	bool IsOnShortTrack;
	BYTE IsTurretLockedDown;
	bool IsRotating;
	bool IsDriving;
	bool IsRocking;
	bool IsLocked;
	ILocomotion* Piggybackee;
	int field_6C;
};

static_assert(sizeof(DriveLocomotionClass) == 0x70, "Invalid size.");
