#pragma once
#include <Ext/FootType/Body.h>

class TerrainTypeClass;
class UnitTypeExtData : public FootTypeExtData
{
public:
	using base_type = UnitTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "UnitTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "UnitTypeClass";



public:

	SHPStruct* TurretShape { nullptr };
	NullableVector<TerrainTypeClass*> DefaultMirageDisguises {};

	UnitTypeExtData(UnitTypeClass* pObj) : FootTypeExtData(pObj)
	{
		this->AbsType = UnitTypeClass::AbsID;
		this->InitializeConstant();
	}
	UnitTypeExtData(UnitTypeClass* pObj, noinit_t nn) : FootTypeExtData(pObj, nn) { }

	virtual ~UnitTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType  type) override
	{
		this->FootTypeExtData::InvalidatePointer(ptr, bRemoved, type);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->FootTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<UnitTypeExtData*>(this)->FootTypeExtData::SaveToStream(Stm);
		const_cast<UnitTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->FootTypeExtData::CalculateCRC(crc);
	}

	UnitTypeClass* This() const { return reinterpret_cast<UnitTypeClass*>(this->AttachedToObject); }
	const UnitTypeClass* This_Const() const { return reinterpret_cast<const UnitTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);

	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template<typename T>
	void Serialize(T& Stm)
	{
		Stm
			.Process(this->TurretShape)
			.Process(this->DefaultMirageDisguises)
			;
	}

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

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }

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