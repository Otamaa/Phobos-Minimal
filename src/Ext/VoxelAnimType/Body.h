#pragma once
#include <VoxelAnimTypeClass.h>
#include <Ext/ObjectType/Body.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

class VoxelAnimTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = VoxelAnimTypeClass;

public:
#pragma region ClassMember

	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types { };
	Valueable<bool> Warhead_Detonate { false };
#pragma region Otamaa
	NullableVector <AnimTypeClass*> SplashList { };//
	Valueable<bool> SplashList_Pickrandom { true };
	Nullable<AnimTypeClass*> WakeAnim { }; //
	Valueable<bool> ExplodeOnWater { false };
	Valueable<bool> Damage_DealtByOwner;
	Nullable<WeaponTypeClass*> Weapon { };
	Valueable<bool> ExpireDamage_ConsiderInvokerVet { false };

	TrailsReader Trails { };

	Valueable<int> TrailerAnim_SpawnDelay { 2 };
#pragma endregion
#pragma endregion
public:

	void Initialize();

	VoxelAnimTypeExtData(VoxelAnimTypeClass* pObj) : ObjectTypeExtData(pObj) { }
	VoxelAnimTypeExtData(VoxelAnimTypeClass* pObj, noinit_t& nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~VoxelAnimTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<VoxelAnimTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<VoxelAnimTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	virtual VoxelAnimTypeClass* This() const override { return reinterpret_cast<VoxelAnimTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const VoxelAnimTypeClass* This_Const() const override { return reinterpret_cast<const VoxelAnimTypeClass*>(this->ObjectTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class VoxelAnimTypeExtContainer final : public Container<VoxelAnimTypeExtData>
{
public:
	static VoxelAnimTypeExtContainer Instance;

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

	virtual bool WriteDataToTheByteStream(VoxelAnimTypeExtData::base_type* key, IStream* pStm) { return true;  };
	virtual bool ReadDataFromTheByteStream(VoxelAnimTypeExtData::base_type* key, IStream* pStm) {  return true; };
};

class NOVTABLE FakeVoxelAnimTypeClass : public VoxelAnimTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	bool _ReadFromINI(CCINIClass* pINI);

	VoxelAnimTypeExtData* _GetExtData() {
		return *reinterpret_cast<VoxelAnimTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeVoxelAnimTypeClass) == sizeof(VoxelAnimTypeClass), "Invalid Size !");