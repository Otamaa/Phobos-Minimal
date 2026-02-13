#pragma once
#include <Ext/FootType/Body.h>

class UnitTypeExtData : public FootTypeExtData
{
public:
	using base_type = UnitTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "UnitTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "UnitTypeClass";
	static COMPILETIMEEVAL unsigned Marker = UuidFirstPart<base_type>::value;
	static COMPILETIMEEVAL auto Marker_str = to_hex_string<Marker>();

public:
	SHPStruct* TurretShape;

	UnitTypeExtData(UnitTypeClass* pObj) : FootTypeExtData(pObj)
		, TurretShape { nullptr }
	{
		this->AbsType = UnitTypeClass::AbsID;
		this->InitializeConstant();
	}
	UnitTypeExtData(UnitTypeClass* pObj, noinit_t nn) : FootTypeExtData(pObj, nn) { }

	virtual ~UnitTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->FootTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		Stm.Process(this->TurretShape)
			;
		this->FootTypeExtData::LoadFromStream(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		Stm.Process(this->TurretShape)
			;
		const_cast<UnitTypeExtData*>(this)->FootTypeExtData::SaveToStream(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->FootTypeExtData::CalculateCRC(crc);
	}

	UnitTypeClass* This() const { return reinterpret_cast<UnitTypeClass*>(this->AttachedToObject); }
	const UnitTypeClass* This_Const() const { return reinterpret_cast<const UnitTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
	{
		if (!this->FootTypeExtData::LoadFromINI(pINI, parseFailAddr))
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

	
	UnitTypeExtData* _GetExtData() {
		return *reinterpret_cast<UnitTypeExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};