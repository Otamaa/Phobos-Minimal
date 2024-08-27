#pragma once

#include <AbstractClass.h>
#include <AircraftClass.h>
#include <SpawnManagerClass.h>

class DECLSPEC_UUID("5230C9A8-846A-47EC-BDA2-7E99445E1D49")
	NewSpawnManagerClass : public AbstractClass
{
public:
	static DynamicVectorClass<NewSpawnManagerClass*> Array;

	struct SpawnControl
	{
		AircraftClass* Spawnee;
		SpawnNodeStatus Status;
		CDTimerClass SpawnTimer;
		bool IsSpawnMissile;
	};

	/**
	*  IPersist
	*/
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID)
	{
		if (pClassID == nullptr)
		{
			return E_POINTER;
		}

		*pClassID = __uuidof(this);

		return S_OK;
	}

	/**
	*  IPersistStream
	*/
	virtual HRESULT __stdcall Load(IStream* pStm)
	{
		return 0;
	}

	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty)
	{
		return 0;
	}

public:
	NewSpawnManagerClass() :
		Owner(nullptr),
		SpawnType(nullptr),
		SpawnCount(0),
		SuspendedTarget(nullptr),
		Target(nullptr),
		Status(SpawnManagerStatus::Idle)
	{
		Array.AddItem(this);
	}

	NewSpawnManagerClass(TechnoClass* owner, AircraftTypeClass* spawns, int spawn_count, int regen_rate, int reload_rate)
	{

	}

	virtual ~NewSpawnManagerClass() override
	{

	}

	/**
	 *  AbstractClass
	 */
	virtual AbstractType __stdcall What_Am_I() const override {
			return (AbstractType)65;
	}

	virtual int Size() const override
	{
		return sizeof(*this);
	}

	virtual void ComputeCRC(CRCEngine& checksum) const override
	{

	}

	virtual void Update() override
	{

	}


public:
	TechnoClass* Owner;
	AircraftTypeClass* SpawnType;
	int SpawnCount;
	int RegenRate;
	int ReloadRate;
	DynamicVectorClass<SpawnControl*> SpawnControls;
	CDTimerClass UpdateTimer;
	CDTimerClass SpawnTimer;
	AbstractClass* SuspendedTarget;
	AbstractClass* Target;
	SpawnManagerStatus Status;
};