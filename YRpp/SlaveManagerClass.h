#pragma once

#include <AbstractClass.h>

class HouseClass;
class InfantryClass;
class InfantryTypeClass;
class TechnoClass;
enum class SlaveManagerStatus : unsigned int {
	Ready = 0,
	Scanning = 1,
	Travelling = 2,
	Deploying = 3,
	Working = 4,
	ScanningAgain = 5,
	PackingUp = 6
};

enum class SlaveControlStatus : unsigned int {
	Unknown = 0,
	ScanningForTiberium = 1,
	MovingToTiberium = 2,
	Harvesting = 3,
	BringingItBack = 4,
	Respawning = 5,
	Dead = 6
};

struct SlaveControl
{
	InfantryClass* Slave;
	SlaveControlStatus State;
	CDTimerClass RespawnTimer;
};

class DECLSPEC_UUID("335AAFE4-2DA6-11D5-BE22-00104B62A16C")
	NOVTABLE SlaveManagerClass : public AbstractClass
{
public:

	static const AbstractType AbsID = AbstractType::SlaveManager;

	//Static
	static COMPILETIMEEVAL constant_ptr<DynamicVectorClass<SlaveManagerClass*>, 0xB0B5F0u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6B1130);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6B1170);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6B1300);

	//Destructor
	virtual ~SlaveManagerClass() override JMP_THIS(0x6B1390);

	//AbstractClass
	virtual AbstractType WhatAmI() const override RT(AbstractType);
	virtual int Size() const override R0;
	virtual void Update() override JMP_THIS(0x6AF5F0);

	// non-virtual
	void SetOwner(TechnoClass *NewOwner)
		{ JMP_THIS(0x6AF580); }

	void CreateSlave(SlaveControl *Node)
		{ JMP_THIS(0x6AF650); }

	void LostSlave(InfantryClass *Slave)
		{ JMP_THIS(0x6B0A20); }

	void Deploy2()
		{ JMP_THIS(0x6B0D60); }

	// switches the slaves to the killer house with cheers and hoorahs
	// note that this->Owner will be NULL once this function is done
	void Killed(TechnoClass *Killer, HouseClass * ForcedOwnerHouse = nullptr)
		{ JMP_THIS(0x6B0AE0); }

	bool ShouldWakeUpNow()
		{ JMP_THIS(0x6B1020); }

	bool ShouldWakeUpNow(InfantryClass* pSlave, const CellStruct& nCell) const
		{ JMP_THIS(0x6B0880); }

	// the slaves will become free citizens without any announcements or cheers, if you don't call Killed() beforehand
	void ZeroOutSlaves();

	// stops scanning, spawning slaves and driving around.
	void SuspendWork() {
		this->RespawnTimer.StartTime = -1;
		if(!this->RespawnTimer.TimeLeft) {
			this->RespawnTimer.TimeLeft = 1;
		}
	}

	// resumes to harvest automatically.
	void ResumeWork() {
		this->RespawnTimer.Resume();
	}

	//Constructor
	SlaveManagerClass(
		TechnoClass* pOwner, InfantryTypeClass* pSlave, int num, int RegenRate,
		int ReloadRate) noexcept : SlaveManagerClass(noinit_t())
	{ JMP_THIS(0x6AF1A0); }

protected:
	explicit __forceinline SlaveManagerClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

public:

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

	TechnoClass* Owner;
	InfantryTypeClass* SlaveType;
	int SlaveCount;
	int RegenRate;
	int ReloadRate;
	DECLARE_PROPERTY(DynamicVectorClass<SlaveControl*>, SlaveNodes);
	DECLARE_PROPERTY(CDTimerClass, RespawnTimer);
	SlaveManagerStatus State;
	int LastScanFrame;
};
static_assert(sizeof(SlaveManagerClass) == 0x64);
