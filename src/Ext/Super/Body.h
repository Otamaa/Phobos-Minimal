#pragma once
#include <SuperClass.h>

#include <Utilities/Container.h>

// cache all super weapon statuses
struct SWStatus
{
	bool Available; //0
	bool PowerSourced; //1
	bool Charging;

	COMPILETIMEEVAL void FORCEDINLINE reset() {
		Available = 0;
		PowerSourced = 0;
		Charging = 0;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange)
	{ return Serialize(Stm); }

	bool Save(PhobosStreamWriter& Stm) const
	{ return const_cast<SWStatus*>(this)->Serialize(Stm); }

	template <typename T>
	bool Serialize(T& Stm)
	{
		return Stm
			.Process(Available)
			.Process(PowerSourced)
			.Process(Charging)
			.Success()
			//&& Stm.RegisterChange(this)
			; // announce this type
	}
};

class SWTypeExtData;
class SuperExtData final : public AbstractExtended
{
public:
	using base_type = SuperClass;

public:

#pragma region ClassMembers

	SWTypeExtData* Type { nullptr };
	bool Temp_IsPlayer { false };
	CellStruct Temp_CellStruct { };
	bool CameoFirstClickDone { false };
	bool FirstClickAutoFireDone { false };
	SWStatus Statusses { };

	CDTimerClass MusicTimer {};
	bool MusicActive {};
#pragma endregion

public:

	SuperExtData(SuperClass* pObj) : AbstractExtended(pObj) { }
	SuperExtData(SuperClass* pObj, noinit_t& nn) : AbstractExtended(pObj, nn) { }

	virtual ~SuperExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtended::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) const
	{
		const_cast<SuperExtData*>(this)->AbstractExtended::Internal_SaveToStream(Stm);
		const_cast<SuperExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	virtual SuperClass* This() const override { return reinterpret_cast<SuperClass*>(this->AbstractExtended::This()); }
	virtual const SuperClass* This_Const() const override { return reinterpret_cast<const SuperClass*>(this->AbstractExtended::This_Const()); }

public:

	static void UpdateSuperWeaponStatuses(HouseClass* pHouse);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class SuperExtContainer final : public Container<SuperExtData>
{
public:
	static SuperExtContainer Instance;

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

	virtual bool WriteDataToTheByteStream(SuperExtData::base_type* key, IStream* pStm) { };
	virtual bool ReadDataFromTheByteStream(SuperExtData::base_type* key, IStream* pStm) { };
};

class SWTypeExtData;
class NOVTABLE FakeSuperClass : public SuperClass
{
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	SuperExtData* _GetExtData() {
		return *reinterpret_cast<SuperExtData**>((DWORD)this + AbstractExtOffset);
	}

	SWTypeExtData* _GetTypeExtData() {
		return *reinterpret_cast<SWTypeExtData**>(((DWORD)this->Type) + AbstractExtOffset);
	}
};

static_assert(sizeof(FakeSuperClass) == sizeof(SuperClass), "Invalid Size !");