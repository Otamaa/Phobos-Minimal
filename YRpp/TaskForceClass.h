/*
	TaskForces as in the AI inis
*/

#pragma once

#include <AbstractTypeClass.h>

//forward declarations
class TechnoTypeClass;

struct TaskForceEntryStruct
{
	int Amount { 0 };
	TechnoTypeClass* Type { nullptr };

	bool operator ==(TaskForceEntryStruct const& rhs) const {
		return Amount == rhs.Amount && Type == rhs.Type;
	}

	bool operator!= (TaskForceEntryStruct const& rhs) const {
		return !((*this) == rhs);
	}
};

class DECLSPEC_UUID("61DE341E-0774-11D2-ACA5-006008055BB5")
	NOVTABLE TaskForceClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::TaskForce;

	//Array
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<TaskForceClass*>, 0xA8E8D0u> const Array {};

	IMPL_Find(TaskForceClass)
	IMPL_FindByName(TaskForceClass)

	static TaskForceClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x6E85F0);
	}

	static int __fastcall FindIndexById(const char* pID) {
		JMP_STD(0x6E8180);
	}

	static int __fastcall FindIndexByName(const char* pID) {
		JMP_STD(0x6E81D0);
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~TaskForceClass() RX;

	//AbstractClass
	virtual AbstractType WhatAmI() const RT(AbstractType);
	virtual int Size() const R0;

	//Constructor
	TaskForceClass(const char* pID) noexcept
		: TaskForceClass(noinit_t())
	{ JMP_THIS(0x6E7E80); }

protected:
	explicit __forceinline TaskForceClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int     Group;
	int     CountEntries;
	bool    IsGlobal;
	TaskForceEntryStruct Entries [0x6];
};
static_assert(sizeof(TaskForceClass) == 0xD4, "Invalid Size !");
