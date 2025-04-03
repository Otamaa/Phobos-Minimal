#pragma once
#include "LocomotionClass.h"

class //DECLSPEC_UUID("4A582747-9839-11d1-B709-00A024DDAFD1") NOVTABLE
	TeleportLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	static COMPILETIMEEVAL OPTIONALINLINE uintptr_t vtable = 0x7F50CC;
	static COMPILETIMEEVAL OPTIONALINLINE uintptr_t ILoco_vtable = 0x7F5000;
	static COMPILETIMEEVAL OPTIONALINLINE uintptr_t IPiggy_vtable = 0x7F4FDC;
	static COMPILETIMEEVAL reference<CLSID const, 0x7E9A90u> const ClassGUID {};

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override R0;
	virtual ULONG __stdcall AddRef() override R0;
	virtual ULONG __stdcall Release() override R0;

	//IPiggyback
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override R0;
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override R0;
	virtual bool __stdcall Is_Ok_To_End() override R0;
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override R0;
	virtual bool __stdcall Is_Piggybacking() override R0;

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
	virtual ~TeleportLocomotionClass() RX;

	//LocomotionClass
	virtual	int Size() override R0;

	//TeleportLocomotionClass
	virtual void vt_entry_28(DWORD dwUnk) RX;
	virtual bool IsStill() R0;

	//Constructor
	TeleportLocomotionClass()
		: TeleportLocomotionClass(noinit_t())
	{ JMP_THIS(0x718000); }

protected:
	explicit __forceinline TeleportLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
		//*((unsigned long*)this) = (unsigned long)0x7F50CC;
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long)0x7F5000;
		// IPiggy
		//*((unsigned long*)this + 18) = (unsigned long)0x7F4FDC;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct MovingDestination;	//Current destination
	CoordStruct LastCoords; //Marked occupation bits there
	bool Moving;	//Is currently moving
	bool unknown_bool_35;
	bool unknown_bool_36;
	int State;
	CDTimerClass Timer;
	ILocomotion* Piggybackee;
};

static_assert(sizeof(TeleportLocomotionClass) == 0x4C ,"Invalid size.");
