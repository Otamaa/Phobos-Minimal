#pragma once

#include <OverlayTypeClass.h>

#include <Utilities/PhobosMap.h>
#include <New/Type/PaletteManager.h>

#include <Ext/ObjectType/Body.h>

class OverlayTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = OverlayTypeClass;

public:

#pragma region ClassMembeers

	CustomPalette Palette { CustomPalette::PaletteMode::Temperate };
	Valueable<int> ZAdjust { 0 };

#pragma endregion

	OverlayTypeExtData(OverlayTypeClass* pObj) : ObjectTypeExtData(pObj) { }
	OverlayTypeExtData(OverlayTypeClass* pObj, noinit_t& nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~OverlayTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved);
	}

	virtual void LoadFromStream(PhobosStreamReader& Stm) override
	{
		this->ObjectTypeExtData::LoadFromStream(Stm);
		this->Serialize(Stm);
	}

	virtual void SaveToStream(PhobosStreamWriter& Stm)
	{
		const_cast<OverlayTypeExtData*>(this)->ObjectTypeExtData::SaveToStream(Stm);
		const_cast<OverlayTypeExtData*>(this)->Serialize(Stm);
	}

	virtual AbstractType WhatIam() const { return base_type::AbsID; }
	virtual int GetSize() const { return sizeof(*this); };

	virtual void CalculateCRC(CRCEngine& crc) const
	{
		this->ObjectTypeExtData::CalculateCRC(crc);
	}

	virtual OverlayTypeClass* This() const override { return reinterpret_cast<OverlayTypeClass*>(this->ObjectTypeExtData::This()); }
	virtual const OverlayTypeClass* This_Const() const override { return reinterpret_cast<const OverlayTypeClass*>(this->ObjectTypeExtData::This_Const()); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

private:
	template <typename T>
	void Serialize(T& Stm);
};

class OverlayTypeExtContainer final : public Container<OverlayTypeExtData>
{
public:
	static OverlayTypeExtContainer Instance;

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

	virtual bool WriteDataToTheByteStream(OverlayTypeExtData::base_type* key, IStream* pStm) {  return true; };
	virtual bool ReadDataFromTheByteStream(OverlayTypeExtData::base_type* key, IStream* pStm) {  return true; };
};

class NOVTABLE FakeOverlayTypeClass : public OverlayTypeClass
{
public:
	bool _ReadFromINI(CCINIClass* pINI);
};