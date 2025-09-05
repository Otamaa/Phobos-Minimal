#pragma once

#include <AircraftTypeClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>

class AircraftTypeExtData final : public TechnoTypeExtData
{
public:
	using base_type = AircraftTypeClass;
	static constexpr unsigned Marker = UuidFirstPart<base_type>::value;

public:

	AircraftTypeExtData(AircraftTypeClass* pObj) : TechnoTypeExtData(pObj) { }
	AircraftTypeExtData(AircraftTypeClass* pObj, noinit_t nn) : TechnoTypeExtData(pObj, nn) { }

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
	static std::map<TechnoTypeClass*, AircraftTypeExtData*> Mapped;

	AircraftTypeExtData* AllocateNoInit(AircraftTypeExtData::base_type* key) {
		auto pExt = new AircraftTypeExtData(key, noinit_t());
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
		for (auto& ext : Array) {
			ext->InvalidatePointer(ptr, bRemoved);
		}
	}

	virtual HRESULT ReadDataFromTheByteStream(base_type_ptr key, extension_type_ptr pExt, IStream* pStm)
	{
		if (!pExt)
		{
			Debug::Log("SaveKey - Could not find value.\n");
			return E_POINTER;
		}

		PhobosByteStream loader(0);
		if (!loader.ReadBlockFromStream(pStm))
		{
			Debug::Log("LoadKey - Failed to read data from save stream?!\n");
			return E_FAIL;
		}

		PhobosStreamReader reader(loader);

		if (reader.Expect(extension_type::Marker))
		{
			pExt->LoadFromStream(reader);

			if (reader.ExpectEndOfBlock())
			{
				auto old = (LONG)key->unknown_18;
				SwizzleManagerClass::Instance->Here_I_Am(old, pExt);
				key->unknown_18 = (LONG)pExt;
				return S_OK;
			}
		}

		return E_FAIL;
	}
};

class NOVTABLE FakeAircraftTypeClass : public AircraftTypeClass {
public:
	HRESULT __stdcall _Load(IStream* pStm);
	HRESULT __stdcall _Save(IStream* pStm, BOOL clearDirty);

	bool _CanAttackMove() { return RulesExtData::Instance()->ExpandAircraftMission; };
	bool _ReadFromINI(CCINIClass* pINI);
};

static_assert(sizeof(FakeAircraftTypeClass) == sizeof(FakeAircraftTypeClass), "Invalid Size !");