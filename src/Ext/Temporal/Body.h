#pragma once
#include <TemporalClass.h>

#include <Utilities/SavegameDef.h>

#include <Utilities/Container.h>

class WeaponTypeClass;
class TemporalExtData final : public AbstractExtended
{
public:
	 using base_type = TemporalClass;

public:
	TemporalExtData(TemporalClass* pObj) : AbstractExtended(pObj) { }
	TemporalExtData(TemporalClass* pObj, noinit_t& nn) : AbstractExtended(pObj, nn) { }

	virtual ~TemporalExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override { }

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->AbstractExtended::Internal_LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm) const
	{
		const_cast<TemporalExtData*>(this)->AbstractExtended::Internal_SaveToStream(Stm);
		const_cast<TemporalExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const { }

	virtual TemporalClass* This() const override { return reinterpret_cast<TemporalClass*>(this->AbstractExtended::This()); }
	virtual const TemporalClass* This_Const() const override { return reinterpret_cast<const TemporalClass*>(this->AbstractExtended::This_Const()); }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class TemporalExtContainer final : public Container<TemporalExtData>
{
public:
	static TemporalExtContainer Instance;

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

	virtual bool WriteDataToTheByteStream(TemporalExtData::base_type* key, IStream* pStm) { };
	virtual bool ReadDataFromTheByteStream(TemporalExtData::base_type* key, IStream* pStm) { };
};

class NOVTABLE FakeTemporalClass : public TemporalClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, bool clearDirty);

	FORCEDINLINE TemporalClass* _AsTemporal() const {
		return (TemporalClass*)this;
	}

	FORCEDINLINE TemporalExtData* _GetExtData() {
		return *reinterpret_cast<TemporalExtData**>(((DWORD)this) + AbstractExtOffset);
	}
};
static_assert(sizeof(FakeTemporalClass) == sizeof(TemporalClass), "Invalid Size !");
