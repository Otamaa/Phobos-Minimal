#pragma once

#include <VoxelAnimClass.h>

#include <Utilities/Container.h>
#include <Utilities/SavegameDef.h>

#include <New/Entity/LaserTrailClass.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

class VoxelAnimTypeExtData;
class VoxelAnimExtData final
{
public:
	static COMPILETIMEEVAL size_t Canary = 0xAAACAACC;
	using base_type = VoxelAnimClass;
	static COMPILETIMEEVAL size_t ExtOffset = 0x144;

	base_type* AttachedToObject {};
	InitState Initialized { InitState::Blank };
public:

	TechnoClass* Invoker { nullptr };
	std::vector<LaserTrailClass> LaserTrails { };
	std::vector<UniversalTrail> Trails { };

	void InvalidatePointer(AbstractClass* ptr, bool bRemoved);
	void LoadFromStream(PhobosStreamReader& Stm) { this->Serialize(Stm); }
	void SaveToStream(PhobosStreamWriter& Stm) { this->Serialize(Stm); }

	void InitializeLaserTrails(VoxelAnimTypeExtData* pTypeExt);

	COMPILETIMEEVAL FORCEDINLINE static size_t size_Of()
	{
		return sizeof(VoxelAnimExtData) -
			(4u //AttachedToObject
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
	OPTIONALINLINE static std::vector<VoxelAnimExtData*> Pool;
	static VoxelAnimExtContainer Instance;

	VoxelAnimExtData* AllocateUnchecked(VoxelAnimClass* key)
	{
		VoxelAnimExtData* val = nullptr;
		if (!Pool.empty()) {
			val = Pool.front();
			Pool.erase(Pool.begin());
			//re-init
		} else {
			val = DLLAllocWithoutCTOR<VoxelAnimExtData>();
		}

		if (val)
		{
			val->VoxelAnimExtData::VoxelAnimExtData();
			val->AttachedToObject = key;
			return val;
		}

		return nullptr;
	}

	VoxelAnimExtData* Allocate(VoxelAnimClass* key)
	{
		if (!key || Phobos::Otamaa::DoingLoadGame)
			return nullptr;

		this->ClearExtAttribute(key);

		if (VoxelAnimExtData* val = AllocateUnchecked(key))
		{
			this->SetExtAttribute(key, val);
			return val;
		}

		return nullptr;
	}

	void Remove(VoxelAnimClass* key)
	{
		if (VoxelAnimExtData* Item = TryFind(key))
		{
			Item->~VoxelAnimExtData();
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

	//CONSTEXPR_NOCOPY_CLASSB(VoxelAnimExtContainer, VoxelAnimExtData, "VoxelAnimClass");
};

class VoxelAnimTypeExtData;
class FakeVoxelAnimClass : public VoxelAnimClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	void _Detach(AbstractClass* target, bool all);
	void _RemoveThis()
	{
		if (this->Type)
			VocClass::PlayIndexAtPos(this->Type->StopSound, this->Location);

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
