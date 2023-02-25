//Locomotor = {4A582741-9839-11d1-B709-00A024DDAFD1}

#pragma once

#include <LocomotionClass.h>

class DECLSPEC_UUID("4A582741-9839-11d1-B709-00A024DDAFD1")
	NOVTABLE DriveLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	// TODO stub virtuals implementations
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) JMP_STD(0x4AF720);
	virtual ULONG __stdcall AddRef() JMP_STD(0x4B4CB0);
	virtual ULONG __stdcall Release() JMP_STD(0x4B4CC0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x4B4830);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x4AF780);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x4AF800);

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
	int field_6C;
};

static_assert(sizeof(DriveLocomotionClass) == 0x70, "Invalid size.");
