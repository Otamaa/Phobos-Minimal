/*
	SmudgeTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>

class DECLSPEC_UUID("5AF2CE78-0634-11D2-ACA4-006008055BB5")
	NOVTABLE SmudgeTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::SmudgeType;

	//Array
	ABSTRACTTYPE_ARRAY(SmudgeTypeClass, 0xA8EC18u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) R0;

	//Destructor
	virtual ~SmudgeTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) R0;
	virtual ObjectClass* CreateObject(HouseClass* pOwner) R0;

	//SmudgeTypeClass
	virtual void DrawIt(const Point2D& Point, const RectangleStruct& Rect, int SmudgeData, int nHeight, const CellStruct& MapCoords) RX;

	static void __fastcall CreateRandomSmudgeFromTypeList(CoordStruct& nWhere, int nVal1, int nVal2, bool bIgnoreBuildings)
		{ JMP_STD(0x6B5C90); }

	static void __fastcall CreateRandomSmudge(CoordStruct& nWhere, int nVal1, int nVal2, bool bIgnoreBuildings)
		{ JMP_STD(0x6B59A0); }

	bool CanPlace(CellStruct& nCell, bool bIgnoreBuildings)
		{ JMP_THIS(0x6B5F80); }

	static void __fastcall TheaterInit(TheaterType nType)
		{ JMP_STD(0x6B5490); }

	//Constructor
	SmudgeTypeClass(const char* pID) noexcept
		: SmudgeTypeClass(noinit_t())
	{ JMP_THIS(0x6B5260); }

protected:
	explicit __forceinline SmudgeTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	int Width;
	int Height;
	bool Crater;
	bool Burn;
};
