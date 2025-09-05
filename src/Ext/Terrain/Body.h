#pragma once
#include <TerrainClass.h>

#include <Utilities/Debug.h>
#include <Utilities/Handle.h>

#include <LightSourceClass.h>
#include <CellClass.h>
#include <AnimClass.h>

#include <Ext/Object/Body.h>

class TerrainExtData final : public ObjectExtData
{
public:
	using base_type = TerrainClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:


#pragma region ClassMember
	Handle<LightSourceClass*, UninitLightSource> LighSource;
	Handle<AnimClass*, UninitAnim> AttachedAnim;
	Handle<AnimClass*, UninitAnim> AttachedFireAnim;
	std::vector<CellStruct> Adjencentcells;
#pragma endregion

public:
	TerrainExtData(TerrainClass* pObj) : ObjectExtData(pObj),
		LighSource(nullptr),
		AttachedAnim(nullptr),
		AttachedFireAnim(nullptr)
	{ }
	TerrainExtData(TerrainClass* pObj, noinit_t nn) : ObjectExtData(pObj, nn) { }

	virtual ~TerrainExtData();

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectExtData::InvalidatePointer(ptr, bRemoved);

		if (this->LighSource.get() == ptr) {
			this->LighSource.release();
		}

		if (this->AttachedAnim.get() == ptr) {
			this->AttachedAnim.release();
		}
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<TerrainExtData*>(this)->ObjectExtData::SaveToStream(Stm);
		const_cast<TerrainExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectExtData::CalculateCRC(crc);
	}

	virtual TerrainClass* This() const override { return reinterpret_cast<TerrainClass*>(this->ObjectExtData::This()); }
	virtual const TerrainClass* This_Const() const override { return reinterpret_cast<const TerrainClass*>(this->ObjectExtData::This_Const()); }

public:

	void InitializeLightSource();
	void InitializeAnim();

public:
	static bool CanMoveHere(TechnoClass* pThis, TerrainClass* pTerrain);

private:
	template <typename T>
	void Serialize(T& Stm);

public:

	static void Unlimbo(TerrainClass* pThis, CoordStruct* pCoord);
};

class TerrainExtContainer final : public Container<TerrainExtData>
{
public:
	static TerrainExtContainer Instance;

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
};

class TerrainTypeExtData;
class NOVTABLE FakeTerrainClass : public TerrainClass
{
public:

	void _Detach(AbstractClass* target, bool all);
	void _AI();

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	DamageState __TakeDamage(
		int* Damage,
		int DistanceToEpicenter,
		WarheadTypeClass* WH,
		TechnoClass* Attacker,
		bool IgnoreDefenses,
		bool PreventsPassengerEscape,
		HouseClass* SourceHouse);

	TerrainExtData* _GetExtData() {
		return *reinterpret_cast<TerrainExtData**>(((DWORD)this) + AbstractExtOffset);
	}

	TerrainTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<TerrainTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}

	void _AnimPointerExpired(AnimClass* pAnim);
};
static_assert(sizeof(FakeTerrainClass) == sizeof(TerrainClass), "Invalid Size !");