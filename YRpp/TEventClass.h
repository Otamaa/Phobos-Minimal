#pragma once

#include <AbstractClass.h>
#include <ProgressTimer.h>

#include <EventArgs.h>

//forward declarations
class SuperClass;
class TechnoClass;
class TeamTypeClass;
class HouseClass;
class DECLSPEC_UUID("4F0EC393-0A55-11D2-ACA7-006008055BB5")
	NOVTABLE TEventClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Event;

	//Static
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<TEventClass*>, 0xB0F1A0u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x71F8C0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x71F930);

	//Destructor
	virtual ~TEventClass() RX;

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override RX;
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;

	virtual int GetArrayIndex() const override R0;

	// you are responsible for doing INI::ReadString and strtok'ing it before calling
	// this func only calls strtok again, doesn't know anything about buffers
	void LoadFromINI()
		{ JMP_THIS(0x71F4E0); }

	// you allocate the buffer for this, and save it to ini yourself after this returns
	// this func only sprintf's the stuff it needs into buffer
	void PrepareSaveToINI(char *buffer) const
		{ JMP_THIS(0x71F390); }

	static TriggerAttachType __fastcall GetAttachType(int eventKind)
		{ JMP_FAST(0x71F680); }

	// used in TriggerClass::HaveEventsOccured , when trigger is repeating
	// both need to be true to check this event as done
	bool GetStateA() const
		{ JMP_THIS(0x71F950); }

	bool GetStateB() const
		{ JMP_THIS(0x71F9C0); }

	// main brain
	bool HasOccured(
		int eventKind,
		HouseClass* pHouse,
		ObjectClass* Object,
		CDTimerClass* ActivationFrame,
		bool* isRepeating ,
		AbstractClass* source) const
			{ JMP_THIS(0x71E940); }

	//Constructor
	TEventClass() noexcept
		: TEventClass(noinit_t())
	{ JMP_THIS(0x71E6A0); }

protected:
	explicit __forceinline TEventClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	int               ArrayIndex;
	TEventClass*      NextEvent;
	TriggerEvent      EventKind;
	TeamTypeClass*    TeamType; // If this event needs to reference a team type, then this is the pointer to the team type object.
	int               Value;
	char		      String[0x1C];
	HouseClass*       House;
};