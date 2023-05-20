/*
	OverlayTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>

//forward declarations
class AnimTypeClass;
class HouseClass;

class DECLSPEC_UUID("5AF2CE79-0634-11D2-ACA4-006008055BB5")
	NOVTABLE OverlayTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::OverlayType;
	static constexpr inline DWORD vtable = 0x7EF600;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<OverlayTypeClass*>, 0xA83D80u> const Array {};

	IMPL_Find(OverlayTypeClass)

	static OverlayTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x5FEC70);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x5FE470);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~OverlayTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest,CoordStruct* pSrc) const override R0;

	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords,HouseClass* pOwner) override R0;
	virtual ObjectClass* CreateObject(HouseClass* pOwner) override R0;

	//OverlayTypeClass
	virtual void Draw(Point2D* pClientCoords, RectangleStruct* pClipRect, int nFrame) RX;

	//Constructor
	OverlayTypeClass(const char* pID) noexcept
		: OverlayTypeClass(noinit_t())
	{ JMP_THIS(0x5FE250); }

protected:
	explicit __forceinline OverlayTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int                ArrayIndex;
	LandType           LandType;
	AnimTypeClass*     CellAnim;
	int                DamageLevels;
	int                Strength;
	bool               Wall;
	bool               Tiberium;
	bool               Crate;
	bool               CrateTrigger;
	bool               NoUseTileLandType;
	bool               IsVeinholeMonster;
	bool               IsVeins;
	bool               ImageLoaded;	//not INI
	bool               Explodes;
	bool               ChainReaction;
	bool               Overrides;
	bool               DrawFlat;
	bool               IsRubble;
	bool               IsARock;
	ColorStruct RadarColor;

};
