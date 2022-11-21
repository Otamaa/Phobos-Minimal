/*
	EMP - no, you're NOT seeing things :P
*/

#pragma once

#include <AbstractClass.h>
class TechnoClass;
class DECLSPEC_UUID("B825CB22-200E-11D2-9FA9-0060089AD458")
	NOVTABLE EMPulseClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::EMPulse;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<EMPulseClass*>, 0x8A3870u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x4C59F0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x4C5A30);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override JMP_STD(0x4C5A80);

	//Destructor
	virtual ~EMPulseClass() override JMP_THIS(0x4C5AC0);

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;
	virtual void CalculateChecksum(Checksummer& checksum) const override JMP_THIS(0x4C59A0);

	void SetCoordBitfieldInRange() const { JMP_THIS(0x4C58C0);}
	void Init(TechnoClass* pInvoker) const { JMP_THIS(0x4C54E0);}

	static void UpdateAll() { JMP_STD(0x4C54A0); }

	//Constructor
	EMPulseClass(CellStruct dwCrd, int nSpread, int nDuration,
		TechnoClass* pGenerator) noexcept : EMPulseClass(noinit_t())
	{ JMP_THIS(0x4C52B0); }

//	EMPulseClass() noexcept : EMPulseClass(noinit_t())
//	{ JMP_THIS(0x4C5370); }

protected:
	explicit __forceinline EMPulseClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CellStruct BaseCoords;
	int Spread;
	int CreationTime;	//frame in which this EMP got created
	int Duration;
};
