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

	static NOINLINE SideClass* __fastcall Find(const char* pID)
	{
		for (auto pItem : *Array){
			if (!CRT::strcmpi(pItem->ID, pID))
				return pItem;
		}

		return nullptr;
	}

	static NOINLINE SideClass* __fastcall  FindOrAllocate(const char* pID)
	{
		if (!pID || CRT::strcmpi(pID, GameStrings::NoneStr()) == 0 || CRT::strcmpi(pID, GameStrings::NoneStrb()) == 0)
			return nullptr;

		if (auto pRet = Find(pID)) {
			return pRet;
		}

		return GameCreate<SideClass>(pID);
	}

	static NOINLINE int __fastcall FindIndexById(const char* pID)
	{
		if(!pID)
			return -1;

		for (int i = 0; i < Array->Count; ++i) {
			if (!CRT::strcmpi(Array->Items[i]->ID, pID)) {
				return i;
			}
		}

		return -1;
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6A4740);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6A4780);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override JMP_STD(0x6A48A0);

	//Destructor
	virtual ~SideClass() override JMP_THIS(0x6A4930);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::Side; }
	virtual int Size() const override { return 0xB4; }

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
