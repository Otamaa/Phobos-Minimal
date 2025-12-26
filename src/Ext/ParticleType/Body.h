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
	ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
	TrailsReader Trails;
	Valueable<bool> ReadjustZ;
	CustomPalette Palette; //
	Valueable<double> DamageRange;
	Valueable<bool> DeleteWhenReachWater;
	std::array<Point2D, (size_t)FacingType::Count> WindMult;
	Valueable<MinMaxValue<int>> Gas_DriftSpeedX;
	Valueable<MinMaxValue<int>> Gas_DriftSpeedY;
	Valueable<bool> Transmogrify;
	Valueable<int> TransmogrifyChance;
	Valueable<UnitTypeClass*> TransmogrifyType;
	Valueable<OwnerHouseKind> TransmogrifyOwner;
	Valueable<bool> Fire_DamagingAnim;
#pragma endregion

	ParticleTypeExtData(ParticleTypeClass* pObj) : ObjectTypeExtData(pObj),
		LaserTrail_Types(),
		Trails(),
		ReadjustZ(true),
		Palette(CustomPalette::PaletteMode::Temperate),
		DamageRange(0.0),
		DeleteWhenReachWater(false),
		WindMult(),
		Gas_DriftSpeedX({ -2 , 2 }),
		Gas_DriftSpeedY({ -2 , 2}),
		Transmogrify(false),
		TransmogrifyChance(-1),
		TransmogrifyType(nullptr),
		TransmogrifyOwner(OwnerHouseKind::Neutral),
		Fire_DamagingAnim(false)
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