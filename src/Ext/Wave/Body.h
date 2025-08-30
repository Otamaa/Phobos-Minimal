#pragma once
#include <WaveClass.h>

#include <Ext/Object/Body.h>
#include <Utilities/Debug.h>

class WeaponTypeClass;

class WaveExtData final : public ObjectExtData
{
public:
	using base_type = WaveClass;
public:
#pragma region ClassMembers

	WeaponTypeClass* Weapon { nullptr };
	int WeaponIdx { -1 };
	bool ReverseAgainstTarget { false };
	CoordStruct SourceCoord { };
	bool CanDoUpdate { false };

#pragma endregion

public:

	WaveExtData(WaveClass* pObj) : ObjectExtData(pObj) { }
	WaveExtData(WaveClass* pObj, noinit_t& nn) : ObjectExtData(pObj, nn) { }

	virtual ~WaveExtData();

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) const
	{
		const_cast<WaveExtData*>(this)->ObjectExtData::SaveToStream(Stm);
		const_cast<WaveExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectExtData::CalculateCRC(crc);
	}

	virtual WaveClass* This() const override { return reinterpret_cast<WaveClass*>(this->ObjectExtData::This()); }
	virtual const WaveClass* This_Const() const override { return reinterpret_cast<const WaveClass*>(this->ObjectExtData::This_Const()); }

public:

	void InitWeaponData();
	void SetWeaponType(WeaponTypeClass* pWeapon, int nIdx);

private:

	template <typename T>
	void Serialize(T& Stm);

public:

	static WaveClass* Create(CoordStruct nFrom, CoordStruct nTo, TechnoClass* pOwner, WaveType nType, AbstractClass* pTarget, WeaponTypeClass* pWeapon, bool FromSourceCoord = false);
	static bool ModifyWaveColor(WORD const src, WORD& dest, int const intensity, WaveClass* const pWave, WaveColorData const* colorDatas);
	static Point3D GetIntent(WeaponTypeClass* pWeapon);
	static ColorStruct GetColor(WeaponTypeClass* pWeapon, WaveClass* pWave);
	static WaveColorData GetWaveColor(WaveClass* pWave);
};

class WaveExtContainer final : public Container<WaveExtData>
{
public:
	static WaveExtContainer Instance;
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

	virtual bool WriteDataToTheByteStream(WaveExtData::base_type* key, IStream* pStm) { };
	virtual bool ReadDataFromTheByteStream(WaveExtData::base_type* key, IStream* pStm) { };

};

class NOVTABLE FakeWaveClass : public WaveClass
{
public:

	void _Detach(AbstractClass* target, bool all);
	void _DamageCell(CoordStruct* pLoc);

	WaveExtData* _GetExtData() {
		return *reinterpret_cast<WaveExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeWaveClass) == sizeof(WaveClass), "Invalid Size !");