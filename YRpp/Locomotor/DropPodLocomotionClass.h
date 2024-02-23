#pragma once
#include "LocomotionClass.h"

enum DropPodDirType : BYTE
{
	DPOD_DIR_NE,
	DPOD_DIR_NW,
	DPOD_DIR_SE,
	DPOD_DIR_SW,
};

class //DECLSPEC_UUID("4A582745-9839-11d1-B709-00A024DDAFD1") NOVTABLE
	DropPodLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	// TODO stub virtuals implementations
	static constexpr inline uintptr_t vtable = 0x7E8344;
	static constexpr inline uintptr_t ILoco_vtable = 0x7E8278;
	static constexpr inline uintptr_t IPiggy_vtable = 0x7E8254;
	static constexpr reference<CLSID const, 0x7E9A70u> const ClassGUID {};

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override R0;
	virtual ULONG __stdcall AddRef() override R0;
	virtual ULONG __stdcall Release() override R0;

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
	virtual void __stdcall Limbo() RX;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~DropPodLocomotionClass() override RX;

	//LocomotionClass
	virtual int Size() override R0;

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

	bool OutOfMap;
	CoordStruct CoordDest;
	ILocomotionPtr Piggybackee;
};

static_assert(sizeof(DropPodLocomotionClass) == 0x30, "Invalid size.");