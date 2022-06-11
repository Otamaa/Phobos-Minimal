#pragma once

#include <AbstractClass.h>

class CCINIClass;
class DECLSPEC_UUID("0B4CA41C-B3A7-11D1-B457-006097C6A979")
	NOVTABLE TubeClass : public AbstractClass
{
public:
		static const AbstractType AbsID = AbstractType::Tube;
		void Read_INI(CCINIClass& ini) JMP_THIS(0x7283C0);
		void Write_INI(CCINIClass& ini) JMP_THIS(0x728280);

		//IPersist
		virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

		//IPersistStream
		virtual HRESULT __stdcall Load(IStream* pStm) R0;
		virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

		//Destructor
		virtual ~TubeClass() RX;

		//AbstractClass
		virtual AbstractType WhatAmI() const RT(AbstractType);
		virtual int	Size() const R0;

		TubeClass() noexcept
			: TubeClass(noinit_t())
		{ JMP_THIS(0x727FD0); }
protected:
	explicit __forceinline TubeClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

public:
	CellStruct Start;
	CellStruct End;
	Direction Facing;
	DWORD DIr[100];
	int DirCount;
};

static_assert(sizeof(TubeClass) == 0x1C4, "Invalid size.");