#pragma once
#include <Ext/TechnoType/Body.h>

class UnitTypeExtData : public TechnoTypeExtData
{
public:
	using base_type = UnitTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "UnitTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "UnitTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:

	UnitTypeExtData(UnitTypeClass* pObj) : TechnoTypeExtData(pObj) {
		this->AbsType = UnitTypeClass::AbsID;
		this->InitializeConstant();
	}
	UnitTypeExtData(UnitTypeClass* pObj, noinit_t nn) : TechnoTypeExtData(pObj, nn) { }

	virtual ~UnitTypeExtData() = default;

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
		const_cast<UnitTypeExtData*>(this)->TechnoTypeExtData::SaveToStream(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->TechnoTypeExtData::CalculateCRC(crc);
	}

	UnitTypeClass* This() const { return reinterpret_cast<UnitTypeClass*>(this->AttachedToObject); }
	const UnitTypeClass* This_Const() const { return reinterpret_cast<const UnitTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
	{
		if (!this->TechnoTypeExtData::LoadFromINI(pINI, parseFailAddr))
			return false;

		return true;
	}

	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }
};

class UnitTypeExtContainer final : public Container<UnitTypeExtData>
	, public ReadWriteContainerInterfaces<UnitTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "UnitTypeExtContainer";
	using base_t = Container<UnitTypeExtData>;
	using ext_t = UnitTypeExtData;

public:
	static UnitTypeExtContainer Instance;

	virtual bool LoadAll(const json& root);
	virtual bool SaveAll(json& root);

	virtual void LoadFromINI(ext_t::base_type* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(ext_t::base_type* key, CCINIClass* pINI);
};

class NOVTABLE FakeUnitTypeClass : public UnitTypeClass
{
public:
	bool _ReadFromINI(CCINIClass* pINI);
};