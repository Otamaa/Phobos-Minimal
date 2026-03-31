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
	

public:

	Nullable<bool> ExtendedAircraftMissions {};
	Nullable<bool> ExtendedAircraftMissions_SmoothMoving {};
	Nullable<bool> ExtendedAircraftMissions_EarlyDescend {};
	Nullable<bool> ExtendedAircraftMissions_RearApproach {};
	Nullable<bool> ExtendedAircraftMissions_FastScramble {};
	Nullable<int> ExtendedAircraftMissions_UnlandDamage {};

	AircraftTypeExtData(AircraftTypeClass* pObj) : FootTypeExtData(pObj)
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
			.Process(this->ExtendedAircraftMissions)
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
			.Process(this->ExtendedAircraftMissions)
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

	AircraftTypeExtData(AircraftTypeExtData&&) noexcept = default;
	AircraftTypeExtData& operator=(AircraftTypeExtData&&) noexcept = default;
	AircraftTypeExtData(const AircraftTypeExtData&) = delete;
	AircraftTypeExtData& operator=(const AircraftTypeExtData&) = delete;

public:
	static bool ExtendedAircraftMissionsEnabled(AircraftClass* pAircraft);
};

class AircraftTypeExtContainer final : public Container<AircraftTypeExtData>
	, public ReadWriteContainerInterfaces<AircraftTypeExtData>
{
public:

	static COMPILETIMEEVAL const char* ClassName = "AircraftTypeExtContainer";

public:
	static AircraftTypeExtContainer Instance;

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }

	virtual void LoadFromINI(AircraftTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(AircraftTypeClass* key, CCINIClass* pINI);
};

class NOVTABLE FakeAircraftTypeClass : public AircraftTypeClass {
public:
	bool _CanAttackMove();
	bool _ReadFromINI(CCINIClass* pINI);
};

static_assert(sizeof(FakeAircraftTypeClass) == sizeof(AircraftTypeClass), "Invalid Size !");