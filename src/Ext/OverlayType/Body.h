#pragma once

#include <OverlayTypeClass.h>

#include <Utilities/PhobosMap.h>
#include <New/Type/PaletteManager.h>

#include <Ext/ObjectType/Body.h>

class OverlayTypeExtData final : public ObjectTypeExtData
{
public:
	using base_type = OverlayTypeClass;
	static COMPILETIMEEVAL const char* ClassName = "OverlayTypeExtData";
	static COMPILETIMEEVAL const char* BaseClassName = "OverlayTypeClass";
	
public:

#pragma region ClassMembeers
	CustomPalette Palette { CustomPalette::PaletteMode::Temperate };
	Valueable<int> ZAdjust {};
	Valueable<bool> IsCanBeBuiltOn {};
	Valueable<bool> CanBeBuiltOn_Remove {};
#pragma endregion

public:
	OverlayTypeExtData(OverlayTypeClass* pObj) : ObjectTypeExtData(pObj)
	{
		this->AbsType = OverlayTypeClass::AbsID;
	}
	OverlayTypeExtData(OverlayTypeClass* pObj, noinit_t nn) : ObjectTypeExtData(pObj, nn) { }

	virtual ~OverlayTypeExtData() = default;

	virtual void InvalidatePointer(AbstractClass* ptr, bool bRemoved, AbstractType type) override
	{
		this->ObjectTypeExtData::InvalidatePointer(ptr, bRemoved, type);
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

	OverlayTypeClass* This() const { return reinterpret_cast<OverlayTypeClass*>(this->AttachedToObject); }
	const OverlayTypeClass* This_Const() const { return reinterpret_cast<const OverlayTypeClass*>(this->AttachedToObject); }

	virtual bool LoadFromINI(CCINIClass* pINI, bool parseFailAddr);
	virtual bool WriteToINI(CCINIClass* pINI) const { return true; }

	static bool CanBeBuiltOn(int overlayTypeIndex, BuildingTypeClass* pBuildingType, bool requireToBeRemovable);
	static void RemoveOverlayFromCell(int overlayTypeIndex, CellClass* pCell, HouseClass* pSource);
	
private:
	template <typename T>
	void Serialize(T& Stm);
};

class OverlayTypeExtContainer final : public Container<OverlayTypeExtData>
	, public ReadWriteContainerInterfaces<OverlayTypeExtData>
{
public:
	static COMPILETIMEEVAL const char* ClassName = "OverlayTypeExtContainer";
public:
	static OverlayTypeExtContainer Instance;

	virtual bool LoadAll(const PhobosStreamReader& stm) { return true; }
	virtual bool SaveAll(PhobosStreamWriter& stm){ return true; }

	virtual void LoadFromINI(OverlayTypeClass* key, CCINIClass* pINI, bool parseFailAddr);
	virtual void WriteToINI(OverlayTypeClass* key, CCINIClass* pINI);
};

class NOVTABLE FakeOverlayTypeClass : public OverlayTypeClass
{
public:
	bool _ReadFromINI(CCINIClass* pINI);
};