/*
	[ScriptTypes]
*/

#pragma once

#include <AbstractTypeClass.h>
#include <ScriptActionNode.h>

//forward declarations
class CCINIClass;
class TechnoTypeClass;

class DECLSPEC_UUID("42F3A647-0789-11D2-ACA5-006008055BB5")
	NOVTABLE ScriptTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::ScriptType;
	static constexpr inline DWORD vtable = 0x7F1008;

	static inline constexpr int MaxActions = 50;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<ScriptTypeClass*>, 0x8B41C8u> const Array {};

	IMPL_Find(ScriptTypeClass)

	static ScriptTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x691C00);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x691BB0);
	}

	// this compare both ID and ININame ?
	static int __fastcall FindIndexByIdAndName(const char* pID) {
		JMP_STD(0x691B40);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x691D50);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x691D90);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x691DE0);

	//Destructor
	virtual ~ScriptTypeClass() RX;

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x691E30);
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;
	virtual int GetArrayIndex() const override { return this->ArrayIndex; }

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x6918A0);
	virtual bool SaveToINI(CCINIClass* pINI) override JMP_THIS(0x6917F0);

	static bool LoadFromINIList(CCINIClass *pINI, bool IsGlobal)
		{ JMP_STD(0x691970); }

	//Constructor
	ScriptTypeClass(const char* pID) noexcept
		: ScriptTypeClass(noinit_t())
	{ JMP_THIS(0x6916B0); }

protected:
	explicit __forceinline ScriptTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int      ArrayIndex;
	bool     IsGlobal;
	int      ActionsCount;
	ScriptActionNode ScriptActions [50];
};
static_assert(sizeof(ScriptTypeClass) == 0x234, "Invalid Size ! ");
