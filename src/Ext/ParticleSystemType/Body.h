#pragma once
#include <ParticleSystemTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/ObjectType/Body.h>

class ParticleSystemTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = ParticleSystemTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "ParticleSystemTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "ParticleSystemTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();
public:

#pragma region ClassMembers
	// ============================================================
	// Large aggregate: std::array
	// ============================================================
	std::array<Point2D, (size_t)FacingType::Count> FacingMult;

	// ============================================================
	// 1-byte aligned: Valueable<bool> (packed together at the end)
	// ============================================================
	Valueable<bool> ApplyOptimization;
	Valueable<bool> AdjustTargetCoordsOnRotation;
	// 2 bools = 2 bytes, pads to 4 for alignment

#pragma endregion

public:
	ParticleSystemTypeExtData(ParticleSystemTypeClass* pObj) : ObjectTypeExtData(pObj)
		// Large aggregate
		, FacingMult()
		// Valueable<bool>
		, ApplyOptimization(true)
		, AdjustTargetCoordsOnRotation(true)
	{
		this->AbsType = ParticleSystemTypeClass::AbsID;
	}

	ParticleSystemTypeExtData(ParticleSystemTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~ParticleSystemTypeExtData() = default;

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
		const_cast<ParticleSystemTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<ParticleSystemTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	ParticleSystemTypeClass* This() const { return reinterpret_cast<ParticleSystemTypeClass*>(this->AttachedToObject); }
	const ParticleSystemTypeClass* This_Const() const { return reinterpret_cast<const ParticleSystemTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true;  }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class ParticleSystemTypeExtContainer final : public Container<ParticleSystemTypeExtData>
	, public ReadWriteContainerInterfaces<ParticleSystemTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "BulletTypeExtContainer";
	using ext_t = ParticleSystemTypeExtData;

public:
	static ParticleSystemTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(ext_t::base_type* key, CCINIClass* pINI);
};

class NOVTABLE FakeParticleSystemTypeClass : public ParticleSystemTypeClass
{
public:

	bool _ReadFromINI(CCINIClass* pINI);

	ParticleSystemTypeExtData* _GetExtData() {
		return *reinterpret_cast<ParticleSystemTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}

};
static_assert(sizeof(FakeParticleSystemTypeClass) == sizeof(ParticleSystemTypeClass), "Invalid Size !");