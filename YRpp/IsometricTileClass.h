#pragma once

#include <ObjectClass.h>
#include <GeneralStructures.h>

//forward declarations
class IsometricTileTypeClass;
struct RectangleStruct;

class DECLSPEC_UUID("0E272DC0-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE IsometricTileClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Isotile;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<IsometricTileClass*>, 0x87F750u> Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//ObjectClass
	virtual ObjectTypeClass* GetType() const R0;
	virtual bool Limbo() R0;
	virtual bool Unlimbo(const CoordStruct& Crd, DirType dFaceDir) R0;
	virtual void Draw(Point2D* pLocation, RectangleStruct* pBounds) const RX;

	//Destructor
	virtual ~IsometricTileClass() RX;

	//Constructor
	IsometricTileClass(int idxType, CellStruct const& location) noexcept
		: IsometricTileClass(noinit_t())
	{ JMP_THIS(0x543780); }

protected:
	explicit __forceinline IsometricTileClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	IsometricTileTypeClass* Type;
};

static_assert(sizeof(IsometricTileClass) == 0xB0);
