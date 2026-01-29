#pragma once
#include <VoxelAnimTypeClass.h>
#include <Ext/ObjectType/Body.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

class VoxelAnimTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = VoxelAnimTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "VoxelAnimTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "VoxelAnimTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:
#pragma region ClassMember

	// =====================================================
	// Containers / complex structs
	// =====================================================

	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
	NullableVector<AnimTypeClass*> SplashList;
	TrailsReader Trails;

	// =====================================================
	// Pointers / nullable pointers
	// =====================================================

	Nullable<AnimTypeClass*> WakeAnim;
	Nullable<WeaponTypeClass*> Weapon;

	// =====================================================
	// Integers
	// =====================================================

	Valueable<int> TrailerAnim_SpawnDelay;

	// =====================================================
	// Booleans / flags
	// =====================================================

	Valueable<bool> Warhead_Detonate;
	Valueable<bool> SplashList_Pickrandom;
	Valueable<bool> ExplodeOnWater;
	Valueable<bool> Damage_DealtByOwner;
	Valueable<bool> ExpireDamage_ConsiderInvokerVet;

#pragma endregion

public:
	VoxelAnimTypeExtData(VoxelAnimTypeClass* pObj) : ObjectTypeExtData(pObj)
		, LaserTrail_Types()
		, SplashList()
		, Trails()
		, WakeAnim()
		, Weapon()
		, TrailerAnim_SpawnDelay(2)
		, Warhead_Detonate(false)
		, SplashList_Pickrandom(true)
		, ExplodeOnWater(false)
		, Damage_DealtByOwner(false)
		, ExpireDamage_ConsiderInvokerVet(false)
	{
		this->AbsType = VoxelAnimTypeClass::AbsID;
		this->Initialize();
	}

	void Initialize();

	VoxelAnimTypeExtData(VoxelAnimTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

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

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
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

	VoxelAnimTypeClass* This() const { return reinterpret_cast<VoxelAnimTypeClass*>(this->AttachedToObject); }
	const VoxelAnimTypeClass* This_Const() const { return reinterpret_cast<const VoxelAnimTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class VoxelAnimTypeExtContainer final : public Container<VoxelAnimTypeExtData>
	, public ReadWriteContainerInterfaces<VoxelAnimTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "VoxelAnimTypeExtContainer";
	using base_t = Container<VoxelAnimTypeExtData>;
	using ext_t = VoxelAnimTypeExtData;

public:
	static VoxelAnimTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(ext_t::base_type* key, CCINIClass* pINI);
};

class NOVTABLE FakeVoxelAnimTypeClass : public VoxelAnimTypeClass
{
public:

	bool _ReadFromINI(CCINIClass* pINI);

	VoxelAnimTypeExtData* _GetExtData() {
		return *reinterpret_cast<VoxelAnimTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeVoxelAnimTypeClass) == sizeof(VoxelAnimTypeClass), "Invalid Size !");