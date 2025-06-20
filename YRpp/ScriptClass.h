/*
	Actual AI Team Scripts
*/

#pragma once

#include <AbstractClass.h>
#include <ScriptTypeClass.h>

class DECLSPEC_UUID("42F3A646-0789-11D2-ACA5-006008055BB5")
	NOVTABLE ScriptClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Script;
	static COMPILETIMEEVAL OPTIONALINLINE DWORD vtable = 0x7F0F78;

	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<ScriptClass*>, 0x8872B0u> const Array {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6915F0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x691630);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x691690);

	//Destructor
	virtual ~ScriptClass() JMP_THIS(0x691460);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x691EC0);
	virtual int Size() const override JMP_THIS(0x691ED0);

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
	bool HasNextMission() const { return this->CurrentMission < this->Type->ActionsCount; }

	const char* get_ID() const;

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
static_assert(sizeof(ScriptClass) == 0x30);
