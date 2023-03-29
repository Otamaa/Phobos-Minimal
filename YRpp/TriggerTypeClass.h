#pragma once

#include <AbstractTypeClass.h>
#include <Helpers/CompileTime.h>

enum class Presistance
{
	Volatile = 0 , SemiPersistant ,Persistent
};

//forward declarations
class CCINIClass;
class HouseTypeClass;
class TActionClass;
class TEventClass;
class TagTypeClass;
class DECLSPEC_UUID("C02D1591-0A2A-11D2-ACA7-006008055BB5")
	NOVTABLE TriggerTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::TriggerType;
	static constexpr inline DWORD vtable = 0x7F5904;

	static constexpr reference<const char* , 0x8449F8u , 3u> const PersistentName {};
	static const char* PersistentNameString(Presistance nPr) { return PersistentName[(int)nPr]; }
	static Presistance PresistentFromName(const char* const nString) { JMP_STD(0x727190); }

	//Array
	static constexpr constant_ptr<DynamicVectorClass<TriggerTypeClass*>, 0x8B4178u> const Array {};

	static NOINLINE TriggerTypeClass* __fastcall Find(const char* pID)
	{
		for (auto pItem : *Array){
			if (!CRT::strcmpi(pItem->ID, pID))
				return pItem;
		}

		return nullptr;
	}

	static TriggerTypeClass* __fastcall FindOrAllocate(const char* pID) {
		JMP_STD(0x727AA0);
	}

	static NOINLINE int __fastcall FindIndexById(const char* pID)
	{
		if(!pID)
			return -1;

		for (int i = 0; i < Array->Count; ++i) {
			if (!CRT::strcmpi(Array->Items[i]->ID, pID)) {
				return i;
			}
		}

		return -1;
	}

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~TriggerTypeClass() RX;

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override RX;
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;
	virtual void ComputeCRC(CRCEngine& checksum) const override RX;

	//AbstractTypeClass
	virtual int GetArrayIndex() const override R0;
	virtual bool LoadFromINI(CCINIClass* pINI) override R0;
	virtual bool SaveToINI(CCINIClass* pINI) override R0;

	//static
	static void __fastcall LoadFromINIList(CCINIClass* pINI)
		{ JMP_STD(0x7275D0); }

	static void __fastcall SaveToINIList(CCINIClass* pINI)
		{ JMP_STD(0x727880); }

	TagTypeClass* __fastcall FindByNameOrID(char const* pName)
		{ JMP_STD(0x727120); }

	//non-virtual
	using Flags = BYTE; // same as trigger and event flags?
	Flags GetFlags() const
		{ JMP_THIS(0x7271E0); }

	// contains at least one Allow Win action
	bool HasAllowWinAction() const
		{ JMP_THIS(0x726FE0); }

	// contains at least one Global Set or Global Cleared event
	bool HasGlobalSetOrClearedEvent(int idxGlobal) const
		{ JMP_THIS(0x727010); }

	// contains at least one Local Set or Local Cleared event
	bool HasLocalSetOrClearedEvent(int idxLocal) const
		{ JMP_THIS(0x727050); }

	// contains at least one Crosses Horizontal Line event
	bool HasCrossesHorizontalLineEvent() const
		{ JMP_THIS(0x726F80); }

	// contains at least one Crosses Vertical Line event
	bool HasCrossesVerticalLineEvent() const
		{ JMP_THIS(0x726F50); }

	// contains at least one Zone Entry By event
	bool HasZoneEntryByEvent() const
		{ JMP_THIS(0x726FB0); }

	// deletes an action from the list
	bool RemoveAction(TActionClass* pAction)
		{ JMP_THIS(0x7279E0); }

	// deletes an event from the list
	bool RemoveEvent(TEventClass* pEvent)
		{ JMP_THIS(0x727A40); }

	//Constructor
	TriggerTypeClass(char const* pName)
		: TriggerTypeClass(noinit_t())
	{ JMP_THIS(0x726C80); }

protected:
	explicit __forceinline TriggerTypeClass(noinit_t)
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	int ArrayIndex;
	bool Difficulty[3]; // easy = 0, normal = 1, hard = 2
	bool Enabled;
	bool MustTransfer; // vehicle thieves must take Tag with it when hijacking
	PROTECTED_PROPERTY(BYTE, align_A1[3]);
	HouseTypeClass* House;
	TriggerTypeClass* NextTrigger;
	TEventClass* FirstEvent;
	TActionClass* FirstAction;
};
