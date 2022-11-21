/*
	[ScriptTypes]
*/

#pragma once

#include <AbstractTypeClass.h>

class CCINIClass;
struct ScriptActionNode
{
public:
	unsigned int BuildINIEntry(char* pTr)
	 { JMP_THIS(0x723CE0); }

	void FillIn(char* pTr)
	 { JMP_THIS(0x723CA0); }

	bool operator==(ScriptActionNode const& rhs) const {
		//return Action == rhs.Action && Argument == rhs.Argument;
		return false; // so , umm we dont really care actually ,.. but , eh whatever
	}

	bool operator!=(ScriptActionNode const& rhs) const {
		//return !((*this) == rhs);
		return true; // so , umm we dont really care actually ,.. but , eh whatever
	}

public:
	int Action { -1 };
	int Argument { 0 } ;
};

//forward declarations
class TechnoTypeClass;

class DECLSPEC_UUID("42F3A647-0789-11D2-ACA5-006008055BB5")
	NOVTABLE ScriptTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::ScriptType;
	static constexpr int ScriptTypeClass::MaxActions = 50;

	//Array
	ABSTRACTTYPE_ARRAY(ScriptTypeClass, 0x8B41C8u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x691D50);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x691D90);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x691DE0);

	//Destructor
	virtual ~ScriptTypeClass() RX;

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x691E30);
	virtual AbstractType WhatAmI() const override { return AbstractType::ScriptType; }
	virtual int Size() const override { return 0x234; }
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
	ArrayWrapper<ScriptActionNode,ScriptTypeClass::MaxActions> ScriptActions;
};
static_assert(sizeof(ScriptTypeClass) == 0x234, "Invalid Size ! ");
