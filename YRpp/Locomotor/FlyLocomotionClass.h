#pragma once
#include "LocomotionClass.h"

class //DECLSPEC_UUID("4A582746-9839-11d1-B709-00A024DDAFD1") NOVTABLE
	FlyLocomotionClass : public LocomotionClass
{
public:
	static constexpr inline uintptr_t vtable = 0x7E8AC0;
	static constexpr inline uintptr_t ILoco_vtable = 0x7E89F4; // vtable + 4
	static const inline CLSID ClassGUID = CLSIDs::Fly();

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override R0;
	virtual ULONG __stdcall AddRef() override R0;
	virtual ULONG __stdcall Release() override R0;

	//ILocomotion
	virtual bool __stdcall Is_Moving() override R0;
	virtual CoordStruct __stdcall Destination() override RT(CoordStruct);
	virtual bool __stdcall Process() override R0;
	virtual void __stdcall Move_To(CoordStruct to) override RX;
	virtual void __stdcall Stop_Moving() override RX;
	virtual void __stdcall Do_Turn(DirStruct coord) override RX;
	virtual Layer __stdcall In_Which_Layer() override RT(Layer);
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override RX;
	virtual void __stdcall Clear_Coords() override RX;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~FlyLocomotionClass() RX;

	//LocomotionClass
	virtual	int Size() override R0;

	//FlyLocomotionClass

	//Constructor
	FlyLocomotionClass()
		: FlyLocomotionClass(noinit_t())
	{ JMP_THIS(0x4CC9A0); }

protected:
	explicit __forceinline FlyLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
	//	*((unsigned long*)this) = (unsigned long)0x7E8AC0;
		// ILoco
	//	*((unsigned long*)this + 1) = (unsigned long)0x7E89F4;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	bool AirportBound;
	CoordStruct MovingDestination;
	CoordStruct XYZ2;
	bool HasMoveOrder;
	int FlightLevel;
	double TargetSpeed;
	double CurrentSpeed;
	bool IsTakingOff;
	bool IsLanding;
	bool WasLanding;
	bool unknown_bool_53;
	DWORD unknown_54;
	DWORD unknown_58;
	bool IsElevating;
	bool unknown_bool_5D;
	bool unknown_bool_5E;
	bool unknown_bool_5F;
};

static_assert(sizeof(FlyLocomotionClass) == 0x60);