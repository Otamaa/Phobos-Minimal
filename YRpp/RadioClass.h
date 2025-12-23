/*
	Base class for WHAT?? I DUNNO =(
*/

#pragma once

#include <MissionClass.h>

//forward declarations
class TechnoClass;

class NOVTABLE RadioClass : public MissionClass
{
public:
	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x70BF5C);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x65AC40);

	//Destructor
	virtual ~RadioClass() JMP_THIS(0x65AEB0);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool bremoved) override JMP_THIS(0x65AAC0);

	//ObjectClass
	virtual bool Limbo() override JMP_THIS(0x65AA80);
	virtual RadioCommand ReceiveCommand(TechnoClass* pSender, RadioCommand command, AbstractClass*& pInOut) override JMP_THIS(0x65A820);

	//RadioClass
	// these are oogly, westwood themselves admitted it, so it's probably even more of a wtf than the rest
	virtual RadioCommand SendToFirstLink(RadioCommand command) JMP_THIS(0x65ACC3); //Transmit_Message__MSG
	virtual RadioCommand SendCommand(RadioCommand command, TechnoClass* pRecipient) JMP_THIS(0x65AAA0); //Transmit_Message__MSG__PTR
	virtual RadioCommand SendCommandWithData(RadioCommand command, AbstractClass* &pInOut, TechnoClass* pRecipient) JMP_THIS(0x65A970); //Transmit_Message__MSG__PARAM__PTR
	virtual void SendToEachLink(RadioCommand command) JMP_THIS(0x65ACE0); //__Transmit_Message_To_All

	// get specific link
	COMPILETIMEEVAL FORCEDINLINE TechnoClass* const GetNthLink(int idx = 0) const {
		return this->RadioLinks.IsAllocated ? this->RadioLinks.Items[idx] : nullptr;
	}

	COMPILETIMEEVAL FORCEDINLINE TechnoClass* const GetRadioContact(int idx = 0) const {
		return this->RadioLinks.Items[idx];
	}
	// Strict check - only returns true if pLink is actually in the list
	COMPILETIMEEVAL	bool ContainsLink(TechnoClass const* pLink) const {
    	if (!pLink) return false;
		const int capacity = this->RadioLinks.Capacity;
		for (int i = 0; i < capacity; ++i) {
			if (this->RadioLinks.Items[i] == pLink)
				return true;
		}
    return false;
}

	// Original game behavior - true if linked OR has free slot
	COMPILETIMEEVAL bool HasLinkOrFreeSlot(TechnoClass const* pLink) const {
		const int capacity = this->RadioLinks.Capacity;
		for (int i = 0; i < capacity; ++i)
		{
			auto slot = this->RadioLinks.Items[i];
			if (!slot || slot == pLink)
				return true;
		}
		return false;
	}

	COMPILETIMEEVAL int FindLinkIndex(TechnoClass const* pLink) const {
		const int count = this->RadioLinks.Capacity;
		if (!pLink || count <= 0)
			return -1;

		int tot = 0;
		for (auto i = this->RadioLinks.Items; *i != pLink; ++i) {
			if (++tot >= count)
				return -1;
		}

		return tot;
	}

	COMPILETIMEEVAL bool HasFreeLink() const {
		const int count = this->RadioLinks.Capacity;
		if (count <= 0)
			return false;

		int tot = 0;
		for (auto i = this->RadioLinks.Items; *i; ++i) {
			if (++tot >= count)
				return false;
		}

		return true;
	}

	// iow. at least one link used
	COMPILETIMEEVAL bool HasAnyLink() const {
		//JMP_THIS(0x65AE30);
		const int count = this->RadioLinks.Capacity;
		if (count <= 0)
			return false;

		int tot = 0;
		for (auto i = this->RadioLinks.Items; !*i; ++i) {
			if (++tot >= count)
				return false;
		}

		return true;
	}

	// resizes the vector and nulls the new elements
	void SetLinkCount(int count)
		{ JMP_THIS(0x65AE60); }

	//Constructor
	RadioClass() noexcept
		: RadioClass(noinit_t())
	{ JMP_THIS(0x65A750); }

protected:
	explicit __forceinline RadioClass(noinit_t) noexcept
		: MissionClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	RadioCommand LastCommands[3]; // new command updates these
	DECLARE_PROPERTY(VectorClass<TechnoClass*>, RadioLinks);	//Docked units etc
};
 static_assert(sizeof(RadioClass) == 0xF0);