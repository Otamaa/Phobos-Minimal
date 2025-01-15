/*
	Smudges
*/

#pragma once

#include <ObjectClass.h>

struct CoordStruct;
class SmudgeTypeClass;
class DECLSPEC_UUID("0E272DC5-9C0F-11D1-B709-00A024DDAFD1")
	NOVTABLE SmudgeClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Smudge;

	//Static
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<SmudgeClass*>, 0xA8B1E0u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~SmudgeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const  override RT(AbstractType);
	virtual int Size() const override R0;

	//Constructor
	SmudgeClass(SmudgeTypeClass* pType , CoordStruct* pcoord, int nOwnerIndex = -1) noexcept
		: SmudgeClass(noinit_t())
	{ JMP_THIS(0x6B4A50); }

protected:
	explicit __forceinline SmudgeClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	SmudgeTypeClass* Type;

};
