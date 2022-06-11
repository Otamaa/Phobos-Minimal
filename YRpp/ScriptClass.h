/*
	Actual AI Team Scripts
*/

#pragma once

#include <AbstractClass.h>

//forward declarations

#include <ScriptTypeClass.h>

class DECLSPEC_UUID("42F3A646-0789-11D2-ACA5-006008055BB5")
	NOVTABLE ScriptClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Script;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	//Destructor
	virtual ~ScriptClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	ScriptActionNode* GetCurrentAction(ScriptActionNode *buffer) const
		{ JMP_THIS(0x691500); }

	ScriptActionNode GetCurrentAction() const
	{
		ScriptActionNode buffer;
		GetCurrentAction(&buffer);
		return buffer;
	}

	ScriptActionNode* GetNextAction(ScriptActionNode *buffer) const
		{ JMP_THIS(0x691540); }

	ScriptActionNode GetNextAction() const
	{
		ScriptActionNode buffer;
		GetNextAction(&buffer);
		return buffer;
	}

	bool HasMissionsRemaining() const { JMP_THIS(0x6915D0); }
	bool ClearMission() const { JMP_THIS(0x691590); }
	bool SetMission(int nLine) const { JMP_THIS(0x6915A0); }
	bool NextMission() { ++this->CurrentMission; return this->HasNextMission(); }
	bool HasNextMission() const { JMP_THIS(0x6915B0); }

	//Constructor
	ScriptClass(ScriptTypeClass* pType) noexcept
		: AbstractClass(noinit_t())
	{ JMP_THIS(0x6913C0); }

protected:
	explicit __forceinline ScriptClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ScriptTypeClass * Type;
	int field_28;
	int CurrentMission; //CurrentLine
};
//static_assert(sizeof(ScriptClass) == 0x30);
