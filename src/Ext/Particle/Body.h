#pragma once
#include <ParticleClass.h>

#include <Helpers/Macro.h>
#include <Utilities/PooledContainer.h>
#include <Utilities/TemplateDef.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/ParticleType/Body.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

class ParticleExtData
{
public:
	static COMPILETIMEEVAL size_t Canary = 0xAAAABBBB;
	using base_type = ParticleClass;
	//static COMPILETIMEEVAL size_t ExtOffset = 0x134;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails { };
	std::vector<UniversalTrail> Trails { };

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(ParticleExtData) -
			(4u //AttachedToObject
				- 4u //inheritance
			 );
	}

	static std::pair<TechnoClass*, HouseClass*> GetOwnership(ParticleClass* pThis);
private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleExtContainer final : public Container<ParticleExtData>
{
public:
	static ParticleExtContainer Instance;
	static ObjectPool<ParticleExtData> pools;

	ParticleExtData* AllocateUnchecked(ParticleClass* key)
	{
		ParticleExtData* val = pools.allocate();

		if (val)
		{
			val->AttachedToObject = key;
		}
		else
		{
			Debug::FatalErrorAndExit("The amount of [ParticleExtData] is exceeded the ObjectPool size %d !", pools.getPoolSize());
		}

		return val;
	}

	void Remove(ParticleClass* key)
	{
		if (ParticleExtData* Item = TryFind(key))
		{
			RemoveExtOf(key, Item);
		}
	}

	void RemoveExtOf(ParticleClass* key, ParticleExtData* Item)
	{
		pools.deallocate(Item);
		this->ClearExtAttribute(key);
	}
};

class ParticleTypeExtData;
class NOVTABLE FakeParticleClass : public ParticleClass
{
public:
	void _Detach(AbstractClass* target, bool all);

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	FORCEDINLINE ParticleClass* _AsParticle() const {
		return (ParticleClass*)this;
	}

	FORCEDINLINE ParticleExtData* _GetExtData() {
		return *reinterpret_cast<ParticleExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEDINLINE ParticleTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<ParticleTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleClass) == sizeof(ParticleClass), "Invalid Size !");