/*
	RadSites
*/

#pragma once

#include <FileSystem.h>
#include <AbstractClass.h>

class ObjectClass;

class DECLSPEC_UUID("623C7584-74E7-11D2-B8F5-006008C809ED")
	NOVTABLE AlphaShapeClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::AlphaShape;
	static constexpr referencemult<char, 0x88A118u, 256, 256> const ShapeArray{};

	//Static
	static constexpr constant_ptr<DynamicVectorClass<AlphaShapeClass*>, 0x88A0F0u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) R0;

	//Destructor
	virtual ~AlphaShapeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int	Size() const R0;

	//Constructor
	AlphaShapeClass(ObjectClass* pObj, int nX, int nY) noexcept
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
	DECLARE_PROPERTY(RectangleStruct, Rect);
	SHPStruct* AlphaImage;
	bool IsObjectGone;	//Set if AttachedTo is NULL.
};

static_assert(sizeof(AlphaShapeClass) == 0x40, "Invalid size.");
