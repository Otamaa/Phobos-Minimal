#pragma once
#include "LocomotionClass.h"

class //DECLSPEC_UUID("55D141B8-DB94-11d1-AC98-006008055BB5")
	NOVTABLE MechLocomotionClass : public LocomotionClass
{
public:
	static COMPILETIMEEVAL OPTIONALINLINE uintptr_t vtable = 0x7EDC38;
	static COMPILETIMEEVAL OPTIONALINLINE uintptr_t ILoco_vtable = 0x7EDB6C;
	static COMPILETIMEEVAL reference<CLSID const, 0x7E9AA0u> const ClassGUID {};

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
	virtual ~MechLocomotionClass() override RX;

	//MechLocomotionClass

	//Constructor
	MechLocomotionClass()
		: MechLocomotionClass(noinit_t())
	{ JMP_THIS(0x5AFEF0);}

protected:
	explicit __forceinline MechLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
		//vtftable
		//*((unsigned long*)this) = (unsigned long)0x7EDC38;
		// ILoco
		//*((unsigned long*)this + 1) = (unsigned long)0x7EDB6C;
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct MovingDestination;
	CoordStruct CoordHeadTo;
	bool IsMoving;
};

static_assert(sizeof(MechLocomotionClass) == 0x34, "Invalid size.");