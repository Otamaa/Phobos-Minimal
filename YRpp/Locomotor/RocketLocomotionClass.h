#pragma once
#include "LocomotionClass.h"

class //DECLSPEC_UUID("B7B49766-E576-11d3-9BD9-00104B972FE8")
	NOVTABLE RocketLocomotionClass : public LocomotionClass
{
public:
	static constexpr inline uintptr_t vtable = 0x7F0BE8;
	static constexpr inline uintptr_t ILoco_vtable = 0x7F0B1C;
	static constexpr reference<CLSID const, 0x7E9AD0u> const ClassGUID {};

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
	virtual ~RocketLocomotionClass() RX;

	//LocomotionClass
	virtual	int Size() override R0;

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
	CDTimerClass TrailerTimer; //timer34
	RocketMissionState MissionState;
	DWORD unknown_44;
	double CurrentSpeed;
	bool NeedToSubmit;
	bool SpawnerIsElite;
	float CurrentPitch;
	DWORD ApogeeDistance;
	DWORD unknown_5C;
};
//static_assert(sizeof(RocketLocomotionClass) == 0x60);