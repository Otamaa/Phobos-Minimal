/*
	Overlays (mainly Ore and Gems)
*/

#pragma once

/*

;GEF in case it wasn't intuitively obvious from the above, this list mirrors an object enumeration
;in overlay.hh. If you want to add something to this list, make sure you add to the enumeration
;or get a programmer to do it for you

 * Ironically, the list they had in overlay.hh doesn't seem to have been updated that much since TS.

 */

#define OVERLAY_GASAND 0x00
#define OVERLAY_GAWALL 0x02
#define OVERLAY_NAWALL 0x1A

#define OVERLAY_VEINS 0x7E
#define OVERLAY_VEINHOLE 0xA7
#define OVERLAY_VEINHOLEDUMMY 0xB2

#define OVERLAY_BRIDGEHEAD11 0x18
#define OVERLAY_BRIDGEHEAD12 0x19

#define OVERLAY_BRIDGEHEAD21 0xED
#define OVERLAY_BRIDGEHEAD22 0xEE

#define OVERLAY_LOBRIDGE1 0x7A
#define OVERLAY_LOBRIDGE2 0x7B
#define OVERLAY_LOBRIDGE3 0x7C
#define OVERLAY_LOBRIDGE4 0x7D

#include <ObjectClass.h>
#include <OverlayTypeClass.h>

class DECLSPEC_UUID("0E272DC7-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE OverlayClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Overlay;
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7EF3D4;

	//Static
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<OverlayClass*>, 0xA8EC50u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~OverlayClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;


	// Gets overlay's tiberium type
	static int __fastcall GetTiberiumType(int overlayTypeIndex)
	{ JMP_THIS(0x5FDD20); }

	//Constructor
	OverlayClass(OverlayTypeClass* pType, const CellStruct& mapCoord, int flag) noexcept : OverlayClass(noinit_t())
		{ JMP_THIS(0x5FC380); }

protected:
	explicit OverlayClass(noinit_t) noexcept : ObjectClass(noinit_t())
		{}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	OverlayTypeClass* Type;
};
