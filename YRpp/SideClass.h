/*
	Sides
*/

#pragma once

#include <AbstractTypeClass.h>

class DECLSPEC_UUID("C53DD372-151E-11D2-8175-006008055BB5")
	NOVTABLE SideClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::Side;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<SideClass*>, 0x8B4120u> const Array {};

	IMPL_Find(SideClass)
	IMPL_FindOrAllocate(SideClass)
	IMPL_FindIndexById(SideClass)

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6A4740);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6A4780);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override JMP_STD(0x6A48A0);

	//Destructor
	virtual ~SideClass() override JMP_THIS(0x6A4930);

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	//Constructor
	SideClass(const char* pID) noexcept
		: SideClass(noinit_t())
	{ JMP_THIS(0x6A4550); }

protected:
	explicit __forceinline SideClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TypeList<int> HouseTypes;	//indices!

};
