#pragma once

#include <VoxelAnimClass.h>

#include <Utilities/PooledContainer.h>
#include <Utilities/SavegameDef.h>

#include <New/Entity/LaserTrailClass.h>

#include <Misc/DynamicPatcher/Trails/Trails.h>

#include <Ext/Object/Body.h>

class VoxelAnimTypeExtData;
class VoxelAnimExtData final : public ObjectExtData
{
public:
	using base_type = VoxelAnimClass;

public:

#pragma region ClassMember
	TechnoClass* Invoker;
	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails;
	std::vector<UniversalTrail> Trails;
	CDTimerClass TrailerSpawnDelayTimer;
#pragma endregion

public:
	VoxelAnimExtData(VoxelAnimClass* pObj) : ObjectExtData(pObj),
		Invoker(nullptr)
	{ }
	VoxelAnimExtData(VoxelAnimClass* pObj, noinit_t nn) : ObjectExtData(pObj, nn) { }

	virtual ~VoxelAnimExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectExtData::InvalidatePointer(ptr, bRemoved);
		AnnounceInvalidPointer(Invoker, ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<VoxelAnimExtData*>(this)->ObjectExtData::SaveToStream(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectExtData::CalculateCRC(crc);
	}

	virtual VoxelAnimClass* This() const override { return reinterpret_cast<VoxelAnimClass*>(this->ObjectExtData::This()); }
	virtual const VoxelAnimClass* This_Const() const override { return reinterpret_cast<const VoxelAnimClass*>(this->ObjectExtData::This_Const()); }


public:

	void InitializeLaserTrails(VoxelAnimTypeExtData* pTypeExt);

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
	static void Clear()
	{
		Array.clear();
	}

	static bool LoadGlobals(PhobosStreamReader& Stm)
	{
		return true;
	}

	static bool SaveGlobals(PhobosStreamWriter& Stm)
	{
		return true;
	}

	static void InvalidatePointer(AbstractClass* const ptr, bool bRemoved)
	{
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(VoxelAnimExtData::base_type* key, IStream* pStm) { return true;  };
	virtual bool ReadDataFromTheByteStream(VoxelAnimExtData::base_type* key, IStream* pStm) {  return true; };

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
		return *reinterpret_cast<VoxelAnimExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	VoxelAnimTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<VoxelAnimTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeVoxelAnimClass) == sizeof(VoxelAnimClass), "Invalid Size !");