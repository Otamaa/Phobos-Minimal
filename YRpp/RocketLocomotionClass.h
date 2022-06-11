//Locomotor = {B7B49766-E576-11d3-9BD9-00104B972FE8}

#pragma once

#include <LocomotionClass.h>

class DECLSPEC_UUID("B7B49766-E576-11d3-9BD9-00104B972FE8") NOVTABLE
	RocketLocomotionClass : public LocomotionClass
{
public:
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	virtual ULONG __stdcall AddRef() R0;
	virtual ULONG __stdcall Release() R0;

	//ILocomotion
	virtual bool __stdcall Is_Moving() R0;
	virtual CoordStruct* __stdcall Destination(CoordStruct* pcoord) R0;
	virtual bool __stdcall Process() R0;
	virtual void __stdcall Move_To(CoordStruct to) RX;
	virtual void __stdcall Stop_Moving() RX;
	virtual void __stdcall Do_Turn(DirStruct coord) RX;
	virtual Layer __stdcall In_Which_Layer() RT(Layer);
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) RX;
	virtual void __stdcall Clear_Coords() RX;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~RocketLocomotionClass() RX;

	//LocomotionClass
	virtual	int Size() R0;

	//RocketLocomotionClass
	double GetCurrentAngle() const
	{ JMP_THIS(0x662240); }

	bool IsInDelay() const
	{ JMP_THIS(0x661F90); }

	bool Func_6620F0(DWORD nIn) const {
		JMP_THIS(0x6620F0);
	}
	//Constructor
	RocketLocomotionClass()
		: RocketLocomotionClass(noinit_t())
	{ JMP_THIS(0x661EC0); }

protected:
	explicit __forceinline RocketLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
		//*((unsigned long*)this) = (unsigned long)0x7F0BE8;
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long)0x7F0B1C;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct MovingDestination;
	RepeatableTimerStruct MissionTimer;
	TimerStruct TrailerTimer; //timer34
	int MissionState;
	DWORD unknown_44;
	double CurrentSpeed;
	bool unknown_bool_4C;
	bool SpawnerIsElite;
	float CurrentPitch;
	DWORD unknown_58;
	DWORD unknown_5C;
};
//static_assert(sizeof(RocketLocomotionClass) == 0x60);