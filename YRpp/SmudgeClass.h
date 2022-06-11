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
	static constexpr constant_ptr<DynamicVectorClass<SmudgeClass*>, 0xA8B1E0u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~SmudgeClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

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
