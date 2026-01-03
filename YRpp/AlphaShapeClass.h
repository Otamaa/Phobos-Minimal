/*
	RadSites
*/

#pragma once

#include <FileSystem.h>
#include <AbstractClass.h>
#include <Point2D.h>

class ObjectClass;

class DECLSPEC_UUID("623C7584-74E7-11D2-B8F5-006008C809ED")
	NOVTABLE AlphaShapeClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::AlphaShape;
	static COMPILETIMEEVAL reference2D<char, 0x88A118u, 256, 256> const ShapeArray{};

	//Static
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<AlphaShapeClass*>, 0x88A0F0u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override R0;

	//Destructor
	virtual ~AlphaShapeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int	ClassSize() const override R0;

	//Constructor
	AlphaShapeClass(ObjectClass* pObj, int nX, int nY) noexcept
		: AlphaShapeClass(noinit_t())
	{ JMP_THIS(0x420960); }

	AlphaShapeClass(ObjectClass* pObj, Point2D nPos) noexcept
		: AlphaShapeClass(noinit_t())
	{ JMP_THIS(0x420960); }

protected:
	explicit __forceinline AlphaShapeClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ObjectClass* AttachedTo;	//To which object is this AlphaShape attached?
	RectangleStruct Rect;
	SHPStruct* AlphaImage;
	bool IsObjectGone;	//Set if AttachedTo is NULL.
};

static_assert(sizeof(AlphaShapeClass) == 0x40, "Invalid size.");
