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
	static constexpr inline DWORD vtable = 0x7F522C;

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
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x71CFD0);

	//ObjectClass
	virtual bool Limbo() override JMP_THIS(0x71C930);
	virtual bool Unlimbo(const CoordStruct& Crd, DirType dFaceDir) override JMP_THIS(0x71D000);
	virtual bool UpdatePlacement(PlacementType value) override { JMP_THIS(0x71BFB0); }
	virtual DamageState ReceiveDamage(int* pDamage, int DistanceFromEpicenter, WarheadTypeClass* pWH,
  ObjectClass* Attacker, bool IgnoreDefenses, bool PreventPassengerEscape, HouseClass* pAttackingHouse) override JMP_THIS(0x71B920);

	void Placement_DrawIt_71C360(const Point2D& nPoint, const RectangleStruct& nRect)
	{ JMP_THIS(0x71C360); }

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

	StageClass Animation;
	TerrainTypeClass* Type;
	bool IsBurning; // this terrain object has been ignited
	bool TimeToDie; // finish the animation and uninit
	RectangleStruct unknown_rect_D0;

};
static_assert(sizeof(TerrainClass) == 0xE0, "Invalid Size!");