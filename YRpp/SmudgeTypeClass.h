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
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F3528;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<SmudgeTypeClass*>, 0xA8EC18u> const Array {};

	IMPL_Find(SmudgeTypeClass)

	static SmudgeTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_FAST(0x6B5910);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_FAST(0x6B5440);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6B5850);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override JMP_STD(0x6B58B0);

	//Destructor
	virtual ~SmudgeTypeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int ClassSize() const override R0;

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x6B56D0);

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override R0;
	virtual ObjectClass* CreateObject(HouseClass* pOwner) override R0;

	//SmudgeTypeClass
	virtual void DrawIt(const Point2D& Point, const RectangleStruct& Rect, int SmudgeData, int nHeight, const CellStruct& MapCoords) RX;

	//craters
	static void __fastcall CreateRandomSmudgeFromTypeList(CoordStruct& nWhere, int nVal1, int nVal2, bool bIgnoreBuildings)
		{ JMP_FAST(0x6B5C90); }

	//scorches
	static void __fastcall CreateRandomSmudge(CoordStruct& nWhere, int nVal1, int nVal2, bool bIgnoreBuildings)
		{ JMP_FAST(0x6B59A0); }

	bool CanPlace(CellStruct& nCell, bool bIgnoreBuildings)
		{ JMP_THIS(0x6B5F80); }

	static void __fastcall TheaterInit(TheaterType nType)
		{ JMP_FAST(0x6B5490); }

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

static_assert(sizeof(SmudgeTypeClass) == 0x2A4 , "Invalid Size !");