/*
	Trees
*/

#pragma once

#include <ObjectClass.h>
#include <ProgressTimer.h>
#include <RectangleStruct.h>

class TerrainTypeClass;
class DECLSPEC_UUID("0E272DCE-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE TerrainClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Terrain;

	//global array
	static constexpr constant_ptr<DynamicVectorClass<TerrainClass*>, 0xA8E988u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~TerrainClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//ObjectClass
	// remove object from the map
	virtual bool Limbo() override JMP_THIS(0x71C930);

	// place the object on the map
	virtual bool Unlimbo(const CoordStruct& Crd, DirType dFaceDir) override JMP_THIS(0x71D000);

	void Placement_DrawIt_71C360(Point2D& nPoint, RectangleStruct& nRect)
	{ JMP_THIS(0x71C360); }

	void UpdatePlacement(PlacementType nType) const
	{ JMP_THIS(0x71BFB0); }

	//Constructor, Destructor
	TerrainClass(TerrainTypeClass* tt, CellStruct coords) noexcept
		: TerrainClass(noinit_t())
	{ JMP_THIS(0x71BB90); }

protected:
	explicit __forceinline TerrainClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ProgressTimer Animation;
	TerrainTypeClass* Type;
	bool IsBurning; // this terrain object has been ignited
	bool TimeToDie; // finish the animation and uninit
	PROTECTED_PROPERTY(BYTE, TerrainPad[0x2]);
	RectangleStruct unknown_rect_D0;

};
static_assert(sizeof(TerrainClass) == 0xE0, "Invalid Size!");