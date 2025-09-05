#pragma once
#include <Ext/TechnoType/Body.h>

class UnitTypeExtData : public TechnoTypeExtData
{
public:
	using base_type = UnitTypeClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

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
	static std::map<TechnoTypeClass*, UnitTypeExtData*> Mapped;

	UnitTypeExtData* AllocateNoInit(UnitTypeExtData::base_type* key)
	{
		auto pExt = new UnitTypeExtData(key, noinit_t());
		Mapped[key] = pExt;

		return pExt;
	}

	static void Clear()
	{
		Mapped.clear();
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

	virtual HRESULT ReadDataFromTheByteStream(base_type_ptr key, extension_type_ptr pExt, IStream* pStm)
	{
			if (!pExt) {
				Debug::Log("SaveKey - Could not find value.\n");
				return E_POINTER;
			}

			PhobosByteStream loader(0);
			if (!loader.ReadBlockFromStream(pStm)) {
				Debug::Log("LoadKey - Failed to read data from save stream?!\n");
				return E_FAIL;
			}

			PhobosStreamReader reader(loader);

		if (reader.Expect(UnitTypeExtData::Marker)) {
			pExt->LoadFromStream(reader);

			if (reader.ExpectEndOfBlock()) {
				auto old = (LONG)key->unknown_18;
				SwizzleManagerClass::Instance->Here_I_Am(old, pExt);
				key->unknown_18 = (LONG)pExt;
				return S_OK;
			}
		}

		return E_FAIL;
	}
};

class NOVTABLE FakeUnitTypeClass : public UnitTypeClass
{
public:

	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _ReadFromINI(CCINIClass* pINI);
};