#pragma once

//Locomotor = {92612C46-F71F-11d1-AC9F-006008055BB5}
#include <LocomotionClass.h>

class  DECLSPEC_UUID("92612C46-F71F-11d1-AC9F-006008055BB5") NOVTABLE
	JumpjetLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	static constexpr inline DWORD vtable = 0x7ECE34;

	enum class State : int
	{
		On_ground = 0,
		Take_off,
		Hover,
		Moving,
		Deploy_to_land,
		Crash
	};

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	virtual ULONG __stdcall AddRef() R0;
	virtual ULONG __stdcall Release() R0;

	//IPiggyback
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) R0;
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) R0;
	virtual bool __stdcall Is_Ok_To_End() R0;
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) R0;
	virtual bool __stdcall Is_Piggybacking() R0;

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
	virtual ~JumpjetLocomotionClass() RX;

	//LocomotionClass
	virtual	int Size() R0;

	//JumjetLocomotionClass
	int GetZCoords() const
	{ JMP_THIS(0x54D820); }

	CoordStruct* GetJumjetCoords() const
	{ JMP_THIS(0x54D6D0); }

	//Constructor
	JumpjetLocomotionClass()
		: JumpjetLocomotionClass(noinit_t())
	{ JMP_THIS(0x54AC40); }

protected:
	explicit __forceinline JumpjetLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
		//*((unsigned long*)this) = (unsigned long)0x7ECE34;
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long)0x7ECD68;
		// IPiggy
		//*((unsigned long*)this + 18) = (unsigned long)0x7ECD44;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int TurnRate;
	int Speed;
	float Climb;
	float Crash;
	int Height;
	float Acceleration;
	float Wobbles;
	int Deviation;
	bool NoWobbles;
	BYTE unknown_3D;
	BYTE unknown_3E;
	BYTE unknown_3F;
	CoordStruct HeadToCoord;
	bool IsMoving;
	BYTE unknown_4D;
	BYTE unknown_4E;
	BYTE unknown_4F;
	State NextState;
	FacingClass Facing;
	BYTE unknown_6C;
	BYTE unknown_6D;
	BYTE unknown_6E;
	BYTE unknown_6F;
	double __currentSpeed;
	double __maxSpeed;
	int __currentHeight;
	BYTE unknown_84;
	BYTE unknown_85;
	BYTE unknown_86;
	BYTE unknown_87;
	double __currentWobble;
	bool DestinationReached;
	BYTE unknown_91;
	BYTE unknown_92;
	BYTE unknown_93;
	ILocomotion* Piggybackee;
};

static_assert(sizeof(JumpjetLocomotionClass) == 0x98, "Invalid size.");