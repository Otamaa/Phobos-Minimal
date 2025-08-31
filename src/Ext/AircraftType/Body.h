#pragma once

#include <AircraftTypeClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>

class AircraftTypeExtData final : public TechnoTypeExtData
{
public:
	using base_type = AircraftTypeClass;

public:

	AircraftTypeExtData(AircraftTypeClass* pObj) : TechnoTypeExtData(pObj) { }
	AircraftTypeExtData(AircraftTypeClass* pObj, noinit_t& nn) : TechnoTypeExtData(pObj, nn) { }

	virtual ~AircraftTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->TechnoTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->TechnoTypeExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<AircraftTypeExtData*>(this)->TechnoTypeExtData::SaveToStream(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const {
		this->TechnoTypeExtData::CalculateCRC(crc);
	}

	virtual AircraftTypeClass* This() const override { return reinterpret_cast<AircraftTypeClass*>(this->TechnoTypeExtData::This()); }
	virtual const AircraftTypeClass* This_Const() const override { return reinterpret_cast<const AircraftTypeClass*>(this->TechnoTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr) {
		if (!this->TechnoTypeExtData::LoadFromINI(pINI, parseFailAddr))
			return false;

		return true;
	}

	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }
};

class AircraftTypeExtContainer final : public Container<AircraftTypeExtData>
{
public:
	static AircraftTypeExtContainer Instance;

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
		for (auto& ext : Array) {
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(AircraftTypeExtData::base_type* key, IStream* pStm) { return true; };
	virtual bool ReadDataFromTheByteStream(AircraftTypeExtData::base_type* key, IStream* pStm) { return true; };
};

class NOVTABLE FakeAircraftTypeClass : public AircraftTypeClass {
public:
	bool _CanAttackMove() { return RulesExtData::Instance()->ExpandAircraftMission; };
};

static_assert(sizeof(FakeAircraftTypeClass) == sizeof(FakeAircraftTypeClass), "Invalid Size !");