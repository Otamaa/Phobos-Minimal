#pragma once

#include <VoxelAnimClass.h>

#include <Utilities/SavegameDef.h>

#include <New/Entity/LaserTrailClass.h>

#include <Ext/Object/Body.h>

class VoxelAnimTypeExtData;
class VoxelAnimExtData final : public ObjectExtData
{
public:
	using base_type = VoxelAnimClass;
	static COMPILETIMEEVAL const char* ClassName = "VoxelAnimExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "VoxelAnimClass";
	static COMPILETIMEEVAL DWORD Canary = 0x4C5379D6;
	
public:

#pragma region ClassMember

	HelperedVector<std::unique_ptr<LaserTrailClass>> LaserTrails {};
	CDTimerClass TrailerSpawnDelayTimer {};
	TechnoClass* Invoker {};

#pragma endregion

public:
public:

	VoxelAnimExtData(VoxelAnimClass* pObj);
	VoxelAnimExtData() = default;

	virtual ~VoxelAnimExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
		this->ObjectExtData::InvalidatePointer(ptr, bRemoved, type);
		switch (type)
		{
		case AbstractType::Unit:
		case AbstractType::Aircraft:
		case AbstractType::Building:
		case AbstractType::Infantry:
			AnnounceInvalidPointer(Invoker, ptr, bRemoved);
			break;
			default: break;
		}
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) override
	{
		const_cast<VoxelAnimExtData*>(this)->ObjectExtData::SaveToStream(Stm);
		const_cast<VoxelAnimExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectExtData::CalculateCRC(crc);
	}

	VoxelAnimClass* This() const { return reinterpret_cast<VoxelAnimClass*>(this->AttachedToObject); }
	const VoxelAnimClass* This_Const() const { return reinterpret_cast<const VoxelAnimClass*>(this->AttachedToObject); }

public:

	void InitializeLaserTrails(VoxelAnimTypeExtData* pTypeExt);

private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static TechnoClass* GetTechnoOwner(VoxelAnimClass* pThis);
};

class VoxelAnimExtContainer final : public Container<VoxelAnimExtData>
	, public ContainerSaveLoad<VoxelAnimExtContainer, VoxelAnimExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "VoxelAnimExtContainer";

public:
	static VoxelAnimExtContainer Instance;

	virtual bool SaveGlobal(PhobosStreamWriter& stm) { return true; }
	virtual bool LoadGlobal(PhobosStreamReader& stm) { return true; }

};

class VoxelAnimTypeExtData;
class NOVTABLE FakeVoxelAnimClass : public VoxelAnimClass
{
public:
	HRESULT __stdcall __Load(IStream* pStm);
	HRESULT __stdcall __Save(IStream* pStm, BOOL fClearDirty);

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