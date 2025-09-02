#pragma once
#include <Ext/TechnoType/Body.h>

class UnitTypeExtData : public TechnoTypeExtData
{
public:
	using base_type = UnitTypeClass;

public:

	UnitTypeExtData(UnitTypeClass* pObj) : TechnoTypeExtData(pObj) { }
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

	virtual UnitTypeClass* This() const override { return reinterpret_cast<UnitTypeClass*>(this->TechnoTypeExtData::This()); }
	virtual const UnitTypeClass* This_Const() const override { return reinterpret_cast<const UnitTypeClass*>(this->TechnoTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr)
	{
		if (!this->TechnoTypeExtData::LoadFromINI(pINI, parseFailAddr))
			return false;

		return true;
	}

	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }
};

class UnitTypeExtContainer final : public Container<UnitTypeExtData>
{
public:
	static UnitTypeExtContainer Instance;

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
		for (auto& ext : Array)
		{
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual bool WriteDataToTheByteStream(UnitTypeExtData::base_type* key, IStream* pStm) { return true;  };
	virtual bool ReadDataFromTheByteStream(UnitTypeExtData::base_type* key, IStream* pStm) { return true;  };
};

class NOVTABLE FakeUnitTypeClass : public UnitTypeClass
{
public:

	bool _ReadFromINI(CCINIClass* pINI);
};