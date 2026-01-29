#pragma once
#include <ParticleTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDefB.h>

#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/PaletteManager.h>

#include <Misc/DynamicPatcher/Trails/TrailsManager.h>

#include <Ext/ObjectType/Body.h>

class ParticleTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = ParticleTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "ParticleTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "ParticleTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

#pragma region ClassMembers
	// ============================================================
	// Large aggregates
	// ============================================================
	TrailsReader Trails;
	CustomPalette Palette;
	std::array<Point2D, (size_t)FacingType::Count> WindMult;

	// ============================================================
	// 24-byte aligned: Vector
	// ============================================================
	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;

	// ============================================================
	// 8-byte aligned: Valueable<pointer>
	// ============================================================
	Valueable<UnitTypeClass*> TransmogrifyType;

	// ============================================================
	// 8-byte aligned: Valueable<double>
	// ============================================================
	Valueable<double> DamageRange;

	// ============================================================
	// 8-byte aligned: Valueable<MinMaxValue<int>> (2 ints = 8 bytes)
	// ============================================================
	Valueable<MinMaxValue<int>> Gas_DriftSpeedX;
	Valueable<MinMaxValue<int>> Gas_DriftSpeedY;

	// ============================================================
	// 4-byte aligned: Valueable<int>, Valueable<enum>
	// ============================================================
	Valueable<int> TransmogrifyChance;
	Valueable<OwnerHouseKind> TransmogrifyOwner;

	// ============================================================
	// 1-byte aligned: Valueable<bool> (packed together at the end)
	// ============================================================
	Valueable<bool> ReadjustZ;
	Valueable<bool> DeleteWhenReachWater;
	Valueable<bool> Transmogrify;
	Valueable<bool> Fire_DamagingAnim;
	// 4 bools = 4 bytes, naturally aligned

#pragma endregion

public:
	ParticleTypeExtData(ParticleTypeClass* pObj) : ObjectTypeExtData(pObj)
		// Large aggregates
		, Trails()
		, Palette(CustomPalette::PaletteMode::Temperate)
		, WindMult()
		// Vector
		, LaserTrail_Types()
		// Valueable<pointer>
		, TransmogrifyType(nullptr)
		// Valueable<double>
		, DamageRange(0.0)
		// Valueable<MinMaxValue<int>>
		, Gas_DriftSpeedX({ -2, 2 })
		, Gas_DriftSpeedY({ -2, 2 })
		// Valueable<int/enum>
		, TransmogrifyChance(-1)
		, TransmogrifyOwner(OwnerHouseKind::Neutral)
		// Valueable<bool>
		, ReadjustZ(true)
		, DeleteWhenReachWater(false)
		, Transmogrify(false)
		, Fire_DamagingAnim(false)
	{
		this->AbsType = ParticleTypeClass::AbsID;
		LaserTrail_Types.reserve(2);
	}

	ParticleTypeExtData(ParticleTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~ParticleTypeExtData() = default;

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
		const_cast<ParticleTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<ParticleTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	ParticleTypeClass* This() const { return reinterpret_cast<ParticleTypeClass*>(this->AttachedToObject); }
	const ParticleTypeClass* This_Const() const { return reinterpret_cast<const ParticleTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleTypeExtContainer final : public Container<ParticleTypeExtData>
	, public ReadWriteContainerInterfaces<ParticleTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "ParticleTypeExtContainer";
	using ext_t = ParticleTypeExtData;

public:
	static ParticleTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(ext_t::base_type* key, CCINIClass* pINI);
};

class NOVTABLE FakeParticleTypeClass : public ParticleTypeClass
{
public:

	bool _ReadFromINI(CCINIClass* pINI);

	ParticleTypeExtData* _GetExtData() {
		return *reinterpret_cast<ParticleTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleTypeClass) == sizeof(ParticleTypeClass), "Invalid Size !");