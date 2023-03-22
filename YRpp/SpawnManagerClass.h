#pragma once

#include <AbstractClass.h>

class HouseClass;
class TechnoClass;
class AircraftTypeClass;

enum class SpawnManagerStatus : unsigned int {
	Idle = 0, // no target or out of range
	Launching = 1, // one launch in progress
	CoolDown = 2 // waiting for launch to complete
};

enum class SpawnNodeStatus : unsigned int {
	Idle = 0, // docked, waiting for target
	TakeOff = 1, // missile tilting and launch
	Preparing = 2, // gathering, waiting
	Attacking = 3, // attacking until no ammo
	Returning = 4, // return to carrier
	//Unused_5, // not used
	Reloading = 6, // docked, reloading ammo and health
	Dead = 7 // respawning
};

struct SpawnNode
{
	TechnoClass* Unit;		//ThisCan be anything Techno that not building ?
	SpawnNodeStatus Status;
	CDTimerClass NodeSpawnTimer;
	BOOL IsSpawnMissile;
};

class DECLSPEC_UUID("0679E981-AD9D-11D3-BE16-00104B62A16C")
	NOVTABLE SpawnManagerClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::SpawnManager;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<SpawnManagerClass*>, 0xB0B880u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6B7ED0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6B7F10);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6B80B0);

	//Destructor
	virtual ~SpawnManagerClass() override JMP_THIS(0x6B8140);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::SpawnManager; }
	virtual int Size() const override { return 0x74; }
	virtual void Update() override JMP_THIS(0x6B7230);

	// non-virtual
	void KillNodes()
		{ JMP_THIS(0x6B7100); }

	void SetTarget(AbstractClass* pTarget)
		{ JMP_THIS(0x6B7B90); }

	bool UpdateTarget()
		{ JMP_THIS(0x6B7C40); }

	void ResetTarget()
		{ JMP_THIS(0x6B7BB0); }

	int CountAliveSpawns() const
		{ JMP_THIS(0x6B7D30); }

	int CountDockedSpawns() const
		{ JMP_THIS(0x6B7D50); }

	int CountLaunchingSpawns() const
		{ JMP_THIS(0x6B7D80); }

	void UnlinkPointer(AbstractClass* pAbs)
		{ JMP_THIS(0x6B7C60); }

	//Constructor
	SpawnManagerClass(
		TechnoClass* pOwner, AircraftTypeClass* pSpawnType, int nMaxNodes,
		int RegenRate, int ReloadRate) noexcept : SpawnManagerClass(noinit_t())
	{ JMP_THIS(0x6B6C90); }

protected:
	explicit __forceinline SpawnManagerClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TechnoClass* Owner;
	AircraftTypeClass* SpawnType;
	int SpawnCount;
	int RegenRate;
	int ReloadRate;
	DECLARE_PROPERTY(DynamicVectorClass<SpawnNode*>, SpawnedNodes);
	DECLARE_PROPERTY(CDTimerClass, UpdateTimer);
	DECLARE_PROPERTY(CDTimerClass, SpawnTimer);
	AbstractClass* Target; //dest ,suspendedtarget
	AbstractClass* NewTarget; //target
	SpawnManagerStatus Status;
};

static_assert(sizeof(SpawnManagerClass) == 0x74, "Invalid Size !");