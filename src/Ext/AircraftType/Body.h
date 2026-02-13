#pragma once

#include <AircraftTypeClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/FootType/Body.h>

class AircraftTypeExtData final : public FootTypeExtData
{
public:
	using base_type = AircraftTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "AircraftTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "AircraftTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:
	Nullable<bool> ExtendedAircraftMissions_SmoothMoving;
	Nullable<bool> ExtendedAircraftMissions_EarlyDescend;
	Nullable<bool> ExtendedAircraftMissions_RearApproach;
	Nullable<bool> ExtendedAircraftMissions_FastScramble;
	Nullable<int> ExtendedAircraftMissions_UnlandDamage;

	AircraftTypeExtData(AircraftTypeClass* pObj) : FootTypeExtData(pObj),
		ExtendedAircraftMissions_SmoothMoving(),
		ExtendedAircraftMissions_EarlyDescend(),
		ExtendedAircraftMissions_RearApproach(),
		ExtendedAircraftMissions_FastScramble(),
		ExtendedAircraftMissions_UnlandDamage()
	{
		this->AbsType = AircraftTypeClass::AbsID;
		this->InitializeConstant();
	}

	AircraftTypeExtData(AircraftTypeClass* pObj, noinit_t nn) : FootTypeExtData(pObj, nn) { }

	virtual ~AircraftTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->FootTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->FootTypeExtData::LoadFromStream(Stm);
		Stm
			.Process(this->ExtendedAircraftMissions_SmoothMoving)
			.Process(this->ExtendedAircraftMissions_EarlyDescend)
			.Process(this->ExtendedAircraftMissions_RearApproach)
			.Process(this->ExtendedAircraftMissions_FastScramble)
			.Process(this->ExtendedAircraftMissions_UnlandDamage)
			;
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<AircraftTypeExtData*>(this)->TechnoTypeExtData::SaveToStream(Stm);

		Stm
			.Process(this->ExtendedAircraftMissions_SmoothMoving)
			.Process(this->ExtendedAircraftMissions_EarlyDescend)
			.Process(this->ExtendedAircraftMissions_RearApproach)
			.Process(this->ExtendedAircraftMissions_FastScramble)
			.Process(this->ExtendedAircraftMissions_UnlandDamage)
			;
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {
		this->FootTypeExtData::CalculateCRC(crc);
	}

	AircraftTypeClass* This() const { return reinterpret_cast<AircraftTypeClass*>(this->AttachedToObject); }
	const AircraftTypeClass* This_Const() const { return reinterpret_cast<const AircraftTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr) override;
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }
};

class AircraftTypeExtContainer final : public Container<AircraftTypeExtData>
	, public ReadWriteContainerInterfaces<AircraftTypeExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "AircraftTypeExtContainer";

public:
	static AircraftTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(AircraftTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(AircraftTypeClass* key, CCINIClass* pINI);
};

class NOVTABLE FakeAircraftTypeClass : public AircraftTypeClass {
public:
	bool _CanAttackMove() { return RulesExtData::Instance()->ExpandAircraftMission; };
	bool _ReadFromINI(CCINIClass* pINI);
};

static_assert(sizeof(FakeAircraftTypeClass) == sizeof(AircraftTypeClass), "Invalid Size !");