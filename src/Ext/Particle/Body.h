#pragma once
#include <ParticleClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Entity/LaserTrailClass.h>
#include <Ext/ParticleType/Body.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

class ParticleExtData final
{
public:
	static constexpr size_t Canary = 0xAAAABBBB;
	using base_type = ParticleClass;
	//static constexpr size_t ExtOffset = 0x134;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	std::vector<LaserTrailClass> LaserTrails { };
	std::vector<UniversalTrail> Trails { };

	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	constexpr FORCEINLINE static size_t size_Of()
	{
		return sizeof(ParticleExtData) -
			(4u //AttachedToObject
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
	static std::vector<ParticleExtData*> Pool;
	static ParticleExtContainer Instance;

	ParticleExtData* AllocateUnchecked(ParticleClass* key)
	{
		ParticleExtData* val = nullptr;
		if (!Pool.empty()) {
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
		} else {
			val = DLLAllocWithoutCTOR<ParticleExtData>();
		}

		if (val) {
			val->ParticleExtData::ParticleExtData();
			val->AttachedToObject = key;
			return val;
		}

		return nullptr;
	}

	ParticleExtData* Allocate(ParticleClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (ParticleExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(ParticleClass* key)
	{
		if (ParticleExtData* Item = TryFind(key))
		{
			Item->~ParticleExtData();
			Item->AttachedToObject = nullptr;
			Pool.push_back(Item);
			this->ClearExtAttribute(key);
		}
	}

	void Clear()
	{
		if (!Pool.empty())
		{
			auto ptr = Pool.front();
			Pool.erase(Pool.begin());
			if (ptr)
			{
				delete ptr;
			}
		}
	}

	//CONSTEXPR_NOCOPY_CLASSB(ParticleExtContainer, ParticleExtData, "ParticleClass");
};

class ParticleTypeExtData;
class FakeParticleClass : public ParticleClass
{
public:
	void _Detach(AbstractClass* target, bool all);

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	FORCEINLINE ParticleClass* _AsParticle() const {
		return (ParticleClass*)this;
	}

	FORCEINLINE ParticleExtData* _GetExtData() {
		return *reinterpret_cast<ParticleExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	FORCEINLINE ParticleTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<ParticleTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleClass) == sizeof(ParticleClass), "Invalid Size !");