#pragma once
#include <WaveClass.h>

#include <Ext/Object/Body.h>
#include <Utilities/Debug.h>

class WeaponTypeClass;

class WaveExtData final : public ObjectExtData
{
public:
	using base_type = WaveClass;
	static COMPILETIMEEVAL const char* ClassName = "WaveExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "WaveClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:
#pragma region ClassMembers

	WeaponTypeClass* Weapon;
	CoordStruct SourceCoord;
	int WeaponIdx;
	bool ReverseAgainstTarget;
	bool CanDoUpdate;

#pragma endregion

public:

	WaveExtData(WaveClass* pObj) : ObjectExtData(pObj)
		, Weapon(nullptr)
		, SourceCoord()
		, WeaponIdx(-1)
		, ReverseAgainstTarget(false)
		, CanDoUpdate(false)
	{
		this->Name = "WaveClass";
		this->AbsType = WaveClass::AbsID;
	}


	WaveExtData(WaveClass* pObj, noinit_t nn) : ObjectExtData(pObj, nn) { }

	virtual ~WaveExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
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

	WaveClass* This() const { return reinterpret_cast<WaveClass*>(this->AttachedToObject); }
	const WaveClass* This_Const() const { return reinterpret_cast<const WaveClass*>(this->AttachedToObject); }

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
	static COMPILETIMEEVAL const char* ClassName = "WaveExtContainer";

public:
	static WaveExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);
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