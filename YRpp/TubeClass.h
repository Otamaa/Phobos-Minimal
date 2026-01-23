#pragma once

#include <AbstractClass.h>

class CCINIClass;
class DECLSPEC_UUID("0B4CA41C-B3A7-11D1-B457-006097C6A979")
	NOVTABLE TubeClass : public AbstractClass
{
public:
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<TubeClass*>, 0x8B4138u> const Array{};
	static const AbstractType AbsID = AbstractType::Tube;

	void Read_INI(CCINIClass& ini) JMP_THIS(0x7283C0);
	void Write_INI(CCINIClass& ini) JMP_THIS(0x728280);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~TubeClass() RX;

		//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int	ClassSize() const override R0;

	TubeClass() noexcept : TubeClass(noinit_t())
	{ JMP_THIS(0x727FD0); }
protected:
	explicit __forceinline TubeClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

public:
	CellStruct EnterCell;
	CellStruct ExitCell;
	int ExitFace;
	int Faces[100];
	int FaceCount;
};

static_assert(sizeof(TubeClass) == 0x1C4, "Invalid size.");