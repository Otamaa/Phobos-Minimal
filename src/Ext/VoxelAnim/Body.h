#pragma once

#include <VoxelAnimClass.h>

#include <Utilities/PooledContainer.h>
#include <Utilities/SavegameDef.h>

#include <New/Entity/LaserTrailClass.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

class VoxelAnimTypeExtData;
class VoxelAnimExtData
{
public:
	static COMPILETIMEEVAL size_t Canary = 0xAAACAACC;
	using base_type = VoxelAnimClass;
	static COMPILETIMEEVAL size_t ExtOffset = 0x144;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	TechnoClass* Invoker { nullptr };
	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails { };
	std::vector<UniversalTrail> Trails { };
	CDTimerClass TrailerSpawnDelayTimer {};

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void InitializeLaserTrails(VoxelAnimTypeExtData* pTypeExt);

	~VoxelAnimExtData();

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(VoxelAnimExtData) -
			(4u //AttachedToObject
				- 4u //inheritance
			 );
	}
private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static TechnoClass* GetTechnoOwner(VoxelAnimClass* pThis);
};

class VoxelAnimExtContainer final : public Container<VoxelAnimExtData>
{
public:
	static VoxelAnimExtContainer Instance;
	static ObjectPool<VoxelAnimExtData> pools;

	VoxelAnimExtData* AllocateUnchecked(VoxelAnimClass* key)
	{
		VoxelAnimExtData* val = pools.allocate();

		if (val)
		{
			val->AttachedToObject = key;
		}
		else
		{
			Debug::FatalErrorAndExit("The amount of [VoxelAnimExtData] is exceeded the ObjectPool size %d !", pools.getPoolSize());
		}

		return val;
	}

	void Remove(VoxelAnimClass* key)
	{
		if (VoxelAnimExtData* Item = TryFind(key))
		{
			RemoveExtOf(key, Item);
		}
	}

	void RemoveExtOf(VoxelAnimClass* key, VoxelAnimExtData* Item)
	{
		pools.deallocate(Item);
		this->ClearExtAttribute(key);
	}
};

class VoxelAnimTypeExtData;
class NOVTABLE FakeVoxelAnimClass : public VoxelAnimClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	void _Detach(AbstractClass* target, bool all);
	void _RemoveThis()
	{
		if (this->Type)
			VocClass::SafeImmedietelyPlayAt(this->Type->StopSound, &this->Location);

		this->ObjectClass::UnInit();
	}

	VoxelAnimExtData* _GetExtData() {
		return *reinterpret_cast<VoxelAnimExtData**>(((DWORD)this) + VoxelAnimExtData::ExtOffset);
	}

	VoxelAnimTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<VoxelAnimTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeVoxelAnimClass) == sizeof(VoxelAnimClass), "Invalid Size !");